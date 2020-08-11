/*On my honour, I have neither given nor received unauthorized aid on this assignment. 
ARPITHA NAGARAJ HEGDE 
UFID: 26601584
*/

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <functional>
#include <set>
#include <bitset>
#include <sstream>
#include <cctype>
#include <cwctype>

#define DICTSIZE 8
#define LINEWIDTH 32

typedef std::unordered_map<std::string, std::pair<int,int> > StrIntMap;
typedef std::pair<std::string, std::pair<int,int> > pair;
bool fillLastLine;

void countInstr(std::istream& in, StrIntMap& inst) 
{
	std::string s;
  int i = 0;
    while (in >> s) {
    if(!inst.count(s)){
    inst[s].first = i; //global order of the instruction
    inst[s].second = 1; //frequency count
    }
    else
        ++inst[s].second;
        i++;
  }
} 
 
std::vector<std::bitset<32> > generateDictionary(StrIntMap& inst)
{
	std::vector<pair> vec;
	std::vector<std::bitset<32> > dict;

  for (auto it = inst.begin(); it != inst.end(); ++it)
    vec.push_back(std::make_pair(it->first, std::make_pair(it->second.first,it->second.second))); //make pair 

	std::sort(vec.begin(),vec.end(),
			[](const pair& l, const pair& r) {
				if(l.second.second != r.second.second) //check if the frequency count is not same 
					return l.second.second > r.second.second;  //return the one with higher frequency count
				return l.second.first < r.second.first; //if frequency count is same, return the one with first occurence
		});	

	dict.reserve(DICTSIZE);
	for (size_t it = 0; it < DICTSIZE; ++it) 
	{
    	dict.push_back(std::bitset<32>(vec[it].first));
	}
	return dict;
}


std::string compressInstuction(std::bitset<32>& line,std::vector<std::bitset<32> >& dict, int RLEcount)
{
	std::string compressed;
	bool compressedFlag = false;
	std::bitset<2> RLEbitCode;
	//std::cout<< "RLEcount:" << RLEcount << " ";
	std::bitset<3> pos;
	std::vector<std::bitset<32> >::iterator it=std::find(dict.begin(), dict.end(), line);
	if( it!= dict.end())
	{
		compressedFlag = true;
		pos = std::bitset<3>(it-dict.begin());
		compressed = "101" + pos.to_string();
	}	

	else
	{
		std::vector<int> trackMismatch[8]; //to keep a track of mismatches wrt each dicitonary and location of each msimatch 
		std::bitset<32> xorRes;
		std::bitset<5> location1;
		std::bitset<5> location2;
		std::bitset<3> dictIndex;
		for(int i = 0; i < DICTSIZE; i++)
		{
			xorRes = dict[i] ^ line;
			//std::cout << "xorRes:" << xorRes << std::endl;
			for(int j = 0; j < 32; j++)
			{
				if(xorRes.test(j))
				{
					//std::cout << i << j << std:: endl;
					trackMismatch[i].push_back(j); //push the location of mismatch
				}
				
			}
		}
		//1-bit mismatch
		if(!compressedFlag)
		{
			for(int i = 0; i < DICTSIZE; i++)
			{
				if(trackMismatch[i].size() == 1)
				{
					location1 = std::bitset<5> (31-trackMismatch[i][0]);
					dictIndex = std::bitset<3>(i);
					compressed = "010" + location1.to_string() + dictIndex.to_string();
					compressedFlag = true;
					break;
				}
			}
		}
		//2-bit mismatch
		if(!compressedFlag)
		{
			for(int i = 0; i < DICTSIZE; i++)
			{
				for(int j = 0; j < 31; j++)
				{
					if(trackMismatch[i].size() == 2 && (trackMismatch[i][0] + 1) == trackMismatch[i][1])
					{
						location2 = std::bitset<5> (31-trackMismatch[i][1]);
						dictIndex = std::bitset<3>(i);
						compressed = "011" + location2.to_string() + dictIndex.to_string();
						compressedFlag = true;
						break;
					}
				}
			}
		}
		
		//Bitmask based approach
		if(!compressedFlag)
		{
			for(int i = 0; i < DICTSIZE; i++)
			{
					size_t size = trackMismatch[i].size();
					int hamEnd = trackMismatch[i][size-1];
					int hamStart = trackMismatch[i][0];
					int hammingDistance = hamEnd - hamStart;	
					location1 = std::bitset<5> (31 - hamEnd);
					dictIndex = std::bitset<3>(i);
					compressed =  "001" + location1.to_string();
					if(size <=4 && hammingDistance < 4)
					{
						//generate bitmask
						int count = 1;
						for(int k = hamEnd; k > hamEnd - 4; k--)
						{
							if(count > size || k != trackMismatch[i][size-count])
							{
								compressed += "0";
							}
							else
							{
								compressed += "1";
								count++;
							}
						}
						
						compressed += dictIndex.to_string();
						compressedFlag = true;
						break;
					}
			}
		}
		//2-bit mismatch anywhere
		if(!compressedFlag)
		{
			for(int i = 0; i < 8; i++)
			{
				for(int j = 0; j < 31; j++)
				{
					if(trackMismatch[i].size() == 2)
					{
						location2 = std::bitset<5> (31-trackMismatch[i][1]);
						location1 = std::bitset<5> (31-trackMismatch[i][0]);
						dictIndex = std::bitset<3>(i);
						compressedFlag = true;
						compressed = "100" + location2.to_string() + location1.to_string() + dictIndex.to_string();
						break;
					}
				}
			}
		}
		if(compressedFlag == false)
			compressed = "110" + line.to_string();
		
	}
	
	//check for RLE
	if(RLEcount > 0)
	{
		
		//compressRLE(RLEcount);
		RLEbitCode = std::bitset<2> (RLEcount-1);
		compressed += "000" + RLEbitCode.to_string();
		//std::cout << compressed << std::endl;
		//return compressed;
	}
	//std::cout << compressed << std::endl;
	return compressed;
	
}

void printToFile(std::string compressedBinary, std::ofstream& outFile, bool& lastLineDone,std::vector<std::bitset<32> >& dict)
{
	static int bit_count = 0;
	bool compressedLinesDone = false;
	for(int i = 0; i < compressedBinary.size(); i++)
   	{
   		if(bit_count == 32)
   		{
   			bit_count = 0; //reset count
   			outFile << "\n"; //add a new line
   		}
   		outFile << compressedBinary[i];
   		bit_count++;
   	}
   	if(lastLineDone && bit_count < 32)
   	{
   		while(32 - bit_count > 0)
   		{
   			outFile << "1";
   			bit_count++;
   		}
   		compressedLinesDone = true;
   		outFile << "\n";
   	}
   	if(compressedLinesDone == true)
   	{
   		outFile << "xxxx";
   		for(int i = 0; i < DICTSIZE; i++)
   		{
   			outFile << "\n" << dict[i];
   		}
   		outFile.close();
   	}

}

void startEncoding(std::vector<std::bitset<32> >& dict)
{
	std::ifstream inFile;
	std::ofstream outFile;

	std::string line;
	std::string compressedInstruction;
	std::bitset<32> prevLine, currentLine;


	bool lastLineDone = false;
	bool RLEflag = false, Dictflag = false;

	int RLEcount = 0;
	int pos = 0;

	inFile.open("original.txt");
	outFile.open("cout.txt");

	inFile >> line;
	prevLine = std::bitset<32>(line);	
	//std::cout << prevLine << std::endl;

	while(!inFile.eof())
	{
		inFile >> line;
		currentLine = std::bitset<32>(line);

		//first priority to RLE
		if(currentLine == prevLine && RLEcount < 4)
		{
			//std::cout << currentLine << std::endl;
			RLEcount++;
		}
		else
		{
				compressedInstruction = compressInstuction(prevLine,dict,RLEcount);
				RLEcount = 0;
		        prevLine = currentLine;
		        printToFile(compressedInstruction,outFile,lastLineDone,dict);			
		}		
	}
	
	compressedInstruction = compressInstuction(prevLine,dict,RLEcount);
	lastLineDone = true;
	printToFile(compressedInstruction,outFile,lastLineDone,dict);
}

std::string readBits(std::string& str, int& startIndex, int length)
{
	std::string tmp;
	std::bitset<3> tmp_bits;
	for (int i = startIndex; i < (startIndex + length); i++)
	{
		if(str[i] == 'x')
		{
			tmp = "111";
			break;
		}
		tmp.push_back(str[i]);
	}
	return tmp;
}

std::bitset<3>  encodingType(std::string& str,int& startIndex)
{
	std::string type;
	std::bitset<3> encType;
	for(int i = startIndex; i < startIndex + 3 ; i++)
	{
		if(str[i] == 'x')
		{
			type = "111";
			break;
		}
		type.push_back(str[i]);
	}
	encType = std::bitset<3>(type);
	return encType;
}

void startDecoding(std::string& str,std::vector<std::bitset<32> >& dict)
{
	int length = 0;
	int startIndex = 0;
	int totalLength = 0;
	
	std::bitset<3> type;
	std::bitset<32> dictEntry;
	std::string binaryInstruction, previousbinInstr;

	std::ofstream outFile;
	outFile.open("dout.txt");
	
	bool completed = false;
	int instructionCount = 0;

	while(!completed)
	{
		binaryInstruction = "";
		type = encodingType(str,startIndex);

		if(type.to_ulong() != 7 && instructionCount != 0)
			outFile << std::endl;
		
		switch(type.to_ulong())
		{
			//RLE
			case 0:
			{
				length = 2;
				startIndex += 3;
				std::bitset<2> bits = std::bitset<2> (readBits(str,startIndex,length));
				startIndex += 2;
				int repeat = bits.to_ulong() + 1;
				for(int i = 0 ; i < repeat; i++)
				{
					binaryInstruction += previousbinInstr;
					if(i != repeat-1)
                        binaryInstruction += "\n";
                    instructionCount++;
				}
				outFile << binaryInstruction;
			}
			break;

			//Bitmask-based
			case 1:
			{
				startIndex += 3;
				std::bitset<5> startlocation = std::bitset<5> (readBits(str,startIndex,5));
				int startLoc = startlocation.to_ulong();
				startIndex += 5;
				std::bitset<4> mask = std::bitset<4> (readBits(str,startIndex,4));
				startIndex += 4;
				std::bitset<3> bits = std::bitset<3> (readBits(str,startIndex,3));
				startIndex += 3;
				std::bitset<32> completeMask; 
				dictEntry = dict[bits.to_ulong()];
				for(int i = 0; i < 4; i++)
                    completeMask[31 - startLoc - i] = mask[3 - i];	
                dictEntry = dictEntry ^ completeMask;
				binaryInstruction = dictEntry.to_string();
				outFile << binaryInstruction;
			}
			break;

			//1-bit mismatch
			case 2:
			{
				startIndex += 3;
				std::bitset<5> location1 = std::bitset<5> (readBits(str,startIndex,5));
				startIndex += 5;
				std::bitset<3> bits = std::bitset<3> (readBits(str,startIndex,3));
				startIndex += 3;
				int loc1 = location1.to_ulong();
				dictEntry = dict[bits.to_ulong()];
				dictEntry.flip(31 - loc1);
				binaryInstruction = dictEntry.to_string();
				outFile << binaryInstruction;
			}
			break;

			//2-bit mismatch
			case 3:
			{
				startIndex += 3;
				std::bitset<5> location1 = std::bitset<5> (readBits(str,startIndex,5));
				int loc1 = location1.to_ulong();
				startIndex += 5;
				std::bitset<3> bits = std::bitset<3> (readBits(str,startIndex,3));
				startIndex += 3;
				dictEntry = dict[bits.to_ulong()];
				dictEntry.flip(31 - loc1);
				dictEntry.flip(31 - (loc1+1));
				binaryInstruction = dictEntry.to_string();
				outFile << binaryInstruction;
			}
			break;

			//2-bit mismatch anywhere
			case 4:
			{
				startIndex += 3;
				std::bitset<5> location1 = std::bitset<5> (readBits(str,startIndex,5));
				int loc1 = location1.to_ulong();
				startIndex += 5;
				std::bitset<5> location2 = std::bitset<5> (readBits(str,startIndex,5));
				int loc2 = location2.to_ulong();
				startIndex += 5;
				std::bitset<3> bits = std::bitset<3> (readBits(str,startIndex,3));
				startIndex += 3;
				dictEntry = dict[bits.to_ulong()];
				dictEntry.flip(31 - loc1);
				dictEntry.flip(31 - loc2);
				binaryInstruction = dictEntry.to_string();
				outFile << binaryInstruction;
			}
			break;

			//Direct matching(Dictionary)
			case 5:
			{
				length = 3;
				startIndex += 3;
				std::bitset<3> bits = std::bitset<3> (readBits(str,startIndex,length));
				startIndex += 3;
				dictEntry = dict[bits.to_ulong()]; 
				binaryInstruction = dictEntry.to_string();
				outFile << binaryInstruction;	
			}
			break;

			//Original Binary
			case 6:
			{
				length = 32;
				startIndex += 3;
				std::bitset<32> bits = std::bitset<32> (readBits(str,startIndex,length));
				startIndex += 32;
				binaryInstruction = bits.to_string();
				outFile << binaryInstruction;
				
			}
			break;

			//if xxxx is encountered
			case 7:
			{
				completed = true;
				outFile.close();
				//	std::cout << "ending..." << std::endl;
				
			}
			break;
		}

		instructionCount++; 

		if(completed)
			break;	
		
		previousbinInstr = binaryInstruction;
	}	
	
}	

int main(int argc, char* argv[])
{
	int param = atoi(argv[1]);
	std::ifstream inFile;
	std::vector<std::bitset<32> > dict;
	char s;
	std::string sComplete;
	
	switch (param)
	{
		case 1: //compression mode
		{
			inFile.open("original.txt");
	    	if(!inFile)
	    	{
	        	std::cout << "unable to open file" << std::endl;
	    	}

		    //checking instruction frequency count
		    StrIntMap instr;
		    countInstr(inFile,instr);

		    inFile.close();

		    dict = generateDictionary(instr);

		    startEncoding(dict);
	    }
	    break;

	    case 2: //decompression mode
	    {
	    	inFile.open("original.txt");
	    	if(!inFile)
	    	{
	        	std::cout << "unable to open file" << std::endl;
	    	}

		    //checking instruction frequency count
		    StrIntMap instr;
		    countInstr(inFile,instr);

		    inFile.close();
		    
		    dict = generateDictionary(instr);

	    	inFile.open("compressed.txt");
	    	if(!inFile)
	    	{
	        	std::cout << "unable to open file" << std::endl;
	    	}

	    	while(!inFile.eof())
	    	{
	    		inFile >> s;
	    		sComplete += s;  
	    	}
			//std::cout << sComplete << std::endl;
		    inFile.close();
		  	
		    startDecoding(sComplete,dict);
	    }
	    break;

    }  
	return 0;	
}
