# AllStarsMemeClash Docs 全明星meme大乱斗



开发过程中的常见错误：

1.   xxxxIncomplete Type: 未引入相关头文件
1.   delete空指针
1.   delete之后不赋值为nullptr

一些问题和解决方法：

1.   在使用两个gif切换的时候，两个QMovie使用同一个槽函数updateGif，来绘制currentFrame, 并且在角色更新逻辑中更改currentGif。 但是却出现了**只绘制一个图像GIF的情况**，让我很是苦恼。询问大模型后得到可能的原因：

     ![image-20250501204801815](https://yamapicgo.oss-cn-nanjing.aliyuncs.com/picgoImage/image-20250501204801815.png)

     提示我在每一次都要重新播放Gif，但是我发现我错误的根源是**在资源初始化的时候只start了一个Gif，我误以为绘制的时候会自动地初始化**

2.   程序崩溃：

     `21:30:26: Starting D:\code\qt\Kinflash\build\Desktop_Qt_6_9_0_MinGW_64_bit-Debug\debug\Kinflash.exe...QPixmap: Must construct a QGuiApplication before a QPixmap
     21:30:34: 进程崩溃了。`在我添加了一个静态成员之后程序崩溃了，还是感谢大模型：![image-20250501213326872](https://yamapicgo.oss-cn-nanjing.aliyuncs.com/picgoImage/image-20250501213326872.png)原因分析： 因为在初始化的时候我使用了new来创建QMovie对象，但是此时QAplication还没有成功地创建，因此出现了错误。

     将初始化的过程放在程序中进行。

3.   attack算法导致程序崩溃！！！！这里五一找bug找了好几天（ai也看不出）十分地离谱：

     -   单独调用addBlood(-1)给用户自己，血量低于0之后会死亡
     -   单独调用addBlood(-1)给player，同样血量低于0之后会死亡
     -   我以为是哪里出现了`double free`,`uninitialize pointer`等等原因，加了一堆检查
     -   我甚至在想是不是发生了多线程里面讲得多线程冲突，甚至用上了`mutux`防止多个attack函数同时调用

     结果让我崩溃：

     ```cpp
      qreal per = 360 / numOfKinves; 
     ```

     就是这小小的一行。。Division by Zero...... 为什么qt不告诉我

4.   delete gameScene的时候出现程序崩溃：错误原因居然是delete之后没有赋值为nullptr！！！ 

5.   **角色无敌？？？** 原来是`void addKnives(int k) { numOfKinves += numOfKinves >= 15 ? 0 : k; }`....

6.   游戏界面和菜单界面相互切换的时候也是有bug，因为在重新new一个gameScene的时候没有connect这个gameScene和gameManager!! (感谢AI主人，我愿意当你的🐶)

7.   画不出跟踪敌人的线？为什么？？？我明明设置了pen和painter了啊？

     ```cpp
     Qwen: 因为你没有将角色的坐标映射到scene的坐标
     ```

8.   程序崩溃： 在角色死亡或者游戏胜利的时候都会出现回到MainMenu但是线程崩溃的情况。 经过多轮调试，发现问题应该出现在下面这个函数中：

     ```cpp
     void GameManager::switchToMainMenu()
     {
         qDebug() << "Switch To MainMenu";
         if (gameScene) {
             // 在删除场景前断开所有信号连接!!!
             gameScene->disconnect();
             stackedWidget->removeWidget(gameScene);
             //这个地方要注意！！！ stackedWidget存储的地址要给它删了，因为用不上了，你要把他delete
             gameScene->hide();
             QTimer::singleShot(1, this, [this]() {
                 if (gameScene) {
                     delete gameScene;
                     gameScene = nullptr;
                 }
             });
         }
         stackedWidget->setCurrentWidget(mainMenu);
     }
     ```

     发现定时器延时删除多少有点问题：1. 设置的时间是1ms，~~我以为是1s~~。 会出现不安全的情况，需要设置一个更长的时间比如100ms  2. 在lambda表达式里面，出现一个逻辑问题：我capture了this指针，删除了this所指的gameScene，但是！！this这个时候指向的已经是新的gameScene了！修改之后: 

     ```cpp
     
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
             auto temp = gameScene;
             QTimer::singleShot(100, this, [temp]() {
                 if (temp) {
                     delete temp;
                     //temp是
                 }
             });
             gameScene = nullptr;
         }
         stackedWidget->setCurrentWidget(mainMenu);
     }
     ```
     这个时候程序的bug已经完全解决了，但是ai告诉我可以直接用`deleteLater`,但是，在实际应用中发现，会出现程序崩溃的情况，原因是多个gameScene共享了许多资源，在删除的时候出现了问题（我猜的）
     
     ---

     问题还没有解决，在过去的一段时间，我总是面临着程序崩溃的情况，

     一直出现类似这样的报错：
     
     ```cpp
     QGraphicsScene::removeItem: item 0x29aa639e510's scene (0x0) is different from this scene (0x29abdf59590)
     QGraphicsScene::removeItem: item 0x29aa60c5db0's scene (0x0) is different from this scene (0x29abdf59590)
     QGraphicsScene::removeItem: item 0x29aa60c64f0's scene (0x0) is different from this scene (0x29abdf59590)
     QGraphicsScene::removeItem: item 0x29aa60c6570's scene (0x0) is different from this scene (0x29abdf59590)
     QGraphicsScene::removeItem: item 0x29aa5711090's scene (0x0) is different from this scene (0x29abdf59590)
     QGraphicsScene::removeItem: item 0x29abdee7970's scene (0x0) is different from this scene (0x29abdf59590)
     QGraphicsScene::removeItem: item 0x29aa63e52c0's scene (0x0) is different from this scene (0x29abdf59590)
     14:59:14: 进程崩溃了。
     ```
     
     结合
     
     
     
     
     
     
     
     
     
     
     
     
     
     



​	



**设置毒区：**

1.   设置两个图片（一个正常，一个是毒区） 毒区作为背景。
2.   将当前正常区域的访问传递给角色，限制其移动。

难点在于第二点，询问大语言模型后得知：`QRectF safeZoneRect = backImage_normal->mapToScene(backImage_normal->boundingRect()).boundingRect();` 可以做到







---

算法部分：

1.   随机位置生成算法：

     使用极坐标变换，和qt随机数生成函数，均匀地生成随机点：

     ```cpp
     QPointF GameScene::randomPositionInCircle(QPointF center, qreal maxRadius)
     {
         double alpha = QRandomGenerator::global()->generateDouble() * 6.28;
         qreal radius = QRandomGenerator::global()->generateDouble() * maxRadius;
         qreal x = radius * qSin(alpha);
         qreal y = radius * qCos(alpha);
         return QPointF(x, y) + center;
     }
     ```

     但是发现大部分的随机点都是在圆心附近，说明算法有问题：

     这个算法的$E(r) =0.5r$,但是$E(r^2)= 0.25r^2$，我们期望随机点按照面积均匀分布，因而将随机数进行开根号，使得有更大的可能性接近外圆周。

     ```cpp
     QPointF GameScene::randomPositionInCircle(QPointF center, qreal maxRadius)
     {
         //只能生成安全区以内的
     
         double alpha = QRandomGenerator::global()->generateDouble() * 6.28;
         qreal radius = sqrt(QRandomGenerator::global()->generateDouble()) * maxRadius;
         qreal x = radius * qSin(alpha);
         qreal y = radius * qCos(alpha);
         return QPointF(x, y) + center;
     }
     
     ```

     

2.   刀的更新算法：

     ```cpp
     // 角色的paint函数
                 qreal per = 360 / numOfKinves;
             for (int i = 0; i < numOfKinves; i++) {
                 painter->drawPixmap(calculateKinvesPosition(startAlpha + per * i), kinfeImage);
                 qDebug() << "Knife: ";
             }
             startAlpha = (startAlpha + 5) % 360;
     //具体计算每一个刀的位置：
     ```

3.   最近用户查找算法

     其实就是暴力遍历

4.   “智能”NPC算法
