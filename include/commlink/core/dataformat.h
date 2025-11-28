/**
 * @file dataformat.h
 * @brief Multi-format message serialization and deserialization
 * @author CommLink Project
 * @version 1.0.0
 */

#ifndef DATAFORMAT_H
#define DATAFORMAT_H

#include <string>

/**
 * @class DataFormat
 * @brief Handles serialization/deserialization of messages in multiple formats
 * 
 * Supports JSON, XML, CSV, TEXT, BINARY, and HEX formats for network communication.
 */
class DataFormat {
public:
    /**
     * @enum Format
     * @brief Supported data formats for message serialization
     */
    enum class Format {
        JSON,   ///< JavaScript Object Notation
        XML,    ///< Extensible Markup Language
        CSV,    ///< Comma-Separated Values
        TEXT,   ///< Plain text
        BINARY, ///< Binary data
        HEX     ///< Hexadecimal representation
    };

    /**
     * @brief Construct a DataFormat handler with specified format
     * @param format The format to use for serialization/deserialization
     */
    explicit DataFormat(Format format);

    /**
     * @brief Serialize data into the configured format
     * @param data Input data to serialize
     * @return Serialized string representation
     */
    std::string serialize(const std::string& data);

    /**
     * @brief Deserialize data from the configured format
     * @param data Serialized data to deserialize
     * @return Deserialized string representation
     */
    std::string deserialize(const std::string& data);

    /**
     * @brief Validate if data conforms to the configured format
     * @param data Data to validate
     * @return true if valid, false otherwise
     */
    bool validate(const std::string& data);

    /**
     * @brief Get the current format
     * @return Current Format enum value
     */
    Format getFormat() const { return m_format; }

private:
    Format m_format; ///< Current serialization format
};

#endif // DATAFORMAT_H
