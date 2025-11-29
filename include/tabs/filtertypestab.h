#ifndef FILTERTYPESTAB_H
#define FILTERTYPESTAB_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class FilterTypesTab;
}
QT_END_NAMESPACE

class DatabaseManager;
class FilterTypesRepository;

class FilterTypesTab : public QWidget
{
    Q_OBJECT

public:
    explicit FilterTypesTab(DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~FilterTypesTab() override;

    void initialize();
    void refreshData();

private slots:
    void onAddFilterTypeButtonClicked();
    void onEditFilterTypeButtonClicked();
    void onDeleteFilterTypeButtonClicked();

private:
    void populateTable();
    bool showFilterTypeDialog(const QString &title, QString &name, int &priority);

    Ui::FilterTypesTab *ui;
    DatabaseManager *m_dbManager;
    FilterTypesRepository *m_repository;
};

#endif // FILTERTYPESTAB_H