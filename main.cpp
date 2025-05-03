#include <QApplication>
#include<QGraphicsView>
#include"GameManager.h"

using namespace std;



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GameManager gameManager;
    gameManager.show();
    return a.exec();
}
