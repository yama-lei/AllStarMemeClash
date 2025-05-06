#include "Player.h"
#include "Prop.h"
#include "qgraphicsscene.h"
#include "qtimer.h"

QList<QMovie *> Player::movingGifs;
QList<QMovie *> Player::standingGifs;
QList<QMovie *> Player::dieGifs;

Player::Player(QPointF pos, QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    setPos(pos);
    kinfeImage = QPixmap(":/images/Props/fc403.png");
    if (movingGifs.isEmpty()) {
        movingGifs = {new QMovie(":/images/figures/moving1.gif"),
                      new QMovie(":/images/figures/moving2.gif"),
                      new QMovie(":/images/figures/moving3.gif"),
                      new QMovie(":/images/figures/moving4.gif"),
                      new QMovie(":/images/figures/moving5.gif"),
                      new QMovie(":/images/figures/moving6.gif")};
    }
    if (dieGifs.isEmpty()) {
        dieGifs = {
            new QMovie(":/images/figures/die.gif"),
        };
    }
    if (standingGifs.isEmpty()) {
        standingGifs = {new QMovie(":/images/figures/standing2.gif"),
                        new QMovie(":/images/figures/standing3.gif"),
                        new QMovie(":/images/figures/standing4.gif")};
    }
}
QPointF Player::calculateKinvesPosition(qreal alpha)
{
    QPoint center = QPoint(0, 0);
    QRect rect = kinfeImage.rect();
    qreal radius = (boundingRect().width() + rect.width()) / 2;
    qreal rad = qDegreesToRadians(alpha); // Qt 自带函数，自动处理弧度转换
    qreal x = center.x() + radius * qSin(rad);
    qreal y = center.y() + radius * qCos(rad);
    return QPointF(x, y);
}

void Player::onDeathAnimationEnd()
{
    this->hide();
    emit playerDied(this);
}
void User::onDeathAnimationEnd()
{
    this->hide();
    emit userDie();
}
void Player::updateState(qreal time, QPointF center, qreal radius)
{
    //Update the position and other state of the pLayer(like blood)
    qDebug() << "This function shoulden't be called!";
}
QPainterPath Player::shape() const
{
    QPainterPath p;
    qreal radius = this->boundingRect().width() / 2;
    p.addEllipse(-radius,
                 -radius,
                 2 * radius,
                 2 * radius); // 这个数值也是测量出来的，这个样子比较符合游戏情境
    return p;
}
void Player::updateGif()
{
    //有currentGif的时候使用currentGif，否则默认是movingGIf
    if (currentGif) {
        currentFrame = (currentGif->currentPixmap());
    } else {
        currentFrame = (movingGif->currentPixmap());
    }
    update(); //这个update是内置函数，自动调用paint，来更新player
}
void Player::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    //调试用的两行
    //这个是角色的范围

    painter->setBrush(Qt::red);
    painter->drawEllipse(boundingRect());
    if (!currentFrame.isNull()) {
        //   QSize frameSize = currentFrame.size();
        //QPoint center = QPoint(-frameSize.width() / 2, -frameSize.height() / 2);
        QPointF center = boundingRect().center();
        QRectF rect = boundingRect();
        painter->drawPixmap(rect.center() - currentFrame.rect().center(),
                            currentFrame); //5.6日正常成功绘画
        if (numOfKinves > 0) {
            qreal per = 360 / numOfKinves;
            for (int i = 0; i < numOfKinves; i++) {
                QPointF point = calculateKinvesPosition(startAlpha + per * i);
                //painter->save();
                //painter->rotate( per * i);
                // painter->translate(point);
                // painter->rotate(startAlpha + per * i);
                painter->drawPixmap(point, kinfeImage); //5.5日，放弃旋转刀的想法
                // painter->translate(-point);
                //painter->restore();
            }
            startAlpha = (startAlpha + 2) % 360;
        }
        painter->setFont(QFont("Arial", 24, QFont::Bold));
        QString text = QString("HP: %1, Knives: %2").arg(playerBlood).arg(numOfKinves);
        painter->drawText(-boundingRect().width() / 2, boundingRect().top(), text);
        //bloodBar
        if (playerBlood > 0) {
            qreal bloodBarWidth = 400, bloodBarHeight = 20;
            QRect bloodBar(boundingRect().left(),
                           boundingRect().top(),
                           bloodBarWidth * playerBlood / defaultPlayerBlood,
                           bloodBarHeight);
            painter->setBrush(Qt::red);
            painter->drawRect(bloodBar);
        }
        qreal functionalBarHeight = boundingRect().top() - 80;

        if (specialState.contains(ATTACKUP)) {
            painter->drawPixmap(-70, functionalBarHeight, QPixmap(":/images/effect/strengthUp.png"));
        }
        if (specialState.contains(SPEEDUP)) {
            painter->drawPixmap(0, functionalBarHeight, QPixmap(":/images/effect/fast.png"));
        }
        if (specialState.contains(HEALTHUP)) {
            painter->drawPixmap(70, functionalBarHeight, QPixmap(":/images/effect/healthup.png"));
        }
        if (specialState.contains(HEALTHDOWN)) {
            painter->drawPixmap(center, QPixmap(":/images/effect/healthdown.png"));
        }
    } else {
        painter->setBrush(Qt::red);
        painter->drawRect(boundingRect());
        //出现错误地时候这样显示图像
    }
}

QRectF Player::boundingRect() const
{
    return QRectF(-180,
                  -180,
                  360,
                  360); //这里的值记得调整！如果太小会出现很奇怪的现象！角色绘制会有重影。
}

void Player::extracted(QList<QGraphicsItem *> &items)
{
    for (auto item : items) {
        if (dynamic_cast<Player *>(item) != nullptr) {
            Player *player = dynamic_cast<Player *>(item);
            if (!closeAttack) {
                this->attack(player);
                qDebug() << "Attack in close";
                QPointer<Player> temp = this;
                QTimer::singleShot(200, [temp]() {
                    if (temp) {
                        temp->closeAttack = false;
                    }
                });
            }
            closeAttack = true;
        }

        if (dynamic_cast<BloodBottle *>(item) != nullptr) {
            addBlood(1);
            specialState.append(HEALTHUP);
            QPointer<Player> weakPlayer = this;
            QTimer::singleShot(1000, this, [weakPlayer]() {
                if (weakPlayer) {
                    weakPlayer->specialState.removeAll(HEALTHUP);
                }
            });

            // 检查道具是否还在场景中
            if (item->scene()) {
                scene()->removeItem(item);
            }
            // qDebug() << "Collide with BloodBottle";
        } else if (dynamic_cast<Bushes *>(item) != nullptr) {
            //  qDebug() << "Collide with Bushes!";
        } else if (dynamic_cast<Knife *>(item) != nullptr) {
            addKnives(1);
            // qDebug() << "Knives is picked!";
            // 检查道具是否还在场景中
            if (item->scene()) {
                scene()->removeItem(item);
            }
        } else if (dynamic_cast<Boot *>(item) != nullptr) {
            if (item->scene()) {
                scene()->removeItem(item);
            }
            qreal deltaSpeed = this->playerSpeed * 0.3;
            addSpeed(deltaSpeed);
            specialState.append(SPEEDUP);

            // 使用QPointer监控this对象，防止定时器触发时对象已被销毁
            QPointer<Player> weakThis = this;
            QTimer::singleShot(5000, [weakThis, deltaSpeed]() {
                // 检查对象是否仍然存在
                if (!weakThis) {
                    return;
                }
                weakThis->addSpeed(-deltaSpeed); //return to the normal state;
                if (weakThis->specialState.contains(SPEEDUP)) {
                    weakThis->specialState.removeAll(SPEEDUP);
                };
            });
        } else if (dynamic_cast<KnifeStrong *>(item) != nullptr) {
            if (item->scene()) {
                scene()->removeItem(item);
            }
            addAttack(1);
            this->kinfeImage = QPixmap(":/images/Props/knife-2.png");
            specialState.append(ATTACKUP);

            // 使用QPointer监控this对象，防止定时器触发时对象已被销毁
            QPointer<Player> weakThis = this;
            QTimer::singleShot(3000, [weakThis]() {
                // 检查对象是否仍然存在
                if (!weakThis) {
                    return;
                }
                weakThis->kinfeImage = QPixmap(":/images/Props/fc403.png");
                weakThis->addAttack(-1); //return to the normal state;
                if (weakThis->specialState.contains(ATTACKUP)) {
                    weakThis->specialState.removeAll(ATTACKUP);
                };
            });
        } else if (dynamic_cast<KnifeToAttack *>(item) != nullptr) {
            auto temp = dynamic_cast<KnifeToAttack *>(item);
            if (this != temp->getOwner()) {
                if (temp->getOwner()) {
                    temp->getOwner()->attack(this);

                    specialState.append(HEALTHDOWN);
                    QPointer<Player> weakPlayer = this;
                    QTimer::singleShot(1000, this, [weakPlayer]() {
                        if (weakPlayer) {
                            weakPlayer->specialState.removeAll(HEALTHDOWN);
                        }
                    });
                }
                qDebug() << "Being Attacked by flying knives";
                if (item->scene()) {
                    scene()->removeItem(item);
                }
            }
        }
    }
}
void Player::handleColliding()
{
    QList<QGraphicsItem *> items = this->collidingItems();
    extracted(items);
}

//----------------------------用户=-----------------------------、、
User::User(QPointF pos, QGraphicsItem *parent)
    : Player(pos, parent)
{
    numOfKinves = 100;
    direction = STAY;
    
    // 检查是否有可用的GIF动画
    if (!movingGifs.isEmpty()) {
        movingGif = new QMovie(":/images/figures/moving1.gif");
    } else {
        movingGif = new QMovie();
        qDebug() << "ERROR: movingGifs为空，使用空QMovie";
    }
    
    idleGif = new QMovie(":/images/figures/standing2.gif");
    dieGif = dieGifs.isEmpty() ? new QMovie() : dieGifs[0];
    currentGif = movingGif;
    
    // 检查GIF是否有效
    if (!movingGif->isValid()) {
        qDebug() << "ERROR: movingGif无效";
    }
    
    if (!idleGif->isValid()) {
        qDebug() << "ERROR: idleGif无效";
    }
    
    // 连接信号
    connect(movingGif, &QMovie::frameChanged, this, &Player::updateGif);
    connect(idleGif, &QMovie::frameChanged, this, &Player::updateGif);
    
    // 启动动画
    idleGif->start();
    movingGif->start();
}

void Player::goDie()
{
    qDebug() << "ERROR: 应该在子类中重写此方法，而不是直接调用";
}

void User::updateState(qreal time, QPointF center, qreal radius)
{
    handleColliding();
    //------------update the position----------//
    QPoint step(0, 0);
    qreal m = time * playerSpeed / 1000;
    switch (direction) {
    case UP:
        step.setY(-m);
        break;
    case DOWN:
        step.setY(m);
        break;
    case LEFT:
        step.setX(-m);
        break;
    case RIGHT:
        step.setX(m);
        break;
    case LU:
        step.setX(-0.7 * abs(m));
        step.setY(-0.7 * abs(m));
        break;
    case LD:
        step.setX(-0.7 * abs(m));
        step.setY(0.7 * abs(m));
        break;
    case RU:
        step.setX(0.7 * abs(m));
        step.setY(-0.7 * abs(m));
        break;
    case RD:
        step.setX(0.7 * abs(m));
        step.setY(0.7 * abs(m));
        //1.7不是什么有多少含义的值，只是一个调整的系数，我发现这样子调整以后运动画面更加合理。
        break;
    case STAY:
        break;
    }
    //check移动

    /*    int adjust = 100;

    if (this->pos().x() + step.x() >= scene()->width() - adjust
        || this->pos().x() + step.x() <= 0 + adjust) {
        step.setX(0);
    } else if (this->pos().y() + step.y() >= scene()->height() - adjust
               || this->pos().y() + step.y() <= 0 + adjust) {
        step.setY(0);
    }*/
    QPointF newPoint = this->pos() + step;
    qreal dis = (newPoint.x() - center.x()) * (newPoint.x() - center.x())
                + (newPoint.y() - center.y()) * (newPoint.y() - center.y());

    if (radius * radius > dis) {
        moveBy(step.x(), step.y());
        //qDebug() << "r: " << radius << " dis: " << dis << " Center : " << center<< "Current Pos: " << pos();
    }
    /*    if (currentRect.contains(this->pos() + step)) {
        moveBy(step.x(), step.y());
    }
*/
    //WARNING: 这个地方死了之后就不要改变gif了
    if (!dieGifs.contains(currentGif)) {
        if (direction != STAY) {
            //
            //  qDebug() << "Move Step " << step << "Current Position: (" << this->x() << this->y() << " )";
            currentGif = movingGif;
        } else {
            currentGif = idleGif;
        }
    }

    if (playerBlood <= 0) {
        goDie();
        isAlive = false;
    }
    if (numOfKinves < 4) {
        timeAccumulator += time;
    } else {
        timeAccumulator = 0;
    }
    if (timeAccumulator >= 1000) {
        timeAccumulator = 0;
        addKnives(1);
    }
}

void User::goDie()
{
    if (!isAlive)
        return;
        
    qDebug() << "I am died";
    isAlive = false;
    dieGif = dieGifs[0];
    currentGif = dieGif;
    if (dieGif->isValid()) {
        dieGif->start();
    } else {
        qDebug() << "ERROR: 死亡动画无效";
    }
    QTimer::singleShot(1000, this, [this]() { emit userDie(); });
}

//--------------------NPC------------------------------------------------
NPC::NPC(QPointF pos, QGraphicsItem *parent)
    : Player(pos, parent)
{
    // 安全地选择一个移动GIF
    if (!movingGifs.isEmpty()) {
        int index = QRandomGenerator::global()->bounded(0, movingGifs.size());
        movingGif = movingGifs[index];
    } else {
        movingGif = new QMovie();
        qDebug() << "ERROR: movingGifs为空，使用空QMovie";
    }
    
    currentGif = movingGif;
    dieGif = dieGifs.isEmpty() ? new QMovie() : dieGifs[0];
    
    if (movingGif && movingGif->isValid()) {
        connect(movingGif, &QMovie::frameChanged, this, &Player::updateGif);
        movingGif->start();
    } else {
        qDebug() << "ERROR: NPC的movingGif无效";
    }
}

void NPC::updateState(qreal time, QPointF center, qreal radius)
{
    handleColliding();
    QPoint step(0, 0);
    qreal m = time * playerSpeed / 1000;
    qreal p = QRandomGenerator::global()->generateDouble();
    if (p <= 0.25) {
        step.setX(m);
    } else if (p <= 0.5) {
        step.setX(-m);
    } else {
        step.setX(0);
    }

    qreal q = QRandomGenerator::global()->generateDouble();
    if (q <= 0.25) {
        step.setY(m);
    } else if (q <= 0.5) {
        step.setY(-m);
    } else {
        step.setY(0);
    }

    QPointF newPoint = this->pos() + step;
    qreal dis = (newPoint.x() - center.x()) * (newPoint.x() - center.x())
                + (newPoint.y() - center.y()) * (newPoint.y() - center.y());

    if (radius * radius > dis) {
        moveBy(step.x(), step.y());
        // qDebug() << "r: " << radius << " dis: " << dis << " Center : " << center << "Current Pos: " << pos();
    }
    if (playerBlood <= 0) {
        goDie();
    }

    if (playerBlood <= 0) {
        goDie();
        isAlive = false;
    }
    if (numOfKinves < 4) {
        timeAccumulator += time;
    } else {
        timeAccumulator = 0;
    }
    if (timeAccumulator >= 1000) {
        timeAccumulator = 0;
        addKnives(1);
    }
}

void NPC::goDie()
{
    //=====================WARNING: 在player这一抽象层只关心实现，已经将所有removeItem，disConnect，delete等逻辑放在了上面一层（gameScene）===========//
    if (!isAlive)
        return;
        
    qDebug() << "NPC go die";
    
    if (dieGifs.isEmpty()) {
        qDebug() << "ERROR: dieGifs为空，无法显示死亡动画";
        isAlive = false;
        emit playerDied(this);
        return;
    }
    //这里的逻辑有点冗余，是之前崩溃的时候加的，即是程序崩溃的时候加的检查，也是我崩溃的时候加的代码。
    currentGif = dieGifs[0];
    if (currentGif == nullptr) {
        qDebug() << "ERROR: 死亡动画为空";
        isAlive = false;
        emit playerDied(this);
        return;
    }

    if (currentGif->isValid()) {
        currentGif->start();
    } else {
        qDebug() << "ERROR: 死亡动画无效";
    }

    // 确保只处理一次死亡
    isAlive = false;
    emit playerDied(this);
}

void Player::attack(Player *other)
{
    if (other == nullptr) {
        qDebug() << "Attack failed: target is null";
        return;
    }
    
    if (!other->isAlive) {
        qDebug() << "Attack failed: target is not alive";
        return;
    }

    if (this->numOfKinves >= 1) {
        this->addKnives(-1);
        qDebug() << "Attack other, knives -1";
    } else {
        qDebug() << "Failed to attack: no knives";
        return;
    }
    
    if (other->numOfKinves <= 0) {
        other->addBlood(-attackPower);
        qDebug() << "The other blood -1";
    } else {
        other->addKnives(-attackPower);
        qDebug() << "The other knife -1";
    }

    //之后要加入特效
}

void NPC::setTarget(Player *enemy)
{
    //注意：这里设计的是每一次更新NPC target只有两种情况：1. target为nullptr在gameScene中被重新赋值为一个新的对象 2. 找到一个新的对象了之后set a timer 将target改为nullptr指导下一次找到目标
    target = enemy;
    if (target != nullptr) {
        QPointer<Player> weakThis = this;
        auto tmp = target; //这里写的不是很规范，因为不能直接capture this->target.无奈之举
        QTimer::singleShot(1000, weakThis, [weakThis, tmp]() {
            weakThis->shootKnives(weakThis, tmp);
        });
    }
}
