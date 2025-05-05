#include "GameManager.h"
GameManager::GameManager(QWidget* parent)
    : QMainWindow(parent)
//Note：构造函数一定要记得调用父类构造函数
{
    //---------------------------设置stackedWidget------------------------//
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);
    mainMenu = new MainMenu(this);
    gameScene = new GameScene(this);
    scorePanel = new ScorePanel(this);
    //创建这些东西的时候记得要设置父对象！！
    this->resize(1280, 720);
    stackedWidget->addWidget(mainMenu);
    stackedWidget->addWidget(gameScene);
    stackedWidget->setCurrentWidget(mainMenu);
    stackedWidget->addWidget(scorePanel);
    //---------------menu的按钮逻辑---------------------------------------//
    connect(mainMenu, &MainMenu::startGame, this, &GameManager::switchToGame);
    connect(mainMenu, &MainMenu::exitGame, []() { exit(0); });
    connect(gameScene, &GameScene::gameOverSignal, this, &GameManager::switchToScorePanel);
    //---------------scorePanel-------------------------------------//
    connect(scorePanel, &ScorePanel::anotherGame, this, &GameManager::switchToGame);
    connect(scorePanel, &ScorePanel::backToMainMenu, this, &GameManager::switchToMainMenu);
}
void GameManager::switchToGame()
{
    qDebug() << "Switch To Game";
    stackedWidget->setCurrentWidget(gameScene);
    gameScene->startGame();
}

void GameManager::switchToMainMenu()
{
    qDebug() << "Switch To MainMenu";
    stackedWidget->setCurrentWidget(mainMenu);
}
void GameManager::switchToScorePanel()
{
    qDebug() << "Switch To ScorePanel";
    scorePanel->updateGameInfo(0, QPair<int, int>(1, 1), (int) 100);
    stackedWidget->setCurrentWidget(scorePanel);
}
