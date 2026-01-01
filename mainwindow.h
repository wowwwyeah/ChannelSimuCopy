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
#include "PttMonitorThread.h"
#include "channelcachemanager.h"
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
    void onExitButtonClicked();

public slots:
    void goToNextWindow();
    void updateStatusBar();

private slots:
    // 处理参数变化信号的槽函数
    void handleParameterChanged(int channelKey, const ChannelSetting& newSetting);
    // 处理通道开关状态变化信号的槽函数
    void onChannelSwitchChanged(int channelNum, bool switchFlag);
private:
    void setupUI();
    void initWindowSize();
    void createPages();
    // 初始化数据库
    void initDataBase();

    void setBtnSize(int width,int height);

    // 控制侦察设备的开关状态
    int setReconSw(int chl, bool flag);
    // 控制干扰器的开关状态
    int setJammerSw(int chl, bool flag);
    //配置侦察设备信道参数
    void setJtCfg(int chl,const ChannelSetting& config);
    //配置干扰器信道参数
    void setGrCfg(int chl,const ChannelSetting& config);

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
    QPushButton *m_exitButton;

    ChannelSelect *m_channelSelect;
    SimuListView *m_simuListView;
    SystemSetting *m_systmSetting;

    DatabaseManager *m_dbManager;
    ConfigManager *m_configManager;
    ChannelParaConifg *m_channelParaConfig;
    PttMonitorThread *m_pttMonitorThread;

};

#endif // MAINWINDOW_H
