#ifndef MAINMENU_H
#define MAINMENU_H
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QWidget>
#include <ui_MainMenu.h>

class MainMenu : public QWidget
{
    Q_OBJECT
private:
    Ui::MainMenu *ui;

public:
    MainMenu(QWidget *parent = nullptr);
    ~MainMenu();
signals:
    void startGame();
    void exitGame();
};

#endif // MAINMENU_H
