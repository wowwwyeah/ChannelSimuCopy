#include <QDebug>
#include <QTimer>
#include <QLabel>
#include <QVBoxLayout>
#include <QMessageBox>
#include "datamanager.h"
#include "matrixwidget.h"

MatrixWidget::MatrixWidget(QWidget *parent)
    : QTableWidget(4, 6, parent)
    , m_longPressTimer(new QTimer(this))
{
    m_highlightedRow = -1;
    m_highlightedCol = -1;
    m_selectedChannel = 0;
    m_pressedChannel = -1;
    m_pressedItem = nullptr;
    m_isLongPressTriggered = false;

    // 初始化表头
    initHeaders();

    // 初始化单元格
    initCells();

    // 设置长按计时器
    m_longPressTimer->setSingleShot(true);
    connect(m_longPressTimer, &QTimer::timeout, this, &MatrixWidget::onLongPressTimeout);

    // 移除原来的连接，使用触摸事件替代
    // connect(this, &QTableWidget::cellClicked, this, &MatrixWidget::changeCellColor);

    // 启用触摸事件
    setAttribute(Qt::WA_AcceptTouchEvents);
}

MatrixWidget::~MatrixWidget()
{
}

void MatrixWidget::initHeaders()
{
    // 设置水平表头（列标题）
    setHorizontalHeaderLabels({"电台1", "电台2", "电台3", "电台4", "频谱仪", "干扰器"});

    // 设置垂直表头（行标题）
    setVerticalHeaderLabels({"电台1", "电台2", "电台3", "电台4"});

    // 设置表头样式
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    horizontalHeader()->setStyleSheet(R"(
        QHeaderView::section {
            font-weight: bold;
            background-color: #4CAF50;
            font-size: 14px; /* 统一字体大小（按需调整：14-18px） */
            font-family: 微软雅黑; /* 可选：统一字体类型，适配嵌入式板 */
            color: #336666; /* 可选：添加文字颜色，避免背景色覆盖 */
        }
    )");

    // 2. 垂直表头样式（与水平表头字体大小一致）
    verticalHeader()->setStyleSheet(R"(
        QHeaderView::section {
            font-weight: bold;
            background-color: #2196F3;
            font-size: 14px; /* 与水平表头字体大小完全一致 */
            font-family: 微软雅黑;
            color: #336666; /* 可选：文字白色更醒目 */
        }
    )");

    this->setStyleSheet(R"(
        /* 表格单元格默认样式 */
        QTableWidget {
            font-size: 14px;
            font-family: 微软雅黑;
            color: #333;
            background-color: #336666; /* 表格整体背景，与夹角色一致 */
        }
        /* 左上角夹角区域（核心：QTableCornerButton） */
        QTableWidget QTableCornerButton::section {
            background-color: #336666; /* 与背景色统一 */
            border: none; /* 去掉默认边框，避免缝隙 */
        }
        /* 选中单元格样式（保持原有效果） */
        QTableWidget::item:selected {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                        stop: 0 #f0f0f0, stop: 1 #e0e0e0);
            border: 2px outset #909090;
            border-radius: 3px;
            font-weight: bold;
            font-size: 14px;
            color: #333;
        }
    )");
}

void MatrixWidget::initCells()
{
    int channelNum = 1;
    for (int row = 0; row < rowCount(); ++row) {
        for (int col = 0; col < columnCount(); ++col) {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);
            item->setBackground(Qt::red);

            if (isUpperTriangle(row, col)) {
                item->setBackground(Qt::red); // 上三角区域-红色
                item->setText(QString("%1").arg(channelNum));
                channelNum++;
            } else {
                item->setBackground(QColor("#336666")); // 下三角区域-背景色
            }
            item->setFlags(item->flags() & Qt::ItemIsEnabled); // 禁用点击
            setItem(row, col, item);
        }
    }
}

bool MatrixWidget::isUpperTriangle(int row, int col) const
{
    // 判断是否为上三角区域(行索引小于列索引)
    return row < col;
}

// 短按触摸：创建/消除凸起效果
void MatrixWidget::handleTouchClick(int row, int column)
{
    QTableWidgetItem *item = this->item(row, column);
    if (isUpperTriangle(row, column) && item)
    {
        if(item->background().color() == Qt::red)
        {
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
                m_selectedChannel = item->text().toInt();
                qDebug() << "创建凸起效果 - 行:" << row << "列:" << column << "通道:" << m_selectedChannel;
            }
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
        if (item->background().color() == Qt::red) {
            item->setBackground(Qt::green);
            qDebug() << "长按变色 - 红→绿 行:" << row << "列:" << column;
        } else if (item->background().color() == Qt::green) {
            item->setBackground(Qt::red);
            qDebug() << "长按变色 - 绿→红 行:" << row << "列:" << column;
        }

        // 如果变色后不是红色且该单元格有凸起效果，移除凸起效果
        if (item->background().color() != Qt::red &&
            row == m_highlightedRow && column == m_highlightedCol) {
            removeCellWidget(row, column);
            m_highlightedRow = -1;
            m_highlightedCol = -1;
            m_selectedChannel = 0;
            qDebug() << "颜色改变，自动移除凸起效果";
        }

        m_pressedChannel = item->text().toInt();
    }
}

// 创建凸起效果的widget
void MatrixWidget::createRaisedCellWidget(int row, int column, QTableWidgetItem *item)
{
    // 创建凸起效果的widget
    QWidget *cellWidget = new QWidget();
    cellWidget->setStyleSheet(
        "QWidget {"
        "   border: 4px outset #a0a0a0;"
        "   background-color: red;"
        "   border-radius: 6px;"
        "}"
        );

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
            m_touchStartPos = touchPoint.pos().toPoint();
            m_pressedItem = itemAt(m_touchStartPos);

            if (m_pressedItem) {
                m_longPressTimer->start(800); // 800ms长按阈值
                m_isLongPressTriggered = false;
                qDebug() << "触摸开始 - 位置:" << m_touchStartPos;
            }
        }
        event->accept();
        break;
    }

    case QEvent::TouchUpdate: {
        if (!m_isLongPressTriggered && !event->touchPoints().isEmpty()) {
            const QTouchEvent::TouchPoint &touchPoint = event->touchPoints().first();
            QPoint currentPos = touchPoint.pos().toPoint();

            // 检查移动距离，超过20像素取消长按
            if ((currentPos - m_touchStartPos).manhattanLength() > 20) {
                m_longPressTimer->stop();
                m_pressedItem = nullptr;
                qDebug() << "移动距离过大，取消长按检测";
            }
        }
        event->accept();
        break;
    }

    case QEvent::TouchEnd:
    case QEvent::TouchCancel: {
        m_longPressTimer->stop();

        if (m_pressedItem) {
            int row = m_pressedItem->row();
            int col = m_pressedItem->column();

            if (m_isLongPressTriggered) {
                // 长按操作已完成（在onLongPressTimeout中处理）
                qDebug() << "长按操作完成 - 行:" << row << "列:" << col;
            } else {
                // 短按点击：处理凸起效果
                qDebug() << "短按点击 - 行:" << row << "列:" << col;
                handleTouchClick(row, col);
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

void MatrixWidget::onLongPressTimeout() {
    if (m_pressedItem) {
        int row = m_pressedItem->row();
        int col = m_pressedItem->column();
        qDebug() << "长按超时触发 - 行:" << row << "列:" << col;

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

// 保持原来的changeCellColor函数作为兼容（可选）
void MatrixWidget::changeCellColor(int row, int column)
{
    // 这个函数现在通过触摸事件处理，保留作为兼容
    handleTouchClick(row, column);
}
