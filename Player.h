#ifndef PLAYER_H
#define PLAYER_H
#include <QGraphicsObject>
#include <QGraphicsScene>
#include <QMovie>
#include <QPainter>
#include <QRandomGenerator>
#include <QThread>
#include <QPointer>
enum Direction { UP, DOWN, LEFT, RIGHT, STAY, LU, LD, RU, RD }; //上下左右，停，坐上左下，右上右下
enum SpecialState { SPEEDUP, SPEEDDOWN, ATTACKUP, ATTACKDOWN, HEALTHUP, HEALTHDOWN };
class Player : public QGraphicsObject
{
    Q_OBJECT

protected:
    qreal timeAccumulator = 0; //这个东西专门用于给角色自动恢复刀的数量；
    bool isAlive = true;
    bool closeAttack = false; //专门用于每秒最多5下的攻击
    int numOfKinves = 4;
    int startAlpha = 0; // 第一把刀的角度
    static QList<QMovie*> movingGifs;
    static QList<QMovie*> standingGifs;
    static QList<QMovie*> dieGifs;
    Direction direction;
    QMovie* movingGif;
    QMovie* idleGif;
    QMovie* currentGif;
    QMovie* dieGif;
    QPixmap currentFrame;
    QPixmap kinfeImage;
    //--- 游戏属性--、、
    const int defaultPlayerBlood = 12;
    int playerBlood = 12;
    qreal playerSpeed = 800;
    int attackPower = 1;
    QList<SpecialState> specialState;

public:
    Player(QPointF pos, QGraphicsItem* parent = nullptr);
    virtual void updateState(qreal delta, QPointF center, qreal radius);
    void setDirection(Direction d) { direction = d; };
    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;
    QRectF boundingRect() const override;
    void extracted(QList<QGraphicsItem*>& items);
    //这个extracted方法是QT自己报出Warning 然后帮我加的，用于更加安全的遍历。
    void handleColliding();
    void addBlood(int b) { playerBlood += b; };
    void addSpeed(int s) { playerSpeed += s; };
    void addKnives(int k)
    {
        numOfKinves += numOfKinves >= 15 && k > 0 ? 0 : k;
        if (numOfKinves < 0) {
            numOfKinves = 0;
        }
    } //注意这个地方是不再增加而不是不再减少，改减少的时候还是要减少
    void addAttack(int a) { attackPower += a; };
    void attack(Player* other);
    QPainterPath shape() const override;
    QPointF calculateKinvesPosition(qreal alpha);
    virtual void goDie();
    bool isPlayerAlive() const { return isAlive; }
    QPixmap getKnifeImage() { return kinfeImage; }
public slots:
    void updateGif();
    void onDeathAnimationEnd();
signals:
    void playerDied(Player* player);
    bool shootKnives(Player* sender, Player* target);
};

class User : public Player
{
    Q_OBJECT
public:
    User(QPointF pos, QGraphicsItem* parent = nullptr);
    void updateState(qreal delta, QPointF center, qreal radius) override;
    void handleKeyPressEvent(QKeyEvent* evnet);
    void handleKeyReleaseEvent(QKeyEvent* event);
    void goDie() override;
    void onDeathAnimationEnd();
signals:
    void userDie();
};

class NPC : public Player
{
public:
    NPC(QPointF pos, QGraphicsItem* parent = nullptr);
    void updateState(qreal delta, QPointF center, qreal radius) override;
    void handleKeyPressEvent(QKeyEvent* evnet);
    void handleKeyReleaseEvent(QKeyEvent* event);
    void goDie() override;
    void setTarget(Player* enemy);
    Player* target = nullptr;
};

#endif // PLAYER_H
