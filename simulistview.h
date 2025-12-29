#ifndef SIMULISTVIEW_H
#define SIMULISTVIEW_H

#include <QWidget>
#include <QPushButton>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include "channelparaconifg.h"

class SimuListView : public QWidget
{
    Q_OBJECT
public:
    explicit SimuListView(QWidget *parent = nullptr);
    ~SimuListView();

    // 声明用于插入数据的函数
    void insertScenarioData(const ModelParaSetting &scenarioData);

signals:

private slots:
    void onDeleteClicked();
    void onImportClicked();
    void onExportClicked();
    void onSelectAllClicked();

private:
    QStandardItemModel *model;
    void initUI();
    void setupConnections();
    void updateRowNumbers();
    bool exportToMultiFiles(const QList<ModelParaSetting> &dataList, const QString &dirPath, const QString &format);
    QTableView *m_tableView;
    QPushButton *m_deleteButton;
    QPushButton *m_importButton;
    QPushButton *m_exportButton;
    QPushButton *m_selectAllButton;

    QList<ModelParaSetting> m_dataList;
};

#endif // SIMULISTVIEW_H
