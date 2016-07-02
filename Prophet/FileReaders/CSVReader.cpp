
#include "CSVReader.hpp"
#include <vector>
#include <map>
using namespace std;


std::string removeAll(std::string str, char c) {
	size_t offset = 0;
	size_t size = str.size();

	size_t i = 0;
	while (i < size - offset) {
		if (str[i + offset] == c) {
			offset++;
		}
		//fast path while c not found
		if (offset != 0) {
			str[i] = str[i + offset];
		}

		i++;
	}

	str.resize(size - offset);
	return str;
}

std::vector<std::string> split(std::string str, char splitChar, int max = -1) {
	std::vector<std::string> v;
	size_t lastSplitIdx = 0;
	int count = 0;
	for (char& c : str) {

		if (splitChar == c && count != 0) {
			v.push_back(str.substr(lastSplitIdx, count));
			lastSplitIdx += count + 1;
			count = 0;
			if (max > 0 && v.size() == max) {
				break;
			}
		}
		else {
			count++;
		}
	}
	if (!(max > 0 && v.size() == max)) {
		if (lastSplitIdx < str.length()) {
			v.push_back(str.substr(lastSplitIdx, str.length() - lastSplitIdx));
		}
	}
	return v;
}


CSVReader::CSVReader(std::string str) {
	original = str;
	file = removeAll(str, '\r');
	std::vector<std::string> firstLine = split(file, '\n', 1);

	if (firstLine.size() >= 1) {
		header = split(firstLine[0], ',');
	}
	else {
		throw std::string("CSV file has no lines");
	}
}

std::vector<std::string> CSVReader::GetHeader() {
	return header;
}

//indexDefinitions: how the file should be loaded. 
Mtx* CSVReader::GetMatrix(std::map<std::string, int> indexDefinitions)
{

	std::cout << "column definitions: " << std::endl;
	for (auto kv : indexDefinitions) {
		std::cout << kv.first << " -> " << kv.second << std::endl;
	}

	bool headerSkipped = false;
	std::vector<std::vector<double>> allRows;
	int numCols = header.size();

	std::vector<std::string> lines = split(file, '\n');

	Mtx* m = new Mtx(lines.size() - 1, header.size());
	int lineIndex = 1;
	for (auto line : lines)
	{
		if (!headerSkipped) {
			headerSkipped = true;
			continue;
		}

		auto cols = split(line, ',');

		if (cols.size() != numCols) {
			throw "Invalid CSV file: Header size != Line size at line " + std::to_string(lineIndex);
		}

		int colIndex = 0;
		for (auto str : cols)
		{
			try {
				
				std::string colName = header[colIndex];
				int correctIndex = indexDefinitions[colName];
				
				double val = std::stod(str);

				(*m)(lineIndex, correctIndex) = val;
				colIndex++;
			}
			catch (...) {
				throw "Error while parsing number: " + str;
			}
		}

		lineIndex++;

	}
	return m;


}
