#include <mpi.h>
#include <iostream>
#include <fstream>

// Define constants
const int MAX_ATTR_LENGTH = 100;
const int MAX_TUPLES = 20000000;
const int MAX_RESULT_LENGTH = 256;
const int MAX_COMMAND_LENGTH = 256;


void safeCopyString(char* dest, const char* src, int maxLen) {
    int i = 0;
    while (src[i] != '\0' && i < maxLen - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

int safeStringLength(const char* str, int maxLen) {
    int len = 0;
    while (str[len] != '\0' && len < maxLen) {
        len++;
    }
    return len;
}

bool safeCompareStrings(const char* str1, const char* str2, int maxLen) {
    int i = 0;
    while (i < maxLen - 1 && str1[i] != '\0' && str2[i] != '\0') {
        if (str1[i] != str2[i]) return false;
        i++;
    }
    return str1[i] == str2[i];
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

    int getNumTuples() const {
        return size;
    }

    void insert(const char* attr1, const char* attr2, int attr3) {
        if (size < capacity) {
            safeCopyString(data[size].attr1, attr1, MAX_ATTR_LENGTH);
            safeCopyString(data[size].attr2, attr2, MAX_ATTR_LENGTH);
            data[size].attr3 = attr3;
            size++;
            std::cout << "Inserted: " << attr1 << ", " << attr2 << ", " << attr3 << std::endl;
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

                safeCopyString(tempResult, "Found: ", MAX_RESULT_LENGTH);
                resultPos = 7;

                int j = 0;
                while (data[i].attr1[j] != '\0' && resultPos < MAX_RESULT_LENGTH - 3) {
                    tempResult[resultPos++] = data[i].attr1[j++];
                }
                tempResult[resultPos++] = ',';
                tempResult[resultPos++] = ' ';

                j = 0;
                while (data[i].attr2[j] != '\0' && resultPos < MAX_RESULT_LENGTH - 3) {
                    tempResult[resultPos++] = data[i].attr2[j++];
                }
                tempResult[resultPos++] = ',';
                tempResult[resultPos++] = ' ';

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

                for (int k = 0; k < numLen / 2; k++) {
                    char temp = numStr[k];
                    numStr[k] = numStr[numLen - 1 - k];
                    numStr[numLen - 1 - k] = temp;
                }

                for (int k = 0; k < numLen && resultPos < MAX_RESULT_LENGTH - 2; k++) {
                    tempResult[resultPos++] = numStr[k];
                }

                tempResult[resultPos++] = '\n';
                tempResult[resultPos] = '\0';

                safeCopyString(result, tempResult, MAX_RESULT_LENGTH);
                std::cout << "Found match: " << result << std::endl;
                return;
            }
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

void parseInputLine(const char* line, char* attr1, char* attr2, int& attr3) {
    int pos = 0;

    attr1[0] = '\0';
    attr2[0] = '\0';
    attr3 = 0;

    if (line[0] == 'I') {
        while (line[pos] != '\0' && line[pos] != '(') pos++;
        if (line[pos] == '\0') return;

        extractValue(line, attr1, pos, MAX_ATTR_LENGTH);
        extractValue(line, attr2, pos, MAX_ATTR_LENGTH);

        while (line[pos] == ' ' || line[pos] == ',') pos++;
        while (line[pos] >= '0' && line[pos] <= '9') {
            attr3 = attr3 * 10 + (line[pos] - '0');
            pos++;
        }
    }
    else if (line[0] == 'S') {
        while (line[pos] != '\0' && (line[pos] != 'W' || line[pos + 1] != 'H')) pos++;
        if (line[pos] == '\0') return;

        pos += 6;

        if (strstr(line + pos, "attr1=") == line + pos) {
            pos += 6;
            int attrPos = 0;
            while (line[pos] != '\0' && line[pos] != ' ' && attrPos < MAX_ATTR_LENGTH - 1) {
                if (line[pos] != ' ') {
                    attr1[attrPos++] = line[pos];
                }
                pos++;
            }
            attr1[attrPos] = '\0';
        }

        while (line[pos] != '\0' && (line[pos] != 'A' || line[pos + 1] != 'N')) pos++;
        if (line[pos] != '\0') {
            pos += 4;
            if (strstr(line + pos, "attr2=") == line + pos) {
                pos += 6;
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

        while (line[pos] != '\0' && (line[pos] != 'A' || line[pos + 1] != 'N')) pos++;
        if (line[pos] != '\0') {
            pos += 4;
            if (strstr(line + pos, "attr3=") == line + pos) {
                pos += 6;
                while (line[pos] >= '0' && line[pos] <= '9') {
                    attr3 = attr3 * 10 + (line[pos] - '0');
                    pos++;
                }
            }
        }
    }
}

void runSingleProcess() {
    Database db;
    std::ifstream inputFile("input.sql");
    std::ofstream outputFile("output.txt", std::ios::out);
    std::ofstream tupleCountFile("tuple_count.txt", std::ios::out);  // Open a file for tuple counts

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

    while (inputFile.getline(command, MAX_COMMAND_LENGTH)) {
        std::cout << "Processing command: " << command << std::endl;

        if (command[0] == 'I') {  // INSERT
            parseInputLine(command, attr1, attr2, attr3);
            db.insert(attr1, attr2, attr3);
        }
        else if (command[0] == 'S') {  // SELECT
            parseInputLine(command, attr1, attr2, attr3);
            char result[MAX_RESULT_LENGTH];
            db.query(attr1, attr2, attr3, result);

            if (result[0] != '\0') {
                outputFile << result;
            }
            else {
                outputFile << "No records found. Query attributes: ";
                outputFile << "attr1=" << attr1 << ", ";
                outputFile << "attr2=" << attr2 << ", ";
                outputFile << "attr3=" << attr3 << "\n";
            }
        }

        // Write the number of tuples after each operation
        tupleCountFile << "Node " << 0 << " has " << db.getNumTuples() << " tuples." << std::endl;
    }

    inputFile.close();
    outputFile.close();
    tupleCountFile.close();  // Close the file
}

void runWorker(int rank, int numWorkers) {
    Database db;
    char buffer1[MAX_ATTR_LENGTH];
    char buffer2[MAX_ATTR_LENGTH];
    int attr3;
    char result[MAX_RESULT_LENGTH];

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
            MPI_Recv(buffer2, MAX_ATTR_LENGTH, MPI_CHAR, 0, 4, MPI_COMM_WORLD, &status);
            MPI_Recv(&attr3, 1, MPI_INT, 0, 5, MPI_COMM_WORLD, &status);

            // Query local database partition
            db.query(buffer1, buffer2, attr3, result);
            MPI_Send(result, MAX_RESULT_LENGTH, MPI_CHAR, 0, 6, MPI_COMM_WORLD);
        }

        // Write the current number of tuples for this worker
        outputFile << "Node " << rank << " has " << db.getNumTuples() << " tuples." << std::endl;
        outputFile.flush();
        //std::cout << "Node " << rank << " has " << db.getNumTuples() << " tuples." << std::endl;
    }

    // Close the output file
    outputFile.close();
}

void runMaster(int numWorkers) {
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

            // Broadcast insert to all workers
            for (int worker = 1; worker <= numWorkers; ++worker) {
                MPI_Send(attr1, MAX_ATTR_LENGTH, MPI_CHAR, worker, 0, MPI_COMM_WORLD);
                MPI_Send(attr2, MAX_ATTR_LENGTH, MPI_CHAR, worker, 1, MPI_COMM_WORLD);
                MPI_Send(&attr3, 1, MPI_INT, worker, 2, MPI_COMM_WORLD);
            }
        }
        else if (command[0] == 'S') {  // SELECT
            parseInputLine(command, attr1, attr2, attr3);
            std::cout << "Parsed SELECT values: " << attr1 << ", " << attr2 << ", " << attr3 << std::endl;

            bool found = false;
            // Query all workers
            for (int worker = 1; worker <= numWorkers; ++worker) {
                MPI_Send(attr1, MAX_ATTR_LENGTH, MPI_CHAR, worker, 3, MPI_COMM_WORLD);
                MPI_Send(attr2, MAX_ATTR_LENGTH, MPI_CHAR, worker, 4, MPI_COMM_WORLD);
                MPI_Send(&attr3, 1, MPI_INT, worker, 5, MPI_COMM_WORLD);

                char result[MAX_RESULT_LENGTH];
                MPI_Status status;
                MPI_Recv(result, MAX_RESULT_LENGTH, MPI_CHAR, worker, 6, MPI_COMM_WORLD, &status);

                if (result[0] != '\0') {
                    outputFile << result;
                    outputFile.flush();
                    found = true;
                    break;  // Once we find a match, we can stop searching
                }
            }

            if (!found) {
                outputFile << "No records found. Query attributes: ";
                outputFile << "attr1=" << attr1 << ", ";
                outputFile << "attr2=" << attr2 << ", ";
                outputFile << "attr3=" << attr3 << "\n";
                outputFile.flush();
            }
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

    // Additional timing variables
    double processStartTime, processEndTime;
    double totalProcessTime = 0.0;

    if (size == 1) {
        processStartTime = MPI_Wtime();
        runSingleProcess();
        processEndTime = MPI_Wtime();
        totalProcessTime = processEndTime - processStartTime;
    }
    else {
        if (rank == 0) {
            processStartTime = MPI_Wtime();
            runMaster(size - 1);
            processEndTime = MPI_Wtime();
            totalProcessTime = processEndTime - processStartTime;
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
        std::cout << "Master Process Time: " << totalProcessTime << " seconds" << std::endl;
    }

    return 0;
}