#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <cctype>
#include <limits>
#include <map>
#include <algorithm>
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


// ---------------- ONE-PASS ASSEMBLER EXTENSION ----------------
struct AsmLine {
    int lineNo = 0;
    string section, source, label, mnemonic, operandText;
    vector<string> operands;
    unsigned int address = 0;
    vector<unsigned char> bytes;
};
struct Fixup { string symbol; int lineNo; size_t byteIndex; unsigned int nextAddress; string section; };
struct SymbolInfo { string name, section; unsigned int address; int lineNo; int forwardRefs = 0, backwardRefs = 0; };

static string lowerText(string s) { for (char &c : s) c = (char)tolower((unsigned char)c); return s; }
static string stripComment(string s) { size_t p=s.find(';'); if(p!=string::npos) s=s.substr(0,p); return trim(s); }
static bool parseInteger(string s, long long &v) {
    s=trim(s); if(s.empty()) return false;
    try { size_t n=0; v=stoll(s,&n,0); return n==s.size(); } catch(...) { return false; }
}
static void appendLE(vector<unsigned char>& b, unsigned long long v, int n) { for(int i=0;i<n;i++){ b.push_back((unsigned char)(v&255)); v >>= 8; } }
static string bytesHex(const vector<unsigned char>& b) {
    stringstream ss; ss << uppercase << hex << setfill('0');
    for(size_t i=0;i<b.size();++i){ if(i) ss << ' '; ss << setw(2) << (unsigned)b[i]; }
    return ss.str();
}
static size_t findLabelColonOutsideQuotes(const string& s) {
    bool inSingle = false, inDouble = false;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '\'' && !inDouble && (i == 0 || s[i-1] != '\\')) inSingle = !inSingle;
        else if (c == '"' && !inSingle && (i == 0 || s[i-1] != '\\')) inDouble = !inDouble;
        else if (c == ':' && !inSingle && !inDouble) return i;
    }
    return string::npos;
}

static bool parseCharacterLiteral(const string& raw, long long& value) {
    string s = trim(raw);
    if (s.size() < 3 || s.front() != '\'' || s.back() != '\'') return false;
    string body = s.substr(1, s.size()-2);
    if (body.size() == 1) { value = (unsigned char)body[0]; return true; }
    if (body == "\\n") { value = 10; return true; }
    if (body == "\\r") { value = 13; return true; }
    if (body == "\\t") { value = 9; return true; }
    if (body == "\\0") { value = 0; return true; }
    if (body == "\\'") { value = '\''; return true; }
    return false;
}

static bool validIdentifier(const string& s) {
    if(s.empty() || !(isalpha((unsigned char)s[0]) || s[0]=='_' || s[0]=='.')) return false;
    for(char c:s) if(!(isalnum((unsigned char)c)||c=='_'||c=='.')) return false;
    return true;
}

void runOnePassAssembler(const string& filename) {
    ifstream in(filename);
    if (!in) { cerr << "Cannot open assembly file: " << filename << "\n"; return; }

    vector<AsmLine> lines;
    vector<string> errors;
    vector<Fixup> fixups;
    map<string, SymbolInfo> symbols;
    map<string, long long> constants;
    vector<string> externals;
    string line, section = "text";
    int lineNo = 0;
    unsigned int dataLoc = 0, bssLoc = 0, textLoc = 0;

    // NASM-style macro preprocessing: %macro name N ... %endmacro
    struct MacroDef { int argc = 0; vector<string> body; };
    map<string, MacroDef> macros;
    vector<pair<int, string>> sourceLines;
    vector<pair<int, string>> rawLines;
    {
        ifstream macroInput(filename);
        string raw; int rawNo = 0;
        while (getline(macroInput, raw)) rawLines.push_back({++rawNo, raw});
    }
    for (size_t i = 0; i < rawLines.size(); ++i) {
        string clean = stripComment(rawLines[i].second);
        string low = lowerText(clean);
        if (low.rfind("%macro", 0) == 0) {
            stringstream ms(clean);
            string directive, name; int argc = 0;
            ms >> directive >> name >> argc;
            MacroDef def; def.argc = argc;
            bool ended = false;
            while (++i < rawLines.size()) {
                string bodyClean = stripComment(rawLines[i].second);
                string bodyLow = lowerText(bodyClean);
                if (bodyLow.rfind("%endmacro", 0) == 0) { ended = true; break; }
                def.body.push_back(rawLines[i].second);
            }
            if (name.empty() || argc < 0 || !ended) {
                errors.push_back("Line " + to_string(rawLines[i < rawLines.size() ? i : rawLines.size()-1].first) +
                                 ": Invalid or unterminated %macro definition");
            } else macros[lowerText(name)] = def;
            continue;
        }
        if (low.rfind("%endmacro", 0) == 0 || low.rfind("%define", 0) == 0) continue;
        sourceLines.push_back(rawLines[i]);
    }
    // Expand macro calls before the one-pass assembly. Expanded statements retain
    // the invocation line number so diagnostics point to the user's macro call.
    vector<pair<int, string>> expandedLines;
    for (const auto& src : sourceLines) {
        string clean = stripComment(src.second);
        string first;
        stringstream ss(clean); ss >> first;
        string key = lowerText(first);
        auto it = macros.find(key);
        if (it == macros.end()) {
            expandedLines.push_back(src);
            continue;
        }
        string argText;
        getline(ss, argText);
        vector<string> args = splitOperands(trim(argText));
        if ((int)args.size() != it->second.argc) {
            errors.push_back("Line " + to_string(src.first) + ": Macro '" + first +
                             "' expects " + to_string(it->second.argc) + " argument(s), got " +
                             to_string(args.size()));
            continue;
        }
        for (string bodyLine : it->second.body) {
            for (int aidx = (int)args.size(); aidx >= 1; --aidx) {
                string marker = "%" + to_string(aidx);
                size_t pos = 0;
                while ((pos = bodyLine.find(marker, pos)) != string::npos) {
                    bodyLine.replace(pos, marker.size(), args[aidx-1]);
                    pos += args[aidx-1].size();
                }
            }
            expandedLines.push_back({src.first, bodyLine});
        }
    }

    auto currentLoc = [&]() -> unsigned int {
        if (section == "data") return dataLoc;
        if (section == "bss") return bssLoc;
        return textLoc;
    };
    auto addSymbol = [&](const string& rawName, const string& sec, unsigned int addr, int ln) {
        // Use one normalized key for declarations and references.
        string name = lowerText(trim(rawName));
        if (!validIdentifier(name)) errors.push_back("Line " + to_string(ln) + ": Invalid label '" + rawName + "'");
        else if (symbols.count(name)) errors.push_back("Line " + to_string(ln) + ": Duplicate symbol '" + rawName + "'");
        else symbols[name] = {name, sec, addr, ln, 0, 0};
    };
    auto regCode = [](string r) -> int {
        r = lowerText(trim(r));
        if (r=="eax") return 0; if (r=="ecx") return 1; if (r=="edx") return 2; if (r=="ebx") return 3;
        if (r=="esp") return 4; if (r=="ebp") return 5; if (r=="esi") return 6; if (r=="edi") return 7;
        return -1;
    };
    auto byteRegCode = [](string r) -> int {
        r = lowerText(trim(r));
        if(r=="al") return 0; if(r=="cl") return 1; if(r=="dl") return 2; if(r=="bl") return 3;
        if(r=="ah") return 4; if(r=="ch") return 5; if(r=="dh") return 6; if(r=="bh") return 7;
        return -1;
    };
    auto parseValue = [&](string raw, long long& value) -> bool {
        string key=lowerText(trim(raw));
        if(parseInteger(key,value)) return true;
        auto it=constants.find(key); if(it!=constants.end()){value=it->second;return true;}
        return false;
    };
    auto memoryInner = [](string operand) -> string {
        operand = lowerText(trim(operand));
        for (const string prefix : {"dword", "word", "byte", "qword", "oword"}) {
            if (operand.rfind(prefix, 0) == 0) { operand = trim(operand.substr(prefix.size())); break; }
        }
        if (operand.size() >= 2 && operand.front() == '[' && operand.back() == ']')
            return trim(operand.substr(1, operand.size()-2));
        return "";
    };
    auto memorySymbol = [&](string operand) -> string {
        string inside = memoryInner(operand);
        if (inside.empty()) return "";
        // [eax], [ebx+4], etc. are register-based addresses, not symbols.
        string compact; for (char c : inside) if (!isspace((unsigned char)c)) compact += c;
        for (const string r : {"eax","ecx","edx","ebx","esp","ebp","esi","edi"}) {
            if (compact == r || compact.rfind(r+"+",0)==0 || compact.rfind(r+"-",0)==0) return "";
        }
        return inside;
    };
    auto emitModRM = [&](vector<unsigned char>& out, int regField, string rmOperand, unsigned int sourceLine) -> bool {
        string inside = memoryInner(rmOperand);
        if (inside.empty()) return false;
        string expr; for (char c : inside) if (!isspace((unsigned char)c)) expr += c;
        string base; int disp = 0; bool hasDisp = false;
        size_t plus = expr.find('+',1), minus = expr.find('-',1);
        size_t split = string::npos;
        if (plus != string::npos) split=plus; if (minus != string::npos && (split==string::npos || minus<split)) split=minus;
        if (split != string::npos) {
            base=expr.substr(0,split); string ds=expr.substr(split);
            long long dv; if (parseInteger(ds,dv)) { disp=(int)dv; hasDisp=true; } else return false;
        } else base=expr;
        int baseReg=regCode(base);
        if (baseReg>=0) {
            if (!hasDisp && baseReg!=5 && baseReg!=4) out.push_back((unsigned char)(((regField&7)<<3)|baseReg));
            else if (disp>=-128 && disp<=127) {
                out.push_back((unsigned char)(0x40|((regField&7)<<3)|(baseReg==4?4:baseReg)));
                if (baseReg==4) out.push_back(0x24); // SIB: no index, ESP base
                out.push_back((unsigned char)disp);
            } else { out.push_back((unsigned char)(0x80|((regField&7)<<3)|(baseReg==4?4:baseReg))); if(baseReg==4) out.push_back(0x24); appendLE(out,(unsigned int)disp,4); }
            return true;
        }
        // Absolute symbol address: mod=00, r/m=101, disp32; resolved after the pass.
        if (!validIdentifier(base)) return false;
        out.push_back((unsigned char)(0x05|((regField&7)<<3)));
        size_t pos=out.size(); appendLE(out,0,4);
        auto it=symbols.find(base);
        if(it!=symbols.end()) { unsigned int v=it->second.address; for(int k=0;k<4;k++) out[pos+k]=(v>>(8*k))&255; }
        else fixups.push_back({base,(int)sourceLine,pos,0,"data"});
        return true;
    };

    for (const auto& sourceEntry : expandedLines) {
        lineNo = sourceEntry.first;
        line = sourceEntry.second;
        string original = line;
        string clean = stripComment(line);
        if (clean.empty()) continue;

        AsmLine a; a.lineNo=lineNo; a.source=trim(original); a.section=section;
        // Labels may be written as "main:" or as "label directive value".
        size_t colon=findLabelColonOutsideQuotes(clean);
        if (colon!=string::npos) {
            string label=trim(clean.substr(0,colon));
            addSymbol(label, section, currentLoc(), lineNo);
            a.label=label;
            clean=trim(clean.substr(colon+1));
            if (clean.empty()) { a.address=currentLoc(); a.section=section; lines.push_back(a); continue; }
        }

        string first, second;
        stringstream ss(clean); ss >> first;
        string firstLower=lowerText(first);
        if (firstLower=="section") {
            ss >> second; second=lowerText(second);
            if (second==".data" || second=="data") section="data";
            else if (second==".bss" || second=="bss") section="bss";
            else if (second==".text" || second=="text" || second==".main" || second=="main") section="text";
            else errors.push_back("Line "+to_string(lineNo)+": Unknown section '"+second+"'");
            a.section=section; a.address=currentLoc(); lines.push_back(a); continue;
        }
        if (firstLower=="global" || firstLower=="bits" || firstLower=="default") {
            a.section=section; a.address=currentLoc(); lines.push_back(a); continue;
        }
        if (firstLower=="extern") {
            string ext; ss >> ext; if(!ext.empty()) externals.push_back(lowerText(ext));
            a.section=section; a.address=currentLoc(); lines.push_back(a); continue;
        }

        // NASM data/BSS declaration: "name dd value", "name resd count".
        string directiveOrMnemonic, rest;
        ss >> directiveOrMnemonic;
        getline(ss, rest); rest=trim(rest);
        string d=lowerText(directiveOrMnemonic);
        auto isDirective = [](const string& x) {
            string v=lowerText(x);
            return v=="db"||v=="dw"||v=="dd"||v=="dq"||v=="resb"||v=="resw"||v=="resd"||v=="resq";
        };
        // EQU constants: support forms such as len equ 12 and len equ $ - string.
        if (lowerText(directiveOrMnemonic)=="equ") {
            string key=lowerText(first); long long value=0; bool ok=false;
            string expr=trim(rest); string exprLow=lowerText(expr);
            size_t minus=exprLow.find('-');
            if (minus!=string::npos) {
                string lhs=trim(exprLow.substr(0,minus)); string rhs=trim(exprLow.substr(minus+1));
                if (lhs=="$") {
                    auto sit=symbols.find(rhs);
                    if (sit!=symbols.end() && sit->second.section=="data") { value=(long long)dataLoc-(long long)sit->second.address; ok=true; }
                    else if (sit!=symbols.end() && sit->second.section=="text") { value=(long long)textLoc-(long long)sit->second.address; ok=true; }
                }
            }
            if (!ok) ok=parseInteger(expr,value);
            if (!ok) errors.push_back("Line "+to_string(lineNo)+": Unsupported EQU expression '"+expr+"'");
            else { constants[key]=value; symbols[key]={key,"equ",(unsigned int)value,lineNo,0,0}; }
            a.section=section; a.address=currentLoc(); lines.push_back(a); continue;
        }
        if (isDirective(d)) {
            // Label declarations are accepted in data/BSS and custom sections too.
            string declarationSection = (d=="resb"||d=="resw"||d=="resd"||d=="resq") ? "bss" : "data";
            unsigned int declarationAddress = declarationSection=="bss" ? bssLoc : dataLoc;
            addSymbol(first, declarationSection, declarationAddress, lineNo);
            a.label=first; a.mnemonic=d; a.operandText=rest;
            a.section=declarationSection; a.address=declarationAddress;
        } else {
            a.mnemonic=firstLower;
            a.operandText=trim(clean.substr(first.size()));
            a.section=section; a.address=currentLoc();
        }

        a.operands=splitOperands(a.operandText);

        if (a.section=="data") {
            if (a.mnemonic=="db"||a.mnemonic=="dw"||a.mnemonic=="dd"||a.mnemonic=="dq") {
                int width=(a.mnemonic=="db"?1:a.mnemonic=="dw"?2:a.mnemonic=="dd"?4:8);
                for (string op : a.operands) {
                    op=trim(op);
                    if(op.size()>=2 && ((op.front()=='"'&&op.back()=='"')||(op.front()=='\''&&op.back()=='\''))) {
                        for(size_t i=1;i+1<op.size();++i) a.bytes.push_back((unsigned char)op[i]);
                    } else {
                        long long n;
                        if(!parseInteger(op,n)) errors.push_back("Line "+to_string(lineNo)+": Invalid data value '"+op+"'");
                        else appendLE(a.bytes,(unsigned long long)n,width);
                    }
                }
                dataLoc += (unsigned int)a.bytes.size();
            } else errors.push_back("Line "+to_string(lineNo)+": Unsupported data directive/instruction '"+a.mnemonic+"'");
            lines.push_back(a); continue;
        }

        if (a.section=="bss") {
            int width=(a.mnemonic=="resb"?1:a.mnemonic=="resw"?2:a.mnemonic=="resd"?4:a.mnemonic=="resq"?8:0);
            long long count=0; string countText=a.operands.size()==1?lowerText(trim(a.operands[0])):"";
            bool countOK = a.operands.size()==1 && parseInteger(countText,count);
            if (!countOK && a.operands.size()==1) { auto cit=constants.find(countText); if(cit!=constants.end()){count=cit->second;countOK=true;} }
            if(width==0) errors.push_back("Line "+to_string(lineNo)+": Unsupported BSS directive '"+a.mnemonic+"'");
            else if(!countOK || count<0) errors.push_back("Line "+to_string(lineNo)+": Invalid reservation count for '"+a.mnemonic+"'");
            else bssLoc += (unsigned int)(width*count);
            lines.push_back(a); continue;
        }

        auto badOperands=[&](int n){ if((int)a.operands.size()!=n) { errors.push_back("Line "+to_string(lineNo)+": '"+a.mnemonic+"' expects "+to_string(n)+" operand(s)"); return true;} return false; };
        auto encodeRegRM = [&](unsigned char opcode, int regField, const string& rm, bool rmIsRegister) -> bool {
            a.bytes.push_back(opcode);
            int rmReg = regCode(rm);
            if (rmReg >= 0 && rmIsRegister) { a.bytes.push_back((unsigned char)(0xC0 | ((regField&7)<<3) | rmReg)); return true; }
            size_t before=a.bytes.size();
            if (emitModRM(a.bytes, regField, rm, (unsigned int)lineNo)) return true;
            a.bytes.resize(before-1); return false;
        };
        if (a.mnemonic=="mov" && !badOperands(2)) {
            int dst=regCode(a.operands[0]), src=regCode(a.operands[1]);
            int bdst=byteRegCode(a.operands[0]), bsrc=byteRegCode(a.operands[1]); long long n=0;
            string memDst=memorySymbol(a.operands[0]), memSrc=memorySymbol(a.operands[1]);
            string inDst=memoryInner(a.operands[0]), inSrc=memoryInner(a.operands[1]);
            if(bdst>=0 && !inSrc.empty()) { a.bytes.push_back(0x8A); if(!emitModRM(a.bytes,bdst,a.operands[1],(unsigned int)lineNo)) errors.push_back("Line "+to_string(lineNo)+": Invalid byte MOV source '"+a.operands[1]+"'"); }
            else if(!inDst.empty() && bsrc>=0) { a.bytes.push_back(0x88); if(!emitModRM(a.bytes,bsrc,a.operands[0],(unsigned int)lineNo)) errors.push_back("Line "+to_string(lineNo)+": Invalid byte MOV destination '"+a.operands[0]+"'"); }
            else if(dst>=0 && parseValue(a.operands[1],n)) { a.bytes.push_back((unsigned char)(0xB8+dst)); appendLE(a.bytes,(unsigned long long)n,4); }
            else if(dst>=0 && !inSrc.empty()) { if(!encodeRegRM(0x8B,dst,a.operands[1],false)) errors.push_back("Line "+to_string(lineNo)+": Invalid MOV memory operand '"+a.operands[1]+"'"); }
            else if(!inDst.empty() && src>=0) { if(!encodeRegRM(0x89,src,a.operands[0],false)) errors.push_back("Line "+to_string(lineNo)+": Invalid MOV memory operand '"+a.operands[0]+"'"); }
            else if(!inDst.empty() && parseValue(a.operands[1],n)) {
                // MOV r/m32, imm32: C7 /0 id (e.g. mov dword[three], 3).
                a.bytes.push_back(0xC7);
                if (emitModRM(a.bytes, 0, a.operands[0], (unsigned int)lineNo)) appendLE(a.bytes, (unsigned long long)n, 4);
                else errors.push_back("Line "+to_string(lineNo)+": Invalid MOV memory destination '"+a.operands[0]+"'");
            }
            else if(dst>=0 && src>=0) { a.bytes.push_back(0x89); a.bytes.push_back((unsigned char)(0xC0+(src*8)+dst)); }
            else if(dst>=0 && validIdentifier(a.operands[1])) { a.bytes.push_back((unsigned char)(0xB8+dst)); size_t pos=a.bytes.size(); appendLE(a.bytes,0,4); fixups.push_back({lowerText(a.operands[1]),lineNo,pos,a.address,"data"}); }
            else errors.push_back("Line "+to_string(lineNo)+": Unsupported MOV operands '"+a.operandText+"'");
        } else if ((a.mnemonic=="add" || a.mnemonic=="sub" || a.mnemonic=="xor" || a.mnemonic=="cmp") && !badOperands(2)) {
            int r=regCode(a.operands[0]), r2=regCode(a.operands[1]); long long imm=0;
            unsigned char rrOpcode = a.mnemonic=="add" ? 0x01 : a.mnemonic=="sub" ? 0x29 : a.mnemonic=="xor" ? 0x31 : 0x39;
            unsigned char regOpcode = a.mnemonic=="add" ? 0x03 : a.mnemonic=="sub" ? 0x2B : a.mnemonic=="xor" ? 0x33 : 0x3B;
            if(r>=0 && r2>=0) { a.bytes.push_back(rrOpcode); a.bytes.push_back((unsigned char)(0xC0+(r2*8)+r)); }
            else if(r>=0 && parseValue(a.operands[1],imm)) { a.bytes.push_back(0x81); a.bytes.push_back((unsigned char)(0xC0 | ((a.mnemonic=="add"?0:a.mnemonic=="sub"?5:a.mnemonic=="xor"?6:7)<<3) | r)); appendLE(a.bytes,(unsigned int)imm,4); }
            else if(r>=0 && !memoryInner(a.operands[1]).empty()) { if(!encodeRegRM(regOpcode,r,a.operands[1],false)) errors.push_back("Line "+to_string(lineNo)+": Invalid "+a.mnemonic+" memory operand '"+a.operands[1]+"'"); }
            else if(!memoryInner(a.operands[0]).empty() && r2>=0) { if(!encodeRegRM(rrOpcode,r2,a.operands[0],false)) errors.push_back("Line "+to_string(lineNo)+": Invalid "+a.mnemonic+" memory operand '"+a.operands[0]+"'"); }
            else if(!memoryInner(a.operands[0]).empty() && parseCharacterLiteral(a.operands[1], imm)) {
                // CMP r/m32, imm8: 83 /7 ib (character constants such as ' ')
                a.bytes.push_back(0x83);
                if (emitModRM(a.bytes, 7, a.operands[0], (unsigned int)lineNo))
                    a.bytes.push_back((unsigned char)imm);
                else errors.push_back("Line "+to_string(lineNo)+": Invalid "+toUpperCase(a.mnemonic)+" memory operand '"+a.operands[0]+"'");
            }
            else if(!memoryInner(a.operands[0]).empty() && parseInteger(a.operands[1], imm)) {
                int ext = a.mnemonic=="add" ? 0 : a.mnemonic=="sub" ? 5 : a.mnemonic=="xor" ? 6 : 7;
                if (imm >= -128 && imm <= 127) {
                    a.bytes.push_back(0x83);
                    if (emitModRM(a.bytes, ext, a.operands[0], (unsigned int)lineNo))
                        a.bytes.push_back((unsigned char)imm);
                    else errors.push_back("Line "+to_string(lineNo)+": Invalid "+toUpperCase(a.mnemonic)+" memory operand '"+a.operands[0]+"'");
                } else {
                    a.bytes.push_back(0x81);
                    if (emitModRM(a.bytes, ext, a.operands[0], (unsigned int)lineNo))
                        appendLE(a.bytes, (unsigned int)imm, 4);
                    else errors.push_back("Line "+to_string(lineNo)+": Invalid "+toUpperCase(a.mnemonic)+" memory operand '"+a.operands[0]+"'");
                }
            }
            else errors.push_back("Line "+to_string(lineNo)+": Unsupported "+toUpperCase(a.mnemonic)+" operands '"+a.operandText+"'");
        } else if(a.mnemonic=="div" && !badOperands(1)) {
            int r=regCode(a.operands[0]);
            if(r>=0) { a.bytes.push_back(0xF7); a.bytes.push_back((unsigned char)(0xF0|r)); }
            else if(!memoryInner(a.operands[0]).empty()) { a.bytes.push_back(0xF7); if(!emitModRM(a.bytes,6,a.operands[0],lineNo)) errors.push_back("Line "+to_string(lineNo)+": Invalid DIV operand '"+a.operands[0]+"'"); }
            else errors.push_back("Line "+to_string(lineNo)+": Invalid DIV operand '"+a.operands[0]+"'");
        } else if(a.mnemonic=="pusha" && badOperands(0)==false) a.bytes.push_back(0x60);
        else if(a.mnemonic=="popa" && badOperands(0)==false) a.bytes.push_back(0x61);
        else if((a.mnemonic=="jg"||a.mnemonic=="jge"||a.mnemonic=="jl"||a.mnemonic=="jle"||a.mnemonic=="je"||a.mnemonic=="jne"||a.mnemonic=="jz"||a.mnemonic=="jnz"||a.mnemonic=="ja"||a.mnemonic=="jb"||a.mnemonic=="jmp") && !badOperands(1)) {
            unsigned char op=a.mnemonic=="jg"?0x7F:a.mnemonic=="jge"?0x7D:a.mnemonic=="jl"?0x7C:a.mnemonic=="jle"?0x7E:(a.mnemonic=="je"||a.mnemonic=="jz")?0x74:(a.mnemonic=="jne"||a.mnemonic=="jnz")?0x75:a.mnemonic=="ja"?0x77:a.mnemonic=="jb"?0x72:0xEB;
            a.bytes.push_back(op); size_t pos=a.bytes.size(); a.bytes.push_back(0);
            string target=lowerText(trim(a.operands[0])); auto it=symbols.find(target);
            if(it!=symbols.end() && it->second.section=="text") { int rel=(int)it->second.address-(int)(a.address+2); if(rel < -128 || rel > 127) errors.push_back("Line "+to_string(lineNo)+": Short jump target out of range: '"+target+"'"); else { a.bytes[pos]=(unsigned char)(signed char)rel; it->second.backwardRefs++; } }
            else fixups.push_back({target,lineNo,pos,a.address+2,"text"});
        } else if((a.mnemonic=="inc" || a.mnemonic=="dec") && !badOperands(1)) {
            int r = regCode(a.operands[0]);
            if (r >= 0) a.bytes.push_back((unsigned char)((a.mnemonic=="inc" ? 0x40 : 0x48) + r));
            else errors.push_back("Line "+to_string(lineNo)+": Unsupported "+toUpperCase(a.mnemonic)+" operand '"+a.operands[0]+"'");
        } else if(a.mnemonic=="ret" && badOperands(0)==false) a.bytes.push_back(0xC3);
        else if((a.mnemonic=="push"||a.mnemonic=="pop") && !badOperands(1)) {
            string op=trim(a.operands[0]); int r=regCode(op); long long imm; string mem=memorySymbol(op);
            if(r>=0) a.bytes.push_back((unsigned char)((a.mnemonic=="push"?0x50:0x58)+r));
            else if(a.mnemonic=="push" && parseInteger(op,imm) && imm>=-128 && imm<=127) { a.bytes.push_back(0x6A); a.bytes.push_back((unsigned char)imm); }
            else if(a.mnemonic=="push" && !memoryInner(op).empty()) { a.bytes.push_back(0xFF); if(!emitModRM(a.bytes,6,op,lineNo)) errors.push_back("Line "+to_string(lineNo)+": Invalid PUSH memory operand '"+op+"'"); }
            else if(a.mnemonic=="push" && validIdentifier(op)) { a.bytes.push_back(0x68); size_t pos=a.bytes.size(); appendLE(a.bytes,0,4); auto it=symbols.find(lowerText(op)); if(it!=symbols.end()) { unsigned int v=it->second.address; for(int k=0;k<4;k++) a.bytes[pos+k]=(v>>(8*k))&255; } else if(find(externals.begin(),externals.end(),lowerText(op))==externals.end()) fixups.push_back({lowerText(op),lineNo,pos,a.address,"data"}); }
            else if(a.mnemonic=="pop" && !memoryInner(op).empty()) { a.bytes.push_back(0x8F); if(!emitModRM(a.bytes,0,op,lineNo)) errors.push_back("Line "+to_string(lineNo)+": Invalid POP memory operand '"+op+"'"); }
            else errors.push_back("Line "+to_string(lineNo)+": Unsupported "+a.mnemonic+" operand '"+op+"'");
        } else if(a.mnemonic=="call" && !badOperands(1)) {
            string target=lowerText(trim(a.operands[0])); a.bytes.push_back(0xE8); appendLE(a.bytes,0,4);
            if(find(externals.begin(),externals.end(),target)==externals.end() && !symbols.count(target)) fixups.push_back({target,lineNo,1,a.address+5,"text"});
        } else if(a.mnemonic=="nop" && badOperands(0)==false) a.bytes.push_back(0x90);
        else errors.push_back("Line "+to_string(lineNo)+": Mnemonic/format not supported: '"+a.mnemonic+"'");
        textLoc += (unsigned int)a.bytes.size();
        lines.push_back(a);
    }

    // Resolve recorded symbol references after the single source pass.
    for(const Fixup& f:fixups) {
        auto it=symbols.find(f.symbol);
        if(it==symbols.end()) {
            bool isExternal=find(externals.begin(),externals.end(),f.symbol)!=externals.end();
            if(!isExternal) errors.push_back("Line "+to_string(f.lineNo)+": Undefined symbol '"+f.symbol+"'");
            continue;
        }
        for(auto& a:lines) if(a.lineNo==f.lineNo) {
            if (f.byteIndex < a.bytes.size() && f.byteIndex + 1 == a.bytes.size() && f.section=="text") {
                // Short conditional/unconditional jump fixup (rel8).
                long long rel = (long long)it->second.address - (long long)f.nextAddress;
                if (rel < -128 || rel > 127) errors.push_back("Line "+to_string(f.lineNo)+": Short jump target out of range: '"+f.symbol+"'");
                else a.bytes[f.byteIndex]=(unsigned char)(signed char)rel;
            } else if(f.byteIndex+3<a.bytes.size()) {
                unsigned int v = (f.section=="text" ? (unsigned int)((long long)it->second.address-(long long)f.nextAddress) : it->second.address);
                for(int k=0;k<4;k++) a.bytes[f.byteIndex+k]=(unsigned char)((v>>(8*k))&255);
            }
            if(it->second.section=="text" && f.section=="text") {
                if(it->second.address < f.nextAddress) it->second.backwardRefs++;
                else it->second.forwardRefs++;
            }
            break;
        }
    }

    ofstream err("error.txt",ios::trunc);
    if(!errors.empty()) {
        for(const string& e:errors) err<<e<<'\n';
        err.close(); ofstream obj("object.txt",ios::trunc); obj.close();
        cout<<"\nAssembly failed. Errors written to error.txt; object.txt is empty.\n";
        for(const string& e:errors) cout<<e<<'\n';
        return;
    }
    err.close();
    ofstream obj("object.txt",ios::trunc);
    cout<<"\nData Section :\n\n"; obj<<"Data Section :\n\n";
    for(const auto& a:lines) if(a.section=="data"&&!a.bytes.empty()) {
        stringstream ad; ad<<uppercase<<hex<<setfill('0')<<setw(8)<<a.address;
        cout<<ad.str()<<"  "<<bytesHex(a.bytes)<<"    "<<a.source<<'\n';
        obj<<ad.str()<<"  "<<bytesHex(a.bytes)<<"    "<<a.source<<'\n';
    }
    cout<<"\nBSS Section :\n\n"; obj<<"\nBSS Section :\n\n";
    for(const auto& kv:symbols) if(kv.second.section=="bss") {
        stringstream ad; ad<<uppercase<<hex<<setfill('0')<<setw(8)<<kv.second.address;
        cout<<ad.str()<<"  <reserved>    "<<kv.first<<'\n';
        obj<<ad.str()<<"  <reserved>    "<<kv.first<<'\n';
    }
    cout<<"\nMain Section :\n\n"; obj<<"\nMain Section :\n\n";
    for(const auto& a:lines) if(a.section=="text"&&!a.bytes.empty()) {
        stringstream ad; ad<<uppercase<<hex<<setfill('0')<<setw(8)<<a.address;
        cout<<ad.str()<<"  "<<bytesHex(a.bytes)<<"    "<<a.source<<'\n';
        obj<<ad.str()<<"  "<<bytesHex(a.bytes)<<"    "<<a.source<<'\n';
    }
    cout<<"\nSymbol Table:\nSymbol\tSection\tAddress\tForward refs\tBackward refs\n";
    obj<<"\nSymbol Table:\nSymbol\tSection\tAddress\tForward refs\tBackward refs\n";
    for(const auto& kv:symbols) {
        stringstream ad; ad<<uppercase<<hex<<setfill('0')<<setw(8)<<kv.second.address;
        cout<<kv.first<<'\t'<<kv.second.section<<'\t'<<ad.str()<<'\t'<<kv.second.forwardRefs<<'\t'<<kv.second.backwardRefs<<'\n';
        obj<<kv.first<<'\t'<<kv.second.section<<'\t'<<ad.str()<<'\t'<<kv.second.forwardRefs<<'\t'<<kv.second.backwardRefs<<'\n';
    }
    obj.close();
    cout<<"\nAssembly successful. Output saved to object.txt; error.txt is empty.\n";
}
// ---------------- END ONE-PASS ASSEMBLER EXTENSION ----------------

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
             << "4. Display Operands and Their Types\n5. Run One-Pass Assembler\n6. Exit\nEnter choice: ";
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
        } else if (choice == 5) {
            runOnePassAssembler(assemblyFilename);
        } else if (choice != 6) {
            cout << "Invalid choice.\n";
        }
    } while (choice != 6);

    deleteBST(root);
    return 0;
}
