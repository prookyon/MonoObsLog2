#ifndef CAMERASTAB_H
#define CAMERASTAB_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class CamerasTab;
}
QT_END_NAMESPACE

class DatabaseManager;

class CamerasTab : public QWidget
{
    Q_OBJECT

public:
    explicit CamerasTab(DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~CamerasTab();

    void initialize();
    void refreshData();

private:
    Ui::CamerasTab *ui;
    DatabaseManager *m_dbManager;
};

#endif // CAMERASTAB_H