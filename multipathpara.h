#ifndef MULTIPATHPARA_H
#define MULTIPATHPARA_H

#include <QWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QScrollArea>
#include <QFrame>
#include "channelparaconifg.h"

class MultiPathPara : public QWidget
{
    Q_OBJECT

public:
    explicit MultiPathPara(QWidget *parent = nullptr);

    // 获取多径数量
    int getMultipathCount() const { return m_multipathCount; }
    // 获取滤波器编号
    int getFilterNum() const { return m_filterNum; }

    // 获取路径参数
    QList<MultiPathType> getMultipathPara() const;
    QVector<double> getDelays() const;
    QVector<double> getAttenuations() const;
    QVector<double> getFrequencyShifts() const;
    QVector<double> getFrequencySpreads() const;

signals:
    void multipathParametersChanged(int count,
                                    const QVector<double>& delays,
                                    const QVector<double>& attenuations,
                                    const QVector<double>& frequencyShifts,
                                    const QVector<double>& frequencySpreads);

public slots:
    void resetToDefaults();

private slots:
    void onPathParameterChanged();
    void onApplyClicked();
    void onResetClicked();
    void onGenerateRandomClicked();

private:
    void setupUI();
    void initConnections();
    void createMultipathCountGroup();
    void createPathParametersGroup();
    void updatePathParametersTable();
    void updatePathParametersFromTable();

    // 多径数量设置
    QGroupBox *m_multipathCountGroup;
    QSlider *m_multipathCountSlider;
    QSpinBox *m_multipathCountSpinBox;
    QSpinBox *m_filterNumSpinBox;
    QLabel *m_multipathCountValueLabel;

    // 路径参数表格
    QGroupBox *m_pathParametersGroup;
    QTableWidget *m_pathTable;

    // 按钮
    QPushButton *m_applyButton;
    QPushButton *m_resetButton;
    QPushButton *m_generateRandomButton;

    // 参数值
    int m_multipathCount;
    int m_filterNum;

    // 新增：多径数量 - 标签+加减按钮
    QLabel *m_multipathValueLabel;
    QPushButton *m_multipathAddBtn;
    QPushButton *m_multipathSubBtn;

    // 新增：滤波器编号 - 标签+加减按钮
    QLabel *m_filterNumValueLabel;
    QPushButton *m_filterAddBtn;
    QPushButton *m_filterSubBtn;

    // 参数范围
    const int MULTIPATH_COUNT_MIN = 1;
    const int MULTIPATH_COUNT_MAX = 5;
    const double DELAY_MIN = 0.0;
    const double DELAY_MAX = 1000.0;  // ns
    const double ATTENUATION_MIN = -50.0;
    const double ATTENUATION_MAX = 0.0;  // dB
    const double FREQ_SHIFT_MIN = -500.0;
    const double FREQ_SHIFT_MAX = 500.0;  // Hz
    const double FREQ_SPREAD_MIN = 0.0;
    const double FREQ_SPREAD_MAX = 200.0;  // Hz
};

#endif // MULTIPATHPARA_H
