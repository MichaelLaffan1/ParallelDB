#include <mpi.h>
#include <iostream>
#include <fstream>

// Define constants
const int MAX_ATTR_LENGTH = 100;
const int MAX_TUPLES = 20000000;
const int MAX_RESULT_LENGTH = 256;
const int MAX_COMMAND_LENGTH = 256;
const int MAX_COLUMNS = 10;  
const int MAX_COLUMN_NAME = 20;

// Custom string length function
int safeStringLength(const char* str, int maxLen) {
    int len = 0;
    while (str[len] != '\0' && len < maxLen) {
        len++;
    }
    return len;
}

// Custom string comparison function
bool safeCompareStrings(const char* str1, const char* str2, int maxLen) {
    int i = 0;
    while (i < maxLen - 1 && str1[i] != '\0' && str2[i] != '\0') {
        if (str1[i] != str2[i]) return false;
        i++;
    }
    return str1[i] == str2[i];
}

// Custom string copy function
void safeCopyString(char* dest, const char* src, int maxLen) {
    int i = 0;
    while (src[i] != '\0' && i < maxLen - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

// Custom string matching function with wildcard support
bool matchesPattern(const char* str, const char* pattern, int maxLen) {
    // Wildcard matching
    if (pattern[0] == '*' && pattern[1] == '\0') return true;

    int strLen = safeStringLength(str, maxLen);
    int patternLen = safeStringLength(pattern, maxLen);

    // Check for prefix wildcard
    if (pattern[patternLen - 1] == '*') {
        if (patternLen - 1 > strLen) return false;
        for (int i = 0; i < patternLen - 1; i++) {
            if (str[i] != pattern[i]) return false;
        }
        return true;
    }

    // Check for exact match
    return safeCompareStrings(str, pattern, maxLen);
}

class SelectQuery {
public:
    char selectedColumns[MAX_COLUMNS][MAX_COLUMN_NAME];
    int selectedColumnCount;
    char attr1Condition[MAX_ATTR_LENGTH];
    char attr2Condition[MAX_ATTR_LENGTH];
    int attr3Condition;

    SelectQuery() {
        selectedColumnCount = 0;
        attr1Condition[0] = '\0';
        attr2Condition[0] = '\0';
        attr3Condition = -1;
    }

    void addSelectedColumn(const char* columnName) {
        if (selectedColumnCount < MAX_COLUMNS) {
            safeCopyString(selectedColumns[selectedColumnCount], columnName, MAX_COLUMN_NAME);
            selectedColumnCount++;
        }
    }

    bool isColumnSelected(const char* columnName) const {
        if (selectedColumnCount == 0) return true;  // If no specific columns, return all

        for (int i = 0; i < selectedColumnCount; ++i) {
            if (safeCompareStrings(selectedColumns[i], columnName, MAX_COLUMN_NAME)) {
                return true;
            }
        }
        return false;
    }
};

void parseSelectQuery(const char* line, SelectQuery& query) {
    int pos = 0;
    // Reset query
    query.selectedColumnCount = 0;
    query.attr1Condition[0] = '\0';
    query.attr2Condition[0] = '\0';
    query.attr3Condition = -1;

    // Parse selected columns
    while (line[pos] != '\0' && line[pos] != 'F' && line[pos] != 'W') {
        if (safeCompareStrings(line + pos, "attr1", 5)) {
            query.addSelectedColumn("attr1");
            pos += 5;
        }
        else if (safeCompareStrings(line + pos, "attr2", 5)) {
            query.addSelectedColumn("attr2");
            pos += 5;
        }
        else if (safeCompareStrings(line + pos, "attr3", 5)) {
            query.addSelectedColumn("attr3");
            pos += 5;
        }
        pos++;
    }

    // Parse WHERE conditions
    while (line[pos] != '\0') {
        // Check for attr1 condition
        if (safeCompareStrings(line + pos, "attr1=", 6)) {
            pos += 6;
            int attrPos = 0;
            query.attr1Condition[0] = '\0';
            while (line[pos] != '\0' && line[pos] != ' ' && line[pos] != 'A' && attrPos < MAX_ATTR_LENGTH - 1) {
                query.attr1Condition[attrPos++] = line[pos++];
            }
            query.attr1Condition[attrPos] = '\0';
        }
        // Check for attr2 condition
        else if (safeCompareStrings(line + pos, "attr2=", 6)) {
            pos += 6;
            int attrPos = 0;
            query.attr2Condition[0] = '\0';
            while (line[pos] != '\0' && line[pos] != ' ' && line[pos] != 'A' && attrPos < MAX_ATTR_LENGTH - 1) {
                query.attr2Condition[attrPos++] = line[pos++];
            }
            query.attr2Condition[attrPos] = '\0';
        }
        // Check for attr3 condition
        else if (safeCompareStrings(line + pos, "attr3=", 6)) {
            pos += 6;
            query.attr3Condition = 0;
            while (line[pos] >= '0' && line[pos] <= '9') {
                query.attr3Condition = query.attr3Condition * 10 + (line[pos] - '0');
                pos++;
            }
        }
        else {
            pos++;
        }
    }
}

struct Tuple {
    char attr1[MAX_ATTR_LENGTH];
    char attr2[MAX_ATTR_LENGTH];
    int attr3;

    Tuple() {
        attr1[0] = '\0';
        attr2[0] = '\0';
        attr3 = 0;
    }

    Tuple(const Tuple& other) {
        safeCopyString(attr1, other.attr1, MAX_ATTR_LENGTH);
        safeCopyString(attr2, other.attr2, MAX_ATTR_LENGTH);
        attr3 = other.attr3;
    }

    Tuple& operator=(const Tuple& other) {
        if (this != &other) {
            safeCopyString(attr1, other.attr1, MAX_ATTR_LENGTH);
            safeCopyString(attr2, other.attr2, MAX_ATTR_LENGTH);
            attr3 = other.attr3;
        }
        return *this;
    }
};

// Custom vector implementation
template <typename T>
class MyVector {
private:
    T* data;
    int capacity;
    int size;

    void resize() {
        int newCapacity = capacity * 2;
        T* newData = new T[newCapacity];
        for (int i = 0; i < size; ++i) {
            newData[i] = data[i];
        }
        delete[] data;
        data = newData;
        capacity = newCapacity;
    }

public:
    MyVector() : capacity(10), size(0) {
        data = new T[capacity];
    }

    ~MyVector() {
        delete[] data;
    }

    void push_back(const T& value) {
        if (size == capacity) {
            resize();
        }
        data[size++] = value;
    }

    T& operator[](int index) {
        return data[index];
    }

    const T& operator[](int index) const {
        return data[index];
    }

    int getSize() const {
        return size;
    }
};

class Database {
private:
    Tuple* data;
    int capacity;
    int size;
    bool* isDeleted;  // Track deleted records
    void formatSelectResult(const Tuple& tuple, const SelectQuery& query, char* result, int& resultPos) {
        bool firstColumn = true;

        if (query.isColumnSelected("attr1")) {
            if (!firstColumn) {
                result[resultPos++] = ',';
                result[resultPos++] = ' ';
            }
            int j = 0;
            while (tuple.attr1[j] != '\0' && resultPos < MAX_RESULT_LENGTH - 2) {
                result[resultPos++] = tuple.attr1[j++];
            }
            firstColumn = false;
        }

        if (query.isColumnSelected("attr2")) {
            if (!firstColumn) {
                result[resultPos++] = ',';
                result[resultPos++] = ' ';
            }
            int j = 0;
            while (tuple.attr2[j] != '\0' && resultPos < MAX_RESULT_LENGTH - 2) {
                result[resultPos++] = tuple.attr2[j++];
            }
            firstColumn = false;
        }

        if (query.isColumnSelected("attr3")) {
            if (!firstColumn) {
                result[resultPos++] = ',';
                result[resultPos++] = ' ';
            }
            int num = tuple.attr3;
            char numStr[12];
            int numLen = 0;

            // Convert number to string (same as previous implementation)
            if (num == 0) {
                numStr[numLen++] = '0';
            }
            else {
                int temp = num;
                while (temp > 0) {
                    numStr[numLen++] = '0' + (temp % 10);
                    temp /= 10;
                }
            }

            // Reverse number string
            for (int k = 0; k < numLen / 2; k++) {
                char temp = numStr[k];
                numStr[k] = numStr[numLen - 1 - k];
                numStr[numLen - 1 - k] = temp;
            }

            // Copy number to result
            for (int k = 0; k < numLen && resultPos < MAX_RESULT_LENGTH - 2; k++) {
                result[resultPos++] = numStr[k];
            }
        }

        result[resultPos++] = '\n';
        result[resultPos] = '\0';
    }

public:
    Database() : capacity(MAX_TUPLES), size(0) {
        data = new Tuple[capacity];
        isDeleted = new bool[capacity]();  // Initialize all to false
    }

    ~Database() {
        delete[] data;
        delete[] isDeleted;
    }

    int getNumTuples() const {
        int activeTuples = 0;
        for (int i = 0; i < size; i++) {
            if (!isDeleted[i]) {
                activeTuples++;
            }
        }
        return activeTuples;
    }

    void insert(const char* attr1, const char* attr2, int attr3) {
        if (size < capacity) {
            safeCopyString(data[size].attr1, attr1, MAX_ATTR_LENGTH);
            safeCopyString(data[size].attr2, attr2, MAX_ATTR_LENGTH);
            data[size].attr3 = attr3;
            size++;
            isDeleted[size] = false;
            std::cout << "Inserted: " << attr1 << ", " << attr2 << ", " << attr3 << std::endl;
        }
    }

    int deleteRecords(const char* whereAttr1, const char* whereAttr2, int whereAttr3, std::ofstream& outputFile) {
        int deletedCount = 0;

        for (int i = 0; i < size; ++i) {
            if (isDeleted[i]) continue;  // Skip already deleted records

            bool attr1Match = (safeStringLength(whereAttr1, MAX_ATTR_LENGTH) == 0) ||
                (whereAttr1[0] == '*') ||
                matchesPattern(data[i].attr1, whereAttr1, MAX_ATTR_LENGTH);

            bool attr2Match = (safeStringLength(whereAttr2, MAX_ATTR_LENGTH) == 0) ||
                (whereAttr2[0] == '*') ||
                matchesPattern(data[i].attr2, whereAttr2, MAX_ATTR_LENGTH);

            bool attr3Match = (whereAttr3 == -1) || (data[i].attr3 == whereAttr3);

            if (attr1Match && attr2Match && attr3Match) {
                // Log the deleted record
                outputFile << "Deleted record " << i << ": "
                    << data[i].attr1 << ", "
                    << data[i].attr2 << ", "
                    << data[i].attr3 << "\n";

                isDeleted[i] = true;
                deletedCount++;
            }
        }

        return deletedCount;
    }

    int update(const char* whereAttr1, const char* whereAttr2, int whereAttr3,
        const char* setAttr1, const char* setAttr2, int setAttr3, std::ofstream& outputFile) {
        int updatedCount = 0;

        for (int i = 0; i < size; ++i) {
            if (isDeleted[i]) continue;  // Skip deleted records
            bool attr1Match = (safeStringLength(whereAttr1, MAX_ATTR_LENGTH) == 0) ||
                (whereAttr1[0] == '*') ||
                matchesPattern(data[i].attr1, whereAttr1, MAX_ATTR_LENGTH);

            bool attr2Match = (safeStringLength(whereAttr2, MAX_ATTR_LENGTH) == 0) ||
                (whereAttr2[0] == '*') ||
                matchesPattern(data[i].attr2, whereAttr2, MAX_ATTR_LENGTH);

            bool attr3Match = (whereAttr3 == -1) || (data[i].attr3 == whereAttr3);

            if (attr1Match && attr2Match && attr3Match) {
                // Store old values for output
                char oldAttr1[MAX_ATTR_LENGTH], oldAttr2[MAX_ATTR_LENGTH];
                int oldAttr3;

                safeCopyString(oldAttr1, data[i].attr1, MAX_ATTR_LENGTH);
                safeCopyString(oldAttr2, data[i].attr2, MAX_ATTR_LENGTH);
                oldAttr3 = data[i].attr3;

                // Perform the update
                if (safeStringLength(setAttr1, MAX_ATTR_LENGTH) > 0) {
                    safeCopyString(data[i].attr1, setAttr1, MAX_ATTR_LENGTH);
                }
                if (safeStringLength(setAttr2, MAX_ATTR_LENGTH) > 0) {
                    safeCopyString(data[i].attr2, setAttr2, MAX_ATTR_LENGTH);
                }
                if (setAttr3 != -1) {
                    data[i].attr3 = setAttr3;
                }

                // Output the before and after values
                outputFile << "Updated record " << i << ":\n";
                outputFile << "  Before: " << oldAttr1 << ", " << oldAttr2 << ", " << oldAttr3 << "\n";
                outputFile << "  After:  " << data[i].attr1 << ", " << data[i].attr2 << ", " << data[i].attr3 << "\n";

                updatedCount++;
            }
        }

        return updatedCount;
    }


    void query(const char* attr1, const char* attr2, int attr3, char* result) {
        result[0] = '\0';
        char tempResult[MAX_RESULT_LENGTH];
        int resultPos = 0;
        bool anyResultFound = false;

        for (int i = 0; i < size; ++i) {
            if (isDeleted[i]) continue;  // Skip deleted records
            // More comprehensive matching logic
            bool attr1Match = (safeStringLength(attr1, MAX_ATTR_LENGTH) == 0) ||
                (attr1[0] == '*') ||
                matchesPattern(data[i].attr1, attr1, MAX_ATTR_LENGTH);

            bool attr2Match = (safeStringLength(attr2, MAX_ATTR_LENGTH) == 0) ||
                (attr2[0] == '*') ||
                matchesPattern(data[i].attr2, attr2, MAX_ATTR_LENGTH);

            bool attr3Match = (attr3 == -1) || (data[i].attr3 == attr3);

            if (attr1Match && attr2Match && attr3Match) {
                // Construct result string
                safeCopyString(tempResult, "Found: ", MAX_RESULT_LENGTH);
                resultPos = 7;

                // Copy attr1
                int j = 0;
                while (data[i].attr1[j] != '\0' && resultPos < MAX_RESULT_LENGTH - 3) {
                    tempResult[resultPos++] = data[i].attr1[j++];
                }
                tempResult[resultPos++] = ',';
                tempResult[resultPos++] = ' ';

                // Copy attr2
                j = 0;
                while (data[i].attr2[j] != '\0' && resultPos < MAX_RESULT_LENGTH - 3) {
                    tempResult[resultPos++] = data[i].attr2[j++];
                }
                tempResult[resultPos++] = ',';
                tempResult[resultPos++] = ' ';

                // Convert attr3 to string
                int num = data[i].attr3;
                char numStr[12];
                int numLen = 0;

                if (num == 0) {
                    numStr[numLen++] = '0';
                }
                else {
                    int temp = num;
                    while (temp > 0) {
                        numStr[numLen++] = '0' + (temp % 10);
                        temp /= 10;
                    }
                }

                // Reverse number string
                for (int k = 0; k < numLen / 2; k++) {
                    char temp = numStr[k];
                    numStr[k] = numStr[numLen - 1 - k];
                    numStr[numLen - 1 - k] = temp;
                }

                // Append number to result
                for (int k = 0; k < numLen && resultPos < MAX_RESULT_LENGTH - 2; k++) {
                    tempResult[resultPos++] = numStr[k];
                }

                tempResult[resultPos++] = '\n';
                tempResult[resultPos] = '\0';

                // Append to results if not already found
                if (!anyResultFound) {
                    safeCopyString(result, tempResult, MAX_RESULT_LENGTH);
                    anyResultFound = true;
                }
                else {
                    // If multiple results found, append to existing result
                    int currentLen = safeStringLength(result, MAX_RESULT_LENGTH);
                    if (currentLen + resultPos < MAX_RESULT_LENGTH) {
                        safeCopyString(result + currentLen, tempResult, MAX_RESULT_LENGTH - currentLen);
                    }
                }

                std::cout << "Found match: " << tempResult << std::endl;
            }
        }
    }
    void enhancedQuery(const SelectQuery& query, char* result) {
        result[0] = '\0';
        char tempResult[MAX_RESULT_LENGTH];
        int resultPos = 0;
        bool anyResultFound = false;

        for (int i = 0; i < size; ++i) {
            // Matching logic
            bool attr1Match = (safeStringLength(query.attr1Condition, MAX_ATTR_LENGTH) == 0) ||
                (query.attr1Condition[0] == '*') ||
                matchesPattern(data[i].attr1, query.attr1Condition, MAX_ATTR_LENGTH);

            bool attr2Match = (safeStringLength(query.attr2Condition, MAX_ATTR_LENGTH) == 0) ||
                (query.attr2Condition[0] == '*') ||
                matchesPattern(data[i].attr2, query.attr2Condition, MAX_ATTR_LENGTH);

            bool attr3Match = (query.attr3Condition == -1) || (data[i].attr3 == query.attr3Condition);

            if (attr1Match && attr2Match && attr3Match) {
                int previousResultPos = resultPos;
                formatSelectResult(data[i], query, tempResult, resultPos);

                if (!anyResultFound) {
                    safeCopyString(result, tempResult, MAX_RESULT_LENGTH);
                    anyResultFound = true;
                }
                else {
                    int currentLen = safeStringLength(result, MAX_RESULT_LENGTH);
                    if (currentLen + (resultPos - previousResultPos) < MAX_RESULT_LENGTH) {
                        safeCopyString(result + currentLen, tempResult + previousResultPos, MAX_RESULT_LENGTH - currentLen);
                    }
                }
            }
        }

        if (!anyResultFound) {
            result[0] = '\0';
        }
    }
};

void extractValue(const char* input, char* output, int& pos, int maxLen) {
    int outIdx = 0;
    while (input[pos] == ' ' || input[pos] == '(') pos++;

    while (input[pos] != '\0' && input[pos] != ',' && input[pos] != ')' && outIdx < maxLen - 1) {
        if (input[pos] != ' ') {
            output[outIdx++] = input[pos];
        }
        pos++;
    }
    output[outIdx] = '\0';
    if (input[pos] == ',') pos++;
}

void parseInputLine(const char* line, char* attr1, char* attr2, int& attr3, char* setAttr1, char* setAttr2, int& setAttr3) {
    int pos = 0;

    // Reset everything to wildcard/empty state
    attr1[0] = '\0';
    attr2[0] = '\0';
    attr3 = -1;
    setAttr1[0] = '\0';
    setAttr2[0] = '\0';
    setAttr3 = -1;

    if (line[0] == 'I') {
        // INSERT parsing remains the same as before
        while (line[pos] != '\0' && line[pos] != '(') pos++;
        if (line[pos] == '\0') return;

        extractValue(line, attr1, pos, MAX_ATTR_LENGTH);
        extractValue(line, attr2, pos, MAX_ATTR_LENGTH);

        while (line[pos] == ' ' || line[pos] == ',') pos++;
        while (line[pos] >= '0' && line[pos] <= '9') {
            attr3 = (attr3 == -1 ? 0 : attr3 * 10) + (line[pos] - '0');
            pos++;
        }
    }
    else if (line[0] == 'S') {
        // More robust SELECT parsing
        while (line[pos] != '\0' && (line[pos] != 'W' || line[pos + 1] != 'H')) pos++;
        if (line[pos] == '\0') return;

        pos += 6;  // Move past "WHERE"

        // Default to wildcard if no conditions specified
        safeCopyString(attr1, "*", MAX_ATTR_LENGTH);
        safeCopyString(attr2, "*", MAX_ATTR_LENGTH);
        attr3 = -1;

        // Parse all possible conditions
        bool hasCondition = false;
        while (line[pos] != '\0') {
            // Check for attr1 condition
            if (safeCompareStrings(line + pos, "attr1=", 6)) {
                pos += 6;
                int attrPos = 0;
                attr1[0] = '\0';  // Reset previous value
                while (line[pos] != '\0' && line[pos] != ' ' && line[pos] != 'A' && attrPos < MAX_ATTR_LENGTH - 1) {
                    attr1[attrPos++] = line[pos++];
                }
                attr1[attrPos] = '\0';
                hasCondition = true;
            }
            // Check for attr2 condition
            else if (safeCompareStrings(line + pos, "attr2=", 6)) {
                pos += 6;
                int attrPos = 0;
                attr2[0] = '\0';  // Reset previous value
                while (line[pos] != '\0' && line[pos] != ' ' && line[pos] != 'A' && attrPos < MAX_ATTR_LENGTH - 1) {
                    attr2[attrPos++] = line[pos++];
                }
                attr2[attrPos] = '\0';
                hasCondition = true;
            }
            // Check for attr3 condition
            else if (safeCompareStrings(line + pos, "attr3=", 6)) {
                pos += 6;
                attr3 = 0;
                while (line[pos] >= '0' && line[pos] <= '9') {
                    attr3 = attr3 * 10 + (line[pos] - '0');
                    pos++;
                }
                hasCondition = true;
            }
            else {
                pos++;
            }
        }

        // If no conditions were found, leave everything as wildcard
        if (!hasCondition) {
            safeCopyString(attr1, "*", MAX_ATTR_LENGTH);
            safeCopyString(attr2, "*", MAX_ATTR_LENGTH);
            attr3 = -1;
        }
    }
    else if (line[0] == 'U') {  // UPDATE
        // Skip "UPDATE"
        while (line[pos] != '\0' && line[pos] != 'S') pos++;
        if (line[pos] == '\0') return;

        // Parse SET clause
        pos += 3;  // Skip "SET"

        // Parse SET values
        while (line[pos] != '\0' && line[pos] != 'W') {
            if (safeCompareStrings(line + pos, "attr1=", 6)) {
                pos += 6;
                int attrPos = 0;
                while (line[pos] != '\0' && line[pos] != ',' && line[pos] != ' ' && attrPos < MAX_ATTR_LENGTH - 1) {
                    setAttr1[attrPos++] = line[pos++];
                }
                setAttr1[attrPos] = '\0';
            }
            else if (safeCompareStrings(line + pos, "attr2=", 6)) {
                pos += 6;
                int attrPos = 0;
                while (line[pos] != '\0' && line[pos] != ',' && line[pos] != ' ' && attrPos < MAX_ATTR_LENGTH - 1) {
                    setAttr2[attrPos++] = line[pos++];
                }
                setAttr2[attrPos] = '\0';
            }
            else if (safeCompareStrings(line + pos, "attr3=", 6)) {
                pos += 6;
                setAttr3 = 0;
                while (line[pos] >= '0' && line[pos] <= '9') {
                    setAttr3 = setAttr3 * 10 + (line[pos] - '0');
                    pos++;
                }
            }
            pos++;
        }

        // Parse WHERE clause (similar to SELECT)
        if (line[pos] == 'W') {
            pos += 5;  // Skip "WHERE"
            while (line[pos] != '\0') {
                if (safeCompareStrings(line + pos, "attr1=", 6)) {
                    pos += 6;
                    int attrPos = 0;
                    while (line[pos] != '\0' && line[pos] != ' ' && line[pos] != 'A' && attrPos < MAX_ATTR_LENGTH - 1) {
                        attr1[attrPos++] = line[pos++];
                    }
                    attr1[attrPos] = '\0';
                }
                else if (safeCompareStrings(line + pos, "attr2=", 6)) {
                    pos += 6;
                    int attrPos = 0;
                    while (line[pos] != '\0' && line[pos] != ' ' && line[pos] != 'A' && attrPos < MAX_ATTR_LENGTH - 1) {
                        attr2[attrPos++] = line[pos++];
                    }
                    attr2[attrPos] = '\0';
                }
                else if (safeCompareStrings(line + pos, "attr3=", 6)) {
                    pos += 6;
                    attr3 = 0;
                    while (line[pos] >= '0' && line[pos] <= '9') {
                        attr3 = attr3 * 10 + (line[pos] - '0');
                        pos++;
                    }
                }
                else {
                    pos++;
                }
            }
        }
    }
    // Add DELETE parsing
    if (line[0] == 'D') {  // DELETE
        int pos = 0;
        // Skip "DELETE FROM"
        while (line[pos] != '\0' && line[pos] != 'W') pos++;
        if (line[pos] == '\0') return;

        pos += 5;  // Skip "WHERE"
        while (line[pos] != '\0') {
            if (safeCompareStrings(line + pos, "attr1=", 6)) {
                pos += 6;
                int attrPos = 0;
                while (line[pos] != '\0' && line[pos] != ' ' && line[pos] != 'A' && attrPos < MAX_ATTR_LENGTH - 1) {
                    attr1[attrPos++] = line[pos++];
                }
                attr1[attrPos] = '\0';
            }
            else if (safeCompareStrings(line + pos, "attr2=", 6)) {
                pos += 6;
                int attrPos = 0;
                while (line[pos] != '\0' && line[pos] != ' ' && line[pos] != 'A' && attrPos < MAX_ATTR_LENGTH - 1) {
                    attr2[attrPos++] = line[pos++];
                }
                attr2[attrPos] = '\0';
            }
            else if (safeCompareStrings(line + pos, "attr3=", 6)) {
                pos += 6;
                attr3 = 0;
                while (line[pos] >= '0' && line[pos] <= '9') {
                    attr3 = attr3 * 10 + (line[pos] - '0');
                    pos++;
                }
            }
            else {
                pos++;
            }
        }
    }
}

void runSingleProcess() {
    Database db;
    std::ifstream inputFile("input.sql");
    std::ofstream outputFile("output.txt", std::ios::out);
    std::ofstream tupleCountFile("tuple_counts.csv", std::ios::out);

    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open input file\n";
        return;
    }

    if (!outputFile.is_open() || !tupleCountFile.is_open()) {
        std::cerr << "Error: Could not open output file\n";
        return;
    }

    char command[MAX_COMMAND_LENGTH];
    char attr1[MAX_ATTR_LENGTH];
    char attr2[MAX_ATTR_LENGTH];
    int attr3;
    char setAttr1[MAX_ATTR_LENGTH];
    char setAttr2[MAX_ATTR_LENGTH];
    int setAttr3;

    while (inputFile.getline(command, MAX_COMMAND_LENGTH)) {
        std::cout << "Processing command: " << command << std::endl;

        if (command[0] == 'I') {  // INSERT
            parseInputLine(command, attr1, attr2, attr3, setAttr1, setAttr2, setAttr3);
            db.insert(attr1, attr2, attr3);
        }
        else if (command[0] == 'S') {  // SELECT
            SelectQuery query;
            parseSelectQuery(command, query);

            char result[MAX_RESULT_LENGTH];
            db.enhancedQuery(query, result);

            if (result[0] != '\0') {
                outputFile << result;
            }
            else {
                outputFile << "No records found. Query attributes: ";
                outputFile << "attr1=" << query.attr1Condition << ", ";
                outputFile << "attr2=" << query.attr2Condition << ", ";
                outputFile << "attr3=" << query.attr3Condition << "\n";
            }

            tupleCountFile << db.getNumTuples() << ",\n";
        }
        else if (command[0] == 'U') {  // UPDATE
            parseInputLine(command, attr1, attr2, attr3, setAttr1, setAttr2, setAttr3);
            int updateCount = db.update(attr1, attr2, attr3, setAttr1, setAttr2, setAttr3, outputFile);
            outputFile << "Total records updated: " << updateCount << "\n\n";
            outputFile.flush();

            // Also log tuple count after update
            tupleCountFile << db.getNumTuples() << ",\n";
        }
        else if (command[0] == 'D') {  // DELETE
            parseInputLine(command, attr1, attr2, attr3, setAttr1, setAttr2, setAttr3);
            int deleteCount = db.deleteRecords(attr1, attr2, attr3, outputFile);
            outputFile << "Total records deleted: " << deleteCount << "\n\n";
            outputFile.flush();

            // Also log tuple count after delete
            tupleCountFile << db.getNumTuples() << ",\n";
        }
    }

    inputFile.close();
    outputFile.close();
    tupleCountFile.close();
}

void runWorker(int rank, int numWorkers) {
    Database db;
    char buffer1[MAX_ATTR_LENGTH];
    char buffer2[MAX_ATTR_LENGTH];
    char setBuffer1[MAX_ATTR_LENGTH];
    char setBuffer2[MAX_ATTR_LENGTH];
    int attr3, setAttr3;
    char result[MAX_RESULT_LENGTH];

    // Buffers for selected columns
    char selectedColumns[MAX_COLUMNS][MAX_COLUMN_NAME];
    int selectedColumnCount;


    // Open output file to write tuple count for each worker
    std::ofstream outputFile("tuple_count.txt", std::ios::app);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Could not open worker output file\n";
        return;
    }

    while (true) {
        MPI_Status status;
        MPI_Recv(buffer1, MAX_ATTR_LENGTH, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == 99) {
            break;
        }

        if (status.MPI_TAG == 0) {  // INSERT
            MPI_Recv(buffer2, MAX_ATTR_LENGTH, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&attr3, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);

            // Only insert if this worker should handle this data
            if ((attr3 % numWorkers) == (rank - 1)) {
                db.insert(buffer1, buffer2, attr3);
            }
        }
        else if (status.MPI_TAG == 3) {  // SELECT
            // Receive selected columns
            MPI_Recv(&selectedColumnCount, 1, MPI_INT, 0, 4, MPI_COMM_WORLD, &status);
            
            // Receive column names if any specific columns are selected
            if (selectedColumnCount > 0) {
                MPI_Recv(selectedColumns, selectedColumnCount * MAX_COLUMN_NAME, MPI_CHAR, 0, 5, MPI_COMM_WORLD, &status);
            }

            // Receive query conditions
            MPI_Recv(buffer2, MAX_ATTR_LENGTH, MPI_CHAR, 0, 6, MPI_COMM_WORLD, &status);
            MPI_Recv(&attr3, 1, MPI_INT, 0, 7, MPI_COMM_WORLD, &status);

            // Prepare SelectQuery
            SelectQuery query;
            
            // Set selected columns
            for (int i = 0; i < selectedColumnCount; ++i) {
                query.addSelectedColumn(selectedColumns[i]);
            }
            
            // Set conditions
            if (safeStringLength(buffer1, MAX_ATTR_LENGTH) > 0) {
                safeCopyString(query.attr1Condition, buffer1, MAX_ATTR_LENGTH);
            }
            if (safeStringLength(buffer2, MAX_ATTR_LENGTH) > 0) {
                safeCopyString(query.attr2Condition, buffer2, MAX_ATTR_LENGTH);
            }
            query.attr3Condition = attr3;

            // Query local database partition
            char workerResult[MAX_RESULT_LENGTH];
            db.enhancedQuery(query, workerResult);
            
            MPI_Send(workerResult, MAX_RESULT_LENGTH, MPI_CHAR, 0, 8, MPI_COMM_WORLD);

            // Send the current tuple count to master
            int tupleCount = db.getNumTuples();
            MPI_Send(&tupleCount, 1, MPI_INT, 0, 9, MPI_COMM_WORLD);
        }
        else if (status.MPI_TAG == 8) {  // UPDATE
            MPI_Recv(buffer2, MAX_ATTR_LENGTH, MPI_CHAR, 0, 9, MPI_COMM_WORLD, &status);
            MPI_Recv(&attr3, 1, MPI_INT, 0, 10, MPI_COMM_WORLD, &status);
            MPI_Recv(setBuffer1, MAX_ATTR_LENGTH, MPI_CHAR, 0, 11, MPI_COMM_WORLD, &status);
            MPI_Recv(setBuffer2, MAX_ATTR_LENGTH, MPI_CHAR, 0, 12, MPI_COMM_WORLD, &status);
            MPI_Recv(&setAttr3, 1, MPI_INT, 0, 13, MPI_COMM_WORLD, &status);

            char updateResult[MAX_RESULT_LENGTH] = "";
            std::ofstream tempOutputFile("temp_output.txt", std::ios::app);
            int updateCount = db.update(buffer1, buffer2, attr3, setBuffer1, setBuffer2, setAttr3, tempOutputFile);
            tempOutputFile.close();

            // Read the temporary file and send its contents
            std::ifstream tempInputFile("temp_output.txt");
            std::string updateDetails((std::istreambuf_iterator<char>(tempInputFile)),
                std::istreambuf_iterator<char>());
            tempInputFile.close();
            std::remove("temp_output.txt");

            // Send update count and details back to master
            MPI_Send(&updateCount, 1, MPI_INT, 0, 14, MPI_COMM_WORLD);
            MPI_Send(updateDetails.c_str(), updateDetails.length() + 1, MPI_CHAR, 0, 15, MPI_COMM_WORLD);
        }
        else if (status.MPI_TAG == 16) {  // DELETE
            MPI_Recv(buffer2, MAX_ATTR_LENGTH, MPI_CHAR, 0, 17, MPI_COMM_WORLD, &status);
            MPI_Recv(&attr3, 1, MPI_INT, 0, 18, MPI_COMM_WORLD, &status);

            std::ofstream tempOutputFile("temp_output.txt", std::ios::app);
            int deleteCount = db.deleteRecords(buffer1, buffer2, attr3, tempOutputFile);
            tempOutputFile.close();

            // Read and send delete details
            std::ifstream tempInputFile("temp_output.txt");
            std::string deleteDetails((std::istreambuf_iterator<char>(tempInputFile)),
                std::istreambuf_iterator<char>());
            tempInputFile.close();
            std::remove("temp_output.txt");

            MPI_Send(&deleteCount, 1, MPI_INT, 0, 19, MPI_COMM_WORLD);
            MPI_Send(deleteDetails.c_str(), deleteDetails.length() + 1, MPI_CHAR, 0, 20, MPI_COMM_WORLD);
        }
    }

    // Close the output file
    outputFile.close();
}

void runMaster(int numWorkers) {
    std::ifstream inputFile("input.sql");
    std::ofstream outputFile("output.txt", std::ios::out);
    std::ofstream tupleCountFile("tuple_counts.csv", std::ios::out);

    char setAttr1[MAX_ATTR_LENGTH];
    char setAttr2[MAX_ATTR_LENGTH];
    int setAttr3;

    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open input file\n";
        return;
    }

    if (!outputFile.is_open()) {
        std::cerr << "Error: Could not open output file\n";
        return;
    }

    char command[MAX_COMMAND_LENGTH];
    char attr1[MAX_ATTR_LENGTH];
    char attr2[MAX_ATTR_LENGTH];
    int attr3;

    while (inputFile.getline(command, MAX_COMMAND_LENGTH)) {
        std::cout << "Processing command: " << command << std::endl;

        if (command[0] == 'I') {  // INSERT
            parseInputLine(command, attr1, attr2, attr3, setAttr1, setAttr2, setAttr3);
            std::cout << "Parsed INSERT values: " << attr1 << ", " << attr2 << ", " << attr3 << std::endl;

            // Broadcast insert to all workers
            for (int worker = 1; worker <= numWorkers; ++worker) {
                MPI_Send(attr1, MAX_ATTR_LENGTH, MPI_CHAR, worker, 0, MPI_COMM_WORLD);
                MPI_Send(attr2, MAX_ATTR_LENGTH, MPI_CHAR, worker, 1, MPI_COMM_WORLD);
                MPI_Send(&attr3, 1, MPI_INT, worker, 2, MPI_COMM_WORLD);
            }
        }
        else if (command[0] == 'S') {  // SELECT
            SelectQuery query;
            parseSelectQuery(command, query);

            // Prepare buffers for query conditions
            char workerAttr1[MAX_ATTR_LENGTH];
            char workerAttr2[MAX_ATTR_LENGTH];
            int workerAttr3;

            // If specific conditions aren't set, use wildcard/default
            safeCopyString(workerAttr1, query.attr1Condition[0] != '\0' ? query.attr1Condition : "*", MAX_ATTR_LENGTH);
            safeCopyString(workerAttr2, query.attr2Condition[0] != '\0' ? query.attr2Condition : "*", MAX_ATTR_LENGTH);
            workerAttr3 = query.attr3Condition;

            // Track tuple counts
            MyVector<int> tupleCounts;
            bool found = false;

            // Query all workers
            for (int worker = 1; worker <= numWorkers; ++worker) {
                // Send enhanced query parameters
                MPI_Send(workerAttr1, MAX_ATTR_LENGTH, MPI_CHAR, worker, 3, MPI_COMM_WORLD);

                // Send selected columns
                int selectedColumnCount = query.selectedColumnCount;
                MPI_Send(&selectedColumnCount, 1, MPI_INT, worker, 4, MPI_COMM_WORLD);

                // Send column names if any specific columns are selected
                if (selectedColumnCount > 0) {
                    MPI_Send(query.selectedColumns, selectedColumnCount * MAX_COLUMN_NAME, MPI_CHAR, worker, 5, MPI_COMM_WORLD);
                }

                // Send other query conditions
                MPI_Send(workerAttr2, MAX_ATTR_LENGTH, MPI_CHAR, worker, 6, MPI_COMM_WORLD);
                MPI_Send(&workerAttr3, 1, MPI_INT, worker, 7, MPI_COMM_WORLD);

                // Receive worker's results
                char workerResult[MAX_RESULT_LENGTH];
                MPI_Status status;
                MPI_Recv(workerResult, MAX_RESULT_LENGTH, MPI_CHAR, worker, 8, MPI_COMM_WORLD, &status);

                if (workerResult[0] != '\0') {
                    outputFile << workerResult;
                    outputFile.flush();
                    found = true;
                }

                // Get the tuple count from the worker
                int workerTupleCount;
                MPI_Recv(&workerTupleCount, 1, MPI_INT, worker, 9, MPI_COMM_WORLD, &status);
                tupleCounts.push_back(workerTupleCount);
            }

            if (!found) {
                outputFile << "No records found.";

                // Output specific conditions used in the query
                outputFile << " Query attributes: ";
                bool firstCondition = true;

                if (query.attr1Condition[0] != '\0' && query.attr1Condition[0] != '*') {
                    outputFile << "attr1=" << query.attr1Condition;
                    firstCondition = false;
                }

                if (query.attr2Condition[0] != '\0' && query.attr2Condition[0] != '*') {
                    if (!firstCondition) outputFile << ", ";
                    outputFile << "attr2=" << query.attr2Condition;
                    firstCondition = false;
                }

                if (query.attr3Condition != -1) {
                    if (!firstCondition) outputFile << ", ";
                    outputFile << "attr3=" << query.attr3Condition;
                }

                outputFile << "\n";
                outputFile.flush();
            }

            // Write counts to CSV
            for (int i = 0; i < tupleCounts.getSize(); ++i) {
                tupleCountFile << (i > 0 ? "," : "") << tupleCounts[i];
            }
            tupleCountFile << "\n";
        }
        else if (command[0] == 'U') {  // UPDATE
            parseInputLine(command, attr1, attr2, attr3, setAttr1, setAttr2, setAttr3);

            int totalUpdated = 0;
            for (int worker = 1; worker <= numWorkers; ++worker) {
                // Send update command to worker
                MPI_Send(attr1, MAX_ATTR_LENGTH, MPI_CHAR, worker, 8, MPI_COMM_WORLD);
                MPI_Send(attr2, MAX_ATTR_LENGTH, MPI_CHAR, worker, 9, MPI_COMM_WORLD);
                MPI_Send(&attr3, 1, MPI_INT, worker, 10, MPI_COMM_WORLD);
                MPI_Send(setAttr1, MAX_ATTR_LENGTH, MPI_CHAR, worker, 11, MPI_COMM_WORLD);
                MPI_Send(setAttr2, MAX_ATTR_LENGTH, MPI_CHAR, worker, 12, MPI_COMM_WORLD);
                MPI_Send(&setAttr3, 1, MPI_INT, worker, 13, MPI_COMM_WORLD);

                // Receive update results
                int workerUpdateCount;
                MPI_Status status;
                MPI_Recv(&workerUpdateCount, 1, MPI_INT, worker, 14, MPI_COMM_WORLD, &status);

                // Receive and write update details
                char updateDetails[MAX_RESULT_LENGTH * 10];  // Larger buffer for multiple updates
                MPI_Recv(updateDetails, MAX_RESULT_LENGTH * 10, MPI_CHAR, worker, 15, MPI_COMM_WORLD, &status);
                if (workerUpdateCount > 0) {
                    outputFile << "Updates from worker " << worker << ":\n";
                    outputFile << updateDetails;
                }

                totalUpdated += workerUpdateCount;
            }

            outputFile << "Total records updated: " << totalUpdated << "\n\n";
            outputFile.flush();
        }
        else if (command[0] == 'D') {  // DELETE
            parseInputLine(command, attr1, attr2, attr3, setAttr1, setAttr2, setAttr3);

            int totalDeleted = 0;
            for (int worker = 1; worker <= numWorkers; ++worker) {
                // Send delete command to worker
                MPI_Send(attr1, MAX_ATTR_LENGTH, MPI_CHAR, worker, 16, MPI_COMM_WORLD);
                MPI_Send(attr2, MAX_ATTR_LENGTH, MPI_CHAR, worker, 17, MPI_COMM_WORLD);
                MPI_Send(&attr3, 1, MPI_INT, worker, 18, MPI_COMM_WORLD);

                // Receive delete results
                int workerDeleteCount;
                MPI_Status status;
                MPI_Recv(&workerDeleteCount, 1, MPI_INT, worker, 19, MPI_COMM_WORLD, &status);

                // Receive and write delete details
                char deleteDetails[MAX_RESULT_LENGTH * 10];
                MPI_Recv(deleteDetails, MAX_RESULT_LENGTH * 10, MPI_CHAR, worker, 20, MPI_COMM_WORLD, &status);
                if (workerDeleteCount > 0) {
                    outputFile << "Deletes from worker " << worker << ":\n";
                    outputFile << deleteDetails;
                }

                totalDeleted += workerDeleteCount;
            }

            outputFile << "Total records deleted: " << totalDeleted << "\n\n";
            outputFile.flush();
        }
    }

    // Send termination signal to all workers
    for (int worker = 1; worker <= numWorkers; ++worker) {
        char terminateMsg = '\0';
        MPI_Send(&terminateMsg, 1, MPI_CHAR, worker, 99, MPI_COMM_WORLD);
    }

    inputFile.close();
    outputFile.close();
}

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double totalStartTime = MPI_Wtime();

    if (size == 1) {
        runSingleProcess();
    }
    else {
        if (rank == 0) {
            runMaster(size - 1);
        }
        else {
            runWorker(rank, size - 1);
        }
    }

    double totalEndTime = MPI_Wtime();

    MPI_Finalize();

    // Calculate and print total execution time on the root process
    double totalExecutionTime = totalEndTime - totalStartTime;

    if (rank == 0) {
        std::cout << "Total Execution Time: " << totalExecutionTime << " seconds" << std::endl;
    }

    return 0;
}