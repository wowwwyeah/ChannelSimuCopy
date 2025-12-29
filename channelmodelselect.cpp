#include "channelmodelselect.h"
#include <QDialogButtonBox>

// AddSceneDialog 实现
AddSceneDialog::AddSceneDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("添加场景");
    setModal(true);
    setMinimumWidth(350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 设置对话框样式
    setStyleSheet(R"(
        QDialog {
            background-color: #336666;
        }
        QLabel {
            color: #99CCCC;
            font-family: "Microsoft YaHei";
            font-size: 12px;
            font-weight: bold;
            margin-bottom: 5px;
        }
        QLineEdit {
            padding: 8px;
            border: 2px solid #559999;
            border-radius: 6px;
            background-color: #224444;
            color: #CCEEEE;
            font-family: "Microsoft YaHei";
            font-size: 12px;
            margin-bottom: 15px;
        }
        QLineEdit:focus {
            border: 2px solid #66AAAA;
            background-color: #225555;
        }
        QLineEdit::placeholder {
            color: #88AAAA;
            font-style: italic;
        }
        QDialogButtonBox {
            margin-top: 10px;
        }
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
    )");

    // 场景名称输入
    QLabel *nameLabel = new QLabel("场景名称:");
    mainLayout->addWidget(nameLabel);

    m_nameLineEdit = new QLineEdit();
    m_nameLineEdit->setPlaceholderText("请输入场景名称...");
    mainLayout->addWidget(m_nameLineEdit);

    // 场景描述输入
    QLabel *descLabel = new QLabel("场景描述 (可选):");
    mainLayout->addWidget(descLabel);

    m_descLineEdit = new QLineEdit();
    m_descLineEdit->setPlaceholderText("请输入场景描述...");
    mainLayout->addWidget(m_descLineEdit);

    // 按钮框
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(buttonBox);

    // 设置默认焦点
    m_nameLineEdit->setFocus();
}

ChannelModelSelect::ChannelModelSelect(QWidget *parent)
    : QWidget(parent)
    , m_radioButtonCounter(0)
{
#if 0
    m_modelTitle = {
        "短波地波山地",
        "短波地波平原",
        "短波地波海上",
        "短波天波单跳",
        "短波天波多跳",
        "短波天波单音",
        "短波天波仰角",
        "",
    };
#endif
    m_modelTitle = {
        "短波天波单跳",
        "模板2",
        "模板3",
        "模板4",
        "模板5",
        "模板6",
        "模板7",
        "",
    };

    m_modelTips = {
        "难度：低级",
        "难度：低级",
        "难度：中级",
        "难度：中级",
        "难度：中级",
        "难度：高级",
        "难度：高级",
        "",
    };

    setupUI();

    // 创建初始的7个单选按钮
    for (int i = 0; i < PROTECTED_BUTTON_COUNT; ++i) {
        createRadioButton(m_modelTitle[i], m_modelTips[i], true);
    }

    reorganizeLayout();
    updateButtonStates();
    setupButtonTooltips(); // 设置按钮提示
}

ChannelModelSelect::~ChannelModelSelect()
{
    for (auto layout : m_columnLayouts) {
        delete layout;
    }
}

void ChannelModelSelect::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);

    // 设置样式表
    this->setStyleSheet(R"(
        /* 主窗口背景 */
        QWidget {
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

        /* RadioButton样式 */
        QRadioButton {
            spacing: 4px;  /* 2px -> 4px */
            color: #4CAF50;
            font-family: "Microsoft YaHei";
            font-size: 16px;  /* 14px -> 16px */
            font-weight: bold;
            background-color: transparent;
            border: none;
            padding: 8px 12px;  /* 6px 10px -> 8px 12px */
            border-radius: 6px;  /* 4px -> 6px */
            margin: 4px 0px;  /* 2px 0px -> 4px 0px */
        }

        QRadioButton:hover {
            background-color: #224444;
            color: #CCEEEE;
        }

        QRadioButton::indicator {
            width: 16px;  /* 12px -> 16px */
            height: 16px;  /* 12px -> 16px */
            border-radius: 3px;  /* 2px -> 3px */
            border: 3px solid #00C6FF;  /* 2px -> 3px */
            background-color: #336666;
        }

        QRadioButton::indicator:checked {
            background-color: #00C6FF;
            border: 3px solid #00C6FF;  /* 2px -> 3px */
        }

        QRadioButton::indicator:unchecked {
            background-color: #336666;
            border: 3px solid #99CCCC;  /* 2px -> 3px */
        }

        QRadioButton::indicator:checked:hover {
            background-color: #CCEEEE;
            border: 3px solid #CCEEEE;  /* 2px -> 3px */
        }

        QRadioButton::indicator:unchecked:hover {
            border: 3px solid #CCEEEE;  /* 2px -> 3px */
        }

        /* 按钮样式 */
        QPushButton {
            background-color: #448888;
            color: #CCEEEE;
            border: none;
            padding: 8px 12px;  /* 2px 4px -> 8px 12px */
            border-radius: 4px;  /* 2px -> 4px */
            font-family: "Microsoft YaHei";
            font-size: 14px;  /* 12px -> 14px */
            font-weight: bold;
            min-width: 60px;  /* 50px -> 60px */
            min-height: 32px;  /* 添加最小高度 */
        }

        QPushButton:hover {
            background-color: #559999;
        }

        QPushButton:pressed {
            background-color: #66AAAA;
        }

        QPushButton:disabled {
            background-color: #225555;
            color: #88AAAA;
        }

        /* 圆形加号按钮样式 */
        QPushButton#addButton {
            background-color: #55AA55;
            color: #CCEEEE;
            border: none;
            border-radius: 30px;  /* 25px -> 30px */
            min-width: 60px;  /* 50px -> 60px */
            min-height: 60px;  /* 50px -> 60px */
            max-width: 60px;  /* 50px -> 60px */
            max-height: 60px;  /* 50px -> 60px */
            font-family: "Microsoft YaHei";
            font-size: 14px;  /* 12px -> 14px */
            font-weight: bold;
            margin: 8px;  /* 5px -> 8px */
        }

        QPushButton#addButton:hover {
            background-color: #66BB66;
            border: 4px solid #CCEEEE;  /* 3px -> 4px */
        }

        QPushButton#addButton:pressed {
            background-color: #44AA44;
        }

        /* 圆形删除按钮样式 */
        QPushButton#delButton {
            background-color: #FF2400;
            color: #CCEEEE;
            border: none;
            border-radius: 30px;  /* 25px -> 30px */
            min-width: 60px;  /* 50px -> 60px */
            min-height: 60px;  /* 50px -> 60px */
            max-width: 60px;  /* 50px -> 60px */
            max-height: 60px;  /* 50px -> 60px */
            font-family: "Microsoft YaHei";
            font-size: 14px;  /* 12px -> 14px */
            font-weight: bold;
            margin: 8px;  /* 5px -> 8px */
        }

        QPushButton#delButton:hover {
            background-color: #FF2400;
            border: 4px solid #CCEEEE;  /* 3px -> 4px */
        }

        QPushButton#delButton:pressed {
            background-color: #FF2400;
        }

        QPushButton#delButton:disabled {
            background-color: #225555;
            color: #88AAAA;
        }

        /* 工具提示样式 */
        QToolTip {
            background-color: #448888;
            color: #CCEEEE;
            border: 3px solid #559999;  /* 2px -> 3px */
            border-radius: 8px;  /* 6px -> 8px */
            padding: 12px;  /* 8px -> 12px */
            font-family: "Microsoft YaHei";
            font-size: 14px;  /* 12px -> 14px */
        }
    )");

    // === 主GroupBox ===
    m_mainGroup = new QGroupBox("信道场景设置");
    QHBoxLayout *mainGroupLayout = new QHBoxLayout(m_mainGroup); // 改为水平布局

    // === 控制区域 - 占1份 ===
    QWidget *controlWidget = new QWidget();
    QVBoxLayout *controlLayout = new QVBoxLayout(controlWidget);
    controlLayout->setAlignment(Qt::AlignTop);
    controlLayout->setSpacing(10);

    // 按钮区域
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(8);

    m_addButton = new QPushButton("添加");
    m_addButton->setMinimumHeight(35);
    m_addButton->setObjectName("addButton"); // 设置对象名用于样式表
    m_addButton->setToolTip("添加新场景");
    connect(m_addButton, &QPushButton::clicked, this, &ChannelModelSelect::addRadioButton);
    buttonLayout->addWidget(m_addButton);

    m_deleteButton = new QPushButton("删除");
    m_deleteButton->setMinimumHeight(35);
    m_deleteButton->setObjectName("delButton"); // 设置对象名用于样式表
    m_deleteButton->setToolTip("删除选中场景");
    connect(m_deleteButton, &QPushButton::clicked, this, &ChannelModelSelect::deleteSelectedRadioButton);
    buttonLayout->addWidget(m_deleteButton);

    controlLayout->addStretch();
    controlLayout->addLayout(buttonLayout);

    // 添加垂直分隔线
    QFrame *verticalLine = new QFrame();
    verticalLine->setFrameShape(QFrame::VLine);
    verticalLine->setFrameShadow(QFrame::Sunken);
    verticalLine->setStyleSheet("background-color: #559999; min-width: 2px; max-width: 2px; margin: 0px 15px;");

    // === 单选按钮区域 - 占4份 ===
    QWidget *radioWidget = new QWidget();
    QVBoxLayout *radioWidgetLayout = new QVBoxLayout(radioWidget);
    radioWidgetLayout->setContentsMargins(0, 0, 0, 0);

    // 单选按钮容器
    m_radioButtonContainer = new QWidget();
    m_radioButtonLayout = new QHBoxLayout(m_radioButtonContainer);
    m_radioButtonLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_radioButtonLayout->setSpacing(10);

    radioWidgetLayout->addWidget(m_radioButtonContainer);

    // 添加到主水平布局
    mainGroupLayout->addWidget(radioWidget);
    //mainGroupLayout->addWidget(verticalLine);
    mainGroupLayout->addWidget(controlWidget);

    // 设置比例：控制区域1份，单选按钮区域4份
    mainGroupLayout->setStretchFactor(radioWidget, 6);
    mainGroupLayout->setStretchFactor(controlWidget, 1);

    m_mainLayout->addWidget(m_mainGroup);
    setLayout(m_mainLayout);

    // 创建按钮组
    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);

}

void ChannelModelSelect::createRadioButton(const QString &text, const QString &tooltip, bool isProtected)
{
    QString buttonText = text.isEmpty() ? QString("场景 %1").arg(++m_radioButtonCounter) : text;

    QRadioButton *radioButton = new QRadioButton(buttonText);
    radioButton->setObjectName(buttonText);

    // 设置受保护属性
    if (isProtected) {
        radioButton->setProperty("protected", true);
        m_protectedButtons.push_back(radioButton);
    }

    m_buttonGroup->addButton(radioButton);
    m_buttonTooltips[radioButton] = tooltip;

    connect(radioButton, &QRadioButton::toggled, this, &ChannelModelSelect::updateButtonStates);
}

void ChannelModelSelect::reorganizeLayout()
{
    // 清空所有列布局
    for (auto layout : m_columnLayouts) {
        QLayoutItem* item;
        while ((item = layout->takeAt(0)) != nullptr) {
            delete item;
        }
        delete layout;
    }
    m_columnLayouts.clear();

    // 清空主布局中的所有列
    QLayoutItem* item;
    while ((item = m_radioButtonLayout->takeAt(0)) != nullptr) {
        delete item;
    }

    // 获取所有单选按钮
    QList<QAbstractButton*> abstractButtons = m_buttonGroup->buttons();
    QList<QRadioButton*> buttons;
    for (QAbstractButton* abstractButton : abstractButtons) {
        QRadioButton* radioButton = qobject_cast<QRadioButton*>(abstractButton);
        if (radioButton) {
            buttons.append(radioButton);
        }
    }

    int totalButtons = buttons.size();

    // 计算需要多少列
    int numColumns = (totalButtons + MAX_ROWS_PER_COLUMN - 1) / MAX_ROWS_PER_COLUMN;

    // 创建列布局
    for (int col = 0; col < numColumns; ++col) {
        QVBoxLayout *columnLayout = new QVBoxLayout();
        columnLayout->setAlignment(Qt::AlignTop);
        columnLayout->setSpacing(2);
        m_columnLayouts.push_back(columnLayout);

        // 添加列布局到主水平布局
        m_radioButtonLayout->addLayout(columnLayout);

        // 设置列间距，除了最后一列
        if (col < numColumns - 1) {
            m_radioButtonLayout->addSpacing(2);
        }
    }

    // 将单选按钮分配到各列
    for (int i = 0; i < totalButtons; ++i) {
        int col = i / MAX_ROWS_PER_COLUMN;
        int row = i % MAX_ROWS_PER_COLUMN;

        if (col < m_columnLayouts.size()) {
            m_columnLayouts[col]->addWidget(buttons[i]);
        }
    }

    // 在各列中添加弹性空间，使内容靠上对齐
    for (auto columnLayout : m_columnLayouts) {
        columnLayout->addStretch();
    }
}

void ChannelModelSelect::setupButtonTooltips()
{
    for (auto& pair : m_buttonTooltips) {
        QRadioButton* button = pair.first;
        const QString& tooltip = pair.second;
        if (!tooltip.isEmpty()) {
            button->setToolTip(tooltip);
        }
    }
}

void ChannelModelSelect::addRadioButton()
{
    // 弹出添加场景对话框
    AddSceneDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        QString sceneName = dialog.getSceneName();
        QString sceneDescription = dialog.getSceneDescription();

        if (sceneName.isEmpty()) {
            QMessageBox::warning(this, "输入错误", "场景名称不能为空！");
            return;
        }

        createRadioButton(sceneName, sceneDescription, false);

        // 重新组织布局
        reorganizeLayout();
        setupButtonTooltips();
        updateButtonStates();
    }

    // 重新组织布局
    reorganizeLayout();
    setupButtonTooltips();
    updateButtonStates();
}

void ChannelModelSelect::deleteSelectedRadioButton()
{
    QRadioButton *selectedButton = nullptr;

    QList<QAbstractButton*> abstractButtons = m_buttonGroup->buttons();
    for (QAbstractButton* abstractButton : abstractButtons) {
        QRadioButton* button = qobject_cast<QRadioButton*>(abstractButton);
        if (button && button->isChecked()) {
            selectedButton = button;
            break;
        }
    }

    if (selectedButton) {
        // 检查是否是受保护的按钮
        if (std::find(m_protectedButtons.begin(), m_protectedButtons.end(), selectedButton) != m_protectedButtons.end()) {
            QMessageBox::information(this, "提示", "这是系统预设场景，不可删除。");
            return;
        }

        // 从按钮组中移除
        m_buttonGroup->removeButton(selectedButton);

        // 从tooltip映射中移除
        auto tooltipIt = m_buttonTooltips.find(selectedButton);
        if (tooltipIt != m_buttonTooltips.end()) {
            m_buttonTooltips.erase(tooltipIt);
        }

        // 删除对象
        selectedButton->deleteLater();

        // 重新组织布局
        reorganizeLayout();
        updateButtonStates();
    } else {
        QMessageBox::information(this, "提示", "请先选择一个要删除的场景。");
    }
}

void ChannelModelSelect::updateButtonStates()
{
    QRadioButton *selectedButton = nullptr;

    QList<QAbstractButton*> abstractButtons = m_buttonGroup->buttons();
    for (QAbstractButton* abstractButton : abstractButtons) {
        QRadioButton* button = qobject_cast<QRadioButton*>(abstractButton);
        if (button && button->isChecked()) {
            selectedButton = button;
            break;
        }
    }

    // 如果选中的是受保护按钮，禁用删除按钮
    if (selectedButton) {
        bool isProtected = std::find(m_protectedButtons.begin(), m_protectedButtons.end(), selectedButton) != m_protectedButtons.end();
        m_deleteButton->setEnabled(!isProtected);
    } else {
        m_deleteButton->setEnabled(false);
    }

    m_deleteButton->setEnabled(selectedButton != nullptr);
    m_addButton->setEnabled(true);
}

QString ChannelModelSelect::getSelectedRadioButtonName() const
{
    // 获取当前选中的单选按钮
    QAbstractButton* checkedButton = m_buttonGroup->checkedButton();

    if (checkedButton) {
        // 转换为 QRadioButton 并获取对象名称
        QRadioButton* radioButton = qobject_cast<QRadioButton*>(checkedButton);
        if (radioButton) {
            return radioButton->objectName();
        }
    }

    // 如果没有选中的按钮，返回空字符串
    return QString();
}

QString ChannelModelSelect::getSelectedRadioButtonText() const
{
    // 获取当前选中的单选按钮
    QAbstractButton* checkedButton = m_buttonGroup->checkedButton();

    if (checkedButton) {
        // 转换为 QRadioButton 并获取显示文本
        QRadioButton* radioButton = qobject_cast<QRadioButton*>(checkedButton);
        if (radioButton) {
            return radioButton->text();
        }
    }

    // 如果没有选中的按钮，返回空字符串
    return QString();
}
