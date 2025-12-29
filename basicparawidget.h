#ifndef TOUCHSLIDERINPUT_H
#define TOUCHSLIDERINPUT_H

#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QTimer>

class TouchSliderInput : public QWidget
{
    Q_OBJECT

public:
    enum InputType {
        IntegerType,
        DecimalType,
        LogarithmicType  // 新增对数类型
    };

    explicit TouchSliderInput(QWidget *parent = nullptr);

    void setLabel(const QString &label);
    void setInputType(InputType type);
    void setRange(double min, double max);
    void setValue(double value);
    void setDecimals(int decimals);
    void setStep(double step);
    void setSliderVisible(bool visible);

    double value() const;
    InputType inputType() const;

signals:
    void valueChanged(double value);
    void editingFinished();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void onSliderValueChanged(int value);
    void onSpinBoxValueChanged(double value);
    void onEditingFinished();

private:
    void setupUI();
    void updateValueByTouch(int delta);
    double sliderToRealValue(int sliderValue) const;
    int realToSliderValue(double realValue) const;
    double logarithmicToLinear(double value) const;
    double linearToLogarithmic(double value) const;

    QLabel *m_label;
    QSlider *m_slider;
    QDoubleSpinBox *m_spinBox;
    QWidget *m_touchArea;
    QHBoxLayout *m_mainLayout;

    InputType m_inputType;
    double m_minValue;
    double m_maxValue;
    double m_currentValue;
    double m_step;
    int m_decimals;

    QPoint m_lastTouchPos;
    bool m_isDragging;
    double m_touchSensitivity;
};

#endif // TOUCHSLIDERINPUT_H
