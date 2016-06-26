#pragma once


#ifndef _FUNCTIONS_HPP_
#define _FUNCTIONS_HPP_



#include <cfloat>
#include "FlensDefs.hpp"

class Functions {
public:

	static double Max(Mtx x) {
		double d = -DBL_MAX;
		for (int i = 1; i <= x.numRows(); i++)
		{
			for (int j = 1; j <= x.numCols(); j++)
			{
				double v = x(i, j);

				if (v > d) d = v;

			}
		}
		return d;
	};

	static double Min(Mtx x) {
		double d = DBL_MAX;
		for (int i = 1; i <= x.numRows(); i++)
		{
			for (int j = 1; j <= x.numCols(); j++)
			{
				double v = x(i, j);

				if (v < d) d = v;
			}
		}
		return d;
	};

};
#endif
