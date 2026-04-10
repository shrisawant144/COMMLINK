#pragma once
#include <QtWidgets/QWidget>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QLabel>
#include <QtCore/QString>
#include <QtNetwork/QHostAddress>

/**
 * @brief Panel for managing server configuration and connections
 * 
 * Handles server protocol selection, port configuration, client management,
 * and broadcast/targeted message sending for TCP, UDP, WebSocket, and HTTP servers.
 */
class ServerPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ServerPanel(QWidget *parent = nullptr);
    ~ServerPanel() override = default;

    // Getters
    QString getProtocol() const;
    int getPort() const;
    QHostAddress getBindAddress() const;
    QString getBindAddressText() const;
    bool isServerRunning() const;

    // Setters
    void setServerState(bool running);
    void setProtocol(const QString &protocol);
    void setPort(int port);
    void setBindAddress(const QString &bindAddress);

    // Client management
    void addClient(const QString &clientInfo);
    void removeClient(const QString &clientInfo);
    void clearClients();
    int getClientCount() const;
    QString getClientAt(int index) const;
    
    // Send mode controls
    QString getSendMode() const;
    QString getTargetClient() const;

signals:
    void startServerRequested();
    void stopServerRequested();
    void protocolChanged(const QString &protocol);
    void sendModeChanged(const QString &mode);
    void configurationChanged();

private slots:
    void onStartClicked();
    void onStopClicked();
    void onProtocolChanged(int index);

private:
    void setupUI();
    void updateClientCount();
    void applyStyles();
    void setupAccessibility();
    void populateBindAddresses();

    // UI Components
    QComboBox *protocolCombo;
    QComboBox *bindAddressCombo;
    QLineEdit *portEdit;
    QPushButton *startBtn;
    QPushButton *stopBtn;
    QListWidget *clientsList;
    QLabel *clientCountLabel;
    QComboBox *sendModeCombo;
    QComboBox *targetClientCombo;

    // State
    bool serverRunning;

    // Constants
    static constexpr int MIN_HEIGHT = 32;
    static constexpr int BTN_HEIGHT = 36;
};
