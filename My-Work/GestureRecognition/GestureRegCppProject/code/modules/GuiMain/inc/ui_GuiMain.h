/********************************************************************************
** Form generated from reading UI file 'GuiMain.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GUIMAIN_H
#define UI_GUIMAIN_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GuiMainClass
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QPushButton *m_config;
    QLineEdit *m_ip;
    QPushButton *m_config_2;
    QLineEdit *m_user;
    QPushButton *m_config_3;
    QLineEdit *m_passwd;
    QPushButton *m_start_det;
    QPushButton *m_register_lav;
    QSpacerItem *horizontalSpacer;
    QGridLayout *gridLayout;
    QLabel *m_img_label;

    void setupUi(QMainWindow *GuiMainClass)
    {
        if (GuiMainClass->objectName().isEmpty())
            GuiMainClass->setObjectName(QString::fromUtf8("GuiMainClass"));
        GuiMainClass->resize(950, 642);
        GuiMainClass->setMinimumSize(QSize(0, 0));
        GuiMainClass->setMaximumSize(QSize(16777215, 16777215));
        GuiMainClass->setFocusPolicy(Qt::StrongFocus);
        centralWidget = new QWidget(GuiMainClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        centralWidget->setFocusPolicy(Qt::StrongFocus);
        centralWidget->setStyleSheet(QString::fromUtf8(""));
        verticalLayout_2 = new QVBoxLayout(centralWidget);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setSizeConstraint(QLayout::SetFixedSize);
        m_config = new QPushButton(centralWidget);
        m_config->setObjectName(QString::fromUtf8("m_config"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(m_config->sizePolicy().hasHeightForWidth());
        m_config->setSizePolicy(sizePolicy);
        m_config->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);border-width: 1px;border-style: solid;border-color: rgba(0, 0, 0, 0);padding:0px 0px;"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/ico_qrc_robot/config.png"), QSize(), QIcon::Normal, QIcon::Off);
        m_config->setIcon(icon);
        m_config->setIconSize(QSize(16, 16));

        horizontalLayout->addWidget(m_config);

        m_ip = new QLineEdit(centralWidget);
        m_ip->setObjectName(QString::fromUtf8("m_ip"));
        sizePolicy.setHeightForWidth(m_ip->sizePolicy().hasHeightForWidth());
        m_ip->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(m_ip);

        m_config_2 = new QPushButton(centralWidget);
        m_config_2->setObjectName(QString::fromUtf8("m_config_2"));
        sizePolicy.setHeightForWidth(m_config_2->sizePolicy().hasHeightForWidth());
        m_config_2->setSizePolicy(sizePolicy);
        m_config_2->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);border-width: 1px;border-style: solid;border-color: rgba(0, 0, 0, 0);padding:0px 0px;"));
        m_config_2->setIcon(icon);
        m_config_2->setIconSize(QSize(16, 16));

        horizontalLayout->addWidget(m_config_2);

        m_user = new QLineEdit(centralWidget);
        m_user->setObjectName(QString::fromUtf8("m_user"));
        sizePolicy.setHeightForWidth(m_user->sizePolicy().hasHeightForWidth());
        m_user->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(m_user);

        m_config_3 = new QPushButton(centralWidget);
        m_config_3->setObjectName(QString::fromUtf8("m_config_3"));
        sizePolicy.setHeightForWidth(m_config_3->sizePolicy().hasHeightForWidth());
        m_config_3->setSizePolicy(sizePolicy);
        m_config_3->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);border-width: 1px;border-style: solid;border-color: rgba(0, 0, 0, 0);padding:0px 0px;"));
        m_config_3->setIcon(icon);
        m_config_3->setIconSize(QSize(16, 16));

        horizontalLayout->addWidget(m_config_3);

        m_passwd = new QLineEdit(centralWidget);
        m_passwd->setObjectName(QString::fromUtf8("m_passwd"));
        sizePolicy.setHeightForWidth(m_passwd->sizePolicy().hasHeightForWidth());
        m_passwd->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(m_passwd);

        m_start_det = new QPushButton(centralWidget);
        m_start_det->setObjectName(QString::fromUtf8("m_start_det"));

        horizontalLayout->addWidget(m_start_det);

        m_register_lav = new QPushButton(centralWidget);
        m_register_lav->setObjectName(QString::fromUtf8("m_register_lav"));

        horizontalLayout->addWidget(m_register_lav);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout);

        gridLayout = new QGridLayout();
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        m_img_label = new QLabel(centralWidget);
        m_img_label->setObjectName(QString::fromUtf8("m_img_label"));

        gridLayout->addWidget(m_img_label, 0, 0, 1, 1);


        verticalLayout->addLayout(gridLayout);

        verticalLayout->setStretch(1, 20);

        verticalLayout_2->addLayout(verticalLayout);

        GuiMainClass->setCentralWidget(centralWidget);

        retranslateUi(GuiMainClass);

        QMetaObject::connectSlotsByName(GuiMainClass);
    } // setupUi

    void retranslateUi(QMainWindow *GuiMainClass)
    {
        GuiMainClass->setWindowTitle(QCoreApplication::translate("GuiMainClass", "Main", nullptr));
        m_config->setText(QCoreApplication::translate("GuiMainClass", "\346\221\204\345\203\217\345\244\264IP:", nullptr));
        m_ip->setText(QCoreApplication::translate("GuiMainClass", "192.168.11.51", nullptr));
        m_config_2->setText(QCoreApplication::translate("GuiMainClass", "\347\224\250\346\210\267\345\220\215\357\274\232", nullptr));
        m_user->setText(QCoreApplication::translate("GuiMainClass", "admin", nullptr));
        m_config_3->setText(QCoreApplication::translate("GuiMainClass", "\345\257\206\347\240\201\357\274\232", nullptr));
        m_passwd->setText(QString());
        m_start_det->setText(QCoreApplication::translate("GuiMainClass", "\345\274\200\345\247\213", nullptr));
        m_register_lav->setText(QCoreApplication::translate("GuiMainClass", "\346\277\200\346\264\273\350\247\243\347\240\201\345\231\250", nullptr));
        m_img_label->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class GuiMainClass: public Ui_GuiMainClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GUIMAIN_H
