#include <iostream>
#include <cctype>
#include <set>
#include <iterator>

using namespace std;

// Intializing Keywords and operators
string keywordsString[] = {"auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern", "float", "for", "goto", "if", "int", "long", "register", "return", "short", "signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while"};
set<string> keywordsSet(keywordsString, keywordsString + sizeof(keywordsString) / sizeof(keywordsString[0]));

string operatorsString[] = {"=", "|", "||", "&&", "==", "!=", "<", ">", "<=", ">=", "+", "-", "*", "/", "%", "&", "!", "++", "--", ".", "->", "(", ")", "[", "]", "{", "}", ";", ":", ","};
set<string> operatorsSet(operatorsString, operatorsString + sizeof(operatorsString) / sizeof(operatorsString[0]));

int main()
{
    char c;
    char p;
    char l;
    string buf;
    set<string>::iterator it;
    // While loop to begin lexical analysis
    while (cin.eof() != true)
    {
        c = cin.get();
        l = cin.peek();
        int dec2 = int(l);
        int dec = int(c);
        // Checking for ignorable cases
        if (dec == 92 || isspace(c))
        {
            p = cin.peek();
            if ((p == 't' && dec == 92) || (p == 'n' && dec == 92) || (p == 'f' && dec == 92) || (p == 'v' && dec == 92) || (p == 'r' && dec == 92))
                continue;
            continue;
        }
        // Cheking for comments
        if (dec == 47 && dec2 == 42)
        {
            p = cin.get();
            dec = int(p);
            while (dec != 47)
            {
                p = cin.get();
                dec = int(p);
            }
            continue;
        }
        // Checks for intergers:
        if (isdigit(c))
        {
            buf += c;
            p = cin.peek();
            while (isdigit(p))
            {
                p = cin.get();
                buf += p;
                p = cin.peek();
            }
            cout << "number:" << buf << endl;
            buf.clear();
            continue;
        }
        // Checking for strings
        if (dec == 34)
        {
            buf += c;
            p = cin.peek();
            dec = int(p);
            while (dec != 34)
            {
                if (dec == 92)
                {
                    p = cin.get();
                    buf += p;
                }
                p = cin.get();
                buf += p;
                dec = int(p);
            }
            cout << "string:" << buf << endl;
            buf.clear();
            continue;
        }
        // Checking for characters
        if (dec == 39)
        {
            buf += c;
            p = cin.peek();
            dec = int(p);
            while (dec != 39)
            {
                p = cin.get();
                buf += p;
                dec = int(p);
            }
            cout << "character:" << buf << endl;
            buf.clear();
            continue;
        }
        // Checking for identifiers and keywords
        if (isalpha(c))
        {
            buf += c;
            p = cin.peek();
            while (isalnum(p))
            {
                p = cin.get();
                buf += p;
                p = cin.peek();
            }
            it = keywordsSet.find(buf);
            if (it != keywordsSet.end())
                cout << "keyword:" << buf << endl;
            else
                cout << "identifier:" << buf << endl;
            buf.clear();
            continue;
        }

        p = cin.peek();
        if (isspace(p) == false)
        {

            buf += c;
            buf += p;
            it = operatorsSet.find(buf);
            if (it != operatorsSet.end())
            {
                cout << "operator:" << buf << endl;
                p = cin.get();
            }
            else
            {
                cout << "operator:" << c << endl;
            }
            buf.clear();
        }
        else
        {
            buf += c;
            it = operatorsSet.find(buf);
            if (it != operatorsSet.end())
                cout << "operator:" << c << endl;
            buf.clear();
        }
    }
}