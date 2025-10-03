#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <bitset>
#include <iomanip>


using namespace std;


unsigned long long binaryToDecimal_u(const string& binary) {
    // Convert binary string to unsigned long long integer
    unsigned long long decimalValue = bitset<64>(binary).to_ullong();  // Handles up to 64-bit binary strings
    return decimalValue;
}

string decimalToHex32(int decimal) {
  // Use bitset to convert decimal to 32-bit binary representation
  bitset<32> binary(decimal);

  // Use stringstream to convert binary to hex
  stringstream ss;
  ss << hex << uppercase << binary.to_ulong(); 
  return ss.str();
}

string hexToBinary(const string& hex, int bits) {
    // Convert the hexadecimal string to an integer (unsigned 32-bit)
    unsigned int num;
    stringstream ss;
    ss << std::hex << hex;  // Use std::hex manipulator to interpret the string as hexadecimal
    ss >> num;

    // Convert the integer to a 32-bit binary string
    if (bits == 32){
        bitset<32> binary(num);
        return binary.to_string();
    }
}

string binaryToHex(const string& binary, size_t hexLength) {
    // Convert the binary string to an unsigned long long integer
    unsigned long long num = bitset<64>(binary).to_ullong();  // Handles up to 64-bit binary strings

    // Create a stringstream to convert the integer to hexadecimal
    stringstream ss;
    ss << std::hex << num;  // Convert integer to hex and store it in the stringstream

    // Get the hexadecimal string
    string hexValue = ss.str();

    // Pad with leading zeros if necessary
    if (hexValue.length() < hexLength) {
        hexValue = string(hexLength - hexValue.length(), '0') + hexValue; // Pad with '0's
    }

    // Return the trimmed hex value according to specified length
    return hexValue.substr(0, hexLength); // Ensure output is exactly hexLength
}


void R_type(string instruction, int x[]){
    string rs1_str = instruction.substr(12,5);
    string rs2_str = instruction.substr(7,5);
    string rsd_str = instruction.substr(20,5);
    unsigned long long rs1 = binaryToDecimal_u(rs1_str);
    //cout << "rs1:" << rs1_str << endl;
    unsigned long long rs2 = binaryToDecimal_u(rs2_str);
    //cout << "rs2:" << rs2 << endl; 
    unsigned long long rsd = binaryToDecimal_u(rsd_str);
    //cout << "rsd:" << rsd << endl;
    x[0] = 0;
    if (instruction.substr(0,7) == "0000000"){
        if(instruction.substr(17, 3) == "000"){
            // add //
            x[rsd] = x[rs1] + x[rs2];
        }
        else if(instruction.substr(17, 3) == "100"){
            // xor //
            x[rsd] = x[rs1] ^ x[rs2];
        }
        else if(instruction.substr(17, 3) == "110"){
            // or //
            x[rsd] = x[rs1] | x[rs2];
            //cout << "x[rsd]" << x[rsd] << endl;
        }
        else if(instruction.substr(17, 3) == "111"){
            // and //
            x[rsd] = x[rs1] & x[rs2];
        }
        else if(instruction.substr(17, 3) == "000"){
            // sll //
            x[rsd] = x[rs1] << x[rs2];
        }
        else if(instruction.substr(17, 3) == "000"){
            // srl //
            x[rsd] = x[rs1] >> x[rs2];
        }
    }
    else if(instruction.substr(0,7) == "0100000"){
        if(instruction.substr(17, 3) == "000"){
            // sub //
            x[rsd] = x[rs1] - x[rs2];
        }
        else if(instruction.substr(17, 3) == "000"){
            // sra //
            x[rsd] = x[rs1] >> x[rs2];
            if (x[rs1] < 0) {
                x[rsd] |= ~(~0 >> x[rs2]);
            }
        }
    }
    //x[0] = 8;
    x[0] = 0;
}



void identifier(string instruction, int x[]){
    string opcode = instruction.substr(25,7);
    if (opcode == "0110011"){
        /* Implement R type instructions */
        //cout << "R-Type" << endl;
        R_type(instruction, x);
    }
    else if (opcode == "0010011"){
        /* Implement I type instructions */
    }
    else if (opcode == "0000011"){
        /* Implement I type instructions of type ld, lb, lh, etc */
    }
    else if (opcode == "0100011"){
        /* Implement S type instructions */
    }
    else if (opcode == "1100011"){
        /* Implement B type instructions */
    }
    else if (opcode == "1100011"){
        /* Implement B type instructions */
    }
}


// Stub functions (implement these later)
void load(const string &filename, int* x) {
    // Construct the terminal command to assemble the input file
    string command = "./assembler " + filename;
    
    // Run the terminal command using the system() function
    int result = system(command.c_str());

    // Check if the command was successful
    if (result == 0) {
        cout << "Successfully ran assembler on: " << filename << endl;
    } else {
        cerr << "Error running assembler command." << endl;
    }
    for (int i = 0; i < 32; i++) {
        x[i] = 0;
    }
    //x[0] = 8;
}


void run(int x[]) {
    ifstream asmFile("input.s");
    ifstream hexFile("output.hex");
    int count = 1;
    int currentLine = 1;
    //bool test = true;
    // Check if both files are open
    if (!asmFile.is_open()) {
        cerr << "Error: Could not open input.s file." << endl;
        return;
    }
    
    if (!hexFile.is_open()) {
        cerr << "Error: Could not open output.hex file." << endl;
        return;
    }

    string asmLine, hexLine;
    //cout << "Executing RISC-V code from output.hex and referencing input.s:" << endl;

    // Loop to read both files line by line
    while (getline(asmFile, asmLine) && getline(hexFile, hexLine)) {

        // Print lines from both files
        /*
        if (currentLine == 8 and test){
            count = 1;
            test = false;
        }
        */

        if (count == currentLine){
            
            cout << "Step" << endl;
        
            /*This part is where we must write a function that takes care of all the registers and 
            memory since, for branch instructions, the program counter that's gonna be displayed 
            below will change*/
        
            cout << "Hex: " << hexLine << endl;
            // "count" might change here due to branch instructions
            /* Be careful about the Program counter this program leads to after branch instructions.... u should
             add or subtarct 1 afaik */
            cout << "Assembly: " << asmLine << "; PC:"<<decimalToHex32(count*4)<<endl;
            count++;
            currentLine++;


            // Shit goes real down in this one small function.....
            string instruction = hexToBinary(hexLine, 32);
            //cout << "instruction : " << instruction << endl;
            identifier(instruction, x);

            // Implement logic to execute hex instruction and reference assembly here

            //cout << "Executed: " << hexLine << " corresponding to " << asmLine << endl << endl;
        }
        else if (count > currentLine){
            currentLine++;
        }
        else {
            asmFile.clear(); // Clear flags like EOF
            hexFile.clear();

            // Move file pointers back to the beginning
            asmFile.seekg(0, ios::beg);
            hexFile.seekg(0, ios::beg);

            // Reset the current line counter
            currentLine = 1;
        }
    }

    // Close the files
    asmFile.close();
    hexFile.close();
}

void regs(int x[]) {
    // Function to print the values of registers
    cout << "Displaying register values..." << endl;
    for (int i = 0; i < 32; i++){
        cout << "x" << i << " : 0x" << decimalToHex32(x[i]) << endl;
    }

}

int main() {
    string command;
    int x[32] = {0};
    //x[0] = 8;

    while (true) {
        cout << "> ";  // Prompt
        cin >> command;

        if (command == "load") {
            string filename;
            cin >> filename;
            load(filename, x);
        } 
        else if (command == "run") {
            run(x);
        } 
        else if (command == "regs") {
            regs(x);
        } 
        else if (command == "exit") {
            cout << "Exiting the simulator. Goodbye!" << endl;
            break;  // Gracefully exit the program
        } 
        else {
            cout << "Unknown command. Please use 'load', 'run', 'regs', or 'exit'." << endl;
        }
    }

    return 0;
}
