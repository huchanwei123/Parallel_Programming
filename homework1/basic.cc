// Description:                                          *
//  Implementation of basic odd-even sort which can only *
//  be swapped with its adjacent elements                *
// Author:                                               *
//  Chan-Wei Hu                                          *
/*********************************************************/
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

using namespace std;

// function initialization
float *Binary2Float(char *filename);

int main(int argc, char *argv[]){
    // initialization for parameters
    float *list;
    int list_len;
    list = Binary2Float(argv[1]);

    return 0;
}

float *Binary2Float(char *filename){
    ifstream inFile;
    ifstream::pos_type size;

    // open the binary file
    inFile.open(filename, ios::binary);

    // set the pointer position to the end of file
    inFile.seekg(0, ios::end);
    size = inFile.tellg();
    cout << "[Info] Start reading " << size/4 << " numbers in the file." << endl;
    
    // set the pointer position to the beginning
    inFile.seekg(0, ios::beg);

    // allocate memory for the array
    float *out = new float(size);
    for(int i=0; i<size/4; i++){
        inFile.read(reinterpret_cast<char*>(&out[i]), sizeof(float));
    }
    
    // check if read in successfully
    if (inFile)
        cout << "[Info] Successfully read in..." << endl;
    else
        throw string("[Error] input read in FAILED");

    return out;
}
