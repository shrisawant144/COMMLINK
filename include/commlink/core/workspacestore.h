#pragma once
#include <QJsonObject>
#include <QStringList>

class WorkspaceStore {
public:
    explicit WorkspaceStore(const QString &directory = QString());
    QStringList names() const;
    bool save(const QString &name, QJsonObject workspace, QString *error = nullptr) const;
    QJsonObject load(const QString &name, QString *error = nullptr) const;
    static QString expand(const QString &text, const QJsonObject &variables, QString *error = nullptr);
private:
    QString directory;
    QString path(const QString &name) const;
};
