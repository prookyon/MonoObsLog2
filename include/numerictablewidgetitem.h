#ifndef NUMERICTABLEWIDGETITEM_H
#define NUMERICTABLEWIDGETITEM_H

#include <QTableWidgetItem>

class NumericTableWidgetItem : public QTableWidgetItem
{
public:
    NumericTableWidgetItem(const QString &text, int type = Type) : QTableWidgetItem(text, type) {}

    bool operator<(const QTableWidgetItem &other) const override
    {
        bool ok1, ok2;
        double val1 = text().toDouble(&ok1);
        double val2 = other.text().toDouble(&ok2);
        if (ok1 && ok2)
        {
            return val1 < val2;
        }
        else
        {
            // fallback to string comparison
            return text() < other.text();
        }
    }
};

#endif