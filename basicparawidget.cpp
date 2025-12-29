#include "basicparawidget.h"
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>
#include <cmath>

TouchSliderInput::TouchSliderInput(QWidget *parent)
    : QWidget(parent)
    , m_label(nullptr)
    , m_slider(nullptr)
    , m_spinBox(nullptr)
    , m_touchArea(nullptr)
    , m_mainLayout(nullptr)
    , m_inputType(DecimalType)
    , m_minValue(0.0)
    , m_maxValue(100.0)
    , m_currentValue(50.0)
    , m_step(1.0)
    , m_decimals(2)
    , m_isDragging(false)
    , m_touchSensitivity(0.5)
{
    setupUI();
}

void TouchSliderInput::setupUI()
{
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setSpacing(20);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);

    // 参数名称标签
    m_label = new QLabel("数值:", this);
    m_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_label->setMinimumWidth(80);
    m_mainLayout->addWidget(m_label);

    // 触摸滑动区域（包含滑动条）
    m_touchArea = new QWidget(this);
    m_touchArea->setObjectName("touchArea");
    m_touchArea->setMinimumHeight(30);
    m_touchArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_touchArea->setMouseTracking(true);

    QVBoxLayout *touchLayout = new QVBoxLayout(m_touchArea);
    touchLayout->setSpacing(0);
    touchLayout->setContentsMargins(0, 0, 0, 0);

    // 滑动条（不再显示数字标签）
    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setRange(0, 1000);
    m_slider->setValue(500);
    m_slider->setPageStep(100);
    m_slider->setFixedWidth(100); // 固定滑动条宽度为250像素
    m_slider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred); // 固定宽度策略
    connect(m_slider, &QSlider::valueChanged, this, &TouchSliderInput::onSliderValueChanged);
    touchLayout->addWidget(m_slider);

    m_mainLayout->addWidget(m_touchArea, 1);

    // 数值显示和微调框
    m_spinBox = new QDoubleSpinBox(this);
    m_spinBox->setRange(m_minValue, m_maxValue);
    m_spinBox->setValue(m_currentValue);
    m_spinBox->setDecimals(m_decimals);
    m_spinBox->setSingleStep(m_step);
    m_spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_spinBox->setFixedSize(90, 35); // 固定宽度和高度
    m_spinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(m_spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TouchSliderInput::onSpinBoxValueChanged);
    connect(m_spinBox, &QDoubleSpinBox::editingFinished, this, &TouchSliderInput::onEditingFinished);
    m_mainLayout->addWidget(m_spinBox);

    // 添加弹性空间使控件左对齐
    m_mainLayout->addStretch();

    // 设置控件垂直居中
    m_mainLayout->setAlignment(Qt::AlignVCenter);

    // 设置样式
    setStyleSheet(R"(
        TouchSliderInput {
            background-color: #ffffff;
            border: 2px solid #e0e0e0;
            border-radius: 12px;
            padding: 5px;
        }
        touchArea {
            background-color: #f8f9fa;
            border-radius: 8px;
            border: 2px solid #dee2e6;
        }
        QSlider::groove:horizontal {
            background: #dee2e6;
            height: 12px;
            border-radius: 6px;
        }
        QSlider::handle:horizontal {
            background: #007bff;
            width: 24px;
            height: 24px;
            margin: -6px 0;
            border-radius: 12px;
            border: 2px solid #0056b3;
        }
        QSlider::sub-page:horizontal {
            background: #28a745;
            border-radius: 6px;
        }
        QDoubleSpinBox {
            font-size: 16px;
            font-weight: bold;
            padding: 8px;
            border: 2px solid #ced4da;
            border-radius: 8px;
            background: white;
            color: #007bff;
            text-align: center;
        }
        QDoubleSpinBox:focus {
            border-color: #007bff;
            background-color: #f0f8ff;
        }
        QLabel {
            color: #4CAF50;          /* 文本颜色 */
            font-family: "Microsoft YaHei";  /* 字体 */
            font-size: 16px;         /* 字号 */
            font-weight: bold;       /* 加粗 */
        }
    )");

    // 设置触摸区域的大小策略，使其更容易触摸
    m_touchArea->setMinimumHeight(60);
    m_slider->setMinimumHeight(40);

}

void TouchSliderInput::setLabel(const QString &label)
{
    m_label->setText(label);
}

void TouchSliderInput::setInputType(InputType type)
{
    m_inputType = type;
    if (m_inputType == IntegerType) {
        m_step = std::max(1.0, m_step);
        m_decimals = 0;
        m_spinBox->setDecimals(0);
    } else {
        m_spinBox->setDecimals(m_decimals);
    }
    m_spinBox->setValue(m_currentValue); // 更新显示
}

void TouchSliderInput::setRange(double min, double max)
{
    m_minValue = min;
    m_maxValue = max;
    m_spinBox->setRange(min, max);
    m_slider->setValue(realToSliderValue(m_currentValue));
}

void TouchSliderInput::setValue(double value)
{
    value = qBound(m_minValue, value, m_maxValue);
    if (qAbs(m_currentValue - value) > 1e-10) {
        m_currentValue = value;
        m_spinBox->setValue(value);
        m_slider->setValue(realToSliderValue(value));
        emit valueChanged(value);
    }
}

void TouchSliderInput::setDecimals(int decimals)
{
    m_decimals = decimals;
    if (m_inputType == DecimalType) {
        m_spinBox->setDecimals(decimals);
    }
    m_spinBox->setValue(m_currentValue); // 更新显示
}

void TouchSliderInput::setStep(double step)
{
    m_step = step;
    m_spinBox->setSingleStep(step);
}

void TouchSliderInput::setSliderVisible(bool visible)
{
    m_slider->setVisible(visible);
    m_touchArea->setVisible(visible);
}

double TouchSliderInput::value() const
{
    return m_currentValue;
}

TouchSliderInput::InputType TouchSliderInput::inputType() const
{
    return m_inputType;
}

void TouchSliderInput::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_touchArea->rect().contains(event->pos())) {
        m_lastTouchPos = event->pos();
        m_isDragging = true;
        //m_touchArea->setStyleSheet("#touchArea { background-color: #e3f2fd; border: 2px solid #2196f3; border-radius: 10px; }");
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void TouchSliderInput::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDragging && (event->buttons() & Qt::LeftButton)) {
        QPoint delta = event->pos() - m_lastTouchPos;
        m_lastTouchPos = event->pos();

        // 根据移动距离调整数值
        updateValueByTouch(delta.x());
        event->accept();
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

void TouchSliderInput::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_isDragging) {
        m_isDragging = false;
        //m_touchArea->setStyleSheet("#touchArea { background-color: #f8f9fa; border: 2px solid #dee2e6; border-radius: 10px; }");
        emit editingFinished();
        event->accept();
    } else {
        QWidget::mouseReleaseEvent(event);
    }
}

void TouchSliderInput::wheelEvent(QWheelEvent *event)
{
    if (m_touchArea->rect().contains(event->position().toPoint())) {
        int delta = event->angleDelta().y() > 0 ? 1 : -1;
        setValue(m_currentValue + delta * m_step);
        event->accept();
    } else {
        QWidget::wheelEvent(event);
    }
}

void TouchSliderInput::onSliderValueChanged(int value)
{
    double realValue = sliderToRealValue(value);
    if (qAbs(m_currentValue - realValue) > 1e-10) {
        m_currentValue = realValue;
        m_spinBox->setValue(realValue);
        emit valueChanged(realValue);
    }
}

void TouchSliderInput::onSpinBoxValueChanged(double value)
{
    if (qAbs(m_currentValue - value) > 1e-10) {
        m_currentValue = value;
        m_slider->setValue(realToSliderValue(value));
        emit valueChanged(value);
    }
}

void TouchSliderInput::onEditingFinished()
{
    emit editingFinished();
}

void TouchSliderInput::updateValueByTouch(int delta)
{
    double sensitivity = m_touchSensitivity;
    if (m_inputType == IntegerType) {
        sensitivity = std::max(1.0, sensitivity);
    }

    double valueChange = delta * sensitivity * m_step;
    setValue(m_currentValue + valueChange);
}

// 对数到线性转换（用于显示值）
double TouchSliderInput::logarithmicToLinear(double value) const
{
    if (value <= 0) return 0;
    return log10(value);
}

// 线性到对数转换（用于存储值）
double TouchSliderInput::linearToLogarithmic(double value) const
{
    return pow(10, value);
}


double TouchSliderInput::sliderToRealValue(int sliderValue) const
{
    double normalized = static_cast<double>(sliderValue) / m_slider->maximum();

    if (m_inputType == LogarithmicType) {
        // 对数映射：将线性滑动条值映射到对数范围
        double logMin = logarithmicToLinear(m_minValue);
        double logMax = logarithmicToLinear(m_maxValue);
        double logValue = logMin + normalized * (logMax - logMin);
        return linearToLogarithmic(logValue);
    } else {
        // 线性映射
        return m_minValue + normalized * (m_maxValue - m_minValue);
    }
}

int TouchSliderInput::realToSliderValue(double realValue) const
{
    realValue = qBound(m_minValue, realValue, m_maxValue);

    if (m_inputType == LogarithmicType) {
        // 对数映射：将对数值映射到线性滑动条范围
        double logMin = logarithmicToLinear(m_minValue);
        double logMax = logarithmicToLinear(m_maxValue);
        double logValue = logarithmicToLinear(realValue);
        double normalized = (logValue - logMin) / (logMax - logMin);
        return static_cast<int>(normalized * m_slider->maximum());
    } else {
        // 线性映射
        double normalized = (realValue - m_minValue) / (m_maxValue - m_minValue);
        return static_cast<int>(normalized * m_slider->maximum());
    }
}
