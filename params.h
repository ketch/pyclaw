#ifndef __PARAMS_H__
#define __PARAMS_H__

typedef float real;

struct pdeParam
{
	int cellsX;				// number of cells on the horizontal, including ghost cells (unpadded)
	int cellsY;				// number of cells on the vertical, including ghost cells
	const int ghostCells;	// ghost cells for the boundaries
	const int numStates;	// number of states of the problem, number of scalars per cell
	const int numWaves;		// number of nonzero waves generated by the Riemann solver (usually as many as there are equations)
	const int numCoeff;		// number of different types medium properties

	//real dt;				// time step is not exactly a problem parameter

	real width;				// physical domain's width
	real height;			// physical domain's height;
	real dx;				// physical domain's spacial discritization size on the horizontal
	real dy;				// physical domain's spacial discritization size on the vertical
	const real startX;		// physical domain's starting x in cartesian coordinates
	const real endX;		// physical domain's ending x in cartesian coordinates
	const real startY;		// physical domain's starting y in cartesian coordinates
	const real endY;		// physical domain's ending y in cartesian coordinates

	real* coefficients;			// physical medium coefficients					//	GPU residents
	real* q;					// data, cells' states							//
	real* qNew;					// intermediate update holder
	real* waveSpeedsX;			// speed of horizontal waves					//
	real* waveSpeedsY;			// speed of vertical waves						//

	pdeParam()
		: cellsX(256), cellsY(256), ghostCells(2), numStates(1), numWaves(1), numCoeff(1), startX(0), endX(1), startY(0), endY(1)
	{
		width = endX-startX;
		height = endY-startY;
		dx = width/cellsX;
		dy = height/cellsY;

		int cells = cellsX*cellsY;
		int horizontal_blocks = ((CELLSX-1 + HORIZONTAL_BLOCKSIZEX-3-1)/(HORIZONTAL_BLOCKSIZEX-3)) * ((CELLSY + HORIZONTAL_BLOCKSIZEY-1)/(HORIZONTAL_BLOCKSIZEY));
		int vertical_blocks = ((CELLSX + VERTICAL_BLOCKSIZEX-1)/(VERTICAL_BLOCKSIZEX)) * ((CELLSY-1 + VERTICAL_BLOCKSIZEY-3-1)/(VERTICAL_BLOCKSIZEY-3));

		cudaError_t alloc1 = cudaMalloc((void**)&coefficients, cells*numCoeff*sizeof(real));
		cudaError_t alloc2 = cudaMalloc((void**)&q, cells*numStates*sizeof(real));
		cudaError_t alloc3 = cudaMalloc((void**)&qNew, cells*numStates*sizeof(real));
		cudaError_t alloc4 = cudaMalloc((void**)&waveSpeedsX, horizontal_blocks*sizeof(real));
		cudaError_t alloc5 = cudaMalloc((void**)&waveSpeedsY, vertical_blocks*sizeof(real));

		real* zerosX = (real*)calloc(horizontal_blocks, sizeof(real));
		real* zerosY = (real*)calloc(vertical_blocks, sizeof(real));
		cudaMemcpy(waveSpeedsX, zerosX, horizontal_blocks*sizeof(real), cudaMemcpyHostToDevice);
		cudaMemcpy(waveSpeedsY, zerosY, vertical_blocks*sizeof(real), cudaMemcpyHostToDevice);
		free(zerosX);
		free(zerosY);
	};
	pdeParam(int cellsX, int cellsY, int ghostCells, int numStates, int numWaves, int numCoeff,
		real startX, real endX, real startY, real endY)
		:
	cellsX(cellsX), cellsY(cellsY), ghostCells(ghostCells), numStates(numStates), numWaves(numWaves), numCoeff(numCoeff),
		startX(startX), endX(endX), startY(startY), endY(endY)
	{
		width = endX-startX;
		height = endY-startY;
		dx = width/cellsX;
		dy = height/cellsY;

		int cells = cellsX*cellsY;
		int horizontal_blocks = ((CELLSX-1 + HORIZONTAL_BLOCKSIZEX-3-1)/(HORIZONTAL_BLOCKSIZEX-3)) * ((CELLSY + HORIZONTAL_BLOCKSIZEY-1)/(HORIZONTAL_BLOCKSIZEY));
		int vertical_blocks = ((CELLSX + VERTICAL_BLOCKSIZEX-1)/(VERTICAL_BLOCKSIZEX)) * ((CELLSY-1 + VERTICAL_BLOCKSIZEY-3-1)/(VERTICAL_BLOCKSIZEY-3));

		cudaError_t alloc1 = cudaMalloc((void**)&coefficients, cells*numCoeff*sizeof(real));
		cudaError_t alloc2 = cudaMalloc((void**)&q, cells*numStates*sizeof(real));
		cudaError_t alloc3 = cudaMalloc((void**)&qNew, cells*numStates*sizeof(real));
		cudaError_t alloc4 = cudaMalloc((void**)&waveSpeedsX, horizontal_blocks*sizeof(real));
		cudaError_t alloc5 = cudaMalloc((void**)&waveSpeedsY, vertical_blocks*sizeof(real));

		// Slightly fragile approach:
		// We have to assign for every "active block" a single real
		// This number has to be exact, but forcing an initial zero, we can
		// allocate an approximate memory size, which works, but feels fragile
		// This way we take into account all blocks, including some that might not be active
		// Note: by active we mean that a block is not at the very edge and participate in
		// the computation by at least a wave ;

		real* zerosX = (real*)calloc(horizontal_blocks, sizeof(real));
		real* zerosY = (real*)calloc(vertical_blocks, sizeof(real));
		cudaMemcpy(waveSpeedsX, zerosX, horizontal_blocks*sizeof(real), cudaMemcpyHostToDevice);
		cudaMemcpy(waveSpeedsY, zerosY, vertical_blocks*sizeof(real), cudaMemcpyHostToDevice);
		free(zerosX);
		free(zerosY);
	};

	void clean()
	{
		cudaFree(waveSpeedsY);
		cudaFree(waveSpeedsX);
		cudaFree(q);
		cudaFree(qNew);
		cudaFree(coefficients);
	};

	// GETTER AND SETTER FUNCTIONS
	// Q
	// Dictates how the memory layout for q will be
	inline __device__ __host__ int getIndex_q(int row, int column, int state)
	{
		// Usual C/C++ row major order
		//return (row*cellsX*numStates + column*numStates + state);
		// state is the slowest moving dimension now, then row, then column
		return (state*cellsX*cellsY + row*cellsX + column);
	}
	inline __device__ real &getElement_q(int row, int column, int state)
	{
		return q[getIndex_q(row, column, state)];
	}
	inline __host__ real &getElement_q_cpu(real* cpu_q, int row, int column, int state)
	{
		return cpu_q[getIndex_q(row, column, state)];
	}
	inline __device__ void setElement_q(int row, int column, int state, real setValue)
	{
		q[getIndex_q(row, column, state)] = setValue;
	}
	inline __host__ void setElement_q_cpu(real* cpu_q, int row, int column, int state, real setValue)
	{
		cpu_q[getIndex_q(row, column, state)] = setValue;
	}

	// QNEW
	// Dictates how the memory layout for qNew will be
	inline __device__ int getIndex_qNew(int row, int column, int state)
	{
		// Usual C/C++ row major order
		//return (row*cellsX*numStates + column*numStates + state);
		// state is the slowest moving dimension now, then row, then column
		return (state*cellsX*cellsY + row*cellsX + column);
	}
	inline __device__ real &getElement_qNew(int row, int column, int state)
	{
		return qNew[getIndex_qNew(row, column, state)];
	}
	inline __device__ void setElement_qNew(int row, int column, int state, real setValue)
	{
		qNew[getIndex_qNew(row, column, state)] = setValue;
	}

	// COEFFICIENTS
	inline __device__ __host__ int getIndex_coeff(int row, int column, int coeff)
	{
		// Usual C/C++ row major order
		//return (row*cellsX*numCoeff + column*numCoeff + coeff);
		// coeff is the slowest moving dimension now, then row, then column
		return (coeff*cellsX*cellsY + row*cellsX + column);
	}
	inline __device__ real &getElement_coeff(int row, int column, int coeff)
	{
		return coefficients[getIndex_coeff(row, column, coeff)];
	}
	inline __host__ real &getElement_coeff_cpu(real* cpu_coeff, int row, int column, int coeff)
	{
		return cpu_coeff[getIndex_coeff(row, column, coeff)];
	}
	inline __device__ void setElement_coeff(int row, int column, int coeff, real setValue)
	{
		coefficients[getIndex_coeff(row, column, coeff)] = setValue;
	}
	inline __host__ void setElement_coeff_cpu(real* cpu_coeff, int row, int column, int coeff, real setValue)
	{
		cpu_coeff[getIndex_coeff(row, column, coeff)] = setValue;
	}

	// WAVESPEEDS
	// Dictates how the memory layout for waveSpeeds will be, we might be better off using all first waves first then second waves then third...
	// instead of having the first wave then second then third... of the first cell, then those of the second cell...
	// that is: return (waveNum*cellsX*cellsY + row*cellsX + col);
	inline __device__ int getIndex_waveSpeed(int row, int column, int waveNum)
	{
		// Usual C/C++ row major order
		return (row*cellsX*numWaves + column*numWaves + waveNum);
	}
	inline __device__ real &getElement_waveSpeedX(int row, int column, int waveNum)
	{
		return waveSpeedsX[getIndex_waveSpeed(row, column, waveNum)];
	}
	inline __device__ void setElement_waveSpeedX(int row, int column, int waveNum, real waveSpeed)
	{
		waveSpeedsX[getIndex_waveSpeed(row, column, waveNum)] = waveSpeed;
	}

	inline __device__ real &getElement_waveSpeedY(int row, int column, int waveNum)
	{
		return waveSpeedsY[getIndex_waveSpeed(row, column, waveNum)];
	}
	inline __device__ void setElement_waveSpeedY(int row, int column, int waveNum, real waveSpeed)
	{
		waveSpeedsY[getIndex_waveSpeed(row, column, waveNum)] = waveSpeed;
	}
};

#endif