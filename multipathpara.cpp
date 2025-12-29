#include "multipathpara.h"
#include <QScrollBar>
#include <random>

MultiPathPara::MultiPathPara(QWidget *parent)
    : QWidget(parent)
    , m_multipathCount(3)
{
    setupUI();
}

void MultiPathPara::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 设置样式表
    this->setStyleSheet(R"(
        MultiPathPara {
            background-color: #336666;
        }

        QGroupBox {
            color: #CCEEEE;
            font-family: "Microsoft YaHei";
            font-size: 14px;
            font-weight: bold;
            border: 2px solid #559999;
            border-radius: 8px;
            margin-top: 1ex;
            padding-top: 10px;
            background-color: #225555;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top center;
            padding: 0 8px;
            background-color: #448888;
            border-radius: 4px;
        }

        QLabel {
            color: #99CCCC;
            font-family: "Microsoft YaHei";
            font-size: 12px;
            font-weight: bold;
        }

        QLabel#valueLabel {
            color: #CCEEEE;
            font-size: 16px;
            font-weight: bold;
            background-color: #224444;
            border-radius: 4px;
            padding: 4px 8px;
            min-width: 60px;
            qproperty-alignment: AlignCenter;
        }

        /* 多径数量滚动条样式 - 紫色渐变 */
        QSlider#multipathCountSlider::groove:horizontal {
            border: 1px solid #9B6BCC;
            height: 8px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #6B4B8B, stop:0.5 #9B6BCC, stop:1 #CCAAFF);
            border-radius: 4px;
        }

        QSlider#multipathCountSlider::handle:horizontal {
            background: qradialgradient(cx:0.5, cy:0.5, radius:0.8,
                stop:0 #CCAAFF, stop:0.8 #9B6BCC, stop:1 #6B4B8B);
            border: 2px solid #9B6BCC;
            width: 20px;
            margin: -8px 0;
            border-radius: 10px;
        }

        QSlider#multipathCountSlider::handle:horizontal:hover {
            background: qradialgradient(cx:0.5, cy:0.5, radius:0.8,
                stop:0 #EECCFF, stop:0.8 #CCAAFF, stop:1 #9B6BCC);
            border: 2px solid #CCAAFF;
        }

        /* 表格样式 */
        QTableWidget {
            background-color: #224444;
            alternate-background-color: #2A5555;
            gridline-color: #559999;
            border: 1px solid #559999;
            border-radius: 4px;
            font-family: "Microsoft YaHei";
            font-size: 11px;
        }

        QTableWidget::item {
            color: #CCEEEE;
            padding: 4px;
            border: none;
        }

        QTableWidget::item:selected {
            background-color: #448888;
            color: #FFFFFF;
        }

        QTableWidget::header {
            background-color: #336666;
            color: #CCEEEE;
            font-weight: bold;
            border: 1px solid #559999;
        }

        QTableWidget::header:section {
            background-color: #448888;
            color: #CCEEEE;
            padding: 6px;
            border: none;
            font-weight: bold;
        }

        /* 数字输入框样式 */
        QSpinBox, QDoubleSpinBox {
            padding: 4px;
            border: 1px solid #559999;
            border-radius: 3px;
            background-color: #224444;
            color: #CCEEEE;
            font-family: "Microsoft YaHei";
            font-size: 11px;
            min-width: 60px;
        }

        QSpinBox::up-button, QSpinBox::down-button,
        QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {
            background-color: #448888;
            border: 1px solid #559999;
            border-radius: 1px;
            width: 12px;
        }

        /* 按钮样式 */
        QPushButton {
            background-color: #448888;
            color: #CCEEEE;
            border: none;
            padding: 8px 16px;
            border-radius: 6px;
            font-family: "Microsoft YaHei";
            font-size: 12px;
            font-weight: bold;
            min-width: 80px;
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
    )");

    // 创建主水平布局来包含箭头和内容
    QHBoxLayout *outerHorizontalLayout = new QHBoxLayout();

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
    outerHorizontalLayout->addWidget(leftArrow);

    // 中央内容区域
    QVBoxLayout *contentLayout = new QVBoxLayout();

    // 创建标题
    QLabel *titleLabel = new QLabel("多径信道参数设置");
    titleLabel->setStyleSheet(R"(
    QLabel {
        color: #CCEEEE;
        font-family: "Microsoft YaHei";
        font-size: 18px;
        font-weight: bold;
        padding: 10px;
        background-color: #225555;
        border-radius: 8px;
        qproperty-alignment: AlignCenter;
    }
)");
    //contentLayout->addWidget(titleLabel);

    // 创建多径数量设置组
    createMultipathCountGroup();
    //contentLayout->addWidget(m_multipathCountGroup);

    // 创建路径参数组
    createPathParametersGroup();
    contentLayout->addWidget(m_pathParametersGroup);

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

    //contentLayout->addLayout(buttonLayout);

    // 初始更新表格
    updatePathParametersTable();

    // 将内容布局添加到外层水平布局
    outerHorizontalLayout->addLayout(contentLayout);

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
    outerHorizontalLayout->addWidget(rightArrow);

    // 设置外层布局的拉伸因子
    outerHorizontalLayout->setStretchFactor(leftArrow, 0);        // 左箭头不拉伸
    outerHorizontalLayout->setStretchFactor(contentLayout, 1);    // 内容区域拉伸
    outerHorizontalLayout->setStretchFactor(rightArrow, 0);       // 右箭头不拉伸

    mainLayout->addLayout(outerHorizontalLayout);
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

    connect(m_multipathCountSlider, &QSlider::valueChanged, this, &MultiPathPara::onMultipathCountChanged);

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

    connect(m_multipathCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MultiPathPara::onMultipathCountChanged);

    spinBoxLayout->addWidget(inputLabel);
    spinBoxLayout->addWidget(m_multipathCountSpinBox);
    spinBoxLayout->addStretch();
    layout->addLayout(spinBoxLayout);
}

void MultiPathPara::createPathParametersGroup()
{
    m_pathParametersGroup = new QGroupBox("多径参数设置");

    QVBoxLayout *layout = new QVBoxLayout(m_pathParametersGroup);

    // 数字输入
    QHBoxLayout *spinBoxLayout = new QHBoxLayout();
    QLabel *inputLabel = new QLabel("多径数量:");
    inputLabel->setStyleSheet("color: #CCAAFF;");
    m_multipathCountSpinBox = new QSpinBox();
    m_multipathCountSpinBox->setRange(MULTIPATH_COUNT_MIN, MULTIPATH_COUNT_MAX);
    m_multipathCountSpinBox->setValue(m_multipathCount);

    connect(m_multipathCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MultiPathPara::onMultipathCountChanged);

    QLabel *filterLabel = new QLabel("滤波器编号:");
    filterLabel->setStyleSheet("color: #CCAAFF;");
    m_filterNumSpinBox = new QSpinBox();
    m_filterNumSpinBox->setRange(MULTIPATH_COUNT_MIN, MULTIPATH_COUNT_MAX);
    m_filterNumSpinBox->setValue(m_multipathCount);

    connect(m_filterNumSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MultiPathPara::onFilterNumChanged);

    spinBoxLayout->addWidget(inputLabel);
    spinBoxLayout->addWidget(m_multipathCountSpinBox);
    spinBoxLayout->addStretch();
    spinBoxLayout->addWidget(filterLabel);
    spinBoxLayout->addWidget(m_filterNumSpinBox);
    layout->addLayout(spinBoxLayout);

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

    // 连接表格变化信号
    connect(m_pathTable, &QTableWidget::cellChanged, this, &MultiPathPara::onPathParameterChanged);

    layout->addWidget(m_pathTable);
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

void MultiPathPara::onMultipathCountChanged(int value)
{
    m_multipathCount = value;
    m_multipathCountValueLabel->setText(QString::number(value));

    // 同步滑块和数字输入框
    m_multipathCountSlider->blockSignals(true);
    m_multipathCountSlider->setValue(value);
    m_multipathCountSlider->blockSignals(false);

    m_multipathCountSpinBox->blockSignals(true);
    m_multipathCountSpinBox->setValue(value);
    m_multipathCountSpinBox->blockSignals(false);

    // 更新表格
    updatePathParametersTable();
}

void MultiPathPara::onFilterNumChanged(int value)
{
    m_filterNum = value;

    m_filterNumSpinBox->blockSignals(true);
    m_filterNumSpinBox->setValue(value);
    m_filterNumSpinBox->blockSignals(false);
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
    m_multipathCountSlider->setValue(3);
    m_multipathCountSpinBox->setValue(3);
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

