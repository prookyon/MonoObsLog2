#ifndef OBJECTSTAB_H
#define OBJECTSTAB_H

#include <QWidget>
#include <QLineEdit>
#include <qwt/qwt_polar_plot.h>
#include <qwt/qwt_polar_marker.h>
#include <qwt/qwt_polar_panner.h>
#include <qwt/qwt_polar_magnifier.h>
#include <qwt/qwt_polar_curve.h>

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

class ObjectsTab : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectsTab(DatabaseManager *dbManager, SettingsManager *settingsManager, QWidget *parent = nullptr);
    ~ObjectsTab() override;

    void initialize();
    void refreshData();

private slots:
    void onAddButtonClicked();
    void onEditButtonClicked();
    void onDeleteButtonClicked();
    void onCoordinatesReceived(double ra, double dec, const QString &objectName) const;
    static void onSimbadError(const QString &error);

private:
    struct ConstellationLine {
        QString name;
        double ra1, dec1, ra2, dec2;
    };

    void populateTable();
    bool showObjectDialog(const QString &title, QString &name, QString &ra, QString &dec, QString &comments);
    void LoadConstellations();

    Ui::ObjectsTab *ui;
    DatabaseManager *m_dbManager;
    SettingsManager *m_settingsManager;
    ObjectsRepository *m_repository;
    SimbadQuery *m_simbadQuery;
    QLineEdit *m_dialogRaEdit;
    QLineEdit *m_dialogDecEdit;
    QwtPolarPanner* m_panner{};
    QwtPolarMagnifier* m_zoomer{};
    QVector<ConstellationLine> m_constellations;
};

#endif // OBJECTSTAB_H