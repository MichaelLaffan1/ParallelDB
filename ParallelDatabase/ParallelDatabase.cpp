#include <mpi.h>
#include <iostream>
#include <fstream>

// Define constants
const int MAX_ATTR_LENGTH = 100;
const int MAX_TUPLES = 100;
const int MAX_RESULT_LENGTH = 256;
const int MAX_COMMAND_LENGTH = 256;

// Safe string copy function
void safeCopyString(char* dest, const char* src, int maxLen) {
    int i = 0;
    while (src[i] != '\0' && i < maxLen - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

// Safe string length function
int safeStringLength(const char* str, int maxLen) {
    int len = 0;
    while (str[len] != '\0' && len < maxLen) {
        len++;
    }
    return len;
}

// Safe string comparison function
bool safeCompareStrings(const char* str1, const char* str2, int maxLen) {
    int i = 0;
    while (i < maxLen - 1 && str1[i] != '\0' && str2[i] != '\0') {
        if (str1[i] != str2[i]) return false;
        i++;
    }
    return str1[i] == str2[i];
}

// Improved Tuple structure with stack-based arrays
struct Tuple {
    char attr1[MAX_ATTR_LENGTH];
    char attr2[MAX_ATTR_LENGTH];
    int attr3;

    Tuple() {
        attr1[0] = '\0';
        attr2[0] = '\0';
        attr3 = 0;
    }

    // Copy constructor
    Tuple(const Tuple& other) {
        safeCopyString(attr1, other.attr1, MAX_ATTR_LENGTH);
        safeCopyString(attr2, other.attr2, MAX_ATTR_LENGTH);
        attr3 = other.attr3;
    }

    // Assignment operator
    Tuple& operator=(const Tuple& other) {
        if (this != &other) {
            safeCopyString(attr1, other.attr1, MAX_ATTR_LENGTH);
            safeCopyString(attr2, other.attr2, MAX_ATTR_LENGTH);
            attr3 = other.attr3;
        }
        return *this;
    }
};

class Database {
private:
    Tuple* data;
    int capacity;
    int size;

public:
    Database() : capacity(MAX_TUPLES), size(0) {
        data = new Tuple[capacity];
    }

    ~Database() {
        delete[] data;
    }

    void insert(const char* attr1, const char* attr2, int attr3) {
        if (size < capacity) {
            safeCopyString(data[size].attr1, attr1, MAX_ATTR_LENGTH);
            safeCopyString(data[size].attr2, attr2, MAX_ATTR_LENGTH);
            data[size].attr3 = attr3;
            size++;
            std::cout << "Worker inserted: " << attr1 << ", " << attr2 << ", " << attr3 << std::endl;
        }
    }

    void query(const char* attr1, const char* attr2, int attr3, char* result) {
        result[0] = '\0';
        char tempResult[MAX_RESULT_LENGTH];
        int resultPos = 0;

        for (int i = 0; i < size; ++i) {
            if (safeCompareStrings(data[i].attr1, attr1, MAX_ATTR_LENGTH) &&
                safeCompareStrings(data[i].attr2, attr2, MAX_ATTR_LENGTH) &&
                data[i].attr3 == attr3) {

                // Format: "Found: attr1, attr2, attr3\n"
                safeCopyString(tempResult, "Found: ", MAX_RESULT_LENGTH);
                resultPos = 7; // Length of "Found: "

                // Add attr1
                int j = 0;
                while (data[i].attr1[j] != '\0' && resultPos < MAX_RESULT_LENGTH - 3) {
                    tempResult[resultPos++] = data[i].attr1[j++];
                }
                tempResult[resultPos++] = ',';
                tempResult[resultPos++] = ' ';

                // Add attr2
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

                // Reverse the number string
                for (int k = 0; k < numLen / 2; k++) {
                    char temp = numStr[k];
                    numStr[k] = numStr[numLen - 1 - k];
                    numStr[numLen - 1 - k] = temp;
                }

                // Add number to result
                for (int k = 0; k < numLen && resultPos < MAX_RESULT_LENGTH - 2; k++) {
                    tempResult[resultPos++] = numStr[k];
                }

                tempResult[resultPos++] = '\n';
                tempResult[resultPos] = '\0';

                safeCopyString(result, tempResult, MAX_RESULT_LENGTH);
                std::cout << "Worker found match: " << result << std::endl;
                return;
            }
        }
    }
};

void extractValue(const char* input, char* output, int& pos, int maxLen) {
    int outIdx = 0;
    // Skip spaces and find opening parenthesis
    while (input[pos] == ' ' || input[pos] == '(') pos++;

    // Copy until comma, closing parenthesis, or end
    while (input[pos] != '\0' && input[pos] != ',' && input[pos] != ')' && outIdx < maxLen - 1) {
        if (input[pos] != ' ') { // Skip spaces
            output[outIdx++] = input[pos];
        }
        pos++;
    }
    output[outIdx] = '\0';
    if (input[pos] == ',') pos++; // Skip comma if present
}

void parseInputLine(const char* line, char* attr1, char* attr2, int& attr3) {
    int pos = 0;

    // Initialize outputs
    attr1[0] = '\0';
    attr2[0] = '\0';
    attr3 = 0;

    // For INSERT, find "VALUES" keyword
    if (line[0] == 'I') {
        while (line[pos] != '\0' && line[pos] != '(') pos++;
        if (line[pos] == '\0') return;

        // Extract values
        extractValue(line, attr1, pos, MAX_ATTR_LENGTH);
        extractValue(line, attr2, pos, MAX_ATTR_LENGTH);

        // Extract numeric value
        while (line[pos] == ' ' || line[pos] == ',') pos++;
        while (line[pos] >= '0' && line[pos] <= '9') {
            attr3 = attr3 * 10 + (line[pos] - '0');
            pos++;
        }
    }
    // For SELECT, parse WHERE conditions
    else if (line[0] == 'S') {
        // Find WHERE clause
        while (line[pos] != '\0' && (line[pos] != 'W' || line[pos + 1] != 'H')) pos++;
        if (line[pos] == '\0') return;

        // Skip "WHERE "
        pos += 6;

        // Parse attr1
        if (strstr(line + pos, "attr1=") == line + pos) {
            pos += 6; // Skip "attr1="
            int attrPos = 0;
            while (line[pos] != '\0' && line[pos] != ' ' && attrPos < MAX_ATTR_LENGTH - 1) {
                if (line[pos] != ' ') {
                    attr1[attrPos++] = line[pos];
                }
                pos++;
            }
            attr1[attrPos] = '\0';
        }

        // Find AND and parse attr2
        while (line[pos] != '\0' && (line[pos] != 'A' || line[pos + 1] != 'N')) pos++;
        if (line[pos] != '\0') {
            pos += 4; // Skip "AND "
            if (strstr(line + pos, "attr2=") == line + pos) {
                pos += 6; // Skip "attr2="
                int attrPos = 0;
                while (line[pos] != '\0' && line[pos] != ' ' && attrPos < MAX_ATTR_LENGTH - 1) {
                    if (line[pos] != ' ') {
                        attr2[attrPos++] = line[pos];
                    }
                    pos++;
                }
                attr2[attrPos] = '\0';
            }
        }

        // Find AND and parse attr3
        while (line[pos] != '\0' && (line[pos] != 'A' || line[pos + 1] != 'N')) pos++;
        if (line[pos] != '\0') {
            pos += 4; // Skip "AND "
            if (strstr(line + pos, "attr3=") == line + pos) {
                pos += 6; // Skip "attr3="
                while (line[pos] >= '0' && line[pos] <= '9') {
                    attr3 = attr3 * 10 + (line[pos] - '0');
                    pos++;
                }
            }
        }
    }
}

void runWorker(int rank) {
    Database db;
    char buffer1[MAX_ATTR_LENGTH];
    char buffer2[MAX_ATTR_LENGTH];
    int attr3;
    char result[MAX_RESULT_LENGTH];

    while (true) {
        MPI_Status status;
        MPI_Recv(buffer1, MAX_ATTR_LENGTH, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == 99) {
            break;
        }

        if (status.MPI_TAG == 0) {  // INSERT
            MPI_Recv(buffer2, MAX_ATTR_LENGTH, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&attr3, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);
            db.insert(buffer1, buffer2, attr3);
        }
        else if (status.MPI_TAG == 3) {  // SELECT
            MPI_Recv(buffer2, MAX_ATTR_LENGTH, MPI_CHAR, 0, 4, MPI_COMM_WORLD, &status);
            MPI_Recv(&attr3, 1, MPI_INT, 0, 5, MPI_COMM_WORLD, &status);

            db.query(buffer1, buffer2, attr3, result);
            MPI_Send(result, MAX_RESULT_LENGTH, MPI_CHAR, 0, 6, MPI_COMM_WORLD);
        }
    }
}

void runMaster(int numNodes) {
    std::ifstream inputFile("input.sql");
    std::ofstream outputFile("output.txt", std::ios::out);

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
            parseInputLine(command, attr1, attr2, attr3);
            std::cout << "Parsed INSERT values: " << attr1 << ", " << attr2 << ", " << attr3 << std::endl;

            int targetNode = 1 + (attr3 % (numNodes));
            MPI_Send(attr1, MAX_ATTR_LENGTH, MPI_CHAR, targetNode, 0, MPI_COMM_WORLD);
            MPI_Send(attr2, MAX_ATTR_LENGTH, MPI_CHAR, targetNode, 1, MPI_COMM_WORLD);
            MPI_Send(&attr3, 1, MPI_INT, targetNode, 2, MPI_COMM_WORLD);
        }
        else if (command[0] == 'S') {  // SELECT
            parseInputLine(command, attr1, attr2, attr3);
            std::cout << "Parsed SELECT values: " << attr1 << ", " << attr2 << ", " << attr3 << std::endl;

            for (int node = 1; node <= numNodes; ++node) {
                MPI_Send(attr1, MAX_ATTR_LENGTH, MPI_CHAR, node, 3, MPI_COMM_WORLD);
                MPI_Send(attr2, MAX_ATTR_LENGTH, MPI_CHAR, node, 4, MPI_COMM_WORLD);
                MPI_Send(&attr3, 1, MPI_INT, node, 5, MPI_COMM_WORLD);

                char result[MAX_RESULT_LENGTH];
                MPI_Status status;
                MPI_Recv(result, MAX_RESULT_LENGTH, MPI_CHAR, node, 6, MPI_COMM_WORLD, &status);

                if (result[0] != '\0') {
                    outputFile << result;
                    outputFile.flush();
                }
            }
        }
    }

    // Send termination signal
    for (int node = 1; node <= numNodes; ++node) {
        char terminateMsg = '\0';
        MPI_Send(&terminateMsg, 1, MPI_CHAR, node, 99, MPI_COMM_WORLD);
    }

    inputFile.close();
    outputFile.close();
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        std::cerr << "This program requires at least 2 processes\n";
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        runMaster(size - 1);
    }
    else {
        runWorker(rank);
    }
    MPI_Finalize();
    return 0;
}