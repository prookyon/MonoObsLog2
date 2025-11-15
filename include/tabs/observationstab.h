#ifndef OBSERVATIONSTAB_H
#define OBSERVATIONSTAB_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class ObservationsTab;
}
QT_END_NAMESPACE

class DatabaseManager;
class ObservationsRepository;
class QStringListModel;
class SettingsManager;

class ObservationsTab : public QWidget
{
    Q_OBJECT

public:
    explicit ObservationsTab(DatabaseManager *dbManager, SettingsManager *settingsManager, QWidget *parent = nullptr);
    ~ObservationsTab();

    void initialize();
    void refreshData();

private slots:
    void onAddObservationButtonClicked();
    void onEditObservationButtonClicked();
    void onDeleteObservationButtonClicked();
    void onFilterSelectionChanged();

private:
    void populateTable();
    void populateComboBoxes();
    void populateObjectFilter();
    bool showObservationDialog(const QString &title, int &sessionId, int &objectId,
                               int &cameraId, int &telescopeId, int &filterId,
                               int &imageCount, int &exposureLength, QString &comments);

    Ui::ObservationsTab *ui;
    DatabaseManager *m_dbManager;
    SettingsManager *m_settingsManager;
    ObservationsRepository *m_repository;
    QStringListModel *m_filterListModel;
    int m_currentFilterObjectId; // -1 for "All Objects"
};

#endif // OBSERVATIONSTAB_H