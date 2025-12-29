// ScreenAdapter.h
#ifndef SCREENADAPTER_H
#define SCREENADAPTER_H

#include <QObject>
#include <QScreen>
#include <QApplication>

class ScreenAdapter : public QObject {
    Q_OBJECT
public:
    static ScreenAdapter& instance() {
        static ScreenAdapter instance;
        return instance;
    }

    void initialize();
    qreal scaleFactor() const { return m_scaleFactor; }

private:
    ScreenAdapter() = default;
    qreal m_scaleFactor = 1.0;
};

#endif // SCREENADAPTER_H
