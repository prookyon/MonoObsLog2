#ifndef FILTERSTAB_H
#define FILTERSTAB_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class FiltersTab;
}
QT_END_NAMESPACE

class DatabaseManager;
class FiltersRepository;
class FilterTypesRepository;

class FiltersTab : public QWidget
{
    Q_OBJECT

public:
    explicit FiltersTab(DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~FiltersTab() override;

    void initialize();
    void refreshData();

private slots:
    void onAddFilterButtonClicked();
    void onEditFilterButtonClicked();
    void onDeleteFilterButtonClicked();

private:
    void populateTable();
    void populateFilterTypeComboBox();
    bool showFilterDialog(const QString &title, QString &name, int &filterTypeId);

    Ui::FiltersTab *ui;
    DatabaseManager *m_dbManager;
    FiltersRepository *m_repository;
    FilterTypesRepository *m_filterTypesRepository;
};

#endif // FILTERSTAB_H