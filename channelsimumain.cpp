#include <QDebug>
#include <QPushButton>
#include <QMessageBox>
#include <QInputDialog>
#include "datamanager.h"
#include "channelselect.h"
#include "channelsimumain.h"
#include "channelbasicpara.h"
#include "ui_channelsimumain.h"

ChannelSimuMain::ChannelSimuMain(QWidget *parent)
    : QWidget{parent},
    ui(new Ui::ChannelSimuMain)
{
    ui->setupUi(this);
    this->initValue();
}

ChannelSimuMain::~ChannelSimuMain()
{
    delete ui;
}

void ChannelSimuMain::initValue()
{
    // 设置背景色
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor("#336666"));
    setPalette(pal);
    this->setWindowTitle("智原科技");
    this->setWindowIcon(QIcon(":/res//image/logo.png"));
    this->setFixedSize(538,370);

    ui->model1BT->click();
    ui->diy1BT->setVisible(false);
    ui->diy2BT->setVisible(false);
    ui->diy3BT->setVisible(false);

}

void ChannelSimuMain::on_exitBT_clicked()
{

}

void ChannelSimuMain::on_backBT_clicked()
{

}

void ChannelSimuMain::on_nextBT_clicked()
{
    if(!checkModelSelected())
    {
        QMessageBox::critical(nullptr, "错误", "请选择场景!");
        return;
    }

    QVariantMap modelData;
    modelData["modelName"] = ui->model1BT->text();
    DataManager::instance()->setPageData("ChannelModel", modelData);
    this->close();
    ChannelBasicPara *cbp= new ChannelBasicPara;
    cbp->show();
}

void ChannelSimuMain::on_modelAddBT_clicked()
{
    bool flag;
    QString modelDiyName = QInputDialog::getText(this, "Input", "Enter name:", QLineEdit::Normal, "", &flag);
    if(modelDiyName.isEmpty())
    {
        QMessageBox::critical(nullptr, "错误", "请输入正确的名称！");
        return;
    }

    flag = createDiyModel(modelDiyName);
    if(!flag)
    {
        qDebug() << "创建自定义场景失败!" << modelDiyName;
        return;
    }
}

bool ChannelSimuMain::checkModelSelected()
{
    bool flag;
    if(ui->model1BT->isChecked())
    {
        goto Exit;
    }
    if(ui->model2BT->isChecked())
    {
        goto Exit;
    }
    if(ui->model3BT->isChecked())
    {
        goto Exit;
    }
    if(ui->model4BT->isChecked())
    {
        goto Exit;
    }
    if(ui->model5BT->isChecked())
    {
        goto Exit;
    }
    if(ui->model6BT->isChecked())
    {
        goto Exit;
    }
    if(ui->model7BT->isChecked())
    {
        goto Exit;
    }

    return flag;

Exit:

    flag = true;
    return flag;
}

bool ChannelSimuMain::createDiyModel(QString diyName)
{

    if(!ui->diy1BT->isChecked())
    {
        ui->diy1BT->setText(diyName);
        ui->diy1BT->click();
        ui->diy1BT->setVisible(true);
        return true;
    }
}
