#pragma once
#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStatusBar>
#include <QtCore/QString>

/**
 * @brief Panel for displaying connection status and application state
 * 
 * Shows real-time status information about client and server connections,
 * protocol information, and other relevant application state.
 */
class StatusPanel : public QWidget
{
    Q_OBJECT

public:
    explicit StatusPanel(QWidget *parent = nullptr);
    ~StatusPanel() override = default;

    // Status updates
    void setClientStatus(const QString &status, bool isConnected);
    void setServerStatus(const QString &status, bool isRunning);
    void setProtocolInfo(const QString &clientProtocol, const QString &serverProtocol);
    void setClientCount(int count);
    void setStatusMessage(const QString &message);

private:
    void setupUI();
    void applyStyles();
    void setupAccessibility();

    // UI Components
    QLabel *clientStatusLabel;
    QLabel *serverStatusLabel;
    QLabel *clientProtocolLabel;
    QLabel *serverProtocolLabel;
    QLabel *clientCountLabel;
    QStatusBar *statusBar;
};
