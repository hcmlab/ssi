#include "..\include\Helper.h"
#include <iostream> 
#include <fstream> 
#include <string> 

Helper::Helper()
{
}


Helper::~Helper()
{
}

//applies wordstemming to the all inputwords seperated by a whitespace
void Helper::stem(const char* inputWords, char* outputWords, std::string lan) {
	char* tmpArray = new char[strlen(inputWords) * sizeof(char)];
	char* returnArray = new char[strlen(inputWords) * sizeof(char)];
	strcpy(tmpArray, inputWords);

	char* word;
	word = strtok(tmpArray, " ");
	//splitting the input string and adding the stemmed words to the returnarray
	int i = 0;
	while (word != NULL)
	{
		std::string wordTmp = std::string(word);
		transform(wordTmp.begin(), wordTmp.end(), wordTmp.begin(), ::tolower);
		if (lan.compare("ger") == 0)
		{
			_stemGerman(wordTmp);
		}
		else if (lan.compare("en") == 0) {
			_stemEnglish(wordTmp);
		}
		

		word = strtok(NULL, " ");
		//just append whitespace if there is another word coming up next
		if(word!=NULL) 
			wordTmp.append(" ");
		if (i == 0)
			strcpy(returnArray, wordTmp.c_str());
		else 
			strcat(returnArray, wordTmp.c_str());
		i++;
	}
	

	strcpy(outputWords, returnArray);
}


void Helper::readNegWords(std::string path, std::list<std::string> *wordList) {
	std::ifstream csvread;
	csvread.open(path, std::ios::in);
	if (csvread){
		//Datei bis Ende einlesen und bei '\n' strings trennen 
		std::string currentWord = "";
		while (getline(csvread, currentWord, '\n'))
		{
			wordList->insert(wordList->cend(), currentWord);
		}

		csvread.close();
	}
	else{
		std::cerr << "Fehler beim Lesen der Negationswortliste!" << std::endl;
	}
}


void Helper::readDic(std::string path, std::list<word> *dictionary) {
	std::ifstream csvread;
	csvread.open(path, std::ios::in);
	if (csvread){
		//Datei bis Ende einlesen und bei '\n' strings trennen 
		std::string currentLine = "";
		std::string currentWord = "";
		while (getline(csvread, currentLine, '\n'))
		{
			word tmpWord;
			int col = 1;
			//get words till the end
			std::size_t prev = 0, pos;
			while ((pos = currentLine.find_first_of(";", prev)) != std::string::npos && col < 5)
			{
				if (pos > prev) {
					currentWord = currentLine.substr(prev, pos-prev);
					transform(currentWord.begin(), currentWord.end(), currentWord.begin(), ::tolower);
					char* tmpStem = new char[currentWord.length() * sizeof(char)];
					if (1) {
						switch (col)
						{
						//german word
						case 1:
							tmpWord.germanWord = std::string(currentWord);
							stem(currentWord.c_str(), tmpStem, "ger");
							tmpWord.germanStem = std::string(tmpStem);
							break;
						//english word
						case 2:
							tmpWord.englishWord = currentWord;
							stem(currentWord.c_str(), tmpStem, "en");
							tmpWord.englishStem = std::string(tmpStem);
							break;
						//ID
						case 3:
							break;
						//ValMean
						case 4:
							tmpWord.val = atof(currentWord.c_str());
							break;
						case 5:
						default:
							break;
						}
					}
					if (0) {
						switch (col)
						{
						//german word
						case 1:
							tmpWord.germanWord = std::string(currentWord);
							stem(currentWord.c_str(), tmpStem, "en");
							tmpWord.germanStem = std::string(tmpStem);
							break;
						//POS
						case 2:
							break;
						//Valence
						case 3:
							tmpWord.val = atof(currentWord.c_str());
							break;
						default:
							break;
						}
					}

				}	
				prev = pos + 1;
				col++;
			}
			//last word
			if (prev < currentLine.length()) {
			}
			//std::cout << s << std::endl; //alle Strings getrennt ausgeben 
			dictionary->insert(dictionary->cend(), tmpWord);
		}

		csvread.close();
	}
	else{
		std::cerr << "Fehler beim Lesen!" << std::endl;
	}		
}