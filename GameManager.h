#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <QDebug>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMainWindow>
#include <QObject>
#include <QStackedWidget>
#include <QWidget>
#include "GameScene.h"
#include "MainMenu.h"
class GameManager : public QMainWindow
{
    Q_OBJECT

private:
    QStackedWidget* stackedWidget;
    MainMenu* mainMenu;
    GameScene* gameScene;

public:
    explicit GameManager(QWidget* parent = nullptr);
    void switchToMainMenu();
    void switchToGame();
};

#endif // GAMEMANAGER_H
