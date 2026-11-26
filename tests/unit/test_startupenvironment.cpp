#include "startupenvironment.h"

#include <QDebug>

int main() {
    const QString nativeExecutable = "/home/user/COMMLINK/build/bin/commlink";
    bool passed = true;
    auto check = [&passed](const QProcessEnvironment& environment, const QString& executable,
                           bool expected, const char* description) {
        if (StartupEnvironment::needsHostSettingsSchemas(environment, executable) != expected) {
            qCritical() << description;
            passed = false;
        }
    };

    QProcessEnvironment environment;
    environment.insert("GSETTINGS_SCHEMA_DIR", "/home/user/snap/code/281/schemas");
    environment.insert("XDG_DATA_DIRS", "/snap/code/281/usr/share:/usr/share");
    check(environment, nativeExecutable, true, "Snap schema paths without SNAP must be handled");

    environment.insert("SNAP", "/snap/code/281");
    check(environment, nativeExecutable, true, "Inherited Snap environment must be handled");
    check(environment, "/snap/code/281/bin/commlink", false,
          "Snap apps must retain bundled schemas");
    environment.remove("SNAP");
    check(environment, "/snap/commlink/1/bin/commlink", false,
          "Snap apps must retain bundled schemas even without SNAP");
    check(environment, "/var/lib/snapd/snap/commlink/1/bin/commlink", false,
          "Alternate Snap mounts must retain bundled schemas");

    environment.remove("GSETTINGS_SCHEMA_DIR");
    check(environment, nativeExecutable, true, "Snap paths in XDG_DATA_DIRS must also be handled");
    environment.insert("GSETTINGS_SCHEMA_DIR", "/usr/share/glib-2.0/schemas");
    check(environment, nativeExecutable, false, "Host schemas must remain unchanged");
    environment.insert("GSETTINGS_SCHEMA_DIR", "/opt/custom/schemas");
    check(environment, nativeExecutable, false, "Explicit custom schemas must remain unchanged");

    environment.remove("GSETTINGS_SCHEMA_DIR");
    environment.insert("XDG_DATA_DIRS", "/usr/local/share:/usr/share:/var/lib/snapd/desktop");
    check(environment, nativeExecutable, false, "Normal desktop data paths need no override");
    check(QProcessEnvironment(), nativeExecutable, false, "Clean terminals need no override");
    check(environment, QString(), false, "Unknown executable paths need no override");

    if (passed) {
        qInfo() << "Startup environment checks passed.";
    }
    return passed ? 0 : 1;
}
