#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QWidget>
#include <QElapsedTimer>
#include <QPointer>
#include "Player.h"
#include "Prop.h"
class GameScene : public QWidget
{
    Q_OBJECT
private:
    bool gameOn = false;
    QPainter* attackLine = nullptr;
    QSizeF SCENE = QSizeF(6000, 6000);
    QPointF sceneCenter = QPointF(3000, 3000);
    QSizeF VIEW = QSizeF(1280, 720);
    QSet<int> pressedKeys;
    QGraphicsView* view = nullptr;
    QGraphicsScene* scene = nullptr;
    User* user = nullptr;
    QGraphicsPixmapItem* backImage_normal = nullptr;
    QGraphicsPixmapItem* backImage_poison = nullptr; //用QGraphicsPixmapItem因为Pixmap没有设置层级的功能,这个是背景毒圈
    QTimer* timer = nullptr;
    QElapsedTimer* elapsedTimer = nullptr;
    QElapsedTimer* timerForRecord; //这个timer专门用于记录游戏时间
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    QList<Player*> players;
    QList<Prop*> props;
    qreal scaleRate = 1;
    void mousePressEvent(QMouseEvent* event);
    QPair<Player*, qreal> closestEnemy(Player* player);
    qreal safetyZoneRadius = 3000;
    QList<QGraphicsObject*> allItems;
    QList<QPair<QPair<QPointF, QPointF>, Prop*>> flyingKnives;
    typedef QPair<QPair<QPointF, QPointF>, Prop*> FlyingProp;

private slots:
    void handlePlayerDeath(Player* player);

public slots:
    void updateGame();
    void handleShootKnives(Player* sender, Player* target);

public:
    explicit GameScene(QWidget *parent = nullptr);
    void initPlayers();
    void extracted();
    void initProps();
    ~GameScene();
    QPointF randomPositionInCircle(QPointF center, qreal maxRadius);
    void shrinkSafetyZone();
    void startGame();
    void endGame();
    void paintEvent(QPaintEvent* event);
    void updateFlyingProp(qreal time);
    qreal flyingPropSpeed = 3000;
    typedef QPair<QPair<int, qreal>, bool> GameInfo;
    GameInfo showGameInfo();
    int currentPlayers = 10;
    const int allPlayers = 10;
    bool win = false;
    qreal gameTime = 0;
    void rockRollingDown(QPointF point);
    bool rockStartRolling = false;
    QPixmap rockImage;
signals:
    void gameOverSignal();
};

#endif // GAMESCENE_H
