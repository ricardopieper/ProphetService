/*

This is the original copyright notice found on fmincg.m

The original fmincg.m was found at Coursera's Andrew Ng Machine Learning course.

Ported to C++ by Ricardo Pieper <ricardo.pieper@gmail.com>




Copyright (C) 2001 and 2002 by Carl Edward Rasmussen. Date 2002-02-13

Minimize a continuous differentialble multivariate function. Starting point
is given by "X" (D by 1), and the function named in the string "f", must
return a function value and a vector of partial derivatives. The Polack-
Ribiere flavour of conjugate gradients is used to compute search directions,
and a line search using quadratic and cubic polynomial approximations and the
Wolfe-Powell stopping criteria is used together with the slope ratio method
for guessing initial step sizes. Additionally a bunch of checks are made to
make sure that exploration is taking place and that extrapolation will not
be unboundedly large. The "length" gives the length of the run: if it is
positive, it gives the maximum number of line searches, if negative its
absolute gives the maximum allowed number of function evaluations. You can
(optionally) give "length" a second component, which will indicate the
reduction in function value to be expected in the first line-search (defaults
to 1.0). The function returns when either its length is up, or if no further
progress can be made (ie, we are at a minimum, or so close that due to
numerical problems, we cannot get any closer). If the function terminates
within a few iterations, it could be an indication that the function value
and derivatives are not consistent (ie, there may be a bug in the
implementation of your "f" function). The function returns the found
solution "X", a vector of function values "fX" indicating the progress made
and "i" the number of iterations (line searches or function evaluations,
depending on the sign of "length") used.


(C) Copyright 1999, 2000 & 2001, Carl Edward Rasmussen

Permission is granted for anyone to copy, use, or modify these
programs and accompanying documents for purposes of research or
education, provided this copyright notice is retained, and note is
made of any changes that have been made.

These programs and documents are distributed without any warranty,
express or implied.  As the programs were written for research
purposes only, they have not been tested to the degree that would be
advisable in any important application.  All use of these programs is
entirely at the user's own risk.


*/

#ifndef _FMINCG_H
#define _FMINCG_H

#include "FlensDefs.hpp"
// Copyright (C) 2001 and 2002 by Carl Edward Rasmussen. Date 2002-02-13

/* Ported to C++ by Ricardo Pieper. Date 2016-02-03 */

// a bunch of constants for line searches:
// RHO and SIG are the constants in the Wolfe-Powell conditions
const double RHO = 0.01;
const double SIG = 0.5;

// don't reevaluate within 0.1 of the limit of the current bracket
const double INT = 0.1;
const double EXT = 3.0;   // extrapolate maximum 3 times the current bracket
const double MAX = 20;    // max 20 function evaluations per line search

						  // maximum allowed slope ratio
const double RATIO = 100;


Mtx fmincg(const std::function<Cost(Mtx)> f,
	Mtx X,
	const unsigned int maxIters);





#endif  // _FMINCG_H
