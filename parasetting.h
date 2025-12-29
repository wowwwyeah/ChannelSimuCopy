#ifndef PARASETTING_H
#define PARASETTING_H

#include <QWidget>
#include <QSpinBox>

class ParaSetting : public QWidget
{
    Q_OBJECT
public:
    explicit ParaSetting(QWidget *parent = nullptr);
    void initUI();
signals:

private:
    QSpinBox *m_noisePowerSB;
    QSpinBox *m_antPowerSB;
    QSpinBox *m_distanceSB;
};

#endif // PARASETTING_H
