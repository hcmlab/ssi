#include "GermanStem.h"
#include "EnglishStem.h"
#include <vector>
#include <map>
#include <list>
struct word {
	std::string germanWord;
	std::string englishWord;
	std::string germanStem;
	std::string englishStem;
	//mean valence 
	double val;
};

class Helper
{
public:

	void stem(const char* inputWords, char* outputWords, std::string lan);
	void Helper::readDic(std::string path, std::list<word> *dictionary);
	void Helper::readNegWords(std::string path, std::list<std::string> *wordList);

	Helper();
	~Helper();

private:

	stemming::GermanStem<char, std::char_traits<char> > _stemGerman;
	stemming::EnglishStem <char, std::char_traits<char> > _stemEnglish;
	
	
};
