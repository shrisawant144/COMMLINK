/**
 * @file filemanager.h
 * @brief File operations for saving and loading messages
 * @author CommLink Project
 * @version 1.0.0
 */

#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <string>

/**
 * @class FileManager
 * @brief Manages file I/O operations for message persistence
 * 
 * Handles saving and loading messages in various formats with automatic
 * directory creation and validation.
 */
class FileManager {
public:
    /**
     * @brief Default constructor
     */
    FileManager() = default;

    /**
     * @brief Save data to a file
     * @param filepath Path to the output file
     * @param data Data to save
     * @return true if successful, false otherwise
     */
    bool saveToFile(const std::string& filepath, const std::string& data);

    /**
     * @brief Load data from a file
     * @param filepath Path to the input file
     * @return File contents as string, empty string on error
     */
    std::string loadFromFile(const std::string& filepath);

    /**
     * @brief Check if a file exists
     * @param filepath Path to check
     * @return true if file exists, false otherwise
     */
    bool fileExists(const std::string& filepath);
};

#endif // FILEMANAGER_H
