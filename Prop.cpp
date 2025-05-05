#include "Prop.h"

Prop::Prop(QPointF pos, QGraphicsObject* parent)
    : QGraphicsObject(parent)
{
    this->setPos(pos);
}
void Prop::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (!image.isNull()) {
        painter->drawPixmap(0, 0, image);
    } else {
        qDebug() << "Error: the image of Prop is failed to load";
    }
    //Debug用
    //painter->setBrush(Qt::red);
    //painter->drawRect(boundingRect());
}

QRectF Prop::boundingRect() const
{
    return QRect(0, 0, image.width(), image.height());
}
QPainterPath Prop::shape() const
{
    return this->QGraphicsObject::shape();
}

//========================================BloodBottle===========================

BloodBottle::BloodBottle(QPointF pos, QGraphicsObject* parent)
    : Prop(pos, parent)
{
    image = QPixmap(":/images/Props/fc272.png");
}

//===================================Bushes========================

Bushes::Bushes(QPointF pos, QGraphicsObject* parent)
    : Prop(pos, parent)
{
    image = QPixmap(":/images/Props/bushes.png");
    this->setScale(4);
}

Knife::Knife(QPointF pos, QGraphicsObject* parent)
    : Prop(pos, parent)
{
    image = QPixmap(":/images/Props/fc403.png");
}

KnifeToAttack::KnifeToAttack(QPointF pos, QGraphicsObject* parent)
    : Prop(pos, parent)
{
    image = QPixmap(":/images/Props/fc403.png");
}

KnifeStrong::KnifeStrong(QPointF pos, QGraphicsObject* parent)
    : Prop(pos, parent)
{
    image = QPixmap(":/images/Props/knife-2.png");
}

Boot::Boot(QPointF pos, QGraphicsObject* parent)
    : Prop(pos, parent)
{
    image = QPixmap(":/images/Props/boot.png");
}
