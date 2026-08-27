#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cctype>

using namespace std;

struct Node
{
    string mnemonic;
    Node* left;
    Node* right;

    Node(string value)
    {
        mnemonic = value;
        left = NULL;
        right = NULL;
    }
};

string trim(string str)
{
    size_t start = str.find_first_not_of(" \t\r\n");

    if (start == string::npos)
        return "";

    size_t end = str.find_last_not_of(" \t\r\n");

    return str.substr(start, end - start + 1);
}

string toUpperCase(string str)
{
    transform(str.begin(), str.end(), str.begin(),
              [](unsigned char c)
              {
                  return toupper(c);
              });

    return str;
}

Node* insertNode(Node* root, string mnemonic)
{
    if (root == NULL)
        return new Node(mnemonic);

    if (mnemonic < root->mnemonic)
        root->left = insertNode(root->left, mnemonic);
    else if (mnemonic > root->mnemonic)
        root->right = insertNode(root->right, mnemonic);

    return root;
}

bool searchNode(Node* root, string mnemonic)
{
    if (root == NULL)
        return false;

    if (root->mnemonic == mnemonic)
        return true;

    if (mnemonic < root->mnemonic)
        return searchNode(root->left, mnemonic);

    return searchNode(root->right, mnemonic);
}

void inorder(Node* root)
{
    if (root == NULL)
        return;

    inorder(root->left);
    cout << root->mnemonic << endl;
    inorder(root->right);
}

void displayBST(Node* root, int space = 0)
{
    if (root == NULL)
        return;

    space += 6;

    displayBST(root->right, space);

    cout << endl;

    for (int i = 6; i < space; i++)
        cout << " ";

    cout << root->mnemonic << endl;

    displayBST(root->left, space);
}

bool isRegister(string operand)
{
    operand = toUpperCase(trim(operand));

    string registers[] =
    {
        "EAX", "ECX", "EDX", "EBX",
        "ESP", "EBP", "ESI", "EDI"
    };

    int size = sizeof(registers) / sizeof(registers[0]);

    for (int i = 0; i < size; i++)
    {
        if (operand == registers[i])
            return true;
    }

    return false;
}

bool isConstant(string operand)
{
    operand = trim(operand);

    if (operand.empty())
        return false;

    int start = 0;

    if (operand[0] == '-' || operand[0] == '+')
        start = 1;

    if (start == operand.length())
        return false;

    for (int i = start; i < operand.length(); i++)
    {
        if (!isdigit(operand[i]))
            return false;
    }

    return true;
}

string identifyOperand(string operand)
{
    operand = trim(operand);

    if (isRegister(operand))
        return "Register";

    if (isConstant(operand))
        return "Constant";

    if (operand.find('[') != string::npos &&
        operand.find(']') != string::npos)
        return "Memory";

    return "Symbol";
}

Node* loadOpcodeFile(Node* root, string fileName)
{
    ifstream opcodeFile(fileName);

    if (!opcodeFile)
    {
        cout << "Error: Cannot open opcode file.\n";
        return root;
    }

    string line;
    bool insideTable = false;

    while (getline(opcodeFile, line))
    {
        line = trim(line);

        if (line == "===== TABLE START =====")
        {
            insideTable = true;
            continue;
        }

        if (line == "===== TABLE END =====")
        {
            insideTable = false;
            break;
        }

        if (!insideTable)
            continue;

        if (line.empty())
            continue;

        if (line.find("Instruction") == 0)
            continue;

        if (line.find("------------") == 0)
            continue;

        size_t separator = line.find('|');

        if (separator == string::npos)
            continue;

        string mnemonic = line.substr(0, separator);

        mnemonic = trim(mnemonic);
        mnemonic = toUpperCase(mnemonic);

        if (!mnemonic.empty())
            root = insertNode(root, mnemonic);
    }

    opcodeFile.close();

    return root;
}

void displayOperands(string fileName)
{
    ifstream assemblyFile(fileName);

    if (!assemblyFile)
    {
        cout << "Error: Cannot open assembly file.\n";
        return;
    }

    string line;
    int lineNumber = 0;

    while (getline(assemblyFile, line))
    {
        lineNumber++;

        line = trim(line);

        if (line.empty())
            continue;

        if (line[0] == ';')
            continue;

        string mnemonic;
        string operands;

        size_t spacePosition = line.find_first_of(" \t");

        if (spacePosition == string::npos)
        {
            mnemonic = line;
            operands = "";
        }
        else
        {
            mnemonic = line.substr(0, spacePosition);
            operands = trim(line.substr(spacePosition + 1));
        }

        mnemonic = toUpperCase(mnemonic);

        size_t commentPosition = operands.find(';');

        if (commentPosition != string::npos)
            operands = trim(operands.substr(0, commentPosition));

        cout << "\nLine " << lineNumber << ": " << line << endl;
        cout << "Mnemonic: " << mnemonic << endl;

        if (operands.empty())
        {
            cout << "No operands." << endl;
            continue;
        }

        stringstream ss(operands);
        string operand;

        while (getline(ss, operand, ','))
        {
            operand = trim(operand);

            cout << operand << " -> "
                 << identifyOperand(operand) << endl;
        }
    }

    assemblyFile.close();
}

void deleteBST(Node* root)
{
    if (root == NULL)
        return;

    deleteBST(root->left);
    deleteBST(root->right);
    delete root;
}

int main()
{
    Node* root = NULL;

    string opcodeFileName;
    string assemblyFileName;

    cout << "Enter opcode file name: ";
    cin >> opcodeFileName;

    root = loadOpcodeFile(root, opcodeFileName);

    cout << "Enter assembly program file name: ";
    cin >> assemblyFileName;

    int choice;

    do
    {
        cout << "\n1. Display Mnemonics\n";
        cout << "2. Display BST\n";
        cout << "3. Search Mnemonic\n";
        cout << "4. Display Operands and Their Types\n";
        cout << "5. Exit\n";

        cout << "\nEnter your choice: ";
        cin >> choice;

        switch (choice)
        {
            case 1:
            {
                if (root == NULL)
                    cout << "No mnemonics available.\n";
                else
                    inorder(root);

                break;
            }

            case 2:
            {
                if (root == NULL)
                    cout << "BST is empty.\n";
                else
                    displayBST(root);

                break;
            }

            case 3:
            {
                string mnemonic;

                cout << "Enter mnemonic to search: ";
                cin >> mnemonic;

                mnemonic = toUpperCase(mnemonic);

                if (searchNode(root, mnemonic))
                    cout << mnemonic << " found.\n";
                else
                    cout << mnemonic << " not found.\n";

                break;
            }

            case 4:
            {
                displayOperands(assemblyFileName);
                break;
            }

            case 5:
                break;

            default:
                cout << "Invalid choice.\n";
        }

    } while (choice != 5);

    deleteBST(root);

    return 0;
}
