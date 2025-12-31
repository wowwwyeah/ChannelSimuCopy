#include "iohandler.h"
#include "systemsetting.h"
#include "simulistview.h"
#include <QLabel>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QVBoxLayout>

SimuListView::SimuListView(QWidget *parent)
    : QWidget{parent}
{
    initUI();
    setupConnections();
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
    title->setStyleSheet("color: #2196F3; font-family: 微软雅黑; font-size: 22px; font-weight: bold; margin: 12px;");
    layout->addWidget(title);

    // 中央内容区域
    QVBoxLayout *contentLayout = new QVBoxLayout();

    QWidget *tableContainer = new QWidget(this);
    tableContainer->setObjectName("tableContainer");
    QHBoxLayout *tableContainerLayout = new QHBoxLayout(tableContainer);
    tableContainerLayout->setContentsMargins(6, 0, 6, 0); // 表格左右边距
    tableContainerLayout->setSpacing(0);

    // ========== 核心修改1：初始化model并绑定到m_tableView ==========
    m_tableView = new QTableView(); // 改用QTableView（推荐），若坚持用QTableWidget需调整接口
    model = new QStandardItemModel(0, 6, this);
    model->setHorizontalHeaderLabels(QStringList() << "列表序号" << "信道编号" << "模拟名称" << "噪声功率（dbm）" << "衰减功率 (dB)" << "多径数量");
    m_tableView->setModel(model); // 绑定model到表格

    // 设置表格属性（适配QTableView）
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection); // 支持多行选中（导出需要）
    m_tableView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    m_tableView->setStyleSheet(R"(
        /* 表格内容字体 */
        QTableView {
            background-color: #336666;
            color: white; /* 文字颜色，避免深色背景看不见 */
            font-size: 14px; /* 表格内容字体大小（默认12px，按需调整） */
            font-family: 微软雅黑; /* 可选：指定字体，提升显示效果 */
        }
        /* 表头字体（单独设置，可更大） */
        QHeaderView::section {
            background-color: #2C5555;
            color: white;
            font-size: 16px; /* 表头字体大小（比内容大2px更醒目） */
            font-weight: bold; /* 表头加粗 */
            border: none; /* 去掉表头边框，保持原有样式 */
        }
        /* 选中行样式（可选，增强视觉） */
        QTableView::item:selected {
            background-color: #4CAF50;
            color: white;
        }
    )");


    QWidget *buttonContainer = new QWidget(this);
    buttonContainer->setObjectName("tableContainer");
    QHBoxLayout *buttonContainerLayout = new QHBoxLayout(buttonContainer);
    buttonContainerLayout->setContentsMargins(6, 0, 6, 0);
    buttonContainerLayout->setSpacing(0);
    // 功能按钮
    m_deleteButton = new QPushButton("删除", this);
    m_importButton = new QPushButton("导入", this);
    m_exportButton = new QPushButton("导出", this);
    m_selectAllButton = new QPushButton("全选", this);

    // 设置按钮样式和大小
    QList<QPushButton*> buttons = {m_importButton, m_exportButton, m_selectAllButton}; // 排除删除按钮
    foreach (QPushButton* btn, buttons) {
        btn->setStyleSheet("QPushButton {" \
                           "    background-color: #F5F5F5;" \
                           "    color: #333333;" \
                           "    font-size: 12px;" \
                           "    border-radius: 10px;" \
                           "    padding: 4px 6px;" \
                           "    border: 1px solid #CCCCCC;" \
                           "}" \
                           "QPushButton:hover {" \
                           "    background-color: #E0E0E0;" \
                           "}" \
                           "QPushButton:disabled {" \
                           "    background-color: #225555;" \
                           "    color: 88AAAA;" \
                           "}");
        // 设置按钮高度固定为50，宽度自适应
        btn->setFixedHeight(50);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    // 设置删除按钮样式为红色
    m_deleteButton->setStyleSheet("QPushButton {" \
                                  "    background-color: #8B2323;" \
                                  "    color: white;" \
                                  "    font-size: 12px;" \
                                  "    border-radius: 10px;" \
                                  "    padding: 4px 6px;" \
                                  "}" \
                                  "QPushButton:hover {" \
                                  "    background-color: #B22222;" \
                                  "}" \
                                  "QPushButton:disabled {" \
                                  "    background-color: #225555;" \
                                  "    color: 88AAAA;" \
                                  "}");
    // 设置删除按钮的大小策略
    m_deleteButton->setFixedHeight(50);
    m_deleteButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);


    tableContainerLayout->addWidget(m_tableView);
    // 设置按钮间距为6
    buttonContainerLayout->setSpacing(6);
    // 调整按钮顺序：全选在左，删除在右
    buttonContainerLayout->addWidget(m_selectAllButton);
    buttonContainerLayout->addWidget(m_importButton);
    buttonContainerLayout->addWidget(m_exportButton);
    buttonContainerLayout->addWidget(m_deleteButton);

    contentLayout->addWidget(tableContainer, 1);
    contentLayout->addWidget(buttonContainer);

    layout->addLayout(contentLayout);

    // 设置布局拉伸因子
    layout->setStretchFactor(title, 0);
    layout->setStretchFactor(contentLayout, 1);

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


// 实现插入数据的函数
void SimuListView::insertScenarioData(const ModelParaSetting &scenarioData)
{
    // 1. 在表格末尾添加一行
    int newRow = model->rowCount();
    model->insertRow(newRow);

    // 2. 创建 QStandardItem  并填充数据
    // 列表序号 (通常是行号 + 1)
    QStandardItem *indexItem = new QStandardItem(QString::number(newRow + 1));
    indexItem->setFlags(indexItem->flags() & ~Qt::ItemIsEditable); // 序号不可编辑
    indexItem->setTextAlignment(Qt::AlignCenter); // 居中显示

    // 信道编号
    QStandardItem *numItem = new QStandardItem(QString::number(scenarioData.channelNum));
    numItem->setTextAlignment(Qt::AlignCenter); // 居中显示

    // 模拟名称
    QStandardItem *nameItem = new QStandardItem(scenarioData.modelName);
    nameItem->setTextAlignment(Qt::AlignCenter); // 居中显示

    // 噪声功率
    QStandardItem *noiseItem = new QStandardItem(QString::number(scenarioData.noisePower, 'f', 2)); // 保留2位小数
    noiseItem->setTextAlignment(Qt::AlignCenter); // 居中显示

    // 衰减功率 (假设 signalAnt 字段对应衰减功率)
    QStandardItem *attenuationItem = new QStandardItem(QString::number(scenarioData.signalAnt, 'f', 2));
    attenuationItem->setTextAlignment(Qt::AlignCenter); // 居中显示

    // 多径数量
    QStandardItem *multiPathNumItem = new QStandardItem(QString::number(scenarioData.multipathNum));
    multiPathNumItem->setTextAlignment(Qt::AlignCenter); // 居中显示

    // 3. 将创建的 Item 设置到表格中
    model->setItem(newRow, 0, indexItem);
    model->setItem(newRow, 1, numItem);
    model->setItem(newRow, 2, nameItem);
    model->setItem(newRow, 3, noiseItem);
    model->setItem(newRow, 4, attenuationItem);
    model->setItem(newRow, 5, multiPathNumItem);

    m_dataList.append(scenarioData);
    qDebug() << "成功插入场景数据: " << scenarioData.channelNum;
}

void SimuListView::updateRowNumbers()
{
    // 遍历model中的所有行，更新序号列
    for (int row = 0; row < model->rowCount(); ++row) {
        QStandardItem *item = model->item(row, 0);
        if (!item) {
            item = new QStandardItem();
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            item->setTextAlignment(Qt::AlignCenter);
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

    // 从后往前删除，避免索引错乱
    QList<int> rows;
    foreach (const QModelIndex &index, selected) {
        rows.append(index.row());
    }
    std::sort(rows.begin(), rows.end(), std::greater<int>());

    // 删除model中的行 + 同步删除m_dataList中的数据
    foreach (int row, rows) {
        model->removeRow(row);
        if (row < m_dataList.size()) {
            m_dataList.removeAt(row); // 同步删除数据列表，避免导出时数据错位
        }
    }

    // 删除后自动更新序号
    updateRowNumbers();
}

void SimuListView::onImportClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "导入配置", "", "配置文件 (*.csv *.json *.xml)");

    // 用户取消选择文件
    if (fileName.isEmpty()) {
        qDebug() << "用户取消导入文件";
        return;
    }

    IOHandler ioHandler;

    ModelParaSetting importedConfig = ioHandler.importDataAutoDetect(fileName);

    insertScenarioData(importedConfig);

    // 5. 提示用户导入成功
    QMessageBox::information(this, tr("导入成功"),
                             tr("配置文件导入成功！\n场景名称：%1\n信道编号：%2").arg(
                                 importedConfig.modelName,
                                 QString::number(importedConfig.channelNum)
                                 )
                             );

    qDebug() << "配置导入成功，场景名称：" << importedConfig.modelName;

}

void SimuListView::onExportClicked()
{
    if (model->rowCount() == 0) {
        QMessageBox::warning(this, "警告", "列表为空，无需导出!");
        return;
    }

    QModelIndexList selectedIndexes = m_tableView->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "导出失败", "请先选中表格中的一行/多行数据！");
        return;
    }

    QList<ModelParaSetting> selectedDataList;
    for (const QModelIndex &index : selectedIndexes) {
        int row = index.row();
        if (row >= 0 && row < m_dataList.size()) {
            selectedDataList.append(m_dataList.at(row));
        }
    }
    if (selectedDataList.isEmpty()) {
        QMessageBox::warning(this, "导出失败", "选中行无有效数据！");
        return;
    }

    // 3. 从SettingManager获取默认配置
    SystemSetting *sysSetting = new SystemSetting();
    QString defaultPath = sysSetting->getExportPath();
    QString defaultFormat = sysSetting->getExportFormat();

    QString confirmMsg = QString(
                             "即将导出 %1 个独立文件，确认以下信息：\n"
                             "📌 默认保存路径：%2\n"
                             "📌 文件命名规则：场景名称_行号.%3\n"
                             "📌 示例：城市场景_1.json、郊区场景_2.json\n\n"
                             "是否继续导出？"
                             ).arg(selectedDataList.size()).arg(defaultPath).arg(defaultFormat);

    // 弹出确认框（仅确认/取消按钮）
    int confirm = QMessageBox::question(
        this,
        "确认导出文件",
        confirmMsg,
        QMessageBox::Yes | QMessageBox::No, // 按钮：是/否
        QMessageBox::No // 默认焦点在“否”，避免误点
        );

    // 用户取消导出
    if (confirm != QMessageBox::Yes) {
        qDebug() << "用户取消多文件导出";
        QMessageBox::information(this, "导出取消", "已取消多文件导出操作");
        return;
    }

    // 6. 用户确认后，选择导出目录（默认路径从SystemSetting读取）
    QString dirPath = QFileDialog::getExistingDirectory(
        this,
        QString("选择多文件导出目录（共%1个文件）").arg(selectedDataList.size()),
        defaultPath
        );
    if (dirPath.isEmpty()) {
        QMessageBox::information(this, "导出取消", "未选择导出目录，操作终止");
        return;
    }

    // 8. 执行多文件循环导出
    bool exportOk = exportToMultiFiles(selectedDataList, dirPath, defaultFormat);

    // 9. 导出结果提示
    if (exportOk) {
        QMessageBox::information(
            this,
            "导出成功",
            QString("✅ 多文件导出完成！\n共导出 %1 个文件到：\n%2")
                .arg(selectedDataList.size()).arg(dirPath)
            );
    } else {
        QMessageBox::critical(
            this,
            "导出失败",
            QString("❌ 部分文件导出失败！\n请检查目录权限：\n%1")
                .arg(dirPath)
            );
    }
}

void SimuListView::onSelectAllClicked()
{
    m_tableView->selectAll();
}

// 辅助函数：模式2 - 多行导出为独立文件（循环生成每个文件）
bool SimuListView::exportToMultiFiles(const QList<ModelParaSetting> &dataList,
                                      const QString &dirPath, const QString &format)
{
    bool allOk = true;
    QDir dir(dirPath);
    IOHandler ioHandler;

    for (int i = 0; i < dataList.size(); ++i) { // 循环遍历每行数据
        const ModelParaSetting &data = dataList.at(i);
        // 文件名：场景名称_行号.json（避免重复）
        QString fileName = QString("%1_%2.%3").arg(data.modelName).arg(i + 1).arg(format);
        QString filePath = dir.filePath(fileName);

        // 导出单个文件（复用原有单文件导出函数）
        bool ok = ioHandler.exportData(data, filePath, format);
        if (!ok) {
            allOk = false;
            qWarning() << "导出失败：" << filePath;
        }
    }
    return allOk;
}

