#ifndef PAGEWIDGET_H
#define PAGEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QProgressBar>
#include <QSlider>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QListWidget>
#include <QTableWidget>
#include <QGroupBox>
#include <QSpinBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QDial>
#include <QLCDNumber>

// 页面类型枚举
enum PageType {
    PAGE_CHANNEL_SELECT,        // 通道选择
    PAGE_SIMU_LIST,             // 模拟列表
    PAGE_STATUS_VIEW,           // 状态展示
    PAGE_MODEL_SELECT,          // 模板选择
    PAGE_PARA_SETTING,          // 参数设置
    PAGE_MULTI_PATH             // 多径设置
};

class PageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PageWidget(PageType type, const QString &title, const QColor &color, QWidget *parent = nullptr);

    void setTitle(const QString &title);
    QString title() const;

    void setColor(const QColor &color);
    QColor color() const;

    PageType pageType() const;

signals:
    void buttonClicked();
    void valueChanged(int value);
    void textChanged(const QString &text);
    void selectionChanged(int index);

private slots:
    void onButtonClicked();
    void onSliderValueChanged(int value);
    void onTextChanged(const QString &text);
    void onComboBoxChanged(int index);

private:
    void setupSimuListUI();
    void setupChannelSelectUI();
    void setupStatusViewUI();
    void setupModelSelectUI();
    void setupParaSettingUI();
    void setupMultiPathUI();

    PageType m_type;
    QLabel *m_titleLabel;
    QColor m_color;

    // 通用控件
    QPushButton *m_importButton;
    QPushButton *m_exportButton;
    QPushButton *m_selectAllButton;
    QProgressBar *m_progressBar;
    QSlider *m_slider;

    // 特定页面的控件
    QLineEdit *m_lineEdit;
    QTextEdit *m_textEdit;
    QComboBox *m_comboBox;
    QCheckBox *m_checkBox;
    QRadioButton *m_radioButton;
    QListWidget *m_listWidget;
    QTableWidget *m_tableWidget;
    QGroupBox *m_groupBox;
    QSpinBox *m_spinBox;
    QDateEdit *m_dateEdit;
    QTimeEdit *m_timeEdit;
    QDial *m_dial;
    QLCDNumber *m_lcdNumber;
};

#endif // PAGEWIDGET_H
