#include "simulistview.h"
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QVBoxLayout>

SimuListView::SimuListView(QWidget *parent)
    : QWidget{parent}
{
    initUI();
}

SimuListView::~SimuListView()
{
}

void SimuListView::initUI()
{
    // 设置背景色
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor("#336666"));
    setPalette(pal);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel("模拟列表");
    title->setAlignment(Qt::AlignmentFlag::AlignCenter);
    title->setStyleSheet("color: #2196F3; font-family: 微软雅黑; font-size: 18px; font-weight: bold; margin: 8px;");
    layout->addWidget(title);

    // 创建水平布局来包含箭头和表格容器
    QHBoxLayout *mainHorizontalLayout = new QHBoxLayout();

    // 左侧箭头（静态提示）
    QLabel *leftArrow = new QLabel("◀");
    leftArrow->setAlignment(Qt::AlignCenter);
    leftArrow->setStyleSheet(
        "QLabel {"
        "   color: rgba(255, 255, 255, 80);"  // 半透明白色
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   background-color: rgba(255, 255, 255, 20);"
        "   border-radius: 15px;"
        "   padding: 5px;"
        "   margin: 10px;"
        "}"
        );
    leftArrow->setFixedSize(80, 80);
    leftArrow->setCursor(Qt::ArrowCursor);  // 普通箭头光标，不可点击
    mainHorizontalLayout->addWidget(leftArrow);

    // 中央内容区域
    QVBoxLayout *contentLayout = new QVBoxLayout();

    QWidget *tableContainer = new QWidget(this);
    tableContainer->setObjectName("tableContainer");
    QHBoxLayout *tableContainerLayout = new QHBoxLayout(tableContainer);
    tableContainerLayout->setContentsMargins(6, 0, 6, 0); // 表格左右边距
    tableContainerLayout->setSpacing(0);

    m_tableView = new QTableWidget();
    m_tableView->setColumnCount(5);
    model = new QStandardItemModel(0, 5, this);
    m_tableView->setHorizontalHeaderLabels(QStringList() << "列表序号" << "模拟名称" << "噪声功率（dbm）" << "衰减功率 (dB)" << "多径数量");
    // 设置表格属性
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    m_tableView->setStyleSheet(R"(
        QTableView {
            background-color: #336666;
        }
)");

    QWidget *buttonContainer = new QWidget(this);
    buttonContainer->setObjectName("tableContainer");
    QHBoxLayout *buttonContainerLayout = new QHBoxLayout(buttonContainer);
    buttonContainerLayout->setContentsMargins(6, 0, 6, 0); // 表格左右边距
    buttonContainerLayout->setSpacing(0);
    // 功能按钮
    m_deleteButton = new QPushButton("删除", this);
    m_importButton = new QPushButton("导入", this);
    m_exportButton = new QPushButton("导出", this);
    m_selectAllButton = new QPushButton("全选", this);

    tableContainerLayout->addWidget(m_tableView);
    buttonContainerLayout->addWidget(m_deleteButton);
    buttonContainerLayout->addWidget(m_importButton);
    buttonContainerLayout->addWidget(m_exportButton);
    buttonContainerLayout->addWidget(m_selectAllButton);

    contentLayout->addWidget(tableContainer, 1); // 表格占据主要空间
    contentLayout->addWidget(buttonContainer);

    mainHorizontalLayout->addLayout(contentLayout);

    // 右侧箭头（静态提示）
    QLabel *rightArrow = new QLabel("▶");
    rightArrow->setAlignment(Qt::AlignCenter);
    rightArrow->setStyleSheet(
        "QLabel {"
        "   color: rgba(255, 255, 255, 80);"  // 半透明白色
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   background-color: rgba(255, 255, 255, 20);"
        "   border-radius: 15px;"
        "   padding: 5px;"
        "   margin: 10px;"
        "}"
        );
    rightArrow->setFixedSize(80, 80);
    rightArrow->setCursor(Qt::ArrowCursor);  // 普通箭头光标，不可点击
    mainHorizontalLayout->addWidget(rightArrow);

    layout->addLayout(mainHorizontalLayout);

    // 设置布局拉伸因子
    layout->setStretchFactor(title, 0);              // 标题不拉伸
    layout->setStretchFactor(mainHorizontalLayout, 1); // 主要内容区域拉伸

    layout->addStretch();
    setLayout(layout);

}

void SimuListView::setupConnections()
{
    connect(m_deleteButton, &QPushButton::clicked, this, &SimuListView::onDeleteClicked);
    connect(m_importButton, &QPushButton::clicked, this, &SimuListView::onImportClicked);
    connect(m_exportButton, &QPushButton::clicked, this, &SimuListView::onExportClicked);
    connect(m_selectAllButton, &QPushButton::clicked, this, &SimuListView::onSelectAllClicked);

    // 当模型数据变化时更新序号
    connect(model, &QStandardItemModel::rowsRemoved, this, &SimuListView::updateRowNumbers);
    connect(model, &QStandardItemModel::rowsInserted, this, &SimuListView::updateRowNumbers);
}

void SimuListView::updateRowNumbers()
{
    for (int row = 0; row < model->rowCount(); ++row) {
        QStandardItem *item = model->item(row, 0);
        if (!item) {
            item = new QStandardItem();
            model->setItem(row, 0, item);
        }
        item->setText(QString::number(row + 1));
    }
}

void SimuListView::onDeleteClicked()
{
    QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择要删除的行!");
        return;
    }

    // 从后往前删除，避免索引变化导致的问题
    QList<int> rows;
    foreach (const QModelIndex &index, selected) {
        rows.append(index.row());
    }

    std::sort(rows.begin(), rows.end(), std::greater<int>());

    foreach (int row, rows) {
        model->removeRow(row);
    }
}

void SimuListView::onImportClicked()
{

}

void SimuListView::onExportClicked()
{
    if (model->rowCount() == 0) {
        QMessageBox::warning(this, "警告", "列表为空，无需导出!");
        return;
    }

    QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择要导出的行!");
        return;
    }




}

void SimuListView::onSelectAllClicked()
{
    m_tableView->selectAll();
}

