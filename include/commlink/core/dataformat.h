#ifndef DATAFORMAT_H
#define DATAFORMAT_H

#include <QByteArray>
#include <QVariant>
#include <QString>

enum class DataFormatType {
    JSON,
    XML,
    CSV,
    TEXT,
    BINARY,
    HEX
};

/**
 * @brief Container for formatted data messages
 * 
 * @section dataformat_flow Data Format Processing Flow
 * 
 * @subsection send_flow Sending Flow
 * 
 * User Input (String) → parseInput() → QVariant → DataMessage → serialize() → QByteArray → Network
 * 
 * Example for JSON:
 * 1. User types: '{"name": "John"}'
 * 2. parseInput() → QJsonDocument (stored as QVariant)
 * 3. DataMessage(JSON, QJsonDocument)
 * 4. serialize() → '{"name":"John"}' (QByteArray)
 * 5. Sent over network
 * 
 * @subsection receive_flow Receiving Flow
 * 
 * Network (QByteArray) → deserialize() → DataMessage → toDisplayString() → Display
 * 
 * Example for JSON:
 * 1. Receive bytes: '{"name":"John"}'
 * 2. deserialize() → DataMessage with QJsonDocument
 * 3. toDisplayString() → Pretty-printed JSON for display
 * 
 * @see @ref CODE_FLOW.md for complete flow documentation
 */
class DataMessage {
public:
    DataFormatType type;  //!< Format type (JSON, XML, CSV, TEXT, BINARY, HEX)
    QVariant data;        //!< Parsed data (QJsonDocument, QString, QByteArray, etc.)

    /**
     * @brief Constructs a DataMessage
     * @param t Format type
     * @param d Data variant
     */
    DataMessage(DataFormatType t = DataFormatType::TEXT, const QVariant& d = QVariant());
    
    /**
     * @brief Serializes DataMessage to bytes for network transmission
     * 
     * @return QByteArray containing serialized data
     * 
     * @flow
     * Converts internal data (QVariant) to bytes based on format:
     * - JSON: QJsonDocument::toJson()
     * - XML: QString::toUtf8()
     * - CSV: QString::toUtf8()
     * - TEXT: QString::toUtf8()
     * - BINARY: QByteArray (as-is)
     * - HEX: QByteArray::toHex()
     * 
     * @note Called by network components before sending
     */
    QByteArray serialize() const;
    
    /**
     * @brief Deserializes bytes to DataMessage
     * 
     * @param bytes Raw bytes from network
     * @param type Expected data format type
     * @return DataMessage with deserialized data
     * 
     * @flow
     * Converts bytes to internal data format:
     * - JSON: QJsonDocument::fromJson()
     * - XML: QString::fromUtf8()
     * - CSV: QString::fromUtf8()
     * - TEXT: QString::fromUtf8()
     * - BINARY: QByteArray (as-is)
     * - HEX: QByteArray::fromHex()
     * 
     * @note Called by network components after receiving
     */
    static DataMessage deserialize(const QByteArray& bytes, DataFormatType type);
    
    /**
     * @brief Converts DataMessage to display string
     * 
     * @return Formatted string for UI display
     * 
     * @flow
     * Formats data for display:
     * - JSON: Pretty-printed (indented)
     * - XML: As-is
     * - CSV: As-is
     * - TEXT: As-is
     * - BINARY: Hex representation with size
     * - HEX: Hex string
     */
    QString toDisplayString() const;
    
    /**
     * @brief Validates input string for given format
     * 
     * @param input Input string to validate
     * @param type Format type to validate against
     * @return true if valid, false otherwise
     * 
     * @flow
     * Validates format-specific syntax:
     * - JSON: Valid JSON syntax
     * - XML: Non-empty string
     * - CSV: Non-empty string
     * - TEXT: Always valid
     * - HEX: Only hex characters (0-9, A-F)
     */
    static bool validateInput(const QString& input, DataFormatType type);
    
    /**
     * @brief Parses input string to QVariant based on format
     * 
     * @param input Input string
     * @param type Format type
     * @return Parsed data as QVariant
     * 
     * @flow
     * Parses string to appropriate type:
     * - JSON: QJsonDocument
     * - XML: QString
     * - CSV: QString
     * - TEXT: QString
     * - BINARY: QByteArray (from hex)
     * - HEX: QByteArray (from hex)
     */
    static QVariant parseInput(const QString& input, DataFormatType type);
};

Q_DECLARE_METATYPE(DataMessage)

#endif // DATAFORMAT_H
