#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QColor>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include "channelselect.h"
#include "simulistview.h"
#include "systemsetting.h"
#include "configmanager.h"
#include "databasemanager.h"


class SwipeStackedWidget;
class PageIndicator;
class SubWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 设置副窗口引用
    void setSubWindow(SubWindow *subWindow);
    int getChannelNum();
    void setChannelPara(const ModelParaSetting &config);

private slots:
    void onSwipeFinished();
    void onPageChanged(int index);
    void goToNextPage();
    void goToPrevPage();
    void goToNextWindow();
    void updateStatusBar();

private:
    void setupUI();
    void initWindowSize();
    void createPages();
    // 初始化数据库
    void initDataBase();

    // 获取电台状态样式
    QString getStatusStyle(const QString &status);
    SubWindow *m_subWindow;  // 副窗口引用

    QTimer *m_timer;
    QLabel *m_label1;
    QLabel *m_label2;
    QLabel *m_label3;
    QLabel *m_label4;
    QLabel *m_indicator1;
    QLabel *m_indicator2;
    QLabel *m_indicator3;
    QLabel *m_indicator4;
    SwipeStackedWidget *m_stackedWidget;
    PageIndicator *m_pageIndicator;
    QVector<QString> m_pageTitle;
    QPushButton *m_leftArrowButton;
    QPushButton *m_rightArrowButton;
    QPushButton *m_prevButton;
    QPushButton *m_nextButton;
    QPushButton *m_startButton;

    ChannelSelect *m_channelSelect;
    SimuListView *m_simuListView;
    SystemSetting *m_systmSetting;

    DatabaseManager *m_dbManager;
    ConfigManager *m_configManager;
    ChannelParaConifg *m_channelParaConfig;

};

#endif // MAINWINDOW_H
