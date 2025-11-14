#ifndef TELESCOPESTAB_H
#define TELESCOPESTAB_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class TelescopesTab;
}
QT_END_NAMESPACE

class DatabaseManager;
class TelescopesRepository;

class TelescopesTab : public QWidget
{
    Q_OBJECT

public:
    explicit TelescopesTab(DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~TelescopesTab();

    void initialize();
    void refreshData();

private slots:
    void onAddTelescopeButtonClicked();
    void onEditTelescopeButtonClicked();
    void onDeleteTelescopeButtonClicked();
    void onApertureOrFocalLengthChanged();

private:
    void populateTable();
    bool showTelescopeDialog(const QString &title, QString &name, int &aperture, double &fRatio, int &focalLength);

    Ui::TelescopesTab *ui;
    DatabaseManager *m_dbManager;
    TelescopesRepository *m_repository;
};

#endif // TELESCOPESTAB_H