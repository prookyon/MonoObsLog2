//
// Created by prook on 28.11.2025.
//

#ifndef MONOOBSLOG_RESULT_H
#define MONOOBSLOG_RESULT_H

#endif //MONOOBSLOG_RESULT_H

#include <QString>

enum class ErrorSeverity {
    Warning,
    Error,
    Critical
};

struct ER {
    ErrorSeverity severity;
    QString errorMessage;

    // Helper for failure
    static ER Warning(const QString &msg) {
        return {ErrorSeverity::Warning, msg};
    }

    static ER Error(const QString &msg) {
        return {ErrorSeverity::Error, msg};
    }

    static ER Critical(const QString &msg) {
        return {ErrorSeverity::Critical, msg};
    }
};