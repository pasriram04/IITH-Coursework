#include <iostream>
#include <sstream>
#include <bitset>

using namespace std;

unsigned long long binaryToDecimal_u(const string& binary) {
    // Convert binary string to unsigned long long integer
    unsigned long long decimalValue = bitset<64>(binary).to_ullong();  // Handles up to 64-bit binary strings
    return decimalValue;
}

int main() {
    string binaryValue = "11100";  // Example binary input
    int hexValue = binaryToDecimal_u(binaryValue);
    cout << "decimal: " << hexValue << endl;

    return 0;
}
