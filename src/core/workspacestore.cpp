#include "commlink/core/workspacestore.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>

WorkspaceStore::WorkspaceStore(const QString& directory)
    : directory(directory.isEmpty()
                    ? QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                          "/workspaces"
                    : directory) {}
QString WorkspaceStore::path(const QString& name) const {
    static const QRegularExpression valid("^[A-Za-z0-9][A-Za-z0-9 _-]{0,63}$");
    return valid.match(name).hasMatch() ? QDir(directory).filePath(name + ".json") : QString();
}
QStringList WorkspaceStore::names() const {
    QStringList result;
    for (const auto& file : QDir(directory).entryList({"*.json"}, QDir::Files, QDir::Name))
        result << file.left(file.size() - 5);
    return result;
}
bool WorkspaceStore::save(const QString& name, QJsonObject workspace, QString* error) const {
    if (error)
        error->clear();
    if (path(name).isEmpty()) {
        if (error)
            *error = "Use 1–64 letters, numbers, spaces, underscores or hyphens; start with a "
                     "letter or number.";
        return false;
    }
    if (!QDir().mkpath(directory)) {
        if (error)
            *error = "Cannot create workspace directory";
        return false;
    }
    workspace["version"] = 1;
    QSaveFile file(path(name));
    if (!file.open(QIODevice::WriteOnly)) {
        if (error)
            *error = file.errorString();
        return false;
    }
    file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    const auto bytes = QJsonDocument(workspace).toJson();
    if (file.write(bytes) != bytes.size() || !file.commit()) {
        if (error)
            *error = file.errorString();
        return false;
    }
    return true;
}
QJsonObject WorkspaceStore::load(const QString& name, QString* error) const {
    if (error)
        error->clear();
    QFile file(path(name));
    if (path(name).isEmpty() || !file.open(QIODevice::ReadOnly) || file.size() > 8 * 1024 * 1024) {
        if (error)
            *error = "Cannot read workspace (maximum size 8 MiB)";
        return {};
    }
    QJsonParseError parse;
    const auto doc = QJsonDocument::fromJson(file.readAll(), &parse);
    if (parse.error != QJsonParseError::NoError || !doc.isObject() ||
        doc.object()["version"].toInt() != 1) {
        if (error)
            *error = "Invalid or unsupported workspace version";
        return {};
    }
    return doc.object();
}
QString WorkspaceStore::expand(const QString& text, const QJsonObject& variables, QString* error) {
    if (error)
        error->clear();
    static const QRegularExpression pattern("\\$\\{([A-Za-z_][A-Za-z0-9_]*)\\}");
    QString result;
    int previous = 0;
    auto matches = pattern.globalMatch(text);
    while (matches.hasNext()) {
        const auto match = matches.next();
        const auto key = match.captured(1);
        if (!variables.contains(key) || !variables[key].isString()) {
            if (error)
                *error = "Undefined workspace variable: " + key;
            return {};
        }
        result += text.mid(previous, match.capturedStart() - previous) + variables[key].toString();
        previous = match.capturedEnd();
    }
    return result + text.mid(previous);
}
