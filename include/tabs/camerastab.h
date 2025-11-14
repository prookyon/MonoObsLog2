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
class CamerasRepository;

class CamerasTab : public QWidget
{
    Q_OBJECT

public:
    explicit CamerasTab(DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~CamerasTab();

    void initialize();
    void refreshData();

private slots:
    void onAddCameraButtonClicked();
    void onEditCameraButtonClicked();
    void onDeleteCameraButtonClicked();

private:
    void populateTable();
    bool showCameraDialog(const QString &title, QString &name, QString &sensor, double &pixelSize, int &width, int &height);

    Ui::CamerasTab *ui;
    DatabaseManager *m_dbManager;
    CamerasRepository *m_repository;
};

#endif // CAMERASTAB_H