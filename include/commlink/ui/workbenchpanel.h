#pragma once
#include <QComboBox>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

class WorkbenchPanel : public QWidget {
    Q_OBJECT
public:
    explicit WorkbenchPanel(QWidget* parent = nullptr);
    QJsonObject options() const {
        return settings;
    }
    void setOptions(const QJsonObject& options) {
        settings = options;
        authorization.clear();
    }
    void setProfiles(const QStringList& names);
    QString profileName() const;
    QString authorizationHeader() const {
        return authorization;
    }
    void setReplayState(bool active, const QString& message);
    void setConnectionActive(bool active);
signals:
    void saveRequested(const QString& name);
    void openRequested(const QString& name);
    void optionsChanged();
    void replayRequested(const QJsonObject& configuration);
    void stopReplayRequested();

private:
    void editOptions();
    void editReplay();
    QPushButton *optionsButton, *replayButton, *stopButton;
    QLabel* replayStatus;
    QString authorization;
    QComboBox* profiles;
    QJsonObject settings;
};
