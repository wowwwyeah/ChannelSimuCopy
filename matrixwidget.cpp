#include <QDebug>
#include <QTimer>
#include <QLabel>
#include <QVBoxLayout>
#include <QMessageBox>
#include "datamanager.h"
#include "matrixwidget.h"
#include "configmanager.h"
#include "fpga_driver.h"
#include "channel_utils.h"
// 定义开关颜色常量
const QColor MatrixWidget::SWITCH_COLOR_ON = QColor("#2E7D32");  // 绿色
const QColor MatrixWidget::SWITCH_COLOR_OFF = QColor("#8B2323"); // 红色
MatrixWidget::MatrixWidget(QWidget *parent)
    : QTableWidget(5, 5, parent)
    , m_longPressTimer(new QTimer(this))
{
    m_highlightedRow = -1;
    m_highlightedCol = -1;
    m_selectedChannel = 0;
    m_pressedChannel = -1;
    m_pressedItem = nullptr;
    m_isLongPressTriggered = false;

    // 初始化开关状态映射
    for (int i = 1; i <= 15; ++i) {
        m_switchStates.insert(i, OFF);
    }

    // 初始化表头
    initHeaders();

    // 初始化单元格
    initCells();

    // 设置长按计时器
    m_longPressTimer->setSingleShot(true);
    connect(m_longPressTimer, &QTimer::timeout, this, &MatrixWidget::onLongPressTimeout);


#ifdef  USE_TOUCH_EVENT
    // 启用触摸事件
    setAttribute(Qt::WA_AcceptTouchEvents);
    qDebug() << "使用触摸事件模式";
#else
    // 启用鼠标跟踪
    setMouseTracking(true);
    setSelectionMode(QAbstractItemView::NoSelection);
    connect(this, &QTableWidget::cellClicked, this, &MatrixWidget::changeCellColor);
    qDebug() << "使用鼠标事件模式";
#endif
}

MatrixWidget::~MatrixWidget()
{
}

void MatrixWidget::initHeaders()
{
    // 设置水平表头（列标题）
    setHorizontalHeaderLabels({"电台1", "电台2", "电台3", "电台4", "干扰器"});

    // 设置垂直表头（行标题）
    setVerticalHeaderLabels({"电台1", "电台2", "电台3", "电台4", "频谱仪"});

    // 设置表头样式
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    horizontalHeader()->setStyleSheet("QHeaderView::section {color:#FFFFFF; font-weight: bold;font-size: 18pt; background-color: #2A3B38;}");
    verticalHeader()->setStyleSheet("QHeaderView::section {color:#FFFFFF; font-weight: bold; font-size: 18pt;background-color: #2A3B38;}");

    this->setStyleSheet(
        "QTableWidget::item {"
        "   font-weight: bold;"
        "   font-size: 18pt;"
        "}"
        "QTableWidget::item:selected {"
        "   background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
        "                               stop: 0 #f0f0f0, stop: 1 #e0e0e0);"
        "   border: 2px outset #909090;"
        "   border-radius: 3px;"
        "   font-weight: bold;"
        "   font-size: 18pt;"
        "}"
        "QTableWidget QTableCornerButton::section {"
        "   background-color: #2A3B38;"
        "}"
        );
}

void MatrixWidget::initCells()
{
    // 创建一个映射表，定义每个单元格应该存储的数字
    // -1表示该单元格不存储数字
    int channelMap[5][5] = {
        {-1, -1, -1, -1, 11},
        { 1, -1, -1, -1, 12},
        { 2,  4, -1, -1, 13},
        { 3,  5,  6, -1, 14},
        { 7,  8,  9, 10, 15}
    };

    for (int row = 0; row < rowCount(); ++row) {
        for (int col = 0; col < columnCount(); ++col) {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);

            int channelNum = channelMap[row][col];
            if (channelNum != -1) {
                item->setBackground(SWITCH_COLOR_OFF); // 使用红色常量作为默认关闭状态
                // 将数字存储在item的自定义属性中
                item->setData(Qt::UserRole, channelNum);
                item->setText(QString::number(channelNum));
                // 设置开关状态（默认关闭）
                m_switchStates[channelNum] = OFF;
            } else {
                item->setBackground(QColor("#336666")); // 无数字的单元格显示背景色
            }
            item->setFlags(item->flags() & Qt::ItemIsEnabled); // 禁用点击
            setItem(row, col, item);
        }
    }
}

bool MatrixWidget::isUpperTriangle(int row, int col) const
{
    // 根据新的布局，重新定义哪些单元格属于可交互区域
    // 这里返回true表示该单元格可以被点击和长按
    int channelMap[5][5] = {
        {-1, -1, -1, -1, 11},
        { 1, -1, -1, -1, 12},
        { 2,  4, -1, -1, 13},
        { 3,  5,  6, -1, 14},
        { 7,  8,  9, 10, 15}
    };
    return channelMap[row][col] != -1;
}

// 短按触摸：创建/消除凸起效果
void MatrixWidget::handleTouchClick(int row, int column)
{
    QTableWidgetItem *item = this->item(row, column);
    if (isUpperTriangle(row, column) && item)
    {
        int channelNum = item->data(Qt::UserRole).toInt();

        // 无论开关状态如何，都可以创建凸起效果
        if (row == m_highlightedRow && column == m_highlightedCol) {
            // 消除凸起效果的widget
            removeCellWidget(row, column);
            m_highlightedRow = -1;
            m_highlightedCol = -1;
            m_selectedChannel = 0;
            qDebug() << "消除凸起效果 - 行:" << row << "列:" << column;
        }
        else
        {
            // 检查是否已有凸起的单元格
            if (m_highlightedRow != -1 && m_highlightedCol != -1)
            {
                // 消除之前的凸起效果
                removeCellWidget(m_highlightedRow, m_highlightedCol);
            }

            // 创建凸起效果的widget
            createRaisedCellWidget(row, column, item);
            m_highlightedRow = row;
            m_highlightedCol = column;
            m_selectedChannel = channelNum;
            qDebug() << "创建凸起效果 - 行:" << row << "列:" << column << "通道:" << m_selectedChannel;
        }
    }
}

// 长按触摸：切换颜色
void MatrixWidget::handleTouchLongPress(int row, int column)
{
    QTableWidgetItem *item = this->item(row, column);
    if (!item) return;

    // 只允许上三角区域变色
    if (isUpperTriangle(row, column)) {
        int channelNum = item->data(Qt::UserRole).toInt();

        // 获取当前状态并切换
        SwitchState currentState = getSwitchState(channelNum);
        SwitchState newState = (currentState == ON) ? OFF : ON;
        bool switchFlag = (newState == ON);

        // 更新开关状态
        setSwitchState(channelNum, newState);

        // 设置新的背景颜色
        item->setBackground(newState == ON ? SWITCH_COLOR_ON : SWITCH_COLOR_OFF);

        // 输出调试信息
        if (newState == ON) {
            qDebug() << "长按变色 - 红→绿 行:" << row << "列:" << column << "开关状态:开" << "信道编号:" << channelNum;
        } else {
            qDebug() << "长按变色 - 绿→红 行:" << row << "列:" << column << "开关状态:关" << "信道编号:" << channelNum;
        }

        // 开关状态切换后，如果该单元格有凸起效果，更新凸起效果的背景色
        if (row == m_highlightedRow && column == m_highlightedCol) {
            // 先移除旧的凸起效果
            removeCellWidget(row, column);
            // 重新创建带有新颜色的凸起效果
            createRaisedCellWidget(row, column, item);
            qDebug() << "颜色改变，更新凸起效果背景色";
        }

        m_pressedChannel = channelNum;

        // 发送通道开关状态变化信号
        setChlSwitchToHw(channelNum, switchFlag);
        qDebug() << "信道编号: channel=" << channelNum << ", switchFlag=" << switchFlag;
    }
}
void MatrixWidget::setChlSwitchToHw(int chl,bool flag){
    if(IS_VALID_DYNAMIC_CHANNEL(chl)){//DAC动态分配信道
        emit channelSwitchChanged(chl, flag);
    }else if(IS_VALID_RECON_CHANNEL(chl)){//侦察设备
        // int retsw=set_chl_sw(static_cast<RS_OUT_E>(chl),flag); //待填
        // if (retsw != FPGA_OK) {
        //     qDebug()<< "Failed to set_ch_sw" << "chl:"<<chl;
        // }
    }else if(IS_VALID_JAMMER(chl)){//干扰器
        //int retsw=set_gr_sw();
        // if (retsw != FPGA_OK) {
        //     qDebug()<< "Failed to set_ch_sw" << "chl:"<<chl;
        // }
    }
}
// 创建凸起效果的widget
void MatrixWidget::createRaisedCellWidget(int row, int column, QTableWidgetItem *item)
{
    // 创建凸起效果的widget
    QWidget *cellWidget = new QWidget();
    
    // 获取通道号和开关状态，使用对应颜色
    int channelNum = item->data(Qt::UserRole).toInt();
    SwitchState state = getSwitchState(channelNum);
    QColor bgColor = (state == ON) ? SWITCH_COLOR_ON : SWITCH_COLOR_OFF;
    
    QString styleSheet = QString(
                             "QWidget {"
                             "   border: 4px outset #a0a0a0;"
                             "   background-color: %1;"
                             "   border-radius: 6px;"
                             "}"
                             ).arg(bgColor.name());

    cellWidget->setStyleSheet(styleSheet);

    QLabel *label = new QLabel(item->text());
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet(
        "QLabel {"
        "   color: white;"
        "   font-weight: bold;"
        "   font-size: 12px;"
        "}"
        );

    QVBoxLayout *layout = new QVBoxLayout(cellWidget);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->addWidget(label);

    this->setCellWidget(row, column, cellWidget);
}

// 移除单元格widget
void MatrixWidget::removeCellWidget(int row, int column)
{
    // 移除单元格widget，恢复为普通的QTableWidgetItem
    QWidget *widget = cellWidget(row, column);
    if (widget) {
        widget->deleteLater();
    }
    setCellWidget(row, column, nullptr);
}
#ifdef  USE_TOUCH_EVENT
// 触摸事件处理
bool MatrixWidget::event(QEvent *event) {
    if (event->type() == QEvent::TouchBegin ||
        event->type() == QEvent::TouchUpdate ||
        event->type() == QEvent::TouchEnd ||
        event->type() == QEvent::TouchCancel) {
        handleTouchEvent(static_cast<QTouchEvent*>(event));
        return true;
    }
    return QTableWidget::event(event);
}

void MatrixWidget::handleTouchEvent(QTouchEvent *event) {
    switch (event->type()) {
    case QEvent::TouchBegin: {
        if (!event->touchPoints().isEmpty()) {
            const QTouchEvent::TouchPoint &touchPoint = event->touchPoints().first();
            // 将触摸点坐标转换为视图坐标
            QPoint viewPos = viewport()->mapFromParent(touchPoint.pos().toPoint());
            m_touchStartPos = viewPos;
            m_pressedItem = itemAt(m_touchStartPos);

            if (m_pressedItem) {
                m_longPressTimer->start(800); // 800ms长按阈值
                m_isLongPressTriggered = false;
                int channelNum = m_pressedItem->data(Qt::UserRole).toInt();
                qDebug() << "触摸开始 - 位置:" << m_touchStartPos << "行:" << row(m_pressedItem) << "列:" << column(m_pressedItem) << "通道:" << channelNum;
            }
        }
        event->accept();
        break;
    }

    case QEvent::TouchUpdate: {
        // 移除移动距离检查，只要长按时间超过800ms即可触发开关操作
        // 即使手指在长按过程中稍微移动也不取消长按检测
        event->accept();
        break;
    }

    case QEvent::TouchEnd:
    case QEvent::TouchCancel: {
        m_longPressTimer->stop();

        if (m_pressedItem) {
            int row = m_pressedItem->row();
            int col = m_pressedItem->column();
            int channelNum = m_pressedItem->data(Qt::UserRole).toInt();

            if (m_isLongPressTriggered) {
                // 长按操作已完成（在onLongPressTimeout中处理）
                qDebug() << "长按操作完成 - 行:" << row << "列:" << col << "通道:" << channelNum;
            } else {
                // 检查触摸结束位置是否仍在同一单元格内
                bool isStillOnSameItem = false;
                if (!event->touchPoints().isEmpty()) {
                    const QTouchEvent::TouchPoint &touchPoint = event->touchPoints().first();
                    QPoint currentPos = viewport()->mapFromParent(touchPoint.pos().toPoint());
                    QTableWidgetItem *currentItem = itemAt(currentPos);
                    isStillOnSameItem = (currentItem == m_pressedItem);
                }

                if (isStillOnSameItem) {
                    // 短按点击：处理凸起效果
                    qDebug() << "短按点击 - 行:" << row << "列:" << col << "通道:" << channelNum;
                    handleTouchClick(row, col);
                } else {
                    // 触摸结束位置不在原单元格内，不处理短按
                    qDebug() << "触摸结束位置不在原单元格内 - 行:" << row << "列:" << col << "通道:" << channelNum;
                }
            }
        }

        m_pressedItem = nullptr;
        m_isLongPressTriggered = false;
        event->accept();
        break;
    }

    default:
        break;
    }
}
#else
// 鼠标按下事件
void MatrixWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_touchStartPos = event->pos();
        m_pressedItem = itemAt(m_touchStartPos);
        if (m_pressedItem) {
            m_selectedChannel = m_pressedItem->data(Qt::UserRole).toInt();
            m_longPressTimer->start(800); // 800ms长按阈值
            m_isLongPressTriggered = false;
            qDebug() << "鼠标按下 - 行:" << row(m_pressedItem) << "列:" << column(m_pressedItem) << "通道:" << m_selectedChannel;
        }
    }
    QTableWidget::mousePressEvent(event);
}

// 鼠标移动事件
void MatrixWidget::mouseMoveEvent(QMouseEvent *event)
{
    // 移除移动距离检查，只要长按时间超过800ms即可触发开关操作
    // 即使鼠标在长按过程中稍微移动也不取消长按检测
    QTableWidget::mouseMoveEvent(event);
}

// 鼠标释放事件
void MatrixWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_longPressTimer->stop();

        if (m_pressedItem && !m_isLongPressTriggered) {
            int row = m_pressedItem->row();
            int col = m_pressedItem->column();
            // 检查释放位置是否仍在同一单元格内
            QPoint releasePos = event->pos();
            QTableWidgetItem *releaseItem = itemAt(releasePos);
            if (releaseItem == m_pressedItem) {
                // 短按点击：处理凸起效果
                handleTouchClick(row, col);
            }
        }

        m_pressedItem = nullptr;
        m_isLongPressTriggered = false;
        event->accept();
        return;
    }
    QTableWidget::mouseReleaseEvent(event);
}
#endif

void MatrixWidget::onLongPressTimeout() {
    if (m_pressedItem) {
        int row = m_pressedItem->row();
        int col = m_pressedItem->column();
        int channelNum = m_pressedItem->data(Qt::UserRole).toInt();
        qDebug() << "长按超时触发 - 行:" << row << "列:" << col << "通道:" << channelNum;

        m_isLongPressTriggered = true;

        // 长按处理：切换颜色
        handleTouchLongPress(row, col);
    }
}

int MatrixWidget::getSelectedChannel()
{
    return m_selectedChannel;
}

int MatrixWidget::getPressedChannel()
{
    return m_pressedChannel;
}

// 获取开关状态
MatrixWidget::SwitchState MatrixWidget::getSwitchState(int channelNum) const
{
    if (m_switchStates.contains(channelNum)) {
        return m_switchStates.value(channelNum);
    }
    return OFF; // 默认返回关闭状态
}

// 设置开关状态
void MatrixWidget::setSwitchState(int channelNum, SwitchState state)
{
    m_switchStates[channelNum] = state;
}

// 保持原来的changeCellColor函数作为兼容（可选）
void MatrixWidget::changeCellColor(int row, int column)
{
    // 这个函数现在通过触摸事件处理，保留作为兼容
    handleTouchClick(row, column);
}
