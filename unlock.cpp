#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

uint32_t unlock_prbs()
{   
    static uint32_t shift_register=0xa551199; // "random" seed value
	int b1 = 0;
	int b2 = 0;
	if (shift_register & (1 << 30))
	{
		b1 = 1;
	}
	if (shift_register & (1 << 27))
	{
		b2 = 1;
	}
	
	shift_register=shift_register << 1;
	shift_register=shift_register | (b1 ^ b2);
	shift_register = shift_register & 0x7fffffff;
    return shift_register ; // return 31 LSB's 
}

int main()
{

	uint32_t unlock_randnum;
	int counter_int;
	string counter_str;
	
	stringstream ss;
	
	fstream ctr_file;
	ctr_file.open("unlock_counter.txt", ios::in);
	
	if (ctr_file.is_open()) {
		cout << "File Located" << endl;
		getline(ctr_file, counter_str);
		ss.str("");
		ss.clear();
		ss << counter_str;
		ss >> counter_int;
		cout << "the counter value in the file is: " << counter_int << endl;
	}
	else {														
		cout << "Cannot Locate file" << endl;				
		exit(0);											
	}														
	ctr_file.close();
	
	for (int i = 0; i < counter_int; i++){
		unlock_randnum = unlock_prbs();
	}
	
	printf("Unlock Code:	 %x\n", unlock_randnum);
	ctr_file.open("unlock_code.txt", ios::out);
	if (ctr_file.is_open()) {
		cout << "File Located..." << endl << "Writing code to file" << endl;
		ctr_file << hex << unlock_randnum;
	}
	else {														
		cout << "Cannot Locate file" << endl;				
		exit(0);											
	}														
	ctr_file.close();
	cout << "End of programme" << endl;
	return 0;	
	
	// * * * * * * * * * * * * * NOTES * * * * * * * * * * * *  //
	// instead of writing the code to file, compare codes using //
	// C++ program. this way NodeJS does not need to compare    //
	// strings... this process will be easier		    //
}
		
		
