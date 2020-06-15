
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <strstream>
#include <sstream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <vector>

#ifndef SVM_COMMON_H
#define SVM_COMMON_H

#define MAX_PATH_LENGTH 256

typedef std::string TString;
typedef std::vector<std::string> TStringArray;

typedef std::pair<int, float> TVectorDim;
typedef std::vector<TVectorDim> TVector;
typedef std::vector<TVector> TVectorArray;

typedef std::vector<float> TFloatArray;

int split(const TString& s, char c, TStringArray& v);
bool cmp(const TVectorDim& p1, const TVectorDim& p2);

int read_sample(TString& s, TVector& x, float& y);
int write_sample(TString& s, TVector& x, float& y);
int batch_read_sample(std::ifstream& is, TVectorArray& x_array, TFloatArray& y_array, int& n);
int batch_write_sample(std::ofstream& os, TVectorArray& x_array, TFloatArray& y_array, int& n);

void print_vector(const TVector& v);

float dot_product(const TVector& v1, const TVector& v2);
TVector operator*(const TVector& v, float f);
TVector operator*(float f, const TVector& v);
TVector operator+(const TVector& v1, const TVector& v2);

#endif
