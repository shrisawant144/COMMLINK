#include "startupenvironment.h"

#include <QFileInfo>

namespace StartupEnvironment {
bool needsHostSettingsSchemas(const QProcessEnvironment& environment, const QString& executable) {
    const QString snapRoot = environment.value("SNAP");
    if (executable.isEmpty() || executable.startsWith("/snap/") ||
        executable.startsWith("/var/lib/snapd/snap/") ||
        (!snapRoot.isEmpty() && executable.startsWith(snapRoot + '/'))) {
        return false;
    }

    // Snap terminals can retain these paths after SNAP and SNAP_NAME are unset.
    // An explicit schema directory takes precedence over XDG_DATA_DIRS in GLib.
    const QString schemaDirectory = environment.value("GSETTINGS_SCHEMA_DIR");
    if (!schemaDirectory.isEmpty()) {
        return schemaDirectory.contains("/snap/");
    }
    return environment.value("XDG_DATA_DIRS").contains("/snap/");
}

void prepareDesktopEnvironment() {
#ifdef Q_OS_LINUX
    const QString executable = QFileInfo("/proc/self/exe").canonicalFilePath();
    if (needsHostSettingsSchemas(QProcessEnvironment::systemEnvironment(), executable) &&
        QFileInfo::exists("/usr/share/glib-2.0/schemas/gschemas.compiled")) {
        qputenv("GSETTINGS_SCHEMA_DIR", "/usr/share/glib-2.0/schemas");
    }
#endif
}
} // namespace StartupEnvironment
