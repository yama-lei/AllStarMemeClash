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
    currentPlayers = allPlayers;
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, SCENE.width(), SCENE.height());
    view = new QGraphicsView(this);
    view->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    view->setScene(scene);
    view->resize(VIEW.width(), VIEW.height());
    view->scale(0.5, 0.5);
    rockImage = QPixmap(":/images/Props/rock.png");
    rockImage.scaled(800, 800);
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
    gameTime = 0;
    currentPlayers = allPlayers;
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

void GameScene::endGame()
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
        timer->stop(); // 先停止定时器
        disconnect(timer, &QTimer::timeout, this, &GameScene::updateGame);
        
        // 永远不直接删除timer，而是使用deleteLater
        timer->deleteLater();
        timer = nullptr;
    }

    // 确保在删除对象前不再有任何未处理的事件
    QCoreApplication::processEvents();

    if (elapsedTimer) {
        delete elapsedTimer;
        elapsedTimer = nullptr;
    }
    pressedKeys.clear();
    QList<QGraphicsObject *> itemsToRemove = allItems;
    for (auto item : std::as_const(itemsToRemove)) {
        if (item != nullptr) {
            if (scene && item->scene() == scene) {
                scene->removeItem(item);
            }
            item->deleteLater();
        }
    }
    allItems.clear();
    players.clear();
    props.clear();
    flyingKnives.clear();
    scaleRate = 1;
    if (backImage_normal) {
        backImage_normal->setScale(scaleRate);
    }
    safetyZoneRadius = scene->width() / 2 * scaleRate;

    // 使用QTimer::singleShot延迟发送信号，确保所有删除操作已完成
    QTimer::singleShot(1000, this, [this]() { emit gameOverSignal(); });
}

void GameScene::initPlayers()
{
    user = new User(randomPositionInCircle(sceneCenter, safetyZoneRadius));
    if (user) {
        connect(user, &User::userDie, [this]() {
            win = false;
            endGame();
        });
        connect(user, &User::shootKnives, this, &GameScene::handleShootKnives);
        players.append(user);
        scene->addItem(user);
        allItems.append(user);
    }

    for (int i = 0; i < allPlayers - 1; i++) {
        NPC *npc = new NPC(randomPositionInCircle(sceneCenter, safetyZoneRadius));
        if (npc) {
            connect(npc, &Player::playerDied, this, &GameScene::handlePlayerDeath);
            connect(npc, &NPC::shootKnives, this, &GameScene::handleShootKnives);
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

    for (int i = 0; i < 50; i++) {
        Prop* prop = new Knife(randomPositionInCircle(sceneCenter, safetyZoneRadius));
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

    for (int i = 0; i < 4; i++) {
        Prop *prop = new Bushes(randomPositionInCircle(sceneCenter, safetyZoneRadius));
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
    // qDebug() << "The key " << event->key() << " is pressed!";
    QWidget::keyPressEvent(event);
}

void GameScene::keyReleaseEvent(QKeyEvent *event)
{
    if (pressedKeys.contains(event->key())) {
        pressedKeys.remove(event->key());
    }
    // qDebug() << "The key " << event->key() << " is released!";

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
        if (pair.first != nullptr && pair.second <= 1200) {
            qDebug() << "Close Enough! Attack!";
            //注意：这个地方是
            //user->attack(pair.first);
            if (user->shootKnives(user, pair.first)) {
            }
        }
    if (event->button() == Qt::RightButton) {
        user->addBlood(-10); //最后记得删掉，这个是自己调试使用的
    }
    }
}

void GameScene::updateGame()
{
    if (!gameOn || !user) {
        return;
    }

    if (players.size() == 1 && players.contains(user)) {
        win = true;
        endGame();
        return;
    }

    qreal delta = 0.0;
    if (elapsedTimer) {
        delta = elapsedTimer->restart();
    } else {
        elapsedTimer = new QElapsedTimer();
        elapsedTimer->start();
    }
    gameTime += delta;
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
    updateFlyingProp(delta);
    //-----------------Update-------------------//
    for (auto player : std::as_const(players)) {
        player->updateState(delta, sceneCenter, safetyZoneRadius);
    }

    view->centerOn(user);
    //这里先保留吧，可能用得上，这里原先的逻辑是所有的人之间都要画线,现在加了一个只有player==user才能
    for (auto player : std::as_const(players)) {
        QPair<Player *, qreal> pair = closestEnemy(player);
        QPointer<QGraphicsScene> weakScene = scene;
        if (player == user && pair.first != nullptr) {
            //=============!!!circle和line指的是游戏要求的瞄准敌人用的===================
            if (pair.second <= 1200) {
                //line

                QGraphicsLineItem *line = new QGraphicsLineItem(
                    QLineF(player->pos(), pair.first->pos()));
                QPen pen(Qt::red);
                pen.setWidth(5);
                pen.setStyle(Qt::DashDotLine);
                line->setPen(pen);
                scene->addItem(line);
                //circle
                auto circle = scene->addEllipse(pair.first->boundingRect(),
                                                QPen(QColor(255, 105, 180, 200),
                                                     15,
                                                     Qt::DashDotLine));
                circle->setPos(pair.first->pos());
                QTimer::singleShot(0, [weakScene, line, circle]() {
                    if (!weakScene) {
                        delete line;
                        delete circle;
                        return;
                    }
                    if (line->scene() == weakScene) {
                        weakScene->removeItem(line);
                    }
                    if (circle->scene() == weakScene) {
                        weakScene->removeItem(circle);
                    }
                    delete line;
                    delete circle;
                });
            }

            //tail这个是移动特效
            auto tail = scene->addEllipse(-20,
                                          -20,
                                          40,
                                          40,
                                          QPen(QColor(80, 80, 80, 120)),
                                          QBrush(QColor(80, 80, 80, 120)));
            tail->setPos(user->pos().x(), user->pos().y() + user->boundingRect().height() / 2.3);
            // 防止场景被删除时出现空指针访问
            //==============注意如果timer设置有延迟的话是可以进行多重绘制的===========
            QTimer::singleShot(128, [weakScene, tail]() {
                if (!weakScene) {
                    delete tail;
                    return;
                }
                if (tail->scene() == weakScene) {
                    weakScene->removeItem(tail);
                }
                delete tail;
            });
            //=====================end============================
        }
        if (player != user && pair.second <= 1200) {
            NPC *npc = dynamic_cast<NPC *>(player);
            if (npc->target == nullptr) {
                npc->setTarget(pair.first);
            //下面这这一行使多余的，因为在setTarget的时候已经调用过一次了
            //  emit player->shootKnives(player, pair.first);
            }
        }
    }
    //==============Rock======================
    if (gameTime / 1000 > 30 && !rockStartRolling) {
        rockRollingDown(user->pos());
        for (int i = 0; i < 5; i++) {
            rockRollingDown(randomPositionInCircle(sceneCenter, safetyZoneRadius));
        }
        rockStartRolling = true;
        QPointer<GameScene> weakThis = this;
        QTimer::singleShot(20000, weakThis, [weakThis]() {
            if (weakThis) {
                weakThis->rockStartRolling = false;
            }
        });
    }
    //==============New Props======================
    if (gameTime / 1000 > 10) {
        qreal q = QRandomGenerator::global()->generateDouble();
        if (q <= 0.05) {
            Prop *prop = new Knife(randomPositionInCircle(sceneCenter, safetyZoneRadius));
            if (prop) {
                props.append(prop);
                scene->addItem(prop);
                allItems.append(prop);
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

    currentPlayers -= 1;
    qDebug() << "handlePlayerDeath is called";
    if (!player) {
        return;
    }
    
    if (!players.contains(player)) {
        return;
    }
    
    // 断开所有连接，防止回调调用已删除的对象
    player->disconnect(this);
    players.removeOne(player);
    // 玩家在allItem里面有备份，所以不用担心瞎删除

    QPointer<QGraphicsScene> weakScene = scene;
    QPointer<Player> weakPlayer = player;
    
    QTimer::singleShot(1000, [weakScene, weakPlayer]() {
        // 安全检查：确保场景和玩家对象仍然有效
        if (!weakScene || !weakPlayer) {
            return;
        }
        if (weakPlayer->scene() == weakScene) {
            weakScene->removeItem(weakPlayer);
        }
    });
    qDebug() << "handlePlayerDeath success";
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
    //这里之前是任何两个角色之间都会划线
    /*    for (auto player : std::as_const(players)) {
        if (!player)
            continue;

        QPair<Player *, qreal> pair = closestEnemy(user);
        if (pair.first != nullptr && pair.second <= 1200) {
            QPointF start = view->mapFromScene(player->pos());
            QPointF end = view->mapFromScene(pair.first->pos());
        }
    }*/
}

void GameScene::updateFlyingProp(qreal time)
{
    //qDebug() << "update flying props";
    QList<FlyingProp> propsToRemove;
    for (auto temp : std::as_const(flyingKnives)) {
        Prop *prop = temp.second;
        if (!prop || !scene || prop->scene() != scene) {
            propsToRemove.append(temp);
            continue;
        }
        QPointF start = temp.first.first, end = temp.first.second;
        QPointF move = end - start;
        qreal dis = sqrt(move.x() * move.x() + move.y() * move.y());
        if (dis <= 0) {
            propsToRemove.append(temp);
            continue;
        }
        qreal cos = move.x() / dis;
        qreal sin = move.y() / dis;
        if (backImage_normal && backImage_normal->contains(prop->pos())) {
            prop->setPos(prop->pos()
                         + QPointF(time * flyingPropSpeed * cos / 1000,
                                   time * flyingPropSpeed * sin / 1000));
            qDebug() << prop->pos();
        } else {
            if (prop) {
                prop->hide();
            }
            propsToRemove.append(temp);
        }
    }
    for (const auto &prop : propsToRemove) {
        flyingKnives.removeAll(prop);
    }
}

void GameScene::handleShootKnives(Player *sender, Player *target)
{
    qDebug() << "shoot knives";
    if (dynamic_cast<NPC *>(sender) != nullptr) {
        QPointer<NPC> npc = dynamic_cast<NPC *>(sender);
        QTimer::singleShot(1000, npc, [npc]() { npc->setTarget(nullptr); });
    }
    Prop *newKnife = new KnifeToAttack(sender->pos(), sender, sender->getKnifeImage());
    scene->addItem(newKnife);
    allItems.append(newKnife);
    flyingKnives.append(FlyingProp(QPair<QPointF, QPointF>(sender->pos(), target->pos()), newKnife));
}

void GameScene::rockRollingDown(QPointF point)
{
    QRectF rect = QRectF(point.x() - 200, point.y() - 200, 400, 400);
    auto tmp = QPixmap(":/images/Props/780.png");
    tmp.scaled(600, 600);
    auto targetImage = scene->addPixmap(tmp);
    targetImage->setPos(point - QPoint(tmp.width() / 2, tmp.height() / 2));
    // auto ellipse = scene->addEllipse(rect, QPen(Qt::yellow), QBrush(Qt::yellow));
    QPointer<QGraphicsScene> weakScene = scene;
    QPointer<GameScene> weakThis = this;
    QTimer::singleShot(1000, weakScene, [weakScene, targetImage, weakThis, rect, point]() {
        if (weakScene) {
            if (targetImage && targetImage->scene() == weakScene) {
                weakScene->removeItem(targetImage);
            }
            auto rock = weakScene->addPixmap(weakThis->rockImage);
            QPointF centerOffset(-rock->pixmap().width() / 2.0, -rock->pixmap().height() / 2.0);
            rock->setPos(point + centerOffset);
            for (auto player : std::as_const(weakThis->players)) {
                if (rect.contains(player->pos())) {
                    player->addBlood(-1);
                }
                QTimer::singleShot(1000, weakScene, [weakScene, rock]() {
                    if (rock) {
                        weakScene->removeItem(rock);
                    }
                });
            }
        }
    });
}
