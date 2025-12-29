#include "pagewidget.h"
#include "matrixwidget.h"
#include "channelselect.h"
#include <QPainter>
#include <QHeaderView>
#include <QDateTime>
#include <QRandomGenerator>

PageWidget::PageWidget(PageType type, const QString &title, const QColor &color, QWidget *parent)
    : QWidget(parent), m_type(type), m_color(color)
{
    // 设置背景色
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, m_color);
    setPalette(pal);

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setMargin(0);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: white;");

    // 根据页面类型设置不同的UI
    switch (m_type) {
    case PAGE_CHANNEL_SELECT:
        setupChannelSelectUI();
        break;
    case PAGE_SIMU_LIST:
        setupSimuListUI();
        break;
    case PAGE_STATUS_VIEW:
        setupStatusViewUI();
        break;
    case PAGE_MODEL_SELECT:
        setupModelSelectUI();
        break;
    case PAGE_PARA_SETTING:
        setupParaSettingUI();
        break;
    case PAGE_MULTI_PATH:
        setupMultiPathUI();
        break;
    }
}

void PageWidget::setupChannelSelectUI()
{
#if 0
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_titleLabel);

    MatrixWidget *matrixWidget = new MatrixWidget();
    mainLayout->addWidget(matrixWidget);
#endif
}

void PageWidget::setupSimuListUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_titleLabel);

    QWidget *tableContainer = new QWidget(this);
    tableContainer->setObjectName("tableContainer");
    QHBoxLayout *tableContainerLayout = new QHBoxLayout(tableContainer);
    tableContainerLayout->setContentsMargins(6, 0, 6, 0); // 表格左右边距
    tableContainerLayout->setSpacing(0);

    // 创建表格
    m_tableWidget = new QTableWidget(5, 3, this);
    m_tableWidget->setHorizontalHeaderLabels({"序号", "名称", "状态"});
    m_tableWidget->verticalHeader()->setVisible(false);
    m_tableWidget->setStyleSheet("QTableWidget { background-color: white; gridline-color: #ddd; }"
                                 "QHeaderView::section { background-color: #f0f0f0; padding: 4px; }");

    // 填充示例数据
    QStringList names = {"1", "2", "3", "4", "5"};
    for (int row = 0; row < 5; ++row) {
        m_tableWidget->setItem(row, 0, new QTableWidgetItem(names[row]));
        m_tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(20 + row * 5)));
        m_tableWidget->setItem(row, 2, new QTableWidgetItem(row % 2 == 0 ? "正常" : "警告"));
    }

    m_tableWidget->resizeColumnsToContents();
    //m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QWidget *buttonContainer = new QWidget(this);
    buttonContainer->setObjectName("tableContainer");
    QHBoxLayout *buttonContainerLayout = new QHBoxLayout(buttonContainer);
    buttonContainerLayout->setContentsMargins(6, 0, 6, 0); // 表格左右边距
    buttonContainerLayout->setSpacing(0);
    // 功能按钮
    m_importButton = new QPushButton("导入", this);
    m_exportButton = new QPushButton("导出", this);
    m_selectAllButton = new QPushButton("全选", this);
    //m_mainButton->setStyleSheet("QPushButton { background-color: white; color: black; border-radius: 8px; padding: 8px; margin: 10px; }"
    //                            "QPushButton:hover { background-color: #E8E8E8; }");

    tableContainerLayout->addWidget(m_tableWidget);
    buttonContainerLayout->addWidget(m_importButton);
    buttonContainerLayout->addWidget(m_exportButton);
    buttonContainerLayout->addWidget(m_selectAllButton);
    mainLayout->addWidget(tableContainer, 1); // 表格占据主要空间
    mainLayout->addWidget(buttonContainer);

    //connect(m_mainButton, &QPushButton::clicked, this, &PageWidget::onButtonClicked);
}

void PageWidget::setupStatusViewUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_titleLabel);
}

void PageWidget::setupModelSelectUI()
{

}
void PageWidget::setupParaSettingUI()
{

}

void PageWidget::setupMultiPathUI()
{

}

void PageWidget::setTitle(const QString &title)
{
    m_titleLabel->setText(title);
}

QString PageWidget::title() const
{
    return m_titleLabel->text();
}

void PageWidget::setColor(const QColor &color)
{
    m_color = color;
    QPalette pal = palette();
    pal.setColor(QPalette::Window, m_color);
    setPalette(pal);
}

QColor PageWidget::color() const
{
    return m_color;
}

PageType PageWidget::pageType() const
{
    return m_type;
}

void PageWidget::onButtonClicked()
{
    emit buttonClicked();
}

void PageWidget::onSliderValueChanged(int value)
{
    emit valueChanged(value);
}

void PageWidget::onTextChanged(const QString &text)
{
    emit textChanged(text);
}

void PageWidget::onComboBoxChanged(int index)
{
    emit selectionChanged(index);
}
