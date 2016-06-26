#pragma once

#ifndef _FLENSCSV_HPP_
#define _FLENSCSV_HPP_
#include "../ML/FlensDefs.hpp"
#include <string>
class CSVReader {
private:
	std::string original;
	std::string file;

	std::vector<std::string> header;

public:
	CSVReader(std::string file);
	std::vector<std::string> GetHeader();
	Mtx* CSVReader::GetMatrix();
};
#endif
