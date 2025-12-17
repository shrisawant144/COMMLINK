#include "commlink/ui/messagepanel.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

MessagePanel::MessagePanel(QWidget *parent)
    : QWidget(parent)
    , formatCombo(nullptr)
    , messageEdit(nullptr)
    , sendBtn(nullptr)
    , loadBtn(nullptr)
    , saveBtn(nullptr)
    , clearBtn(nullptr)
{
    setupUI();
    applyStyles();
    setupAccessibility();
}

void MessagePanel::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto *group = new QGroupBox("Message Composition");
    auto *layout = new QVBoxLayout(group);

    // Format selection
    auto *formatLayout = new QHBoxLayout();
    formatLayout->addWidget(new QLabel("Format:"));
    
    formatCombo = new QComboBox();
    formatCombo->addItems({"JSON", "XML", "CSV", "Text", "Binary", "Hex"});
    formatCombo->setMinimumHeight(MIN_HEIGHT);
    formatCombo->setToolTip(
        "JSON: Structured data with key-value pairs\n"
        "XML: Tagged hierarchical data\n"
        "CSV: Comma-separated tabular data\n"
        "Text: Plain text messages\n"
        "Binary: Raw binary data (hex encoded)\n"
        "Hex: Hexadecimal representation"
    );
    connect(formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MessagePanel::onFormatChanged);
    
    formatLayout->addWidget(formatCombo, 1);
    layout->addLayout(formatLayout);

    // Message editor
    messageEdit = new QTextEdit();
    messageEdit->setMinimumHeight(EDIT_MIN_HEIGHT);
    messageEdit->setPlaceholderText("Enter your message here...");
    messageEdit->setAcceptRichText(false);
    messageEdit->setToolTip("Enter message content to send. Format depends on selected data format.");
    layout->addWidget(messageEdit);

    // Buttons
    auto *btnLayout = new QHBoxLayout();
    
    loadBtn = new QPushButton("Load");
    loadBtn->setMinimumHeight(MIN_HEIGHT);
    loadBtn->setToolTip("Load message from file (Ctrl+O)");
    connect(loadBtn, &QPushButton::clicked, this, &MessagePanel::loadMessageRequested);
    
    saveBtn = new QPushButton("Save");
    saveBtn->setMinimumHeight(MIN_HEIGHT);
    saveBtn->setToolTip("Save message to file (Ctrl+S)");
    connect(saveBtn, &QPushButton::clicked, this, &MessagePanel::saveMessageRequested);
    
    clearBtn = new QPushButton("Clear");
    clearBtn->setMinimumHeight(MIN_HEIGHT);
    clearBtn->setToolTip("Clear message content");
    connect(clearBtn, &QPushButton::clicked, messageEdit, &QTextEdit::clear);
    
    sendBtn = new QPushButton("Send Message");
    sendBtn->setMinimumHeight(BTN_HEIGHT);
    sendBtn->setToolTip("Send message to connected peer (Ctrl+Return)");
    connect(sendBtn, &QPushButton::clicked, this, &MessagePanel::onSendClicked);
    
    btnLayout->addWidget(loadBtn);
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(clearBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(sendBtn);
    
    layout->addLayout(btnLayout);

    mainLayout->addWidget(group);
}

void MessagePanel::applyStyles()
{
    sendBtn->setStyleSheet(
        "QPushButton { "
        "font-weight: bold; "
        "background-color: #007bff; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 8px 16px; "
        "}"
        "QPushButton:hover { background-color: #0056b3; }"
        "QPushButton:pressed { background-color: #004085; }"
        "QPushButton:disabled { background-color: #6c757d; }"
    );

    loadBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #6c757d; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 6px 12px; "
        "}"
        "QPushButton:hover { background-color: #5a6268; }"
    );

    saveBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #6c757d; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 6px 12px; "
        "}"
        "QPushButton:hover { background-color: #5a6268; }"
    );

    clearBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #ffc107; "
        "color: #212529; "
        "border: none; "
        "border-radius: 4px; "
        "padding: 6px 12px; "
        "}"
        "QPushButton:hover { background-color: #e0a800; }"
    );
}

void MessagePanel::onFormatChanged(int index)
{
    Q_UNUSED(index);
    emit formatChanged(getDataFormat());
}

void MessagePanel::onSendClicked()
{
    emit sendRequested();
}

// Getters
QString MessagePanel::getMessage() const
{
    return messageEdit->toPlainText();
}

QString MessagePanel::getDataFormat() const
{
    return formatCombo->currentText();
}

DataFormatType MessagePanel::getFormat() const
{
    return stringToFormat(formatCombo->currentText());
}

// Setters
void MessagePanel::setMessage(const QString &message)
{
    messageEdit->setPlainText(message);
}

void MessagePanel::setDataFormat(const QString &format)
{
    int index = formatCombo->findText(format);
    if (index >= 0) {
        formatCombo->setCurrentIndex(index);
    }
}

void MessagePanel::clearMessage()
{
    messageEdit->clear();
}

void MessagePanel::setSendButtonEnabled(bool enabled)
{
    sendBtn->setEnabled(enabled);
}

// Helper
DataFormatType MessagePanel::stringToFormat(const QString &formatStr) const
{
    if (formatStr == "JSON") return DataFormatType::JSON;
    if (formatStr == "XML") return DataFormatType::XML;
    if (formatStr == "CSV") return DataFormatType::CSV;
    if (formatStr == "Text") return DataFormatType::TEXT;
    if (formatStr == "Binary") return DataFormatType::BINARY;
    if (formatStr == "Hex") return DataFormatType::HEX;
    return DataFormatType::TEXT;
}

void MessagePanel::setupAccessibility()
{
    // Format combo
    formatCombo->setAccessibleName("Message Format Selection");
    formatCombo->setAccessibleDescription("Select the format for the message: JSON, XML, CSV, Text, Binary, or Hex");
    
    // Message edit
    messageEdit->setAccessibleName("Message Content");
    messageEdit->setAccessibleDescription("Enter or edit the message content to send");
    
    // Buttons
    sendBtn->setAccessibleName("Send Message");
    sendBtn->setAccessibleDescription("Send the message with the selected format and destination");
    
    loadBtn->setAccessibleName("Load Message");
    loadBtn->setAccessibleDescription("Load a message from a file");
    
    saveBtn->setAccessibleName("Save Message");
    saveBtn->setAccessibleDescription("Save the current message to a file");
    
    clearBtn->setAccessibleName("Clear Message");
    clearBtn->setAccessibleDescription("Clear the message content area");
}
