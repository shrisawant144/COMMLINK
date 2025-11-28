#include "../include/filemanager.h"
#include <cassert>
#include <iostream>
#include <fstream>

void testSaveAndLoadJson() {
    FileManager fm;
    std::string testFile = "/tmp/test_commlink.json";
    std::string testData = R"({"test":"data"})";
    
    bool saved = fm.saveToFile(testFile, testData);
    assert(saved);
    
    std::string loaded = fm.loadFromFile(testFile);
    assert(loaded == testData);
    
    std::remove(testFile.c_str());
    std::cout << "✓ Save and load JSON test passed\n";
}

void testInvalidPath() {
    FileManager fm;
    std::string loaded = fm.loadFromFile("/invalid/path/file.json");
    assert(loaded.empty());
    std::cout << "✓ Invalid path handling test passed\n";
}

int main() {
    std::cout << "Running FileManager tests...\n";
    testSaveAndLoadJson();
    testInvalidPath();
    std::cout << "All tests passed!\n";
    return 0;
}
