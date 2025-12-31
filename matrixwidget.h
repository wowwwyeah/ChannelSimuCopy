#ifndef MATRIXWIDGET_H
#define MATRIXWIDGET_H

#include <QTableWidget>
#include <QTimer>
#include <QTouchEvent>
#include <QMouseEvent>
#include <QHeaderView>

class MatrixWidget : public QTableWidget
{
    Q_OBJECT

public:
    explicit MatrixWidget(QWidget *parent = nullptr);
    ~MatrixWidget();

    int getSelectedChannel();
    int getPressedChannel();
    bool isUpperTriangle(int row, int col) const;

    // 开关颜色常量
    static const QColor SWITCH_COLOR_ON;
    static const QColor SWITCH_COLOR_OFF;

    // 开关状态枚举
    enum SwitchState {
        OFF,
        ON
    };

    // 获取开关状态
    SwitchState getSwitchState(int channelNum) const;

    // 设置开关状态
    void setSwitchState(int channelNum, SwitchState state);

public slots:
    void changeCellColor(int row, int column);

protected:
    void initCells();
    void handleTouchClick(int row, int column);
    void handleTouchLongPress(int row, int column);

#ifdef  USE_TOUCH_EVENT
    bool event(QEvent *event) override;
    void handleTouchEvent(QTouchEvent *event);
#else
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
#endif

private slots:
    void onLongPressTimeout();
signals:
    // 通道开关状态变化信号
    void channelSwitchChanged(int channelNum, bool switchFlag);
private:
    void initHeaders();
    void createRaisedCellWidget(int row, int column, QTableWidgetItem *item);
    void removeCellWidget(int row, int column);
    //下发信道开关至硬件
    void setChlSwitchToHw(int chl,bool flag);

    QTimer *m_longPressTimer;

    // 凸起效果相关
    int m_highlightedRow;
    int m_highlightedCol;
    int m_selectedChannel;

    // 触摸事件相关
    QTableWidgetItem *m_pressedItem;
    QPoint m_touchStartPos;
    bool m_isLongPressTriggered;
    int m_pressedChannel;

    // 开关状态映射
    QMap<int, SwitchState> m_switchStates;
    //QMap<int,GR_OUT_E> m_
};

#endif // MATRIXWIDGET_H
