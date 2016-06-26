#include "NeuralNetwork.hpp"
#include <iostream>
#include <random>
#include <climits>
#include <limits>
#include <chrono>
#include "fmincg.h"

void NeuralNetwork::logStr(std::string str) {
	if (m_verbose) {
	//	std::cout << str << std::endl;
	}
}

template<typename M>
void NeuralNetwork::printSizeMtx(std::string str, M & x) {
	if (m_verbose) {
	//	logStr(str + " size is " + std::to_string(x.numRows()) + " " + std::to_string(x.numCols()));
	}
}

template<typename M>
void printMtx(M & x) {
	std::cout << x << std::endl;
}


Mtx NeuralNetwork::activation(Mtx  m) {

	Mtx::IndexVariable i, j;
	Mtx G(m.numRows(), m.numCols());

	G(i, j) = 1.0 / (1.0 + Exp(-m(i, j)));

	return G;
}

Mtx NeuralNetwork::activationDerivative(Mtx m) {

	Mtx::IndexVariable i, j;

	Mtx s = activation(m);

	Mtx g(m.numRows(), m.numCols());

	g(i, j) = s(i, j) * (1 - s(i, j));

	return g;
}


Mtx NeuralNetwork::randInitializeWeights(int lIn, int lOut) {
	//L_out, 1 + L_in

	double min = 0;
	double max = 1;

	std::uniform_real_distribution<> gen(min, max); // uniform, unbiased

	auto time = std::chrono::system_clock::now()
		.time_since_epoch()
		.count();

	std::mt19937 rng(time);

	Mtx W(lOut, lIn);

	double epsilon = 0.12;

	for (int i = 1; i <= lOut; i++) {
		for (int j = 1; j <= lIn; j++) {
			W(i, j) = ((gen(rng) * 2) * epsilon) - epsilon;
		}
	}

	return W;

}

Mtx NeuralNetwork::unroll(Mtx x) {

	int _x = x.numRows() * x.numCols();

	Mtx UnrollX = MtxView(FSView(_x, 1, x.data(), _x));

	return UnrollX;
}



Mtx NeuralNetwork::unroll(Mtx x, Mtx y) {

	Mtx UnrollX = unroll(x);
	Mtx UnrollY = unroll(y);

	int rows = UnrollX.numRows() + UnrollY.numRows();

	Mtx Unrolled(rows, 1);

	Underscore<Mtx::IndexType>  _;

	Unrolled(_(1, UnrollX.numRows()), _) = UnrollX;
	Unrolled(_(UnrollX.numRows() + 1, rows), _) = UnrollY;

	return Unrolled;
}


Mtx NeuralNetwork::reshape(Mtx  x, int rows, int cols) {

	Mtx Reshaped = MtxView(FSView(rows,
		cols, x.data(), rows));

	return Reshaped;

}

template<typename M>
Mtx NeuralNetwork::prependColumn(M  x, double val) {

	Mtx y(x.numRows(), 1 + x.numCols());
	Underscore<typename M::IndexType>  _;
	y(_, _(2, 1 + x.numCols())) = x;
	y(_, _(1, 1)) = val;

	return y;
}

template<typename M>
Mtx NeuralNetwork::prependRow(M  x, double val) {

	Mtx y(1 + x.numRows(), x.numCols());
	Underscore<typename M::IndexType>  _;
	y(_(2, 1 + x.numRows()), _) = x;
	y(_(1, 1), _) = val;

	return y;
}

Mtx NeuralNetwork::mapLabels(Mtx labels) {

	Mtx mapped(labels.numRows(), 10);
	Underscore<Mtx::IndexType>  _;

	for (int i = 1; i <= labels.numRows(); i++) {

		double value = labels(i, 1);

		switch ((int)value) {
		case 1: mapped(i, _(1, 10)) = 1, 0, 0, 0, 0, 0, 0, 0, 0, 0; break;
		case 2: mapped(i, _(1, 10)) = 0, 1, 0, 0, 0, 0, 0, 0, 0, 0; break;
		case 3: mapped(i, _(1, 10)) = 0, 0, 1, 0, 0, 0, 0, 0, 0, 0; break;
		case 4: mapped(i, _(1, 10)) = 0, 0, 0, 1, 0, 0, 0, 0, 0, 0; break;
		case 5: mapped(i, _(1, 10)) = 0, 0, 0, 0, 1, 0, 0, 0, 0, 0; break;
		case 6: mapped(i, _(1, 10)) = 0, 0, 0, 0, 0, 1, 0, 0, 0, 0; break;
		case 7: mapped(i, _(1, 10)) = 0, 0, 0, 0, 0, 0, 1, 0, 0, 0; break;
		case 8: mapped(i, _(1, 10)) = 0, 0, 0, 0, 0, 0, 0, 1, 0, 0; break;
		case 9: mapped(i, _(1, 10)) = 0, 0, 0, 0, 0, 0, 0, 0, 1, 0; break;
		case 10: mapped(i, _(1, 10)) = 0, 0, 0, 0, 0, 0, 0, 0, 0, 1; break;
		default: break;
		}
	}

	return mapped;
}

double NeuralNetwork::sumAll(Mtx x) {
	auto vec = x.vectorView();
	double sum = 0;
	for (int i = 1; i <= vec.length(); i++) {
		sum += vec(i);
	}
	return sum;
}
Mtx NeuralNetwork::predictDigits(Mtx dataSet, Mtx Theta1,
	Mtx Theta2) {

	Mtx a1 = prependColumn(dataSet, 1);
	Mtx ffa2 = activation(a1 * transpose(Theta1));
	Mtx a2 = prependColumn(ffa2, 1);


	Mtx output = activation(a2 * transpose(Theta2));

	Mtx result(output.numRows(), 1);

	for (int i = 1; i <= output.numRows(); i++) {

		double max = -1;

		int index = 0;

		for (int j = 1; j <= output.numCols(); j++) {

			double current = output(i, j);

			if (current > max) {
				max = current;
				index = j;
			}
		}
		result(i, 1) = index;
	}

	return result;

}

Mtx NeuralNetwork::Predict(Mtx dataSet, Mtx Theta1,
	Mtx Theta2) {
	
	Mtx a1 = prependColumn(dataSet, 1);
	
	Mtx ffa2 = activation(a1 * transpose(Theta1));
		
	Mtx a2 = prependColumn(ffa2, 1);
	
	Mtx result = activation(a2 * transpose(Theta2));
	
	return result;
}


Cost NeuralNetwork::costFunction(Mtx nnParams,
	int inputLayerSize,
	int hiddenLayerSize,
	int numLabels,
	Mtx  dataSet,
	Mtx  labels,
	double lambda) {

	m_verbose = false;

	Cost r;

	Mtx::IndexVariable i, j;
	Underscore<Mtx::IndexType>  _;

	Mtx Theta1 = reshape(nnParams, hiddenLayerSize, inputLayerSize + 1);
	int elemsForTheta1 = (inputLayerSize + 1) * hiddenLayerSize;

	Mtx skipTheta1 = nnParams(_(elemsForTheta1 + 1, nnParams.numRows()), _);
	Mtx Theta2 = reshape(skipTheta1, numLabels, hiddenLayerSize + 1);
	double m = dataSet.numRows();


	//Vectorized feedforward algorithm

	/*

	This is how we predictDigits stuff.

	In this algorithm, instead of iterating through each row,
	we just pretend we're doing it all at once. That's why its harder
	to understand, but it runs fast.
	*/

	//The dataset looks like:

	/*
	datasample 1: data1, data2, data3...
	datasample 2: data1, data2, data3...
	datasample 3: data1, data2, data3...
	*/
	//each data{n} is a node in the input layer.


	//Theta1 is a matrix that contains the weight values for the nodes.

	//It is our best guess for what are the correct 
	//weights for the neural network.

	//For each node, there's a row with N columns, each column being the weight value
	//in respect to that node in the layer before. 
	//It looks like that:

	/*
	hiddenNode1: weightForNode1, weightForNode2, weightForNode3
	hiddenNode2: weightForNode1, weightForNode2, weightForNode3
	hiddenNode3: weightForNode1, weightForNode2, weightForNode3
	*/

	//So if each input row has 5 inputs, and our hidden layer has 3 nodes,
	//we will end up having a total of 15 Theta1 values.

	//the hiddenNode1 value will be: 
	/*

	for datasample in dataset (for each row)

		for weight in theta1(1), data in datasample
			sum += weight * data;

		hiddenNode1 = sigmoid(sum);
	*/


	//Remember that theta1 is a matrix of all weights for all nodes in the hidden layer.
	//and the number of ROWS in that matrix MATCHES the number HIDDEN NODES! 

	//So, if we want to calculate for every hiddenNode, we would do:

	/*
	for (datasample in dataset) "for each row of training data"

		for (nodeweights, nodeindex) in theta1 "for each hidden node"

			assert(nodeweights.length() == datasample.length()) "they are the same size"

			for (weight in nodeweights), (data in datasample) "foreach weight and data, every data is an input node"

				hiddenNodes[nodeindex] += weight * data;

			hiddenNodes[nodeindex] = activation(hiddenNodes[nodeindex])
	*/

	//For the output layer, we'd just do the same, using theta2 instead of theta1. 
	//It's the very same thing.

	/*
	for (hiddennode in hiddenNodes)

		for (nodeweights, nodeindex) in theta2

			assert(nodeweights.length() == hiddennode.length())

			for (weight in nodeweights), (node in hiddennode)

				output[nodeindex] += weight * node;

			output[nodeindex] = activation(output[nodeindex])
	*/


	//We're good, but we want to do it for ALL the data AT ONCE. We want to predictDigits it super fast.
	//In that case, we need vectorization.


	//Ok, first we add the bias term to the rows so that the data looks like:

	//datasample 1: 1, data1, data2, data3...
	//datasample 2: 1, data1, data2, data3...
	//datasample 3: 1, data1, data2, data3...

	Mtx layer1 = prependColumn(dataSet, 1);

	//This is our activation function. Yes, the way
	//that layer1 * transpose(Theta1) is just like 
	//what has described before! 

	//Matrix Multiplication has that power.
	Mtx layer2 = prependColumn(activation(layer1 * transpose(Theta1)), 1);

	Mtx output = activation(layer2 * transpose(Theta2));


	//Calculates the cost
	if (errorType == LogisticError) //used for classification problems
	{
		Mtx firstPart(output.numRows(), output.numCols());
		Mtx secondPart(output.numRows(), output.numCols());

		firstPart(i, j) = -(labels(i, j)) * Log(output(i, j));
		secondPart(i, j) = (1 - labels(i, j)) * Log(1 - output(i, j));

		double term = (1 / m)  * sumAll(firstPart - secondPart);

		//calculates the regularization term
		//skips the bias weight
		Mtx Theta1NoBias = Theta1(_, _(2, Theta1.numCols()));
		Mtx Theta2NoBias = Theta2(_, _(2, Theta2.numCols()));

		//square them
		Theta1NoBias(i, j) = Theta1NoBias(i, j) * Theta1NoBias(i, j);
		Theta2NoBias(i, j) = Theta2NoBias(i, j) * Theta2NoBias(i, j);

		double sum = sumAll(unroll(Theta1NoBias, Theta2NoBias));

		double regularization = sum * (lambda / (2 * m));

		r.CostValue = term + regularization;
	}
	else if (errorType == MeanSquaredError || errorType == RootMeanSquaredError) //lets use that one for reg problems
	{
		Mtx diffs(output.numRows(), output.numCols());
		diffs(i, j) = output(i, j) - labels(i, j);
		diffs(i, j) = diffs(i, j) *  diffs(i, j);

		if (errorType == MeanSquaredError) {
			r.CostValue = (1 / m) * sumAll(diffs);
		}
		else {
			r.CostValue = (1 / (2 * m)) * sumAll(diffs);
		}
	}

	std::cout << r.CostValue << std::endl;

	//now lets do the backprop

	//Create a matrix for the theta1 gradient terms
	Mtx Theta1_Gradient(Theta1.numRows(), Theta1.numCols());

	//Create a matrix for the theta2 gradient terms
	Mtx Theta2_Gradient(Theta2.numRows(), Theta2.numCols());

	//a1 contains 

	Mtx bpLayer1 = activation(transpose(Theta1 * transpose(layer1)));
	Mtx bpLayer1Bias = prependColumn(bpLayer1, 1);
	Mtx bpLayer2 = activation(transpose(Theta2 * transpose(bpLayer1Bias)));
	Mtx outputError = bpLayer2 - labels;
	Mtx d2 = transpose(transpose(Theta2) * transpose(outputError));
	Mtx _d2sigmoid = prependColumn(activationDerivative(bpLayer1), 1);
	d2(i, j) = d2(i, j) * _d2sigmoid(i, j);
	Mtx d2_skip = d2(_, _(2, d2.numCols()));
	Theta1_Gradient = Theta1_Gradient + (transpose(d2_skip) * layer1);
	Theta2_Gradient = Theta2_Gradient + (transpose(outputError) * bpLayer1Bias);

	auto Theta1SkipColumn1 = Theta1(_, _(2, Theta1.numCols()));
	auto Theta2SkipColumn1 = Theta2(_, _(2, Theta2.numCols()));

	Theta1_Gradient = (1 / m) * Theta1_Gradient + (lambda / m) * prependColumn(Theta1SkipColumn1, 0);

	Theta2_Gradient = (1 / m) * Theta2_Gradient + (lambda / m) * prependColumn(Theta2SkipColumn1, 0);


	r.Gradient = unroll(Theta1_Gradient, Theta2_Gradient);

	m_verbose = true;
	return r;

}


void NeuralNetwork::TrainDigits2(Mtx* X, Mtx* y, Mtx* Theta1, Mtx* Theta2, Mtx* RandomTheta1, Mtx* RandomTheta2) {
	Underscore<Mtx::IndexType>  _;
	int inputLayerSize = X->numCols();
	int hiddenLayerSize = 25;
	int numLabels = 10;

	Mtx labels = mapLabels(*y);

	//add +1 for the bias unit
	Mtx initialTheta1 = randInitializeWeights(inputLayerSize + 1, hiddenLayerSize);// *RandomTheta1;///;
	Mtx initialTheta2 = randInitializeWeights(hiddenLayerSize + 1, numLabels);//*RandomTheta2;///;
	Mtx initialNNParams = unroll(initialTheta1, initialTheta2);

	double lambda = 0.1;

	m_verbose = true;

	std::function<Cost(Mtx)> cf =
		[this, inputLayerSize, hiddenLayerSize, numLabels,
		X, labels, lambda](Mtx nnParams) {


		return costFunction(nnParams, inputLayerSize, hiddenLayerSize,
			numLabels, *X, labels, lambda);

	};

	Mtx nn_params = fmincg(cf, initialNNParams, 10);

	Mtx Trained_Theta1 = reshape(nn_params, hiddenLayerSize, inputLayerSize + 1);

	int elemsForTheta1 = (inputLayerSize + 1) * hiddenLayerSize;

	Mtx skipTheta1 = nn_params(_(elemsForTheta1 + 1, nn_params.numRows()), _);

	Mtx Trained_Theta2 = reshape(skipTheta1, numLabels, hiddenLayerSize + 1);

	Mtx predictionTrain = predictDigits(*X, Trained_Theta1, Trained_Theta2);


	int correct = 0;

	for (int i = 1; i <= predictionTrain.numRows(); i++) {

		if (predictionTrain(i, 1) == (*y)(i, 1)) {
			correct++;
		}
	}

}




void NeuralNetwork::TrainDigits(Mtx* X, Mtx* y) {
	Underscore<Mtx::IndexType>  _;

	int inputLayerSize = X->numCols();
	int hiddenLayerSize = hiddenNodes;
	int numLabels = 10;

	Mtx labels = mapLabels(*y);

	//add +1 for the bias unit
	Mtx initialTheta1 = randInitializeWeights(inputLayerSize + 1, hiddenLayerSize);// *RandomTheta1;///;
	Mtx initialTheta2 = randInitializeWeights(hiddenLayerSize + 1, numLabels);//*RandomTheta2;///;
	Mtx initialNNParams = unroll(initialTheta1, initialTheta2);

	double lambda = 0.1;

	m_verbose = true;

	std::function<Cost(Mtx)> cf =
		[this, inputLayerSize, hiddenLayerSize, numLabels,
		X, labels, lambda](Mtx nnParams) {

		return costFunction(nnParams, inputLayerSize, hiddenLayerSize,
			numLabels, *X, labels, lambda);

	};

	Mtx nn_params = fmincg(cf, initialNNParams, 100);

	Mtx Trained_Theta1 = reshape(nn_params, hiddenLayerSize, inputLayerSize + 1);

	int elemsForTheta1 = (inputLayerSize + 1) * hiddenLayerSize;

	Mtx skipTheta1 = nn_params(_(elemsForTheta1 + 1, nn_params.numRows()), _);

	Mtx Trained_Theta2 = reshape(skipTheta1, numLabels, hiddenLayerSize + 1);

	Mtx predictionTrain = predictDigits(*X, Trained_Theta1, Trained_Theta2);

	int correct = 0;

	for (int i = 1; i <= predictionTrain.numRows(); i++) {

		if (predictionTrain(i, 1) == (*y)(i, 1)) {
			correct++;
		}
	}
}


std::vector<Parameter> NeuralNetwork::Train(ErrorType _errorType, Mtx* X, Mtx* y) {
	Underscore<Mtx::IndexType>  _;

	int inputLayerSize = X->numCols();
	int hiddenLayerSize = hiddenNodes;
	int numLabels = (*y).numCols();
	errorType = _errorType;
	Mtx labels = *y;

	//add +1 for the bias unit
	Mtx initialTheta1 = randInitializeWeights(inputLayerSize + 1, hiddenLayerSize);// *RandomTheta1;///;
	Mtx initialTheta2 = randInitializeWeights(hiddenLayerSize + 1, numLabels);//*RandomTheta2;///;
	Mtx initialNNParams = unroll(initialTheta1, initialTheta2);

	double lambda = 0.1;

	m_verbose = true;

	std::function<Cost(Mtx)> cf =
		[this, inputLayerSize, hiddenLayerSize, numLabels,
		X, labels, lambda](Mtx nnParams) {

		return costFunction(nnParams, inputLayerSize, hiddenLayerSize,
			numLabels, *X, labels, lambda);

	};

	Mtx nn_params = fmincg(cf, initialNNParams, 100);

	Mtx Trained_Theta1 = reshape(nn_params, hiddenLayerSize, inputLayerSize + 1);

	int elemsForTheta1 = (inputLayerSize + 1) * hiddenLayerSize;

	Mtx skipTheta1 = nn_params(_(elemsForTheta1 + 1, nn_params.numRows()), _);

	Mtx Trained_Theta2 = reshape(skipTheta1, numLabels, hiddenLayerSize + 1);

	std::vector<Parameter> params;

	params.push_back({
		"theta1", Trained_Theta1
	});
	params.push_back({
		"theta2", Trained_Theta2
	});

	return params;

}



NeuralNetwork::NeuralNetwork(int _hiddenNodes, double _lambda) {
	hiddenNodes = _hiddenNodes;
	lambda = _lambda;
}

NeuralNetwork::NeuralNetwork() { }