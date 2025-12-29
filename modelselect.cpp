#include "modelselect.h"
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

ModelSelect::ModelSelect(QWidget *parent)
    : QWidget{parent}
{
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
    initUI();
}

void ModelSelect::initUI()
{
    int i = 0;
    // 设置背景色
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor("#336666"));
    setPalette(pal);
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel("模板选择");
    title->setAlignment(Qt::AlignmentFlag::AlignCenter);
    title->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px;");
    layout->addWidget(title);

    QWidget *tableContainer = new QWidget(this);
    tableContainer->setObjectName("tableContainer");
    QGridLayout *tableContainerLayout = new QGridLayout(tableContainer);

    // 添加按钮到网格布局中，总共添加15个按钮（5行 * 3列）
    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 3; ++col) {
            // 创建按钮，并设置文本（例如：行号和列号的组合）
            QString buttonText;
            if(i < m_modelTitle.size())
            {
                buttonText = m_modelTitle[i];
                i++;
            }
            else
            {
                buttonText = m_modelTitle[7];
            }
            QPushButton *button = new QPushButton(buttonText);
            // 将按钮添加到网格布局中，指定行和列位置
            tableContainerLayout->addWidget(button, row, col);
        }
    }

    QWidget *buttonContainer = new QWidget(this);
    buttonContainer->setObjectName("buttonContainer");
    QHBoxLayout *buttonContainerLayout = new QHBoxLayout(buttonContainer);
    buttonContainerLayout->setContentsMargins(6, 0, 6, 0); // 表格左右边距
    buttonContainerLayout->setSpacing(0);
    // 功能按钮
    m_deleteButton = new QPushButton("删除", this);
    m_addButton = new QPushButton("添加", this);
    //m_mainButton->setStyleSheet("QPushButton { background-color: white; color: black; border-radius: 8px; padding: 8px; margin: 10px; }"
    //                            "QPushButton:hover { background-color: #E8E8E8; }");

    buttonContainerLayout->addWidget(m_deleteButton);
    buttonContainerLayout->addWidget(m_addButton);
    layout->addWidget(tableContainer, 1); // 表格占据主要空间
    layout->addWidget(buttonContainer);
}
