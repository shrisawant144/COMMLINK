#pragma once
#include <QWidget>
#include <QComboBox>
#include <QJsonObject>
#include <QPushButton>

class WorkbenchPanel : public QWidget {
    Q_OBJECT
public:
    explicit WorkbenchPanel(QWidget *parent = nullptr);
    QJsonObject options() const { return settings; }
    void setOptions(const QJsonObject &options) { settings = options; }
    void setProfiles(const QStringList &names);
    QString profileName() const;
signals:
    void saveRequested(const QString &name);
    void openRequested(const QString &name);
    void optionsChanged();
private:
    void editOptions();
    QComboBox *profiles;
    QJsonObject settings;
};
