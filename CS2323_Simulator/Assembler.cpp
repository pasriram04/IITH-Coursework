#include <bits/stdc++.h>

using namespace std;

string binaryToHex(const string& binaryStr) {
    bool isBinary = true;
    for (char c : binaryStr) {
        if (c != '0' && c != '1') {
            isBinary = false;
            break;
        }
    }
    if (isBinary) {
        if (binaryStr.size() < 32) {
            return "Missing Arguments";
        }
    } else {
        return binaryStr;  // Non-binary string (error), return as is
    }
    bitset<32> bits(binaryStr);
    unsigned long decimalValue = bits.to_ulong();
    stringstream hexStream;
    hexStream << hex << setfill('0') << setw(8) << decimalValue;
    return hexStream.str();
}

// This function converts Immediate Strings to Integers, without assuming any sign. 
// The overflow value is the maximum possible output. 

// In case it output crosses the overflow value for binary or hex, 
// function assumes it is a signed output and treats first bit as sign bit.
// Range has to be manually tested after using this function.
int to_integer_imm(const string& imm, int overflow_value, int count) {
    int imm_int = 0;
    if (imm[0] == '0' && imm[1] == 'b') {
        try {
            imm_int = stoi(imm.substr(2, imm.size()), nullptr, 2);
        } catch (const invalid_argument& e) {
            cout << "Line " << count << ": Invalid Immediate" << endl;
            exit(0);
        } catch (const out_of_range& e) {
            cout << "Line " << count << ": Invalid Immediate" << endl;
            exit(0);  
        }
        if (imm_int >= overflow_value)
            imm_int -= 2 * overflow_value;
    } else if (imm[0] == '0' && imm[1] == 'x') {
        try {
            imm_int = stoi(imm.substr(2, imm.size()), nullptr, 16);
        } catch (const invalid_argument& e) {
            cout << "Line " << count << ": Invalid Immediate" << endl;
            exit(0); 
        } catch (const out_of_range& e) {
            cout << "Line " << count << ": Invalid Immediate" << endl;
            exit(0);  
        }
        if (imm_int >= overflow_value)
            imm_int -= 2 * overflow_value;
    } else {
        try {
            imm_int = stoi(imm, nullptr, 10);
        } catch (const invalid_argument& e) {
            cout << "Line " << count << ": Invalid Immediate" << endl;
            exit(0);
        } catch (const out_of_range& e) {
            cout << "Line " << count << ": Invalid Immediate" << endl;
            exit(0); 
        }
    }
    return imm_int;
}

string decimal_to_binary(int decimalNumber, int N, bool signed_, int count) {
    string result;
    int max_value, min_value;
    // Maximum value for N bits
    if(!signed_)
    {
        max_value = (1 << N) - 1;  // 2^N - 1
        min_value = 0;
    }
    else
    {
        max_value = (1 << (N-1)) - 1;
        min_value = -max_value - 1;
    }

    // Check for overflow
    if (decimalNumber > max_value || decimalNumber < min_value) {
        cout << "Line " << count << ": Invalid Immediate" << endl;
        exit(0);
    }

    // Convert to binary based on N
    if (N == 5) {
        bitset<5> binaryNumber(decimalNumber);
        result = binaryNumber.to_string();
    } else if (N == 12) {
        bitset<12> binaryNumber(decimalNumber);
        result = binaryNumber.to_string();
    } else if (N == 13) {
        bitset<13> binaryNumber(decimalNumber);
        result = binaryNumber.to_string();
    } else if (N == 20) {
        bitset<20> binaryNumber(decimalNumber);
        result = binaryNumber.to_string();
    } else if (N == 21) {
        bitset<21> binaryNumber(decimalNumber);
        result = binaryNumber.to_string();
    } else {
        cout << "Line " << count << ": Unsupported Bit Width" << endl;
        exit(0);
    }
    return result;
}

int register_memory(const string& x_smthng, int count) {
    if (x_smthng.substr(0, 1) == "x") {
        int reg_num;
        try {
            reg_num = stoi(x_smthng.substr(1).c_str());
        } catch (const invalid_argument& e) {
            cout << "Line " << count << ": Invalid Register Address" << endl;
            exit(0); 
        } catch (const out_of_range& e) {
            cout << "Line " << count << ": Invalid Register Address" << endl;
            exit(0);  
        }
        if (reg_num < 32 && reg_num >= 0) {
            return reg_num;
        }
    } else if (x_smthng.substr(0, 1) == "t") {
        int reg_num;
        try {
            reg_num = stoi(x_smthng.substr(1).c_str());
        } catch (const invalid_argument& e) {
            cout << "Line " << count << ": Invalid Register Address" << endl;
            exit(0);
        } catch (const out_of_range& e) {
            cout << "Line " << count << ": Invalid Register Address" << endl;
            exit(0); 
        }
        if (reg_num >= 0 && reg_num <= 2) {
            return reg_num + 5;
        } else if (reg_num >= 3 && reg_num <= 6) {
            return reg_num + 25;
        }
    } else if (x_smthng.substr(0, 1) == "s") {
        int reg_num;
        try {
            reg_num = stoi(x_smthng.substr(1).c_str());
        } catch (const invalid_argument& e) {
            cout << "Line " << count << ": Invalid Register Address" << endl;
            exit(0);
        } catch (const out_of_range& e) {
            cout << "Line " << count << ": Invalid Register Address" << endl;
            exit(0);  
        }
        if (reg_num == 0) {
            return 8;
        } else if (reg_num == 1) {
            return 9;
        } else if (reg_num >= 2 && reg_num <= 11) {
            return reg_num + 16;
        }
    } else if (x_smthng.substr(0, 1) == "a") {
        int reg_num;
        try {
            reg_num = stoi(x_smthng.substr(1).c_str());
        } catch (const invalid_argument& e) {
            cout << "Line " << count << ": Invalid Register Address" << endl;
            exit(0);
        } catch (const out_of_range& e) {
            cout << "Line " << count << ": Invalid Register Address" << endl;
            exit(0); 
        }
        if (reg_num >= 0 && reg_num <= 7) {
            return reg_num + 10;
        }
    } else if (x_smthng == "zero") {
        return 0;
    } else if (x_smthng == "ra") {
        return 1;
    } else if (x_smthng == "sp") {
        return 2;
    } else if (x_smthng == "gp") {
        return 3;
    } else if (x_smthng == "tp") {
        return 4;
    } else if (x_smthng == "fp") {
        return 8;
    }
    return -1;
}

string R_type_instruction(string rd, string rs1, string rs2, const string& operation, int count) {
    string opcode = "0110011";
    int rd_int, rs1_int, rs2_int;
    rd_int = register_memory(rd, count);
    rs1_int = register_memory(rs1, count);
    rs2_int = register_memory(rs2, count);
    if (rd_int == -1 || rs1_int == -1 || rs2_int == -1) {
        cout << "Line " << count << ": Invalid Register Address" << endl;
        exit(0);
    }
    string rd_str = decimal_to_binary(rd_int, 5, false, count);
    string rs1_str = decimal_to_binary(rs1_int, 5, false, count);
    string rs2_str = decimal_to_binary(rs2_int, 5, false, count);

    map<string, string> R_f3;
    R_f3["add"] = "000";
    R_f3["sub"] = "000";
    R_f3["and"] = "111";
    R_f3["or"] = "110";
    R_f3["xor"] = "100";
    R_f3["sll"] = "001";
    R_f3["srl"] = "101";
    R_f3["sra"] = "101";

    map<string, string> R_f7;
    R_f7["add"] = "0000000";
    R_f7["sub"] = "0100000";
    R_f7["and"] = "0000000";
    R_f7["or"] = "0000000";
    R_f7["xor"] = "0000000";
    R_f7["sll"] = "0000000";
    R_f7["srl"] = "0000000";
    R_f7["sra"] = "0100000";

    string instruction = R_f7[operation] + rs2_str + rs1_str + R_f3[operation] + rd_str + opcode;
    return instruction;
}

string I_type_instruction(string rd, string rs1, string imm, const string& operation, int count) {
    string opcode = "0000011";
    if (operation == "andi" || operation == "addi" || operation == "ori" || operation == "xori" ||
        operation == "slli" || operation == "srli" || operation == "srai") {
        opcode = "0010011";
    }
    if (operation == "jalr") {
        opcode = "1100111";
    }
    int rd_int, rs1_int, imm_int;
    rd_int = register_memory(rd, count);
    rs1_int = register_memory(rs1, count);
    imm_int = to_integer_imm(imm, 2048, count);

    if (rd_int == -1 || rs1_int == -1) {
        cout << "Line " << count << ": Invalid Register Address" << endl;
        exit(0);
    }
    if (imm_int > 2047 || imm_int < -2048) {
        cout << "Line " << count << ": Invalid Immediate" << endl;
        exit(0);
    }
    if ((imm_int > 63 || imm_int < 0) && (operation == "srai" || operation == "srli" || operation == "slli")) {
        cout << "Line " << count << ": Invalid Immediate" << endl;
        exit(0);
    }

    string rd_str = decimal_to_binary(rd_int, 5, false, count);
    string rs1_str = decimal_to_binary(rs1_int, 5, false, count);
    string imm_str = decimal_to_binary(imm_int, 12, true, count);

    map<string, string> R_f3;
    R_f3["addi"] = "000";
    R_f3["xori"] = "100";
    R_f3["ori"] = "110";
    R_f3["andi"] = "111";
    R_f3["slli"] = "001";
    R_f3["srli"] = "101";
    R_f3["srai"] = "101";

    R_f3["lb"] = "000";
    R_f3["lh"] = "001";
    R_f3["lw"] = "010";
    R_f3["ld"] = "011";
    R_f3["lbu"] = "100";
    R_f3["lhu"] = "101";
    R_f3["lwu"] = "110";

    R_f3["jalr"] = "000";

    if (operation == "srai") // funct7
    {
        imm_str[1] = '1';
    }

    string instruction = imm_str + rs1_str + R_f3[operation] + rd_str + opcode;
    return instruction;
}

string S_type_instruction(string rs2, string rs1, string imm, const string& operation, int count) {
    string opcode = "0100011";
    int rs2_int, rs1_int, imm_int;
    rs2_int = register_memory(rs2, count);
    rs1_int = register_memory(rs1, count);
    imm_int = to_integer_imm(imm, 2048, count);

    if (rs2_int == -1 || rs1_int == -1) {
        cout << "Line " << count << ": Invalid Register Address" << endl;
        exit(0);
    }
    if (imm_int > 2047 || imm_int < -2048) {
        cout << "Line " << count << ": Invalid Immediate" << endl;
        exit(0);
    }

    string rs2_str = decimal_to_binary(rs2_int, 5, false, count);
    string rs1_str = decimal_to_binary(rs1_int, 5, false, count);
    string imm_str = decimal_to_binary(imm_int, 12, true, count);

    string imm115 = imm_str.substr(0, 7);
    string imm40 = imm_str.substr(7, 5);

    map<string, string> R_f3;
    R_f3["sb"] = "000";
    R_f3["sh"] = "001";
    R_f3["sw"] = "010";
    R_f3["sd"] = "011";

    string instruction = imm115 + rs2_str + rs1_str + R_f3[operation] + imm40 + opcode;
    return instruction;
}
string B_type_instruction(string rs1, string rs2, int offset, const string& operation, int count) {
    string opcode = "1100011";
    int rs1_int, rs2_int;
    
    rs1_int = register_memory(rs1, count);
    rs2_int = register_memory(rs2, count);
    
    if (rs1_int == -1 || rs2_int == -1) {
        cout << "Line " << count << ": Invalid Register Address" << endl;
        exit(0);
    }
    
    string rs1_str = decimal_to_binary(rs1_int, 5, false, count);
    string rs2_str = decimal_to_binary(rs2_int, 5, false, count);

    map<string, string> B_f3;
    B_f3["beq"]  = "000";
    B_f3["bne"]  = "001";
    B_f3["blt"]  = "100";
    B_f3["bge"]  = "101";
    B_f3["bltu"] = "110";
    B_f3["bgeu"] = "111";

    string imm_value = decimal_to_binary(offset, 13, true, count);
    string substring_4_1 = imm_value.substr(8, 4);
    string substring_10_5 = imm_value.substr(2, 6);
    
    string instruction = imm_value.substr(0, 1) + substring_10_5 + rs2_str + rs1_str + B_f3[operation] + substring_4_1 + imm_value.substr(1, 1) + opcode;
    return instruction;
}

string J_type_instruction(string rsd, int offset, const string& operation, int count) {
    string opcode = "1101111";
    int rsd_int;
    
    rsd_int = register_memory(rsd, count);
    
    if (rsd_int == -1) {
        cout << "Line " << count << ": Invalid Register Address" << endl;
        exit(0);
    }
    if (offset > 1048575 || offset < -1048576) {
        cout << "Line " << count << ": Invalid Immediate" << endl;
        exit(0);
    }
    
    string rsd_str = decimal_to_binary(rsd_int, 5, false, count);
    string imm_value = decimal_to_binary(offset, 21, true, count);
    string substring_10_1 = imm_value.substr(10, 10);
    string substring_19_12 = imm_value.substr(1, 8);
    
    string instruction = imm_value.substr(0, 1) + substring_10_1 + imm_value.substr(9, 1) + substring_19_12 + rsd_str + opcode;
    return instruction;
}

string U_type_instruction(string rd, int imm_int, const string& operation, int count) {
    string opcode = "0110111";
    int rd_int = register_memory(rd, count);
    
    if (rd_int == -1) {
        cout << "Line " << count << ": Invalid Register Address" << endl;
        exit(0);
    }
    if (imm_int > 1048575 || imm_int < 0) {
        cout << "Line " << count << ": Invalid Immediate" << endl;
        exit(0);
    }
    
    string rd_str = decimal_to_binary(rd_int, 5, false, count);
    string imm_str = decimal_to_binary(imm_int, 20, false, count);
    
    string instruction = imm_str + rd_str + opcode;
    return instruction;
}

char identifier(const char *operand) {
    set<string> R_type = {"add", "sub", "and", "or", "xor", "sll", "srl", "sra"};
    set<string> I_type = {"addi", "andi", "ori", "xori", "slli", "srli", "srai", "jalr", "ld", "lw", "lh", "lb", "lwu", "lhu", "lbu"};
    set<string> S_type = {"sd", "sw", "sh", "sb"};
    set<string> B_type = {"beq", "bne", "blt", "bge", "bltu", "bgeu"};
    set<string> J_type = {"jal"};
    set<string> U_type = {"lui"};
    
    for (const string& instruction : R_type) {
        if (strcmp(operand, instruction.c_str()) == 0) {
            return 'r';
        }
    }
    for (const string& instruction : I_type) {
        if (strcmp(operand, instruction.c_str()) == 0) {
            return 'i';
        }
    }
    for (const string& instruction : S_type) {
        if (strcmp(operand, instruction.c_str()) == 0) {
            return 's';
        }
    }
    for (const string& instruction : B_type) {
        if (strcmp(operand, instruction.c_str()) == 0) {
            return 'b';
        }
    }
    for (const string& instruction : J_type) {
        if (strcmp(operand, instruction.c_str()) == 0) {
            return 'j';
        }
    }
    for (const string& instruction : U_type) {
        if (strcmp(operand, instruction.c_str()) == 0) {
            return 'u';
        }
    }
    return 'E';
}

// All the functions below are just for properly taking the input. Once the input is taken
// everything is just stored in the vector "word"
// From now on, every all analysis we do happens from on the vector "word"

void printMap(const map<string, int>& myMap) {
    for (const auto& pair : myMap) {
        cout << pair.first << ": " << pair.second << endl;
    }
}

void tokenize(const string &line, vector<string> &words) {
    stringstream ss(line);
    string word;
    
    while (ss >> word) {
        word.erase(remove(word.begin(), word.end(), ','), word.end());
        
        size_t start = word.find('(');
        size_t end = word.find(')');
        
        if (start != string::npos && end == word.length() - 1 && start > 0) {
            string part1 = word.substr(start + 1, end - start - 1);
            string part2 = word.substr(0, start);
            words.push_back(part1);
            words.push_back(part2);
        } else {
            words.push_back(word);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <input file>" << endl;
        exit(0);
    }

    ifstream inputFile(argv[1]);
    
    if (!inputFile) {
        cerr << "Error opening input file" << endl;
        exit(0);
    }

    string line;
    int count = 0;
    map<string, int> offsets;
    
    while (getline(inputFile, line)) {
        vector<string> words;
        tokenize(line, words);
        count++;
        
        if (words[0].back() == ':') {
            string label = words[0].substr(0, words[0].length() - 1);
            for (const auto& pair : offsets) {
                if(pair.first == label)
                {
                    cout << "Line " << count << ": Duplicate Label" << endl;
                    exit(0);
                }
            }
            offsets[label] = count;
        }
    }

    inputFile.clear();
    inputFile.seekg(0, ios::beg);
    count = 0;
    vector<string> output;
    while (getline(inputFile, line)) {
        count++;
        vector<string> words;
        tokenize(line, words);
        
        if (words[0].back() == ':') {
            words.erase(words.begin());
        }
        
        if (words.empty()) {
            continue;
        }
        
        int offset;
        
        switch (identifier(words[0].c_str())) {
            case 'r':
                if(words.size() != 4)
                {
                    cout << "Line " << count << ": Incorrect Number of Tokens" << endl;
                    exit(0);
                }
                output.push_back(binaryToHex(R_type_instruction(words[1], words[2], words[3], words[0], count)));
                break;
            case 'i':
                if(words.size() != 4)
                {
                    cout << "Line " << count << ": Incorrect Number of Tokens" << endl;
                    exit(0);
                }
                output.push_back(binaryToHex(I_type_instruction(words[1], words[2], words[3], words[0], count)));
                break;
            case 's':
                if(words.size() != 4)
                {
                    cout << "Line " << count << ": Incorrect Number of Tokens" << endl;
                    exit(0);
                }
                output.push_back(binaryToHex(S_type_instruction(words[1], words[2], words[3], words[0], count)));
                break;
            case 'b':
                if(words.size() != 4)
                {
                    cout << "Line " << count << ": Incorrect Number of Tokens" << endl;
                    exit(0);
                }
                if ((words[3][0] >= '0' && words[3][0] <= '9') || words[3][0] == '-') { //Check negative case if it works
                    output.push_back(binaryToHex(B_type_instruction(words[1], words[2], to_integer_imm(words[3], 4096, count), words[0], count)));
                }
                else if (offsets.find(words[3]) != offsets.end()) {
                    offset = (offsets[words[3]] - count) * 4;
                    output.push_back(binaryToHex(B_type_instruction(words[1], words[2], offset, words[0], count)));
                } else {
                    cout << "Line " << count << ": Branch not identified" << endl;
                    exit(0);
                }
                break;
            case 'j':
                if(words.size() != 3)
                {
                    cout << "Line " << count << ": Incorrect Number of Tokens" << endl;
                    exit(0);
                }
                if ((words[2][0] >= '0' && words[2][0] <= '9') || words[2][0] == '-') {
                    output.push_back(binaryToHex(J_type_instruction(words[1], to_integer_imm(words[2], 1048576, count), words[0], count)));
                } else if (offsets.find(words[2]) != offsets.end()) {
                    offset = (offsets[words[2]] - count) * 4;
                    output.push_back(binaryToHex(J_type_instruction(words[1], offset, words[0], count)));
                } else {
                    cout << "Line " << count << ": Branch not identified" << endl;
                    exit(0);
                }
                break;
            case 'u':
                if(words.size() != 3)
                {
                    cout << "Line " << count << ": Incorrect Number of Tokens" << endl;
                    exit(0);
                }
                if (words[2][0] >= '0' && words[2][0] <= '9') {// Debug: between 0 to 9, it should go to this, otherwise it shouldn't
                    output.push_back(binaryToHex(U_type_instruction(words[1], to_integer_imm(words[2], 1048576, count), words[0], count)));
                } else if (offsets.find(words[2]) != offsets.end()) {
                    offset = (offsets[words[2]] - count) * 4;
                    output.push_back(binaryToHex(U_type_instruction(words[1], offset, words[0], count)));
                } else {
                    if(words[2][0] >= '-')
                    {
                        cout << "Line " << count << ": Invalid Immediate" << endl;
                        exit(0);
                    }
                    cout << "Line " << count << ": Branch not identified" << endl;
                    exit(0);
                }
                break;
            default:
                cout << "Line " << count << ": Invalid Instruction" << endl;
                exit(0);
        };
    }
    inputFile.close();

    ofstream myFile("output.hex");
    for(auto it:output){
        myFile<<it<<endl;
    }
    myFile.close();
    return 0;
}