#ifndef OBSERVATIONSTAB_H
#define OBSERVATIONSTAB_H

#include <QWidget>
#include <QValidator>
#include <QComboBox>

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
class QMenu;

class ObservationsTab : public QWidget
{
    Q_OBJECT

public:
    explicit ObservationsTab(DatabaseManager *dbManager, SettingsManager *settingsManager, QWidget *parent = nullptr);
    ~ObservationsTab() override;

    void initialize();
    void refreshData();

private slots:
    void onAddObservationButtonClicked();
    void onEditObservationButtonClicked();
    void onDeleteObservationButtonClicked();
    void onFilterSelectionChanged();
    void onExportToHtmlClicked();
    void onExportToExcelClicked();
    void onExploreSessionsFolder();

private:
    void populateTable();
    void populateComboBoxes() const;
    void populateObjectFilter() const;
    bool showObservationDialog(const QString &title, int &sessionId, int &objectId,
                               int &cameraId, int &telescopeId, int &filterId,
                               int &imageCount, int &exposureLength, QString &comments);

    Ui::ObservationsTab *ui;
    DatabaseManager *m_dbManager;
    SettingsManager *m_settingsManager;
    ObservationsRepository *m_repository;
    QStringListModel *m_filterListModel;
    QMenu *m_exportMenu;
    QMenu *m_rightClickMenu{};
    int m_currentFilterObjectId; // -1 for "All Objects"
};

class ComboBoxItemValidator : public QValidator
{
    Q_OBJECT

public:
    explicit ComboBoxItemValidator(QComboBox* combo, QObject* parent = nullptr)
        : QValidator(parent), m_combo(combo) {}

    State validate(QString& input, int& pos) const override
    {
        Q_UNUSED(pos);

        if (input.isEmpty()) {
            return Intermediate;
        }

        // 2. Check for an Exact Match
        // Qt::MatchFixedString ensures strict matching, CaseInsensitive is usually better for UX
        int index = m_combo->findText(input, Qt::MatchFixedString);

        // If found exactly, it is valid
        if (index != -1) {
            return Acceptable;
        }

        // 3. Check for Partial Match (Intermediate)
        // We loop through items to see if the input is a prefix of any item
        for (int i = 0; i < m_combo->count(); ++i) {
            QString itemText = m_combo->itemText(i);

            // Check if item starts with input (Case Sensitive for strictness)
            if (itemText.startsWith(input, Qt::CaseInsensitive)) {
                return Intermediate;
            }
        }

        // 4. No match found at all
        return Invalid;
    }

    void fixup(QString& input) const override
    {
        // auto-correct to the first matching entry
        for (int i = 0; i < m_combo->count(); ++i) {
            const QString item = m_combo->itemText(i);
            if (item.startsWith(input, Qt::CaseInsensitive)) {
                input = item;
                return;
            }
        }
    }

private:
    QComboBox* m_combo;
};

#endif // OBSERVATIONSTAB_H