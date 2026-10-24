#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <cctype>
#include <limits>
using namespace std;

struct OpcodeInfo {
    string mnemonic;
    string opcode;
    int operandCount = 0;
    string operand1;
    string operand2;
    string modrm;
    string extension;
};

struct Node {
    OpcodeInfo info;
    Node* left;
    Node* right;
    Node(const OpcodeInfo& value) : info(value), left(nullptr), right(nullptr) {}
};

string trim(const string& text) {
    size_t start = 0;
    while (start < text.size() && isspace(static_cast<unsigned char>(text[start]))) start++;
    size_t end = text.size();
    while (end > start && isspace(static_cast<unsigned char>(text[end - 1]))) end--;
    return text.substr(start, end - start);
}

string toUpperCase(string text) {
    for (char& c : text) c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
    return text;
}

string toLowerCase(string text) {
    for (char& c : text) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return text;
}

vector<string> splitPipe(const string& line) {
    vector<string> fields;
    string field;
    stringstream ss(line);
    while (getline(ss, field, '|')) fields.push_back(trim(field));
    return fields;
}

Node* searchNode(Node* root, const string& mnemonic) {
    if (!root) return nullptr;
    if (mnemonic == root->info.mnemonic) return root;
    if (mnemonic < root->info.mnemonic) return searchNode(root->left, mnemonic);
    return searchNode(root->right, mnemonic);
}

Node* insertNode(Node* root, const OpcodeInfo& info) {
    if (!root) return new Node(info);
    if (info.mnemonic < root->info.mnemonic) root->left = insertNode(root->left, info);
    else if (info.mnemonic > root->info.mnemonic) root->right = insertNode(root->right, info);
    else {
        if (root->info.opcode.empty()) root->info.opcode = info.opcode;
        if (root->info.operandCount == 0) root->info.operandCount = info.operandCount;
        if (root->info.operand1.empty()) root->info.operand1 = info.operand1;
        if (root->info.operand2.empty()) root->info.operand2 = info.operand2;
        if (root->info.modrm.empty()) root->info.modrm = info.modrm;
        if (root->info.extension.empty()) root->info.extension = info.extension;
    }
    return root;
}

void inorder(Node* root) {
    if (!root) return;
    inorder(root->left);
    cout << root->info.mnemonic << '\n';
    inorder(root->right);
}

void displayBST(Node* root, int space = 0, int gap = 6) {
    if (!root) return;
    space += gap;
    displayBST(root->right, space, gap);
    cout << '\n';
    for (int i = gap; i < space; i++) cout << ' ';
    cout << root->info.mnemonic << '\n';
    displayBST(root->left, space, gap);
}

void deleteBST(Node* root) {
    if (!root) return;
    deleteBST(root->left);
    deleteBST(root->right);
    delete root;
}

bool isRegister(string operand) {
    operand = toLowerCase(trim(operand));
    const vector<string> registers = {"eax", "ebx", "ecx", "edx", "esi", "edi", "esp", "ebp",
                                      "ax", "bx", "cx", "dx", "al", "ah", "bl", "bh", "cl", "ch", "dl", "dh",
                                      "si", "di", "sp", "bp"};
    for (const string& reg : registers) if (operand == reg) return true;
    return false;
}

bool isConstant(const string& operand) {
    string value = trim(operand);
    if (value.empty()) return false;
    size_t i = (value[0] == '+' || value[0] == '-') ? 1 : 0;
    if (i == value.size()) return false;
    for (; i < value.size(); i++) if (!isdigit(static_cast<unsigned char>(value[i]))) return false;
    return true;
}

string getOperandType(const string& rawOperand) {
    string operand = trim(rawOperand);
    if (isRegister(operand)) return "Register";
    if (isConstant(operand)) return "Constant";
    if (operand.size() >= 2 && operand.front() == '[' && operand.back() == ']') return "Memory";
    return "Symbol";
}

bool isString(const string& value) {
    return value.size() >= 2 && value.front() == '"' && value.back() == '"';
}

bool isNumber(const string& value) {
    string s = trim(value);
    if (s.empty()) return false;
    for (char c : s) if (!isdigit(static_cast<unsigned char>(c))) return false;
    return true;
}

bool isDataDirective(string directive) {
    directive = toLowerCase(trim(directive));
    return directive == "db" || directive == "dw" || directive == "dd" || directive == "dq";
}

int getDataSize(string directive) {
    directive = toLowerCase(trim(directive));
    if (directive == "db") return 1;
    if (directive == "dw") return 2;
    if (directive == "dd") return 4;
    if (directive == "dq") return 8;
    return 0;
}

string numberToLittleEndian(unsigned long long number, int bytes) {
    stringstream output;
    output << uppercase << hex << setfill('0');
    for (int i = 0; i < bytes; i++) {
        output << setw(2) << static_cast<unsigned int>(number & 0xFF);
        number >>= 8;
    }
    return output.str();
}

string stringToHex(const string& value) {
    stringstream output;
    output << uppercase << hex << setfill('0');
    for (unsigned char c : value) output << setw(2) << static_cast<unsigned int>(c);
    return output.str();
}

string createDataHex(const string& directive, const string& value) {
    if (isString(value)) return stringToHex(value.substr(1, value.size() - 2));
    if (isNumber(value)) {
        try {
            return numberToLittleEndian(stoull(value), getDataSize(directive));
        } catch (...) {
            return "";
        }
    }
    return "";
}

int getActualDataSize(const string& directive, const string& value) {
    if (isString(value)) return static_cast<int>(value.size() - 2);
    return getDataSize(directive);
}

bool isSeparatorLine(const string& line) {
    string s = trim(line);
    if (s.empty()) return true;
    for (char c : s) if (c != '-' && c != '+' && c != '=' && c != '|' && c != ' ') return false;
    return true;
}

bool loadOpcodeFile(const string& filename, Node*& root) {
    ifstream file(filename);
    if (!file) {
        cout << "Cannot open opcode file: " << filename << '\n';
        return false;
    }
    string line;
    bool inTable = false;
    bool sawStartMarker = false;
    int loaded = 0;
    while (getline(file, line)) {
        string cleaned = trim(line);
        if (cleaned == "===== TABLE START =====") { inTable = true; sawStartMarker = true; continue; }
        if (cleaned == "===== TABLE END =====") break;
        if (cleaned.empty() || isSeparatorLine(cleaned)) continue;
        if (sawStartMarker && !inTable) continue;

        OpcodeInfo info;
        vector<string> fields = splitPipe(cleaned);
        if (cleaned.find('|') != string::npos) {
            if (fields.empty()) continue;
            string first = fields[0];
            string upperFirst = toUpperCase(first);
            if (upperFirst == "INSTRUCTION" || upperFirst == "MNEMONIC" || first.empty()) continue;
            info.mnemonic = toUpperCase(first);
            if (fields.size() > 1) info.opcode = fields[1];
            if (fields.size() > 2 && isNumber(fields[2])) info.operandCount = stoi(fields[2]);
            if (fields.size() > 3) info.operand1 = fields[3];
            if (fields.size() > 4) info.operand2 = fields[4];
            if (fields.size() > 5) info.modrm = fields[5];
            if (fields.size() > 6) info.extension = fields[6];
        } else {
            stringstream ss(cleaned);
            if (!(ss >> info.opcode >> info.mnemonic >> info.operandCount >> info.operand1
                     >> info.operand2 >> info.modrm >> info.extension)) {
                string only = trim(cleaned);
                if (only.find_first_of(" \t") != string::npos) continue;
                info.mnemonic = toUpperCase(only);
            } else {
                info.mnemonic = toUpperCase(info.mnemonic);
            }
        }
        if (info.mnemonic.empty()) continue;
        root = insertNode(root, info);
        loaded++;
    }
    if (loaded == 0) cout << "No mnemonics were loaded. Check the opcode file format.\n";
    return loaded > 0;
}

vector<string> splitOperands(const string& text) {
    vector<string> operands;
    string current;
    int bracketDepth = 0;
    bool inQuotes = false;
    for (char c : text) {
        if (c == '"') inQuotes = !inQuotes;
        if (!inQuotes && c == '[') bracketDepth++;
        if (!inQuotes && c == ']') bracketDepth--;
        if (c == ',' && !inQuotes && bracketDepth == 0) {
            string part = trim(current);
            if (!part.empty()) operands.push_back(part);
            current.clear();
        } else current += c;
    }
    string part = trim(current);
    if (!part.empty()) operands.push_back(part);
    return operands;
}

void displayOperands(const string& filename, Node* root) {
    ifstream file(filename);
    if (!file) {
        cout << "Cannot open assembly file: " << filename << '\n';
        return;
    }
    string line;
    int lineNumber = 0;
    unsigned long long offset = 0;
    while (getline(file, line)) {
        lineNumber++;
        string original = line;
        size_t comment = line.find(';');
        if (comment != string::npos) line = line.substr(0, comment);
        line = trim(line);
        if (line.empty()) continue;

        stringstream ss(line);
        string first;
        ss >> first;
        string lowerFirst = toLowerCase(first);
        if (lowerFirst == "section" || lowerFirst == "global" || lowerFirst == "extern") {
            cout << setw(5) << lineNumber << "  " << setw(8) << setfill('0') << hex << offset
                 << setfill(' ') << "  " << original << dec << '\n';
            continue;
        }
        if (!first.empty() && first.back() == ':') {
            cout << setw(5) << lineNumber << "  " << setw(8) << setfill('0') << hex << offset
                 << setfill(' ') << "  " << original << dec << '\n';
            continue;
        }

        string second;
        ss >> second;
        if (!second.empty() && isDataDirective(second)) {
            string value;
            getline(ss, value);
            value = trim(value);
            if (!isNumber(value) && !isString(value)) {
                cout << "Invalid Data at line " << lineNumber << ": " << original << '\n';
                continue;
            }
            string hexData = createDataHex(second, value);
            cout << setw(5) << lineNumber << "  " << uppercase << hex << setw(8) << setfill('0')
                 << offset << setfill(' ') << "  " << left << setw(24) << hexData << " "
                 << first << " " << second << " " << value << right << dec << '\n';
            offset += static_cast<unsigned long long>(getActualDataSize(second, value));
            continue;
        }

        string mnemonic = first;
        string operandText;
        getline(ss, operandText);
        if (!second.empty()) operandText = second + operandText;
        if (!mnemonic.empty() && mnemonic.back() == ':') {
            cout << setw(5) << lineNumber << "  " << original << '\n';
            continue;
        }
        mnemonic = toUpperCase(mnemonic);
        Node* match = searchNode(root, mnemonic);
        vector<string> operands = splitOperands(operandText);
        cout << "\nLine " << lineNumber << " | Mnemonic: " << mnemonic
             << " | Status: " << (match ? "Found" : "Not Found") << '\n';
        if (match && !match->info.opcode.empty()) cout << "Opcode: " << match->info.opcode << '\n';
        for (size_t i = 0; i < operands.size(); i++) {
            cout << "Operand " << (i + 1) << ": " << operands[i]
                 << " | Type: " << getOperandType(operands[i]) << '\n';
        }
    }
}

int main() {
    Node* root = nullptr;
    string opcodeFilename, assemblyFilename;
    cout << "Enter opcode file name: ";
    getline(cin, opcodeFilename);
    cout << "Enter assembly file name: ";
    getline(cin, assemblyFilename);

    if (!loadOpcodeFile(opcodeFilename, root)) {
        deleteBST(root);
        return 1;
    }

    int choice;
    do {
        cout << "\n1. Display Mnemonics\n2. Display BST\n3. Search Mnemonic\n"
             << "4. Display Operands and Their Types\n5. Exit\nEnter choice: ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid choice.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (choice == 1) {
            inorder(root);
        } else if (choice == 2) {
            displayBST(root);
        } else if (choice == 3) {
            string mnemonic;
            cout << "Enter mnemonic to search: ";
            getline(cin, mnemonic);
            mnemonic = toUpperCase(trim(mnemonic));
            Node* found = searchNode(root, mnemonic);
            if (found) {
                cout << "Mnemonic found: " << found->info.mnemonic << '\n';
                if (!found->info.opcode.empty()) cout << "Opcode: " << found->info.opcode << '\n';
            } else cout << "Mnemonic not found.\n";
        } else if (choice == 4) {
            displayOperands(assemblyFilename, root);
        } else if (choice != 5) {
            cout << "Invalid choice.\n";
        }
    } while (choice != 5);

    deleteBST(root);
    return 0;
}
