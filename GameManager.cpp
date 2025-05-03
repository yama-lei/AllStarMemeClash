#include"GameManager.h"
GameManager::GameManager(QWidget* parent)
    : QMainWindow(parent)
//Note：构造函数一定要记得调用父类构造函数
{
    //---------------------------设置stackedWidget------------------------//
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);
    mainMenu = new MainMenu(this);
    gameScene = new GameScene(this);
    //创建这些东西的时候记得要设置父对象！！
    this->resize(1280, 720);
    stackedWidget->addWidget(mainMenu);
    stackedWidget->addWidget(gameScene);
    stackedWidget->setCurrentWidget(mainMenu);
    //---------------menu的按钮逻辑---------------------------------------//
    connect(mainMenu, &MainMenu::startGame, this, &GameManager::switchToGame);
    connect(mainMenu, &MainMenu::exitGame, []() { exit(0); });
    connect(gameScene, &GameScene::gameOverSignal, this, &GameManager::switchToMainMenu);
}
void GameManager::switchToGame()
{
    qDebug() << "Switch To Game";
    
    // 如果之前的场景被延迟删除但还没完成，强制设为 nullptr
    if (gameScene == nullptr) {
        gameScene = new GameScene(this);
        // 重新连接信号
        connect(gameScene, &GameScene::gameOverSignal, this, &GameManager::switchToMainMenu);
        stackedWidget->addWidget(gameScene);
    }
    
    gameScene->startGame();
    stackedWidget->setCurrentWidget(gameScene);
}

void GameManager::switchToMainMenu()
{
    qDebug() << "Switch To MainMenu";

    if (gameScene) {
        // 在删除场景前断开所有信号连接!!!
        gameScene->disconnect();
        stackedWidget->removeWidget(gameScene);
        //这个地方要注意！！！ stackedWidget存储的地址要给它删了，因为用不上了，你要把他delete
        gameScene->hide();
        // 使用定时器延迟删除，让Qt完成所有挂起的事件处理
        QTimer::singleShot(0, this, [this]() {
            if (gameScene) {
                delete gameScene;
                gameScene = nullptr;
            }
        });
    }
    stackedWidget->setCurrentWidget(mainMenu);
}
