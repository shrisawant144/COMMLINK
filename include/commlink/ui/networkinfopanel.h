#pragma once

#include <QtWidgets/QWidget>

class QTextEdit;
class QPushButton;

/**
 * @brief Panel for displaying local network information and reachability guidance
 *
 * Shows local interface addresses, the currently selected server bind address,
 * and simple guidance for LAN and internet access.
 */
class NetworkInfoPanel : public QWidget
{
    Q_OBJECT

public:
    explicit NetworkInfoPanel(QWidget *parent = nullptr);
    ~NetworkInfoPanel() override = default;

    void updateServerContext(const QString &protocol, const QString &bindAddress, int port);

private slots:
    void refreshNetworkInfo();

private:
    void setupUI();
    void applyStyles();
    void setupAccessibility();
    QString buildNetworkSummary() const;
    QString buildAddressSummary() const;
    QString buildReachabilitySummary() const;
    QString protocolSchemeHint() const;

    QTextEdit *infoEdit;
    QPushButton *refreshBtn;

    QString currentProtocol;
    QString currentBindAddress;
    int currentPort;

    static constexpr int BTN_HEIGHT = 32;
};
