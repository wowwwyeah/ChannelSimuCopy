#ifndef CHANNELBASICPARA_H
#define CHANNELBASICPARA_H

#include <QWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSlider>
#include <QDial>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>

class ChannelBasicPara : public QWidget
{
    Q_OBJECT

public:
    explicit ChannelBasicPara(QWidget *parent = nullptr);

    // 获取参数值
    double getNoisePower() const { return m_noisePower; }
    double getAttenuationPower() const { return m_attenuationPower; }
    double getCommunicationDistance() const { return m_communicationDistance; }

signals:
    void parametersChanged(double noisePower, double attenuationPower, double communicationDistance);
    void settingsApplied();

public slots:
    void resetToDefaults();

private slots:
    void onNoisePowerSliderChanged(int value);
    void onNoisePowerDialChanged(int value);
    void onNoisePowerSpinBoxChanged(double value);

    void onAttenuationPowerSliderChanged(int value);
    void onAttenuationPowerDialChanged(int value);
    void onAttenuationPowerSpinBoxChanged(double value);

    void onDistanceSliderChanged(int value);
    void onDistanceDialChanged(int value);
    void onDistanceSpinBoxChanged(double value);

    void onApplyClicked();
    void onResetClicked();

private:
    void setupUI();
    void createPowerParametersGroup(); // 合并创建功率参数组
    void createCommunicationDistanceGroup();
    void updateNoisePower(double value);
    void updateAttenuationPower(double value);
    void updateCommunicationDistance(double value);

    // 功率参数组（噪声功率 + 衰减功率）
    QGroupBox *m_powerGroup;

    // 噪声功率组件
    QSlider *m_noisePowerSlider;
    QDial *m_noisePowerDial;
    QDoubleSpinBox *m_noisePowerSpinBox;
    QLabel *m_noisePowerValueLabel;

    // 衰减功率组件
    QSlider *m_attenuationPowerSlider;
    QDial *m_attenuationPowerDial;
    QDoubleSpinBox *m_attenuationPowerSpinBox;
    QLabel *m_attenuationPowerValueLabel;

    // 通信距离组件
    QGroupBox *m_distanceGroup;
    QSlider *m_distanceSlider;
    QDial *m_distanceDial;
    QDoubleSpinBox *m_distanceSpinBox;
    QLabel *m_distanceValueLabel;

    // 按钮
    QPushButton *m_applyButton;
    QPushButton *m_resetButton;

    // 参数值
    double m_noisePower;
    double m_attenuationPower;
    double m_communicationDistance;

    // 参数范围
    const double NOISE_POWER_MIN = -120.0;
    const double NOISE_POWER_MAX = 0.0;
    const double ATTENUATION_POWER_MIN = 0.0;
    const double ATTENUATION_POWER_MAX = 100.0;
    const double DISTANCE_MIN = 1.0;
    const double DISTANCE_MAX = 1000.0;
};

#endif // CHANNELBASICPARA_H
