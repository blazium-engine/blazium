#include <iostream>
#include <fstream>
#include <string>
#include <vector>

struct Pairs
{
	Pairs(std::string keyName, std::string valueName) { key = keyName; value = valueName; }
	std::string key = "", value = "";
};

struct Section
{
	Section(std::string name = "") { sectionName = name; }
	std::string sectionName = "";
	std::vector<Pairs> pairs;
};

bool doesItContain(const std::string &code, const char character)
{
	for(uint64_t i = 0; i < code.size(); i++)
	{
		if(code[i] == character) { return true; }
	}
	return false;
}

bool isLineEmpty(const std::string line)
{
	for(uint64_t i = 0; i < line.size(); i++)
	{
		if(line[i] != ' ' && line[i] != '\t') { return false; }
	}
	return true;
}

bool isPairValid(const std::string line)
{
	std::string left_side = "", word = "";
	bool passedEqualSign = false;
	for(uint64_t i = 0; i < line.size(); i++)
	{
		if(line[i] == '=' && !passedEqualSign) { left_side = word; word = ""; continue; }
		else if(line[i] == '=' && passedEqualSign) { return false; }
		
		if(line[i] != ' ' && line[i] != '\t') { word += line[i]; }
	}
	
	return (left_side.size() > 0 && word.size() > 0);
}

bool isSectionNameValid(const std::string line)
{
	if(line.size() < 3) { return false; }
	else if(line[0] != '[' || line[line.size()-1] != ']') { return false; }
	
	std::string word = "";
	for(uint64_t i = 1; i < line.size()-1; i++)
		{ if(line[i] != ' ' && line[i] != '\t') { word += line[i]; } }
	
	return word.size() > 0;
}

Pairs givePair(const std::string line)
{
	Pairs z("", "");
	std::string left_side = "", word = "";
	for(uint64_t i = 0; i < line.size(); i++)
	{
		if(line[i] == '=') { left_side = word; word = ""; continue; }
		if(line[i] != ' ' && line[i] != '\t') { word += line[i]; }
	}
	z.key = left_side;
	z.value = word;
	return z;
}

std::string giveSectionName(const std::string line)
{
	std::string nw = "";
	for(uint64_t i = 1; i < line.size()-1; i++) { nw += line[i]; }
	return nw;
}

class INI
{
	public:
		/* File reads disregard all comments */
		bool read	(char *file_name); // 1 is success, 0 is file not found
		bool write	(char *file_name); // 1 is success, 0 is nothing to write

		/* Shows to std::cout (standard output) */
		void show();		
		void showSection		(std::string sectionName);
		
		/* false means that nothing has been edited; true means successfully edited */
		bool editSectionName	(std::string oldName, std::string newName);
		bool setValue			(std::string sectionName, std::string keyName, std::string valueName); // either changes a value (if the key exists), either adds a new key-value pair
		
		/* If section or key not found, it returns an empty string ---> "" */
		std::string giveValue	(std::string sectionName, std::string keyName);
		
	private:
		std::vector<Section> ini_file;
};

bool INI::read(char *file_name)
{
	std::string line;
	std::ifstream file(file_name);
	if(file.is_open())
	{
		while(getline(file, line))
		{
			if(isLineEmpty(line) || line[0] == ';' || line[0] == '#') { continue; }
			else if(isSectionNameValid(line))
				{ ini_file.push_back(Section(giveSectionName(line))); }
			else if(isPairValid(line))
				{ ini_file[ini_file.size()-1].pairs.push_back(givePair(line)); }
		}
		file.close();
	}
	else
	{
		std::cout << "Could not find " << file_name << "!" << std::endl;
		return false; // not found!
	}
	
	return true;
}

bool INI::write(char *file_name)
{
	if(ini_file.size() == 0)
	{
		std::cerr << "There is no data to write!\n";
		return false;
	}
	
	std::ofstream output(file_name);
	for(uint64_t i = 0; i < ini_file.size(); i++)
	{
		output << "[" << ini_file[i].sectionName << "]" << std::endl;
		for(uint64_t j = 0; j < ini_file[i].pairs.size(); j++)
		{
			output << ini_file[i].pairs[j].key << " = " << ini_file[i].pairs[j].value << std::endl;
		}
		output << std::endl;
	}
	output.flush();
	output.close();
	
	return true;
}

void INI::show()
{
	for(uint64_t i = 0; i < ini_file.size(); i++)
	{
		std::cout << "[" << ini_file[i].sectionName << "]\n";
		for(uint64_t j = 0; j < ini_file[i].pairs.size(); j++)
		{
			std::cout << ini_file[i].pairs[j].key << " = " << ini_file[i].pairs[j].value << "\n";
		}
		std::cout << "\n";
	}
}

void INI::showSection(std::string sectionName)
{
	for(uint64_t i = 0; i < ini_file.size(); i++)
	{
		if(ini_file[i].sectionName != sectionName) { continue; }
		std::cout << "[" << sectionName << "]\n";
		for(uint64_t j = 0; j < ini_file[i].pairs.size(); j++)
		{
			std::cout << ini_file[i].pairs[j].key << " = " << ini_file[i].pairs[j].value << "\n";
		}
		std::cout << "\n";
	}
}

bool INI::editSectionName(std::string oldName, std::string newName)
{
	for(uint64_t i = 0; i < ini_file.size(); i++)
	{
		if(ini_file[i].sectionName == oldName)
			{ ini_file[i].sectionName = newName; return true; }
	}
	return false; //nothing changed
}

bool INI::setValue(std::string sectionName, std::string keyName, std::string valueName)
{
	for(uint64_t i = 0; i < ini_file.size(); i++)
	{
		if(ini_file[i].sectionName != sectionName) { continue; }
		for(uint64_t j = 0; j < ini_file[i].pairs.size(); j++)
		{
			if(ini_file[i].pairs[j].key == keyName)
				{ ini_file[i].pairs[j].value = valueName; return true; }
		}
		ini_file[i].pairs.push_back(Pairs(keyName, valueName));
		return true;
	}
	return false; //could not find section name, nothing set
}

std::string INI::giveValue(std::string sectionName, std::string keyName)
{
	for(uint64_t i = 0; i < ini_file.size(); i++)
	{
		if(ini_file[i].sectionName != sectionName) { continue; }
		for(uint64_t j = 0; j < ini_file[i].pairs.size(); j++)
		{
			if(ini_file[i].pairs[j].key == keyName)
				{ return ini_file[i].pairs[j].value; }
		}
	}
	return ""; //empty returns signals: nothing found
}

int main(int argc, char* argv[])
{	
	if(argc < 2) { std::cerr << "\nUsage: ./readiniVector [file name]\n"; return 1; }
	
	INI ini_file;
	ini_file.read(argv[1]);
	ini_file.write("test.ini");
	ini_file.show();
	
	return 0;
}
