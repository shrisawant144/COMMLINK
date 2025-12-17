#pragma once
#include <QtWidgets/QWidget>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QPushButton>
#include <QtCore/QString>
#include "../core/dataformat.h"

/**
 * @brief Panel for message composition and data format management
 * 
 * Provides UI for composing messages in various formats (JSON, XML, CSV, Text, Binary, Hex)
 * and managing message file operations (load/save).
 */
class MessagePanel : public QWidget
{
    Q_OBJECT

public:
    explicit MessagePanel(QWidget *parent = nullptr);
    ~MessagePanel() override = default;

    // Getters
    QString getMessage() const;
    QString getDataFormat() const;
    DataFormatType getFormat() const;

    // Setters
    void setMessage(const QString &message);
    void setDataFormat(const QString &format);
    void clearMessage();

    // UI state
    void setSendButtonEnabled(bool enabled);

signals:
    void sendRequested();
    void formatChanged(const QString &format);
    void loadMessageRequested();
    void saveMessageRequested();

private slots:
    void onFormatChanged(int index);
    void onSendClicked();

private:
    void setupUI();
    void applyStyles();
    void setupAccessibility();
    DataFormatType stringToFormat(const QString &formatStr) const;

    // UI Components
    QComboBox *formatCombo;
    QTextEdit *messageEdit;
    QPushButton *sendBtn;
    QPushButton *loadBtn;
    QPushButton *saveBtn;
    QPushButton *clearBtn;

    // Constants
    static constexpr int MIN_HEIGHT = 32;
    static constexpr int BTN_HEIGHT = 36;
    static constexpr int EDIT_MIN_HEIGHT = 150;
};
