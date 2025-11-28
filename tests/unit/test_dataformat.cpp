#include "../include/dataformat.h"
#include <cassert>
#include <iostream>

void testJsonSerialization() {
    DataFormat df(DataFormat::Format::JSON);
    std::string input = R"({"key":"value","number":42})";
    std::string serialized = df.serialize(input);
    assert(!serialized.empty());
    std::cout << "✓ JSON serialization test passed\n";
}

void testXmlSerialization() {
    DataFormat df(DataFormat::Format::XML);
    std::string input = "<root><key>value</key></root>";
    std::string serialized = df.serialize(input);
    assert(!serialized.empty());
    std::cout << "✓ XML serialization test passed\n";
}

void testCsvSerialization() {
    DataFormat df(DataFormat::Format::CSV);
    std::string input = "name,age\nJohn,30";
    std::string serialized = df.serialize(input);
    assert(!serialized.empty());
    std::cout << "✓ CSV serialization test passed\n";
}

int main() {
    std::cout << "Running DataFormat tests...\n";
    testJsonSerialization();
    testXmlSerialization();
    testCsvSerialization();
    std::cout << "All tests passed!\n";
    return 0;
}
