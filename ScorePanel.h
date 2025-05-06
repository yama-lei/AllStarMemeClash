#ifndef SCOREPANEL_H
#define SCOREPANEL_H

#include <QObject>
#include <QWidget>
#include <ui_ScorePanel.h>
class ScorePanel : public QWidget
{
    Q_OBJECT
    Ui::ScorePanel *ui;

public:
    explicit ScorePanel(QWidget *parent = nullptr);
    void updateGameInfo(bool win, int allPlayers, int rank, int surviveTime);
signals:
    void backToMainMenu();
    void anotherGame();
};

#endif // SCOREPANEL_H
