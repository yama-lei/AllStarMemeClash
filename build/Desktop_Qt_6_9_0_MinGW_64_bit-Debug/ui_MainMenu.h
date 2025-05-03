/********************************************************************************
** Form generated from reading UI file 'MainMenu.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINMENU_H
#define UI_MAINMENU_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainMenu
{
public:
    QPushButton *pushButtonStartGame;
    QPushButton *pushButtonExitGame;

    void setupUi(QWidget *MainMenu)
    {
        if (MainMenu->objectName().isEmpty())
            MainMenu->setObjectName("MainMenu");
        MainMenu->resize(1280, 960);
        MainMenu->setMinimumSize(QSize(1280, 720));
        pushButtonStartGame = new QPushButton(MainMenu);
        pushButtonStartGame->setObjectName("pushButtonStartGame");
        pushButtonStartGame->setGeometry(QRect(250, 540, 241, 91));
        QFont font;
        font.setPointSize(19);
        pushButtonStartGame->setFont(font);
        pushButtonExitGame = new QPushButton(MainMenu);
        pushButtonExitGame->setObjectName("pushButtonExitGame");
        pushButtonExitGame->setGeometry(QRect(720, 540, 241, 91));
        pushButtonExitGame->setFont(font);

        retranslateUi(MainMenu);

        QMetaObject::connectSlotsByName(MainMenu);
    } // setupUi

    void retranslateUi(QWidget *MainMenu)
    {
        MainMenu->setWindowTitle(QCoreApplication::translate("MainMenu", "Form", nullptr));
        pushButtonStartGame->setText(QCoreApplication::translate("MainMenu", "\345\274\200\345\247\213\346\270\270\346\210\217", nullptr));
        pushButtonExitGame->setText(QCoreApplication::translate("MainMenu", "\345\277\215\347\227\233\351\200\200\345\207\272", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainMenu: public Ui_MainMenu {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINMENU_H
