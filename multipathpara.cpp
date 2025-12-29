#include "multipathpara.h"
#include <QScrollBar>
#include <random>

MultiPathPara::MultiPathPara(QWidget *parent)
    : QWidget(parent)
    , m_multipathCount(3)
    , m_filterNum(3)
{
    setupUI();
    initConnections();
}

void MultiPathPara::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 设置样式表
    this->setStyleSheet(R"(
        MultiPathPara {
            background-color: #336666;
        }

        /* 主GroupBox样式 */
        QGroupBox {
            color: #CCEEEE;
            font-family: "Microsoft YaHei";
            font-size: 20px;  /* 16px -> 20px */
            font-weight: bold;
            border: 3px solid #559999;  /* 2px -> 3px */
            border-radius: 12px;  /* 8px -> 12px */
            margin-top: 2ex;  /* 1ex -> 2ex */
            padding-top: 4px;  /* 2px -> 4px */
            background-color: #225555;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top center;
            padding: 2px 12px;  /* 0 8px -> 2px 12px */
            background-color: #448888;
            border-radius: 6px;  /* 4px -> 6px */
            font-size: 18px;  /* 添加字体大小 */
        }

        /* 标签样式 */
        QLabel {
            color: #99CCCC;
            font-family: "Microsoft YaHei";
            font-size: 18px;  /* 16px -> 18px */
            font-weight: bold;
            margin: 4px 0px;  /* 添加边距 */
        }

        QLabel#valueLabel {
            color: #CCEEEE;
            font-size: 18px;  /* 16px -> 18px */
            font-weight: bold;
            background-color: #224444;
            border-radius: 6px;  /* 4px -> 6px */
            padding: 8px 12px;  /* 4px 8px -> 8px 12px */
            min-width: 80px;  /* 60px -> 80px */
            min-height: 24px;  /* 添加最小高度 */
            qproperty-alignment: AlignCenter;
        }

        /* 多径数量滚动条样式 - 紫色渐变 */
        QSlider#multipathCountSlider::groove:horizontal {
            border: 2px solid #9B6BCC;  /* 1px -> 2px */
            height: 12px;  /* 8px -> 12px */
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #6B4B8B, stop:0.5 #9B6BCC, stop:1 #CCAAFF);
            border-radius: 6px;  /* 4px -> 6px */
        }

        QSlider#multipathCountSlider::handle:horizontal {
            background: qradialgradient(cx:0.5, cy:0.5, radius:0.8,
                stop:0 #CCAAFF, stop:0.8 #9B6BCC, stop:1 #6B4B8B);
            border: 3px solid #9B6BCC;  /* 2px -> 3px */
            width: 24px;  /* 20px -> 24px */
            margin: -10px 0;  /* -8px 0 -> -10px 0 */
            border-radius: 12px;  /* 10px -> 12px */
        }

        QSlider#multipathCountSlider::handle:horizontal:hover {
            background: qradialgradient(cx:0.5, cy:0.5, radius:0.8,
                stop:0 #EECCFF, stop:0.8 #CCAAFF, stop:1 #9B6BCC);
            border: 3px solid #CCAAFF;  /* 2px -> 3px */
        }

        /* 表格样式 */
        QTableWidget {
            background-color: #224444;
            alternate-background-color: #2A5555;
            gridline-color: #559999;
            border: 2px solid #559999;  /* 1px -> 2px */
            border-radius: 6px;  /* 4px -> 6px */
            font-family: "Microsoft YaHei";
            font-size: 14px;  /* 11px -> 14px */
        }

        QTableWidget::item {
            color: #CCEEEE;
            padding: 8px;  /* 4px -> 8px */
            border: none;
            min-height: 24px;  /* 添加最小高度 */
        }

        QTableWidget::item:selected {
            background-color: #448888;
            color: #FFFFFF;
        }

        QTableWidget::header {
            background-color: #336666;
            color: #CCEEEE;
            font-weight: bold;
            border: 2px solid #559999;  /* 1px -> 2px */
        }

        QTableWidget::header:section {
            background-color: #448888;
            color: #CCEEEE;
            padding: 10px;  /* 6px -> 10px */
            border: none;
            font-weight: bold;
            font-size: 14px;  /* 添加字体大小 */
            min-height: 28px;  /* 添加最小高度 */
        }

        /* 数字输入框样式 */
        QSpinBox, QDoubleSpinBox {
            padding: 8px 12px;  /* 4px -> 8px 12px */
            border: 2px solid #559999;  /* 1px -> 2px */
            border-radius: 6px;  /* 3px -> 6px */
            background-color: #224444;
            color: #CCEEEE;
            font-family: "Microsoft YaHei";
            font-size: 14px;  /* 11px -> 14px */
            min-width: 80px;  /* 60px -> 80px */
            min-height: 36px;  /* 添加最小高度 */
        }

        QSpinBox::up-button, QSpinBox::down-button,
        QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {
            background-color: #448888;
            border: 2px solid #559999;  /* 1px -> 2px */
            border-radius: 3px;  /* 1px -> 3px */
            width: 20px;  /* 12px -> 20px */
            height: 15px;  /* 添加高度 */
        }

        QSpinBox::up-button:hover, QSpinBox::down-button:hover,
        QDoubleSpinBox::up-button:hover, QDoubleSpinBox::down-button:hover {
            background-color: #559999;
        }

        /* 按钮样式 */
        QPushButton {
            background-color: #448888;
            color: #CCEEEE;
            border: none;
            padding: 12px 24px;  /* 8px 16px -> 12px 24px */
            border-radius: 8px;  /* 6px -> 8px */
            font-family: "Microsoft YaHei";
            font-size: 16px;  /* 12px -> 16px */
            font-weight: bold;
            min-width: 100px;  /* 80px -> 100px */
            min-height: 44px;  /* 添加最小高度 */
        }

        QPushButton:hover {
            background-color: #559999;
        }

        QPushButton:pressed {
            background-color: #66AAAA;
        }

        QPushButton#generateButton {
            background-color: #9B6BCC;
        }

        QPushButton#generateButton:hover {
            background-color: #AC7CDD;
        }

        QPushButton#generateButton:pressed {
            background-color: #8A5BBB;
        }
    )");
    // 创建多径数量设置组
    createMultipathCountGroup();
    //mainLayout->addWidget(m_multipathCountGroup);

    // 创建路径参数组
    createPathParametersGroup();
    mainLayout->addWidget(m_pathParametersGroup);

    // 添加按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_generateRandomButton = new QPushButton("随机生成");
    m_generateRandomButton->setObjectName("generateButton");
    m_resetButton = new QPushButton("重置参数");
    m_applyButton = new QPushButton("应用设置");

    connect(m_generateRandomButton, &QPushButton::clicked, this, &MultiPathPara::onGenerateRandomClicked);
    connect(m_resetButton, &QPushButton::clicked, this, &MultiPathPara::onResetClicked);
    connect(m_applyButton, &QPushButton::clicked, this, &MultiPathPara::onApplyClicked);

    buttonLayout->addWidget(m_generateRandomButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_resetButton);
    buttonLayout->addWidget(m_applyButton);

    //mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    // 初始更新表格
    updatePathParametersTable();
}

void MultiPathPara::createMultipathCountGroup()
{
    m_multipathCountGroup = new QGroupBox("多径数量设置");

    QVBoxLayout *layout = new QVBoxLayout(m_multipathCountGroup);

    // 值显示标签
    QHBoxLayout *valueLayout = new QHBoxLayout();
    QLabel *currentLabel = new QLabel("多径数量:");
    m_multipathCountValueLabel = new QLabel(QString::number(m_multipathCount));
    m_multipathCountValueLabel->setObjectName("valueLabel");

    valueLayout->addWidget(currentLabel);
    valueLayout->addWidget(m_multipathCountValueLabel);
    valueLayout->addStretch();
    //layout->addLayout(valueLayout);

    // 滚动条
    QHBoxLayout *sliderLayout = new QHBoxLayout();
    QLabel *minLabel = new QLabel(QString::number(MULTIPATH_COUNT_MIN));
    QLabel *maxLabel = new QLabel(QString::number(MULTIPATH_COUNT_MAX));

    minLabel->setStyleSheet("color: #CCAAFF;");
    maxLabel->setStyleSheet("color: #CCAAFF;");

    m_multipathCountSlider = new QSlider(Qt::Horizontal);
    m_multipathCountSlider->setObjectName("multipathCountSlider");
    m_multipathCountSlider->setRange(MULTIPATH_COUNT_MIN, MULTIPATH_COUNT_MAX);
    m_multipathCountSlider->setValue(m_multipathCount);
    m_multipathCountSlider->setTickPosition(QSlider::TicksBelow);
    m_multipathCountSlider->setTickInterval(1);

    sliderLayout->addWidget(minLabel);
    sliderLayout->addWidget(m_multipathCountSlider);
    sliderLayout->addWidget(maxLabel);
    //layout->addLayout(sliderLayout);

    // 数字输入
    QHBoxLayout *spinBoxLayout = new QHBoxLayout();
    QLabel *inputLabel = new QLabel("精确设置:");
    inputLabel->setStyleSheet("color: #CCAAFF;");
    m_multipathCountSpinBox = new QSpinBox();
    m_multipathCountSpinBox->setRange(MULTIPATH_COUNT_MIN, MULTIPATH_COUNT_MAX);
    m_multipathCountSpinBox->setValue(m_multipathCount);

    //connect(m_multipathCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
    //        this, &MultiPathPara::onMultipathCountChanged);

    spinBoxLayout->addWidget(inputLabel);
    spinBoxLayout->addWidget(m_multipathCountSpinBox);
    spinBoxLayout->addStretch();
    layout->addLayout(spinBoxLayout);
}

void MultiPathPara::createPathParametersGroup()
{
    m_pathParametersGroup = new QGroupBox("多径参数设置");

    QVBoxLayout *layout = new QVBoxLayout(m_pathParametersGroup);

    // 水平布局：多径数量（标签+加减） + 滤波器编号（标签+加减）
    QHBoxLayout *ctrlLayout = new QHBoxLayout();

    // ========== 1. 多径数量：标签 + 减号 + 数值 + 加号 ==========
    QWidget *multipathWidget = new QWidget();
    QHBoxLayout *multipathLayout = new QHBoxLayout(multipathWidget);
    multipathLayout->setContentsMargins(0, 0, 0, 0); // 去掉内边距

    QLabel *inputLabel = new QLabel("多径数量:");
    inputLabel->setStyleSheet("color: #CCAAFF; font-size: 18px;");
    // 减号按钮
    m_multipathSubBtn = new QPushButton("-");
    m_multipathSubBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #2C5555;
            color: white;
            border: none;
            width: 24px;    /* 缩小按钮宽度 */
            height: 24px;   /* 缩小按钮高度 */
            font-size: 18px;/* 加减号字体同步缩小 */
            font-weight: bold;
            border-radius: 3px 0 0 3px; /* 圆角同步缩小 */
            padding: 0px;   /* 去掉按钮内边距 */
        }
        QPushButton:hover {
            background-color: #4CAF50;
        }
        QPushButton:disabled {
            background-color: #666666;
            color: #999999;
        }
    )");

    // 数值显示标签
    m_multipathValueLabel = new QLabel(QString::number(m_multipathCount));
    m_multipathValueLabel->setStyleSheet(R"(
        QLabel {
            color: #CCAAFF;
            font-size: 18px;
            font-weight: bold;
            background-color: #2C5555;
            padding: 5px 10px;
            border-radius: 4px;
        }
    )");
    m_multipathValueLabel->setAlignment(Qt::AlignCenter); // 数值居中

    // 加号按钮
    m_multipathAddBtn = new QPushButton("+");
    m_multipathAddBtn->setStyleSheet(m_multipathSubBtn->styleSheet());

    // 添加到多径数量布局
    multipathLayout->addWidget(inputLabel);
    multipathLayout->addWidget(m_multipathSubBtn);
    multipathLayout->addWidget(m_multipathValueLabel);
    multipathLayout->addWidget(m_multipathAddBtn);
    multipathLayout->addStretch(); // 右侧拉伸

    // ========== 2. 滤波器编号：标签 + 减号 + 数值 + 加号 ==========
    QWidget *filterWidget = new QWidget();
    QHBoxLayout *filterLayout = new QHBoxLayout(filterWidget);
    filterLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *filterLabel = new QLabel("滤波器编号:");
    filterLabel->setStyleSheet("color: #CCAAFF; font-size: 18px;");

    // 减号按钮
    m_filterSubBtn = new QPushButton("-");
    m_filterSubBtn->setStyleSheet(m_multipathSubBtn->styleSheet()); // 复用样式

    // 数值显示标签
    m_filterNumValueLabel = new QLabel(QString::number(m_filterNum));
    m_filterNumValueLabel->setStyleSheet(m_multipathValueLabel->styleSheet()); // 复用样式
    m_filterNumValueLabel->setAlignment(Qt::AlignCenter);

    // 加号按钮
    m_filterAddBtn = new QPushButton("+");
    m_filterAddBtn->setStyleSheet(m_multipathAddBtn->styleSheet()); // 复用样式

    // 添加到滤波器编号布局
    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(m_filterSubBtn);
    filterLayout->addWidget(m_filterNumValueLabel);
    filterLayout->addWidget(m_filterAddBtn);
    filterLayout->addStretch();

    // ========== 3. 布局整合 ==========
    ctrlLayout->addWidget(multipathWidget);
    ctrlLayout->addStretch(); // 两个控件之间拉伸
    ctrlLayout->addWidget(filterWidget);
    layout->addLayout(ctrlLayout);

    // 创建表格
    m_pathTable = new QTableWidget();
    m_pathTable->setColumnCount(5);
    m_pathTable->setHorizontalHeaderLabels(QStringList() << "路径索引" << "相对时延 (ns)" << "衰减功率 (dB)" << "路径频移 (Hz)" << "频扩 (Hz)");

    // 设置表格属性
    m_pathTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_pathTable->verticalHeader()->setVisible(false);
    m_pathTable->setAlternatingRowColors(true);
    m_pathTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pathTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pathTable->setEditTriggers(QAbstractItemView::AllEditTriggers);

    m_pathTable->setStyleSheet(R"(
        /* 表格内容字体 */
        QTableWidget {
            background-color: #336666; /* 与SimuListView保持配色一致 */
            color: white; /* 文字颜色，避免深色背景看不见 */
            font-size: 14px; /* 内容字体大小（按需调整：14-18px） */
            font-family: 微软雅黑; /* 可选：嵌入式板推荐 DejaVu Sans/文泉驿微米黑 */
        }
        /* 表头字体（单独设置，更大更醒目） */
        QHeaderView::section {
            background-color: #2C5555;
            color: white;
            font-size: 16px; /* 表头字体大小（比内容大2px） */
            font-weight: bold; /* 表头加粗 */
            border: none; /* 去掉边框，保持样式统一 */
        }
        /* 选中行样式（可选，增强视觉） */
        QTableWidget::item:selected {
            background-color: #4CAF50;
            color: white;
        }
        /* 编辑状态下的单元格字体（可选，保持一致） */
        QTableWidget::item:editable {
            font-size: 14px;
            color: white;
        }
    )");

    layout->addWidget(m_pathTable);
}

void MultiPathPara::initConnections()
{
    connect(m_multipathSubBtn, &QPushButton::clicked, this, [this]() {
        if (m_multipathCount > MULTIPATH_COUNT_MIN) {
            m_multipathCount--;
            m_multipathValueLabel->setText(QString::number(m_multipathCount));
            updatePathParametersTable();
        }
    });
    connect(m_multipathAddBtn, &QPushButton::clicked, this, [this]() {
        if (m_multipathCount < MULTIPATH_COUNT_MAX) {
            m_multipathCount++;
            m_multipathValueLabel->setText(QString::number(m_multipathCount));
            updatePathParametersTable();
        }
    });
    connect(m_filterSubBtn, &QPushButton::clicked, this, [this]() {
        if (m_filterNum > MULTIPATH_COUNT_MIN) {
            m_filterNum--;
            m_filterNumValueLabel->setText(QString::number(m_filterNum));
            updatePathParametersTable();
        }
    });
    connect(m_filterAddBtn, &QPushButton::clicked, this, [this]() {
        if (m_filterNum < MULTIPATH_COUNT_MAX) {
            m_filterNum++;
            m_filterNumValueLabel->setText(QString::number(m_filterNum));
            updatePathParametersTable();
        }
    });

    // 连接表格变化信号
    connect(m_pathTable, &QTableWidget::cellChanged, this, &MultiPathPara::onPathParameterChanged);
}

void MultiPathPara::updatePathParametersTable()
{
    // 断开信号避免循环触发
    m_pathTable->blockSignals(true);

    // 设置行数
    m_pathTable->setRowCount(m_multipathCount);

    // 填充表格数据
    for (int i = 0; i < m_multipathCount; ++i) {
        // 路径索引（只读）
        QTableWidgetItem *indexItem = new QTableWidgetItem(QString("路径 %1").arg(i + 1));
        indexItem->setFlags(indexItem->flags() & ~Qt::ItemIsEditable);
        indexItem->setTextAlignment(Qt::AlignCenter);
        indexItem->setBackground(QColor(0x44, 0x88, 0x88));
        m_pathTable->setItem(i, 0, indexItem);

        // 相对时延 (ns)
        QTableWidgetItem *delayItem = new QTableWidgetItem(QString::number(i * 100.0, 'f', 1));
        delayItem->setTextAlignment(Qt::AlignCenter);
        m_pathTable->setItem(i, 1, delayItem);

        // 衰减功率 (dB)
        QTableWidgetItem *attenuationItem = new QTableWidgetItem(QString::number(-i * 5.0, 'f', 1));
        attenuationItem->setTextAlignment(Qt::AlignCenter);
        m_pathTable->setItem(i, 2, attenuationItem);

        // 路径频移 (Hz)
        QTableWidgetItem *freqShiftItem = new QTableWidgetItem(QString::number(i * 50.0 - 100.0, 'f', 1));
        freqShiftItem->setTextAlignment(Qt::AlignCenter);
        m_pathTable->setItem(i, 3, freqShiftItem);

        // 频扩 (Hz)
        QTableWidgetItem *freqSpreadItem = new QTableWidgetItem(QString::number(i * 10.0 + 10.0, 'f', 1));
        freqSpreadItem->setTextAlignment(Qt::AlignCenter);
        m_pathTable->setItem(i, 4, freqSpreadItem);
    }

    // 重新连接信号
    m_pathTable->blockSignals(false);
}

void MultiPathPara::updatePathParametersFromTable()
{
    // 在实际应用中，这里可以更新内部数据结构
    // 目前数据直接存储在表格中，需要时从表格读取
}

void MultiPathPara::onPathParameterChanged()
{
    updatePathParametersFromTable();
}

void MultiPathPara::onApplyClicked()
{
    updatePathParametersFromTable();
    emit multipathParametersChanged(
        m_multipathCount,
        getDelays(),
        getAttenuations(),
        getFrequencyShifts(),
        getFrequencySpreads()
        );
}

void MultiPathPara::onResetClicked()
{
    resetToDefaults();
}

void MultiPathPara::onGenerateRandomClicked()
{
    std::random_device rd;
    std::mt19937 gen(rd());

    m_pathTable->blockSignals(true);

    for (int i = 0; i < m_multipathCount; ++i) {
        // 相对时延：递增但有一定随机性
        std::uniform_real_distribution<double> delayDist(i * 80.0, i * 120.0);
        double delay = std::min(delayDist(gen), DELAY_MAX);
        m_pathTable->item(i, 1)->setText(QString::number(delay, 'f', 1));

        // 衰减功率：递减但有一定随机性
        std::uniform_real_distribution<double> attenDist(-i * 6.0, -i * 4.0);
        double attenuation = std::max(attenDist(gen), ATTENUATION_MIN);
        m_pathTable->item(i, 2)->setText(QString::number(attenuation, 'f', 1));

        // 路径频移：在正负范围内随机
        std::uniform_real_distribution<double> shiftDist(FREQ_SHIFT_MIN, FREQ_SHIFT_MAX);
        double freqShift = shiftDist(gen);
        m_pathTable->item(i, 3)->setText(QString::number(freqShift, 'f', 1));

        // 频扩：在合理范围内随机
        std::uniform_real_distribution<double> spreadDist(FREQ_SPREAD_MIN, FREQ_SPREAD_MAX);
        double freqSpread = spreadDist(gen);
        m_pathTable->item(i, 4)->setText(QString::number(freqSpread, 'f', 1));
    }

    m_pathTable->blockSignals(false);
    updatePathParametersFromTable();
}

void MultiPathPara::resetToDefaults()
{
    m_multipathCount = 3;
    m_multipathCountValueLabel->setText("3");
    updatePathParametersTable();
}

// 获取路径参数的方法
QVector<double> MultiPathPara::getDelays() const
{
    QVector<double> delays;
    for (int i = 0; i < m_multipathCount; ++i) {
        delays.append(m_pathTable->item(i, 1)->text().toDouble());
    }
    return delays;
}

QVector<double> MultiPathPara::getAttenuations() const
{
    QVector<double> attenuations;
    for (int i = 0; i < m_multipathCount; ++i) {
        attenuations.append(m_pathTable->item(i, 2)->text().toDouble());
    }
    return attenuations;
}

QVector<double> MultiPathPara::getFrequencyShifts() const
{
    QVector<double> shifts;
    for (int i = 0; i < m_multipathCount; ++i) {
        shifts.append(m_pathTable->item(i, 3)->text().toDouble());
    }
    return shifts;
}

QVector<double> MultiPathPara::getFrequencySpreads() const
{
    QVector<double> spreads;
    for (int i = 0; i < m_multipathCount; ++i) {
        spreads.append(m_pathTable->item(i, 4)->text().toDouble());
    }
    return spreads;
}

QList<MultiPathType> MultiPathPara::getMultipathPara() const
{
    QList<MultiPathType> multiParas;
    for (int i = 0; i < m_multipathCount; ++i) {
        MultiPathType config;
        config.pathNum = i + 1;
        config.relativDelay = m_pathTable->item(i, 1)->text().toDouble();
        config.antPower = m_pathTable->item(i, 2)->text().toDouble();
        config.freShift = m_pathTable->item(i, 3)->text().toDouble();
        config.freSpread = m_pathTable->item(i, 4)->text().toDouble();
        multiParas.append(config);
    }

    return multiParas;
}

