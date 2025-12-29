#ifndef SIMULISTVIEW_H
#define SIMULISTVIEW_H

#include <QWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QStandardItemModel>

class SimuListView : public QWidget
{
    Q_OBJECT
public:
    explicit SimuListView(QWidget *parent = nullptr);
    ~SimuListView();

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
    QTableWidget *m_tableView;
    QPushButton *m_deleteButton;
    QPushButton *m_importButton;
    QPushButton *m_exportButton;
    QPushButton *m_selectAllButton;
};

#endif // SIMULISTVIEW_H
