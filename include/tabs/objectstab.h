#ifndef OBJECTSTAB_H
#define OBJECTSTAB_H

#include <QWidget>
#include <QLineEdit>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class ObjectsTab;
}
QT_END_NAMESPACE

class DatabaseManager;
class SimbadQuery;
class ObjectsRepository;
class SettingsManager;
class QCustomPlot;
class QCPPolarGraph;

class ObjectsTab : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectsTab(DatabaseManager *dbManager, SettingsManager *settingsManager, QWidget *parent = nullptr);
    ~ObjectsTab();

    void initialize();
    void refreshData();

private slots:
    void onAddButtonClicked();
    void onEditButtonClicked();
    void onDeleteButtonClicked();
    void onCoordinatesReceived(double ra, double dec, const QString &objectName);
    void onSimbadError(const QString &error);

private:
    void populateTable();
    bool showObjectDialog(const QString &title, QString &name, QString &ra, QString &dec);

    Ui::ObjectsTab *ui;
    DatabaseManager *m_dbManager;
    SettingsManager *m_settingsManager;
    ObjectsRepository *m_repository;
    SimbadQuery *m_simbadQuery;
    QLineEdit *m_dialogRaEdit;
    QLineEdit *m_dialogDecEdit;
    QCustomPlot *m_plot;
    QCPPolarGraph *m_graph;
};

#endif // OBJECTSTAB_H