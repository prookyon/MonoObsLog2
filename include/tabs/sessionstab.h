#ifndef SESSIONSTAB_H
#define SESSIONSTAB_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class SessionsTab;
}
QT_END_NAMESPACE

class DatabaseManager;
class SessionsRepository;

class SessionsTab : public QWidget
{
    Q_OBJECT

public:
    explicit SessionsTab(DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~SessionsTab();

    void initialize();
    void refreshData();

private slots:
    void onAddButtonClicked();
    void onEditButtonClicked();
    void onDeleteButtonClicked();

private:
    void populateTable();
    bool showSessionDialog(const QString &title, QString &name, QString &startDate, QString &comments);

    Ui::SessionsTab *ui;
    DatabaseManager *m_dbManager;
    SessionsRepository *m_repository;
};

#endif // SESSIONSTAB_H