#include "ScorePanel.h"

ScorePanel::ScorePanel(QWidget *parent)
    : QWidget{parent}
    , ui(new Ui::ScorePanel)
{
    ui->setupUi(this);
    connect(ui->backToMainMenu, &QPushButton::clicked, this, &ScorePanel::backToMainMenu);
    connect(ui->anotherGame, &QPushButton::clicked, this, &ScorePanel::anotherGame);
}

void ScorePanel::updateGameInfo(bool win, int allPlayers, int rank, int surviveTime)
{
    surviveTime /= 1000;
    qDebug() << "WIn: " << win << " Rank" << rank << " surviveTime" << surviveTime;
    if (win) {
        ui->winOrLoseText->setText("Endless win! We are never tired of wining! 赢而不麻！");
    } else {
        ui->winOrLoseText->setText("胜负乃兵家常事！");
    }
    ui->rankText->setText(QString("Rank: %1/%2").arg(rank).arg(allPlayers));
    ui->surviveTimeText->setText(
        QString("SurviveTime: %1min %2s").arg(surviveTime / 60).arg(surviveTime % 60));
}
