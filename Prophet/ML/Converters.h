#pragma once
#include "FlensDefs.hpp"


Mtx GetMatrix(char* data, int rows, int cols) {

	double* buffer = reinterpret_cast<double*>(data);

	return Mtx(rows, cols, buffer);
}