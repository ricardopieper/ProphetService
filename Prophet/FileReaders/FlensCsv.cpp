#include "FlensCsv.hpp"
#include <vector>
using namespace std;

std::vector<std::string> split(std::string str, char splitChar) {
	std::vector<std::string> v;
	int lastSplitIdx = 0;
	int count = 0;
	for (char& c : str) {

		if (splitChar == c && count != 0) {
			v.push_back(str.substr(lastSplitIdx, count));
			lastSplitIdx += count + 1;
			count = 0;
		}
		else {
			count++;
		}
	}

	if (lastSplitIdx < str.length()) {
		v.push_back(str.substr(lastSplitIdx, str.length() - lastSplitIdx));
	}

	return v;
}

Mtx* FlensCsv::LoadMatrix(const char* fullPath) {

	ifstream ifs(fullPath);
	string line;
	if (ifs.is_open()) {

		//format expected:
		//header, header, header
		//data, data, data
		//data, data, data

		//skip first
		getline(ifs, line);

		int colunas = split(line, ',').size();

		std::vector<std::vector<double>> allRows;

		while (getline(ifs, line)) {
			std::vector<double> cols;

			auto strings = split(line, ',');

			for (auto str : strings) {
				cols.push_back(std::stod(str));
			}

			allRows.push_back(cols);
		}

		Mtx* m = new Mtx(allRows.size(), colunas);

		for (int i = 1; i <= allRows.size(); i++) {
			for (int j = 1; j <= colunas; j++) {
				(*m)(i, j) = allRows[i-1][j-1];
			}

		}

		return m;
	}
	return NULL;
}
