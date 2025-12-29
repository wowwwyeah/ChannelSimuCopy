#ifndef CHANNELMODELSELECT_H
#define CHANNELMODELSELECT_H

#include <QWidget>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QLineEdit>
#include <QScrollArea>
#include <QMessageBox>
#include <QFrame>
#include <vector>

// 添加场景对话框
class AddSceneDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddSceneDialog(QWidget *parent = nullptr);
    QString getSceneName() const { return m_nameLineEdit->text().trimmed(); }
    QString getSceneDescription() const { return m_descLineEdit->text().trimmed(); }

private:
    QLineEdit *m_nameLineEdit;
    QLineEdit *m_descLineEdit;
};

class ChannelModelSelect : public QWidget
{
    Q_OBJECT

public:
    ChannelModelSelect(QWidget *parent = nullptr);
    ~ChannelModelSelect();

    //获取当前选中的单选按钮名称
    QString getSelectedRadioButtonName() const;
    QString getSelectedRadioButtonText() const; // 可选：获取显示文本

private slots:
    void addRadioButton();
    void deleteSelectedRadioButton();
    void updateButtonStates();

private:
    void setupUI();
    void createRadioButton(const QString &text, const QString &tooltip = "", bool isProtected = false);
    void reorganizeLayout(); // 重新组织布局
    void setupButtonTooltips(); // 设置按钮提示信息

    QVBoxLayout *m_mainLayout;

    // 主GroupBox
    QGroupBox *m_mainGroup;
    QHBoxLayout *m_mainGroupLayout;

    // 标题区域
    QLabel *m_titleLabel;

    QVBoxLayout *m_controlLayout;
    QLineEdit *m_inputLineEdit;
    QPushButton *m_addButton;
    QPushButton *m_deleteButton;

    // 单选按钮区域
    QWidget *m_radioButtonContainer;
    QHBoxLayout *m_radioButtonLayout;
    std::vector<QVBoxLayout*> m_columnLayouts;

    QButtonGroup *m_buttonGroup;
    QVector<QString> m_modelTitle;
    QVector<QString> m_modelTips;
    std::map<QRadioButton*, QString> m_buttonTooltips; // 存储按钮和对应的备注信息
    std::vector<QRadioButton*> m_protectedButtons; // 受保护的按钮（前7个）

    int m_radioButtonCounter;
    const int MAX_ROWS_PER_COLUMN = 5; // 每列最大行数
    const int PROTECTED_BUTTON_COUNT = 7; // 前7个按钮受保护
};

#endif // CHANNELMODELSELECT_H
