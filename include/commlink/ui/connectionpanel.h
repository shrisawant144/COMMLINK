#pragma once
#include <QtWidgets/QWidget>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLabel>
#include <QtCore/QString>

/**
 * @brief Panel for managing client connections
 * 
 * Handles protocol selection, host/port configuration, and connection management
 * for TCP, UDP, WebSocket, and HTTP client connections.
 */
class ConnectionPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionPanel(QWidget *parent = nullptr);
    ~ConnectionPanel() override = default;

    // Getters
    QString getProtocol() const;
    QString getHost() const;
    int getPort() const;
    QString getHttpMethod() const;
    bool isHttpPollingEnabled() const;
    bool isConnected() const;

    // Setters
    void setConnectionState(bool connected);
    void setProtocol(const QString &protocol);
    void setHost(const QString &host);
    void setPort(int port);

signals:
    void connectRequested();
    void disconnectRequested();
    void protocolChanged(const QString &protocol);
    void httpPollingToggled(bool enabled);

private slots:
    void onProtocolChanged(int index);
    void onConnectClicked();

private:
    void setupUI();
    void updateFieldVisibility();
    void applyStyles();
    void setupAccessibility();

    // UI Components
    QComboBox *protocolCombo;
    QComboBox *httpMethodCombo;
    QLineEdit *hostEdit;
    QLineEdit *portEdit;
    QPushButton *connectBtn;
    QCheckBox *httpPollingCheckbox;
    QLabel *infoLabel;

    // State
    bool connected;

    // Constants
    static constexpr int MIN_HEIGHT = 32;
    static constexpr int BTN_HEIGHT = 36;
};
