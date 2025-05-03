#include "MainMenu.h"
MainMenu::MainMenu(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::MainMenu)
{
    ui->setupUi(this);
    connect(ui->pushButtonExitGame, &QPushButton::clicked, this, &MainMenu::exitGame);
    connect(ui->pushButtonStartGame, &QPushButton::clicked, this, &MainMenu::startGame);
}
MainMenu::~MainMenu()
{
    delete ui;
}
