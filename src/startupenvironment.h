#pragma once

#include <QProcessEnvironment>
#include <QString>

namespace StartupEnvironment {
bool needsHostSettingsSchemas(const QProcessEnvironment& environment, const QString& executable);
void prepareDesktopEnvironment();
} // namespace StartupEnvironment
