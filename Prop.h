#ifndef PROP_H
#define PROP_H
#include <QDebug>
#include <QGraphicsObject>
#include <QPainter>
class Prop : public QGraphicsObject
{
    Q_OBJECT

protected:
    QPixmap image;

public:
    Prop(QPointF pos, QGraphicsObject* parent = nullptr);
    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    //  virtual void checkColliding();
};

//--------------------------------Blood bottle---------------//

class BloodBottle : public Prop
{
public:
    BloodBottle(QPointF pos, QGraphicsObject* parent = nullptr);
    //void checkColliding() override;
    // QPainterPath shape() const override;
};

class Bushes : public Prop
{
public:
    Bushes(QPointF pos, QGraphicsObject* parent = nullptr);
    //void checkColliding() override;
};
//==========================Knives
class Knife : public Prop
{
public:
    Knife(QPointF pos, QGraphicsObject* parent = nullptr);
};

class Boot : public Prop
{
public:
    Boot(QPointF pos, QGraphicsObject* parent = nullptr);
};

class KnifeStrong : public Prop
{
public:
    KnifeStrong(QPointF pos, QGraphicsObject* parent = nullptr);
};
#endif // PROP_H
