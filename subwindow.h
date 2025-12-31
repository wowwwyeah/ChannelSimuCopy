#ifndef SUBWINDOW_H
#define SUBWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include "mainwindow.h"
#include "channelmodelselect.h"
#include "channelbasicpara.h"
#include "multipathpara.h"

class SwipeStackedWidget;
class PageIndicator;

class SubWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit SubWindow(QWidget *parent = nullptr);
    ~SubWindow();

    // 设置主窗口引用
    void setMainWindow(MainWindow *mainWindow);
    void setBtnSize(int width,int height);
private slots:
    void onSwipeFinished();
    void onPageChanged(int index);
    void goToNextPage();
    void goToPrevPage();
    void updateStatusBar();
    void closeSubWindow();
    void startChannelSimu();
signals:
    // 配置更新信号，当updateConfigInMap函数被调用时发出
    void configUpdated(const QString& key, const ModelParaSetting& config);
private:
    void setupUI();
    void initWindowSize();
    void createPages();
    //配置侦察设备信道参数
    void setJtCfg(int chl,const ModelParaSetting& config);
    //配置干扰器信道参数
    void setGrCfg(int chl,const ModelParaSetting& config);
    // 获取电台状态样式
    QString getStatusStyle(const QString &status);

    MainWindow *m_mainWindow;  // 主窗口引用
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
    QPushButton *m_prevButton;
    QPushButton *m_nextButton;
    QPushButton *m_startButton;
    QPushButton *m_backButton;

    ChannelModelSelect *m_channelModelSelect;
    ChannelBasicPara *m_channelBasicPara;
    MultiPathPara *m_multipathPara;

};

#endif // SUBWINDOW_H
