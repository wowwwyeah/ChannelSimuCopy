#include "mainwindow.h"
#include "subwindow.h"
#include "screensaver.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 禁用 Qt 高 DPI 缩放（关键，避免嵌入式屏幕显示模糊或尺寸异常）
    app.setAttribute(Qt::AA_DisableHighDpiScaling);
    // （可选）设置屏幕设备像素比为 1（适合物理分辨率屏幕）
    app.setAttribute(Qt::AA_Use96Dpi);

    // --- 初始化屏保 ---
    // 设置5分钟（300000毫秒）无操作后触发屏保
    ScreenSaver screenSaver(&app, 1 * 60 * 1000);

    // 安装全局事件过滤器，以监听所有用户输入
    GlobalEventFilter eventFilter(&screenSaver);
    app.installEventFilter(&eventFilter);

    app.setApplicationName("信道模拟器系统");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("MyCompany");

    // 创建主窗口和副窗口
    MainWindow *mainWindow = new MainWindow();
    SubWindow *subWindow = new SubWindow();

    // 设置窗口相互引用
    mainWindow->setSubWindow(subWindow);
    subWindow->setMainWindow(mainWindow);

    // 显示主窗口
    mainWindow->show();

    return app.exec();
}
