#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QWidget>
#include "Player.h"
#include "Prop.h"
#include "QElapsedTimer"
class GameScene : public QWidget
{
    Q_OBJECT
private:
    bool gameStart = false;
    QPainter* attackLine;
    QSizeF SCENE = QSizeF(6000, 6000);
    QSizeF VIEW = QSizeF(1280, 720);
    QSet<int> pressedKeys;
    QGraphicsView* view;
    QGraphicsScene* scene;
    User* user;
    QGraphicsPixmapItem* backImage_normal;
    QGraphicsPixmapItem*
        backImage_poison; //用QGraphicsPixmapItem因为Pixmap没有设置层级的功能,这个是背景毒圈
    QTimer* timer;
    QElapsedTimer* elapsedTimer;
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    QList<Player*> players;
    QList<Prop*> props;
    qreal scaleRate = 1;
    void mousePressEvent(QMouseEvent* event);
    QPair<Player*, qreal> closestEnemy(Player* player);
private slots:
    void handlePlayerDeath(Player* player);

public slots:
    void updateGame();
    void gameOverSlot();

public:
    explicit GameScene(QWidget *parent = nullptr);
    void initPlayers();
    void extracted();
    void initProps();
    ~GameScene();
    QPointF randomPosition();
    void shrinkSafetyZone();
    void startGame() { gameStart = true; }
    void paintEvent(QPaintEvent* event);
signals:
    void gameOverSignal();
};

#endif // GAMESCENE_H
