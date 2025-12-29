#ifndef MODELSELECT_H
#define MODELSELECT_H

#include <QWidget>
#include <QListView>
#include <QPushButton>
#include <QStringListModel>

class ModelSelect : public QWidget
{
    Q_OBJECT
public:
    explicit ModelSelect(QWidget *parent = nullptr);
    void initUI();

signals:

private:
    QListView *m_leftListView;
    QListView *m_rightListView;
    QPushButton *m_deleteButton;
    QPushButton *m_addButton;
    QPushButton *m_importButton;
    QPushButton *m_exportButton;
    QPushButton *m_selectAllButton;
    QStringListModel *leftModel;
    QStringListModel *rightModel;
    QVector<QString> m_modelTitle;
};

#endif // MODELSELECT_H
