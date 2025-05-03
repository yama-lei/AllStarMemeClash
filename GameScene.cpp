#include "GameScene.h"
QPointF GameScene::randomPosition()
{
    double x = QRandomGenerator::global()->generateDouble() * (SCENE.width() - 200.0) + 100.0;
    double y = QRandomGenerator::global()->generateDouble() * (SCENE.height() - 200.0) + 100.0;
    return QPointF(x, y);
}

GameScene::GameScene(QWidget *parent)
    : QWidget(parent)
{
    //------------------初始化-----------//
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GameScene::updateGame);
    timer->start(16);
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    elapsedTimer = new QElapsedTimer();
    elapsedTimer->start();
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, SCENE.width(), SCENE.height());
    view = new QGraphicsView(this);
    view->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    view->setScene(scene);
    view->resize(VIEW.width(), VIEW.height());
    view->scale(0.5, 0.5);
    pressedKeys = QSet<int>();
    //-------------------资源load--------------------------------------//
    backImage_poison = scene->addPixmap(QPixmap(":/images/green.png"));
    backImage_normal = scene->addPixmap(QPixmap(":/images/yellow.png"));
    backImage_normal->setPos(0, 0);
    backImage_normal->setZValue(0);
    scene->setSceneRect(0, 0, SCENE.width(), SCENE.height());
    scene->setBackgroundBrush((backImage_poison)->pixmap());
    backImage_normal->setTransformOriginPoint(SCENE.width() / 2, SCENE.height() / 2);
    initPlayers();
    initProps();
    view->centerOn(user);
    //---------------------其他逻辑------------------------------------//
}
void GameScene::gameOverSlot()
{
    //游戏结束画面
}
void GameScene::initPlayers()
{
    user = new User(randomPosition());
    connect(user, &User::userDie, this, &GameScene::gameOverSignal);

    players.append(user);
    for (int i = 0; i < 10; i++) {
        NPC* npc = new NPC(randomPosition());
        connect(npc, &Player::playerDied, this, &GameScene::handlePlayerDeath);

        players.append(npc);
    }
    for (auto player : std::as_const(players)) {
        scene->addItem(player);
    }
}

void GameScene::initProps()
{
    for (int i = 0; i < 20; i++) {
        props.append(new BloodBottle(randomPosition()));
    }

    for (int i = 0; i < 160; i++) {
        props.append(new Knife(randomPosition()));
    }
    for (int i = 0; i < 8; i++) {
        props.append(new Bushes(randomPosition()));
    }
    for (auto prop : std::as_const(props)) {
        scene->addItem(prop);
    }
}

GameScene::~GameScene()
{
    // 先停止计时器，防止在析构过程中触发更新
    if (timer) {
        timer->stop();
    }
    
    // 断开所有信号连接
    this->disconnect();
    
    // 清理定时器
    if (elapsedTimer != nullptr) {
        delete elapsedTimer;
        elapsedTimer = nullptr;
    }
    
    if (timer != nullptr) {
        delete timer;
        timer = nullptr;
    }
    
    // 清理所有道具
    for (auto &prop : props) {
        if (prop != nullptr) {
            scene->removeItem(prop);
            delete prop;
            prop = nullptr;
        }
    }
    props.clear();
    
    // 清理所有玩家对象
    for (auto &player : players) {
        if (player != nullptr) {
            scene->removeItem(player);
            delete player;
            player = nullptr;
        }
    }
    players.clear();
    
    // 清理场景
    if (scene) {
        scene->clear();
    }
}

void GameScene::keyPressEvent(QKeyEvent *event)
{
    pressedKeys.insert(event->key());
    qDebug() << "The key " << event->key() << " is pressed!";
    QWidget::keyPressEvent(event);
}

void GameScene::keyReleaseEvent(QKeyEvent *event)
{
    if (pressedKeys.contains(event->key())) {
        pressedKeys.remove(event->key());
    }
    qDebug() << "The key " << event->key() << " is released!";
    switch (event->key()) {
    case Qt::Key_A:
    case Qt::Key_D:
    case Qt::Key_W:
    case Qt::Key_S:
        user->setDirection(STAY);
        break;
    default:
        break;
    }
    QWidget::keyReleaseEvent(event);
}

void GameScene::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPair<Player *, qreal> pair = closestEnemy(user);
        if (pair.second <= 600) {
            qDebug() << "Close Enough! Attack!";
            user->attack(pair.first);
        }
    }
    if (event->button() == Qt::RightButton) {
        user->addBlood(-10);
    }
}

void GameScene::updateGame()
{
    if (!gameStart) {
        return;
    }
    if (players.size() == 1) {
        emit gameOverSignal();
    }

    qreal delta = elapsedTimer->restart();
    //-----------------------处理按钮----------------------//
    QList<int> keyMove;
    for (auto it = pressedKeys.begin(); it != pressedKeys.end(); it++) {
        if (*it == Qt::Key_W || *it == Qt::Key_S || *it == Qt::Key_A || *it == Qt::Key_D) {
            keyMove.append(*it);
        }
    }

    if (keyMove.size() == 1 || keyMove.size() > 2) {
        //我个人觉得同时按下三个移动键，朝着第一个按下的键移动就好,所以逻辑整合到一起了
        switch (keyMove[0]) {
        case Qt::Key_W:
            user->setDirection(UP);
            break;
        case Qt::Key_S:
            user->setDirection(DOWN);
            break;
        case Qt::Key_A:
            user->setDirection(LEFT);
            break;
        case Qt::Key_D:
            user->setDirection(RIGHT);
            break;
        default:
            break;
        }
    } else if (keyMove.size() == 2) {
        //
        if (keyMove.contains(Qt::Key_W) && keyMove.contains(Qt::Key_A)) {
            user->setDirection(LU);
        } else if (keyMove.contains(Qt::Key_W) && keyMove.contains(Qt::Key_D)) {
            user->setDirection(RU);
        } else if (keyMove.contains(Qt::Key_S) && keyMove.contains(Qt::Key_A)) {
            user->setDirection(LD);
        } else if (keyMove.contains(Qt::Key_S) && keyMove.contains(Qt::Key_D)) {
            user->setDirection(RD);
        } else {
            //同时摁下上和下，左和右，不理会
            user->setDirection(STAY);
        }
    }
    shrinkSafetyZone();
    //-----------------Update-------------------//
    for (auto player : std::as_const(players)) {
        player->updateState(delta,
                            backImage_normal->mapToScene(backImage_normal->boundingRect())
                                .boundingRect());
    }

    view->centerOn(user);
    for (auto player : std::as_const(players)) {
        QPair<Player *, qreal> pair = closestEnemy(player);
        if (pair.second <= 600) {
            QGraphicsLineItem *line = new QGraphicsLineItem(
                QLineF(player->pos(), pair.first->pos()));
            QPen pen(Qt::red);
            pen.setWidth(5);
            pen.setStyle(Qt::DashDotLine);
            line->setPen(pen);
            scene->addItem(line);
            QTimer::singleShot(16, [line, this]() {
                scene->removeItem(line);
                delete line;
            });
            qreal q = QRandomGenerator::global()->generateDouble();
            if (q <= 0.008) {
                if (player != user) {
                    player->attack(pair.first);
                }
            }
        }
    }
}

void GameScene::shrinkSafetyZone()
{
    if (scaleRate >= 0.5) {
        scaleRate -= 0.00001;
        backImage_normal->setScale(scaleRate);
        //  qDebug() << backImage_normal->mapToScene(backImage_normal->boundingRect()).boundingRect();
    }
}

void GameScene::handlePlayerDeath(Player *player)
{
    // 从玩家列表中移除死亡的玩家
    players.removeOne(player);
    qDebug() << "Player removed from game";
}

QPair<Player *, qreal> GameScene::closestEnemy(Player *player)
{
    qreal squareOfDistance = 1145141919810;
    Player *cloestEnemy = nullptr;
    for (auto enemy : std::as_const(players)) {
        if (enemy != player) {
            qreal dis = ((enemy->pos().x() - player->pos().x())
                             * (enemy->pos().x() - player->pos().x())
                         + (enemy->pos().y() - player->pos().y())
                               * (enemy->pos().y() - player->pos().y()));
            if (dis <= squareOfDistance) {
                squareOfDistance = dis;
                cloestEnemy = enemy;
            }
        }
    }
    return QPair<Player *, qreal>(cloestEnemy, sqrt(squareOfDistance));
}

void GameScene::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    for (auto player : std::as_const(players)) {
        QPair<Player *, qreal> pair = closestEnemy(user);
        if (pair.second <= 600 && pair.first != nullptr) {
            QPointF start = view->mapFromScene(player->pos());
            QPointF end = view->mapFromScene(pair.first->pos());
        }
    }
}
