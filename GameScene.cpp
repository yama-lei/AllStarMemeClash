#include "GameScene.h"
QPointF GameScene::randomPositionInCircle(QPointF center, qreal maxRadius)
{
    //只能生成安全区以内的

    double alpha = QRandomGenerator::global()->generateDouble() * 6.28;
    qreal radius = sqrt(QRandomGenerator::global()->generateDouble()) * maxRadius;
    qreal x = radius * qSin(alpha);
    qreal y = radius * qCos(alpha);
    return QPointF(x, y) + center;
}

GameScene::GameScene(QWidget *parent)
    : QWidget(parent)
{
    //------------------初始化-----------//

    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, SCENE.width(), SCENE.height());
    view = new QGraphicsView(this);
    view->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    view->setScene(scene);
    view->resize(VIEW.width(), VIEW.height());
    view->scale(0.5, 0.5);
    
    QPixmap poisonPixmap(":/images/green.png");
    QPixmap normalPixmap(":/images/yellow.png");
    
    if (poisonPixmap.isNull() || normalPixmap.isNull()) {
        qDebug() << "错误：无法加载背景图像资源";
        // 调试
        scene->setBackgroundBrush(Qt::black);
    } else {
        backImage_poison = scene->addPixmap(poisonPixmap);
        backImage_normal = scene->addPixmap(normalPixmap);
        backImage_normal->setPos(0, 0);
        backImage_normal->setZValue(0);
        scene->setBackgroundBrush((backImage_poison)->pixmap());
        backImage_normal->setTransformOriginPoint(SCENE.width() / 2, SCENE.height() / 2);
    }
    
    scene->setSceneRect(0, 0, SCENE.width(), SCENE.height());

    // 先不调用centerOn，因为user还未初始化！
    //================5.4更新：先前的逻辑移到了startGame里面===================//
    //startGame();注意这个地方不要调用 ！要等到GameManager的信号
}
void GameScene::startGame()
{
    // 2025-5-4: 此处逻辑可能有些混乱，简而言之，是保险起见，多次删除场景的道具和人物（有检查的）

    gameOn = false;

    for (auto item : std::as_const(allItems)) {
        if (item != nullptr) {
            scene->removeItem(item);
            item->deleteLater();
        }
    }
    players.clear();
    props.clear();
    allItems.clear();

    scaleRate = 1;
    if (backImage_normal) {
        backImage_normal->setScale(scaleRate);
    }
    safetyZoneRadius = scene->width() / 2 * scaleRate;

    pressedKeys.clear();
    if (timer) {
        disconnect(timer, &QTimer::timeout, this, &GameScene::updateGame);
        delete timer;
        timer = nullptr;
    }
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GameScene::updateGame);
    timer->start(16);
    if (elapsedTimer) {
        delete elapsedTimer;
        elapsedTimer = nullptr;
    }
    elapsedTimer = new QElapsedTimer();
    elapsedTimer->start();
    initPlayers();
    initProps();
    gameOn = true;
    if (user) {
        view->centerOn(user);
    }
}

void GameScene::endGame(bool win)
{
    if (win) {
        qDebug() << "User win";
    } else {
        qDebug() << "User lose";
    }

    // 先将gameOn设为false，防止updateGame继续执行
    gameOn = false;

    // 停止并清理定时器
    if (timer) {
        // 断开连接是安全的，即使定时器已无效
        disconnect(timer, &QTimer::timeout, this, &GameScene::updateGame);
        
        // 永远不直接删除timer，而是使用deleteLater
        timer->deleteLater();
        timer = nullptr;
    }
    if (elapsedTimer) {
        delete elapsedTimer;
        elapsedTimer = nullptr;
    }
    pressedKeys.clear();
    for (auto item : std::as_const(allItems)) {
        if (item != nullptr) {
            // 让Qt的事件循环安全地删除对象
            item->deleteLater();
        }
    }
    allItems.clear();
    players.clear();
    props.clear();

    scaleRate = 1; //回到原先的安全区大小
    if (backImage_normal) {
        backImage_normal->setScale(scaleRate);
    }
    safetyZoneRadius = scene->width() / 2 * scaleRate;

    emit gameOverSignal(win);
}

void GameScene::initPlayers()
{
    user = new User(randomPositionInCircle(sceneCenter, safetyZoneRadius));
    if (user) {
        connect(user, &User::userDie, [this]() { endGame(0); });
        players.append(user);
        scene->addItem(user);
        allItems.append(user);
    }

    for (int i = 0; i < 3; i++) {
        NPC *npc = new NPC(randomPositionInCircle(sceneCenter, safetyZoneRadius));
        if (npc) {
            connect(npc, &Player::playerDied, this, &GameScene::handlePlayerDeath);
            players.append(npc);
            scene->addItem(npc);
            allItems.append(npc);
        }
    }
}

void GameScene::initProps()
{
    for (int i = 0; i < 5; i++) {
        Prop* prop = new BloodBottle(randomPositionInCircle(sceneCenter, safetyZoneRadius));
        if (prop) {
            props.append(prop);
            scene->addItem(prop);
            allItems.append(prop);
        }
    }

    for (int i = 0; i < 5; i++) {
        Prop* prop = new Knife(randomPositionInCircle(sceneCenter, safetyZoneRadius));
        if (prop) {
            props.append(prop);
            scene->addItem(prop);
            allItems.append(prop);
        }
    }

    for (int i = 0; i < 0; i++) {
        Prop* prop = new Bushes(randomPositionInCircle(sceneCenter, safetyZoneRadius));
        if (prop) {
            props.append(prop);
            scene->addItem(prop);
            allItems.append(prop);
        }
    }

    for (int i = 0; i < 5; i++) {
        Prop* prop = new Boot(randomPositionInCircle(sceneCenter, safetyZoneRadius));
        if (prop) {
            props.append(prop);
            scene->addItem(prop);
            allItems.append(prop);
        }
    }

    for (int i = 0; i < 5; i++) {
        Prop* prop = new KnifeStrong(randomPositionInCircle(sceneCenter, safetyZoneRadius));
        if (prop) {
            props.append(prop);
            scene->addItem(prop);
            allItems.append(prop);
        }
    }
}

GameScene::~GameScene()
{
    if (timer) {
        timer->stop();
        disconnect(timer, &QTimer::timeout, this, &GameScene::updateGame);
        delete timer;
        timer = nullptr;
    }
    this->disconnect();

    if (elapsedTimer != nullptr) {
        delete elapsedTimer;
        elapsedTimer = nullptr;
    }

    if (scene) {
        scene->clear();
    }
    for (auto item : std::as_const(allItems)) {
        if (item != nullptr) {
            item->deleteLater();
        }
    }
    allItems.clear();
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
    
    if (user) {
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
    }
    
    QWidget::keyReleaseEvent(event);
}

void GameScene::mousePressEvent(QMouseEvent *event)
{
    if (!user || !gameOn) {
        return;
    }
    
    if (event->button() == Qt::LeftButton) {
        QPair<Player *, qreal> pair = closestEnemy(user);
        if (pair.first != nullptr && pair.second <= 600) {
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
    if (!gameOn || !user) {
        return;
    }

    if (players.size() == 1 && players.contains(user)) {
        endGame(true);
        return;
    }

    qreal delta = 0.0;
    if (elapsedTimer) {
        delta = elapsedTimer->restart();
    } else {
        elapsedTimer = new QElapsedTimer();
        elapsedTimer->start();
    }

    //-----------------------处理按钮----------------------//
    QList<int> keyMove;
    for (auto it = pressedKeys.begin(); it != pressedKeys.end(); it++) {
        if (*it == Qt::Key_W || *it == Qt::Key_S || *it == Qt::Key_A || *it == Qt::Key_D) {
            keyMove.append(*it);
        }
    }

    if (!keyMove.isEmpty()) {
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
    }
    shrinkSafetyZone();
    //-----------------Update-------------------//
    for (auto player : std::as_const(players)) {
        player->updateState(delta, sceneCenter, safetyZoneRadius);
    }

    view->centerOn(user);
    for (auto player : std::as_const(players)) {
        QPair<Player *, qreal> pair = closestEnemy(player);
        if (pair.first != nullptr && pair.second <= 600) {
            QGraphicsLineItem *line = new QGraphicsLineItem(
                QLineF(player->pos(), pair.first->pos()));
            QPen pen(Qt::red);
            pen.setWidth(5);
            pen.setStyle(Qt::DashDotLine);
            line->setPen(pen);
            scene->addItem(line);
            
            // 防止场景被删除时出现空指针访问
            QPointer<QGraphicsScene> weakScene = scene;
            // 不能使用QPointer处理QGraphicsLineItem，因为它不是QObject的子类
            //这个地方我也在怀疑 非要使用这种方法的合理性，但是本着程序能跑就不要动的原则，算了
            QTimer::singleShot(16, [weakScene, line]() {
                // 检查场景是否仍然存在
                if (!weakScene) {
                    delete line; // 无论如何都要删除线条，避免内存泄漏
                    return;
                }
                
                // 如果线条仍在场景中，从场景中移除
                if (line->scene() == weakScene) {
                    weakScene->removeItem(line);
                }
                delete line;
            });
            
            qreal q = QRandomGenerator::global()->generateDouble();
            if (q <= 0.02) {
                //NPC automatically attack each other;
                if (player != user) {
                    player->attack(pair.first);
                }
            }
        }
    }
}

void GameScene::shrinkSafetyZone()
{
    if (scaleRate >= 0.5 && backImage_normal != nullptr) {
        scaleRate -= 0.00001;
        backImage_normal->setScale(scaleRate);
        //  qDebug() << backImage_normal->mapToScene(backImage_normal->boundingRect()).boundingRect();
    }
    safetyZoneRadius = scene->width() / 2 * scaleRate;
}

void GameScene::handlePlayerDeath(Player *player)
{
    // 增加安全检查
    if (!player) {
        return;
    }
    
    if (!players.contains(player)) {
        return;
    }

    player->disconnect(this); // 断开所有连接到this的信号

    players.removeOne(player);
    qDebug() << "玩家已从游戏中移除";

    // 玩家在allItem里面有备份，所以不要瞎删除
}

QPair<Player *, qreal> GameScene::closestEnemy(Player *player)
{
    // 如果玩家列表中没有足够的玩家，则返回空结果
    if (players.size() <= 1) {
        return QPair<Player *, qreal>(nullptr, 999999999);
    }
    
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
    // 如果没有找到敌人，返回较大的距离值，确保后续不会触发空指针访问
    if (cloestEnemy == nullptr) {
        return QPair<Player *, qreal>(nullptr, 999999999);
    }
    return QPair<Player *, qreal>(cloestEnemy, sqrt(squareOfDistance));
}

void GameScene::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    // 虽然这里有冗余。
    if (!gameOn || !user || players.isEmpty()) {
        return;
    }

    for (auto player : std::as_const(players)) {
        if (!player)
            continue;

        QPair<Player *, qreal> pair = closestEnemy(user);
        if (pair.first != nullptr && pair.second <= 600) {
            QPointF start = view->mapFromScene(player->pos());
            QPointF end = view->mapFromScene(pair.first->pos());
        }
    }
}
