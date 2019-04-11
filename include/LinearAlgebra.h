/*
 * LinearAlgebra.h
 *
 *  Created on: Jul 29, 2018
 *      Author: dzhou
 */

#ifndef LINEARALGEBRA_H_
#define LINEARALGEBRA_H_

#include <algorithm>
#include <string.h>

class MatrixAlgo{
public:
	static void setOpenBlasThread(int threads);
	static void multi(int rowa, int n, int colb, const double* a, const double* b, double* c);
	static void strassenMulti(int rowa, int n, int colb, const double* a, const double* b, double* c);
	static void naiveMulti(int rowa, int n, int colb, const double* a, const double* b, double* c);
	static void naiveMulti(int rowa, int n, int colb, int segmentSizeInBit, const double* a, const double* b, double** c);
	static void naiveMulti(int rowa, int n, int colb, int segmentSizeInBit, const double** a, const double** b, double* c);
	static void naiveMulti(int rowa, int n, int colb, int segmentSizeInBit, const double** a, const double** b, double** c);
	static void naiveMulti(int rowa, int n, int colb, int segmentSizeInBit, const double** a, const double* b, double* c);
	static void naiveMulti(int rowa, int n, int colb, int segmentSizeInBit, const double** a, const double* b, double** c);
	static void naiveMulti(int rowa, int n, int colb, int segmentSizeInBit, const double* a, const double** b, double* c);
	static void naiveMulti(int rowa, int n, int colb, int segmentSizeInBit, const double* a, const double** b, double** c);

	/**
	 * Calculate the product of A' and A
	 * row : the number of rows of the original matrix
	 * col : the number of columns of the original matrix
	 * matrix : the input matrix, i.e. row x col
	 * m : the output square matrix, i.e. row x row
	 */
	static void naiveTransposedMulti(int row, int col, const double* matrix, double* m);
	static void naiveTransposedMulti(int row, int col, int segmentSizeInBit, const double** matrix, double* m);

	static void inverse(int n, const double* a, double* b);
	static void solve(int n, const double* a, const double* b, double* x);
	static double det(int n, const double* a);

	/*
	 * matMultiVec: out = mat * vec
	 */
	static void matMultiVec(int rowNum, int colNum, const double *mat, const double *vec, double *out);
	static void matMultiVec(int rowNum, int colNum, int segmentSizeInBit, const double **mat, const double *vec, double *out);
	static void matMultiVec(int rowNum, int colNum, int segmentSizeInBit, const double **mat, const double *vec, double **out);
	static void matMultiVec(int rowNum, int colNum, int segmentSizeInBit, const double **mat, const double **vec, double *out);

	/*
	 * matTransMultiVec: out = mat.T * vec
	 */
	static void matTransMultiVec(int rowNum, int colNum, const double *mat, const double *vec, double *out);
	static void matTransMultiVec(int rowNum, int colNum, int segmentSizeInBit, const double **mat, const double *vec, double *out);
	static void matTransMultiVec(int rowNum, int colNum, int segmentSizeInBit, const double **mat, const double **vec, double *out);

	/*
	 * matTransMultiMat: out = mat.T * mat
	 */
	static void matTransMultiMat(int rowNum, int colNum, const double *mat, double *out) ;
	static void matTransMultiMat(int rowNum, int colNum, int segmentSizeInBit, const double **mat, double *out) ;

	/*
	 * matTransMultiVecMultiMat: out = mat.T * vec.elementWiseMulti(mat)
	 */
	static void matTransMultiVecMultiMat(int rowNum, int colNum, const double *mat, const double *vec, double *out);
	static void matTransMultiVecMultiMat(int rowNum, int colNum, int segmentSizeInBit, const double **mat, const double *vec, double *out) ;
	static void matTransMultiVecMultiMat(int rowNum, int colNum, int segmentSizeInBit, const double **mat, const double **vec, double *out);

	/**
	 * rows: the number of rows of the old matrix
	 * columns: the number of rows of the old matrix
	 */
	template <class T>
	static void transpose(int rows, int columns, const T* oldData, T* newData){
		int row=0;
		int col=0;
		int start=0;
		int size=rows*columns;
		T* dest=newData;
		while(start<size){
			*dest=*oldData++;
			++row;
			++start;
			if(row>=rows){
				row=0;
				++col;
				dest=newData+col;
			}
			else
				dest+=columns;
		}
	}

	template <class T>
	static void transpose(int rows, int columns, int segmentSizeInBit, const T** oldData, T** newData){
		int segmentSize = 1 << segmentSizeInBit;
		int segmentMask = segmentSize - 1;
		int size = rows*columns;
		int curS = 0;
		int endS = size / segmentSize + (size % segmentSize ? 1 : 0);
		int row=0;
		int col=0;
		int destCursor = 0;

		while(curS < endS){
			const T* srcData = oldData[curS];
			int count = std::min(segmentSize, size - segmentSize * curS);
			for(int i=0; i<count; ++i){
				newData[destCursor >> segmentSizeInBit][destCursor & segmentMask] = srcData[i];
				++row;
				if(row>=rows){
					++col;
					row=0;
					destCursor = col;
				}
				else
					destCursor+=columns;
			}
			++curS;
		}
	}

private:
	inline static double dotProduct(const double** a, int startSegA, int segAOffset, const double** b, int startSegB, int segBOffset, int segmentSize, int len){
		double sum = 0;
		while(len){
			int count = std::min(len, std::min(segmentSize - segAOffset, segmentSize - segBOffset));
			const double* rowA = a[startSegA] + segAOffset;
			const double* rowB = b[startSegB] + segBOffset;
			for(int i=0; i<count; ++i){
				sum += rowA[i] * rowB[i];
			}
			if(count == len)
				break;
			segAOffset += count;
			if(segAOffset >= segmentSize){
				++startSegA;
				segAOffset -= segmentSize;
			}
			segBOffset += count;
			if(segBOffset >= segmentSize){
				++startSegB;
				segBOffset -= segmentSize;
			}
			len -= count;
		}
		return sum;
	}

	inline static double dotProduct(const double* a, const double** b, int startSegB, int segBOffset, int segmentSize, int len){
		double sum = 0;
		while(len){
			int count = std::min(len, segmentSize - segBOffset);
			const double* rowB = b[startSegB] + segBOffset;
			for(int i=0; i<count; ++i){
				sum += a[i] * rowB[i];
			}
			if(count == len)
				break;
			a += count;
			segBOffset += count;
			if(segBOffset >= segmentSize){
				++startSegB;
				segBOffset -= segmentSize;
			}
			len -= count;
		}
		return sum;
	}

	template <typename T>
	static void copyVector(T **dest, T *src, int segmentSizeInBit, int length) {
	    int segmentSize = 1 << segmentSizeInBit;
	    int start = 0;
	    int segmentId = 0;
	    while (start < length) {
	        int segmentLength = std::min(length - start, segmentSize);
	        T *data = dest[segmentId];
	        memcpy(data, src + start, sizeof(T) * segmentLength);
	        segmentId++;
	        start += segmentLength;
	    }
	}


	static double *copyHugeMatrixToArray(int rowNum, int colNum, int segmentSizeInBit, const double **mat);

	static int croutLU(int n, double* s, double* d);
	static void croutSolve(int n, int p, const double*LU,  const double*b, double*x);
};


#endif /* LINEARALGEBRA_H_ */
