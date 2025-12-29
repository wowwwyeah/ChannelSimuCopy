#include "subwindow.h"
#include "mainwindow.h"
#include "swipestackedwidget.h"
#include "pageindicator.h"
#include "pagewidget.h"
#include "configmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScreen>
#include <QToolBar>
#include <QStatusBar>
#include <QApplication>

SubWindow::SubWindow(QWidget *parent)
    : QMainWindow{parent}
{
    m_pageTitle = {
        "模板选择",
        "基本参数",
        "多径设置",
    };
    setStyleSheet("QMainWindow { background-color: #336666; }");
    setupUI();
    createPages();

    updateStatusBar();

    // 创建定时器
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &SubWindow::updateStatusBar);

    // 启动定时器，每1000毫秒（1秒）触发一次
    m_timer->start(5000);
}

SubWindow::~SubWindow()
{
    m_timer->stop();
}

void SubWindow::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

void SubWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 创建滑动堆叠窗口
    m_stackedWidget = new SwipeStackedWidget(this);
    mainLayout->addWidget(m_stackedWidget);

    // 创建页面指示器
    m_pageIndicator = new PageIndicator(m_pageTitle.size(), this);
    mainLayout->addWidget(m_pageIndicator);

    //创建关闭工具栏
    QToolBar *closetoolBar = new QToolBar(this);
    addToolBar(closetoolBar);

    // 添加左侧弹性空间
    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    closetoolBar->addWidget(spacer);

    // 添加关闭动作
    QAction *closeAction = new QAction("返回", this);
    closeAction->setShortcut(QKeySequence::Close); // 设置快捷键 Ctrl+W
    closeAction->setStatusTip("回到主菜单"); // 状态栏提示
    closetoolBar->setStyleSheet("color: #2196F3;");

    // 连接信号槽
    connect(closeAction, &QAction::triggered, this, &SubWindow::closeSubWindow);

    // 添加到工具栏
    closetoolBar->addAction(closeAction);


    // 创建导航工具栏
    QToolBar *toolBar = new QToolBar(this);
    addToolBar(Qt::BottomToolBarArea, toolBar);

    // 添加左侧弹性空间
    QWidget *leftSpacer = new QWidget();
    leftSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolBar->addWidget(leftSpacer);

    // 添加按钮
    m_prevButton = new QPushButton("上一页", this);
    m_nextButton = new QPushButton("下一页", this);
    m_startButton = new QPushButton("开始模拟", this);
    m_prevButton->setEnabled(true);
    m_startButton->setEnabled(true);
    toolBar->addWidget(m_prevButton);
    toolBar->addSeparator();
    toolBar->addWidget(m_startButton);
    toolBar->addSeparator();
    toolBar->addWidget(m_nextButton);

    // 设置按钮样式
    m_startButton->setStyleSheet("QPushButton {"
                                 "    background-color: #238E23;"
                                 "    color: white;"
                                 "    font-size: 12px;"
                                 "    border-radius: 10px;"
                                 "    padding: 4px 6px;"
                                 "}"
                                 "QPushButton:hover {"
                                 "    background-color: #32CD32;"
                                 "}");

    m_prevButton->setStyleSheet("QPushButton {"
                                 "    background-color: #D98719;"
                                 "    color: white;"
                                 "    font-size: 12px;"
                                 "    border-radius: 10px;"
                                 "    padding: 4px 6px;"
                                 "}"
                                 "QPushButton:hover {"
                                 "    background-color: #FF7F00;"
                                 "}"
                                 "QPushButton:disabled {"
                                 "    background-color: #225555;"
                                 "    color: 88AAAA;"
                                 "}");
    m_nextButton->setStyleSheet("QPushButton {"
                                "    background-color: #D98719;"
                                "    color: white;"
                                "    font-size: 12px;"
                                "    border-radius: 10px;"
                                "    padding: 4px 6px;"
                                "}"
                                "QPushButton:hover {"
                                "    background-color: #FF7F00;"
                                "}"
                                "QPushButton:disabled {"
                                "    background-color: #225555;"
                                "    color: 88AAAA;"
                                "}");


    // 添加右侧弹性空间
    QWidget *rightSpacer = new QWidget();
    rightSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolBar->addWidget(rightSpacer);

    connect(m_prevButton, &QPushButton::clicked, this, &SubWindow::goToPrevPage);
    connect(m_nextButton, &QPushButton::clicked, this, &SubWindow::goToNextPage);
    connect(m_startButton, &QPushButton::clicked, this, &SubWindow::startChannelSimu);

    // 启用滑动（默认已启用）
    m_stackedWidget->enableSwipe(true);

    // 设置动画时长
    m_stackedWidget->setAnimationDuration(400);

    // 连接信号
    connect(m_stackedWidget, &SwipeStackedWidget::swipeFinished, this, &SubWindow::onSwipeFinished);
    connect(m_stackedWidget, &SwipeStackedWidget::pageChanged, this, &SubWindow::onPageChanged);

    // 状态栏
    // 创建状态圆点
    m_indicator1 = new QLabel();
    m_indicator1->setFixedSize(12, 12);
    m_indicator1->setStyleSheet(getStatusStyle("停止"));
    m_label1 = new QLabel("电台1", this);
    m_label1->setStyleSheet("color:white;");
    m_indicator2 = new QLabel();
    m_indicator2->setFixedSize(12, 12);
    m_indicator2->setStyleSheet(getStatusStyle("停止"));
    m_label2 = new QLabel("电台2", this);
    m_label2->setStyleSheet("color:white;");
    m_indicator3 = new QLabel();
    m_indicator3->setFixedSize(12, 12);
    m_indicator3->setStyleSheet(getStatusStyle("停止"));
    m_label3 = new QLabel("电台3", this);
    m_label3->setStyleSheet("color:white;");
    m_indicator4 = new QLabel();
    m_indicator4->setFixedSize(12, 12);
    m_indicator4->setStyleSheet(getStatusStyle("停止"));
    m_label4 = new QLabel("电台4", this);
    m_label4->setStyleSheet("color:white;");

    // 使用 addPermanentWidget 添加到右侧
    statusBar()->addPermanentWidget(m_indicator1);
    statusBar()->addPermanentWidget(m_label1);
    statusBar()->addPermanentWidget(m_indicator2);
    statusBar()->addPermanentWidget(m_label2);
    statusBar()->addPermanentWidget(m_indicator3);
    statusBar()->addPermanentWidget(m_label3);
    statusBar()->addPermanentWidget(m_indicator4);
    statusBar()->addPermanentWidget(m_label4);
    // Remove separator lines by setting stylesheet
    statusBar()->setStyleSheet("QStatusBar::item { border: none; }");
    statusBar()->showMessage("准备就绪");

    initWindowSize();
}

void SubWindow::initWindowSize()
{
    // 获取 Firefly 开发板的屏幕（支持多屏幕，取主屏幕）
    QScreen *screen = QApplication::primaryScreen();
    if (!screen) {
        qWarning() << "无法获取屏幕信息，使用默认大小";
        return;
    }

    // 获取屏幕可用区域（排除任务栏/标题栏，推荐）
    QRect availableRect = screen->availableGeometry();
    // 获取屏幕完整区域（包含任务栏，适合全屏）
    QRect fullRect = screen->geometry();

    // 打印屏幕分辨率（调试用，可查看 Firefly 实际屏幕大小）
    qDebug() << "Firefly 屏幕可用分辨率：" << availableRect.width() << "x" << availableRect.height();
    qDebug() << "Firefly 屏幕完整分辨率：" << fullRect.width() << "x" << fullRect.height();

    // 方案 2：窗口居中显示 + 适应屏幕80%大小（更友好）
    int w = availableRect.width();
    int h = availableRect.height() - 35;
    this->resize(w, h);
    //this->move((availableRect.width() - w)/2, (availableRect.height() - h)/2);  // 居中

}

void SubWindow::createPages()
{
    m_channelModelSelect = new ChannelModelSelect();
    m_channelBasicPara = new ChannelBasicPara();
    m_multipathPara = new MultiPathPara();
    m_stackedWidget->addWidget(m_channelModelSelect);
    m_stackedWidget->addWidget(m_channelBasicPara);
    m_stackedWidget->addWidget(m_multipathPara);

    m_stackedWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_pageIndicator->setCurrentIndex(0);
}

void SubWindow::onPageButtonClicked()
{
    PageWidget *page = qobject_cast<PageWidget*>(sender());
    if (page) {
        statusBar()->showMessage(QString("点击了: %1").arg(page->title()));
    }
}

void SubWindow::onSwipeFinished()
{
    int currentIndex = m_stackedWidget->currentIndex();
    int totalPages = m_stackedWidget->count();

    // 更新状态栏消息，显示循环信息
    QString message = QString("当前页面: %1/%2").arg(currentIndex + 1).arg(totalPages);
    statusBar()->showMessage(message);
}

void SubWindow::onPageChanged(int index)
{
    m_prevButton->setEnabled(true);
    m_nextButton->setEnabled(true);
    m_startButton->setEnabled(index < 1);
    m_pageIndicator->setCurrentIndex(index);
    statusBar()->showMessage(QString("当前页面: %1/%2").arg(index + 1).arg(m_pageTitle.size()));
}

void SubWindow::goToNextPage()
{
    int current = m_stackedWidget->currentIndex();
    int total = m_stackedWidget->count();
    int next = (current + 1) % total;  // 循环到下一个页面

    m_stackedWidget->setCurrentIndex(next);
    onPageChanged(next);
}

void SubWindow::goToPrevPage()
{
    int current = m_stackedWidget->currentIndex();
    int total = m_stackedWidget->count();
    int prev = (current - 1 + total) % total;  // 循环到上一个页面

    m_stackedWidget->setCurrentIndex(prev);
    onPageChanged(prev);

}

void SubWindow::closeSubWindow()
{
    if(m_mainWindow)
    {
        m_mainWindow->setGeometry(this->geometry());
        m_mainWindow->show();
    }

    this->close();
}

void SubWindow::startChannelSimu()
{
    ModelParaSetting config;
    config.channelNum = m_mainWindow->getChannelNum();
    config.modelName = m_channelModelSelect->getSelectedRadioButtonText();
    config.noisePower = m_channelBasicPara->getNoisePower();
    config.signalAnt = m_channelBasicPara->getAttenuationPower();
    config.comDistance = m_channelBasicPara->getCommunicationDistance();
    config.filterNum = m_multipathPara->getFilterNum();
    config.multipathNum = m_multipathPara->getMultipathCount();
    config.multipathType = m_multipathPara->getMultipathPara();

    QMutexLocker locker(&globalMutex);
    globalParaMap[config.modelName] = config;

    m_mainWindow->setChannelPara(config);
}



QString SubWindow::getStatusStyle(const QString &status)
{
    if (status == "发射" || status == "接收") {
        return "QLabel { background-color: #4CAF50; border-radius: 6px; border: 1px solid #2E7D32; }";
    } else if (status == "警告" || status == "进行中") {
        return "QLabel { background-color: #FFC107; border-radius: 6px; border: 1px solid #F57F17; }";
    } else if (status == "错误" || status == "失败") {
        return "QLabel { background-color: #F44336; border-radius: 6px; border: 1px solid #C62828; }";
    } else if (status == "停止" || status == "禁用") {
        return "QLabel { background-color: #9E9E9E; border-radius: 6px; border: 1px solid #616161; }";
    } else {
        return "QLabel { background-color: #2196F3; border-radius: 6px; border: 1px solid #0D47A1; }";
    }
}

void SubWindow::updateStatusBar()
{
    QMutexLocker locker(&globalMutex);
    switch(globalStatusMap[1].radioState)
    {
        case RADIO_DISABLE:
            m_label1->setText(QString("电台1:停止"));
            m_indicator1->setStyleSheet(getStatusStyle("停止"));
            break;
        case RADIO_RECEIVE:
            m_label1->setText(QString("电台1:接收"));
            m_indicator1->setStyleSheet(getStatusStyle("接收"));
            break;
        case RADIO_TRANSMIT:
            m_label1->setText(QString("电台1:发射，%1dbm").arg(globalStatusMap[1].txPower));
            m_indicator1->setStyleSheet(getStatusStyle("发射"));
            break;
        case RADIO_ALARM:
            m_label1->setText(QString("电台1:发射，%1dbm").arg(globalStatusMap[1].txPower));
            m_indicator1->setStyleSheet(getStatusStyle("警告"));
            break;
    }

    switch(globalStatusMap[2].radioState)
    {
        case RADIO_DISABLE:
            m_label2->setText(QString("电台2:停止"));
            m_indicator2->setStyleSheet(getStatusStyle("停止"));
            break;
        case RADIO_RECEIVE:
            m_label2->setText(QString("电台2:接收"));
            m_indicator2->setStyleSheet(getStatusStyle("接收"));
            break;
        case RADIO_TRANSMIT:
            m_label2->setText(QString("电台2:发射，%1dbm").arg(globalStatusMap[2].txPower));
            m_indicator2->setStyleSheet(getStatusStyle("发射"));
            break;
        case RADIO_ALARM:
            m_label2->setText(QString("电台2:发射，%1dbm").arg(globalStatusMap[2].txPower));
            m_indicator2->setStyleSheet(getStatusStyle("警告"));
            break;
    }

    switch(globalStatusMap[3].radioState)
    {
        case RADIO_DISABLE:
            m_label3->setText(QString("电台3:停止"));
            m_indicator3->setStyleSheet(getStatusStyle("停止"));
            break;
        case RADIO_RECEIVE:
            m_label3->setText(QString("电台3:接收"));
            m_indicator3->setStyleSheet(getStatusStyle("接收"));
            break;
        case RADIO_TRANSMIT:
            m_label3->setText(QString("电台3:发射，%1dbm").arg(globalStatusMap[3].txPower));
            m_indicator3->setStyleSheet(getStatusStyle("发射"));
            break;
        case RADIO_ALARM:
            m_label3->setText(QString("电台3:发射，%1dbm").arg(globalStatusMap[3].txPower));
            m_indicator3->setStyleSheet(getStatusStyle("警告"));
            break;
    }

    switch(globalStatusMap[4].radioState)
    {
        case RADIO_DISABLE:
            m_label4->setText(QString("电台4:停止"));
            m_indicator4->setStyleSheet(getStatusStyle("停止"));
            break;
        case RADIO_RECEIVE:
            m_label4->setText(QString("电台4:接收"));
            m_indicator4->setStyleSheet(getStatusStyle("接收"));
            break;
        case RADIO_TRANSMIT:
            m_label4->setText(QString("电台4:发射，%1dbm").arg(globalStatusMap[4].txPower));
            m_indicator4->setStyleSheet(getStatusStyle("发射"));
            break;
        case RADIO_ALARM:
            m_label4->setText(QString("电台4:发射，%1dbm").arg(globalStatusMap[4].txPower));
            m_indicator4->setStyleSheet(getStatusStyle("警告"));
            break;
    }
}
