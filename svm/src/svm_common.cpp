#include "svm_common.h"

using std::ifstream;
using std::ofstream;
using std::cerr;
using std::cout;
using std::endl;
using std::ostringstream;

int split(const TString& s, char c, TStringArray& v){
    TString::size_type i = 0;
    TString::size_type j = s.find(c);
    if (j != TString::npos){   
        while (j != TString::npos){
            v.push_back(s.substr(i, j - i));
            i = ++j;
            j = s.find(c, j);
            if (j == TString::npos){
                v.push_back(s.substr(i, s.length()));
            }
        }
    }
    else{
        v.push_back(s);
    }
    return 0;
}

bool cmp(const TVectorDim& p1, const TVectorDim& p2){
    return p1.first < p2.first;
}

float dot_product(const TVector& v1, const TVector& v2){
    float dot = 0.0;
    int p1 = 0;
    int p2 = 0;
    int a1 = -1;
    int a2 = -1;
    while (p1 < v1.size() && p2 < v2.size()){
        a1 = v1[p1].first;
        a2 = v2[p2].first;
        if (a1 == a2){
            dot += v1[p1].second * v2[p2].second;
            p1++;
            p2++;
        }
        else if (a1 > a2){
            p2++;
        }
        else{
            p1++;
        }
    }
    return dot;
}

void print_TVector(const TVector& v){
    for (int i = 0; i < v.size(); i++){
        cerr << v[i].first << ':' << v[i].second << endl;
    }
}

TVector operator*(const TVector& v, float f){
    TVector p;
    for (int i = 0; i < v.size(); i++){
        p.push_back(TVectorDim(v[i].first, v[i].second * f));
    }
    return p;
}

TVector operator*(float f, const TVector& v){
    return v * f;
}

TVector operator+(const TVector& v1, const TVector& v2){
    TVector s;
    int p1 = 0;
    int p2 = 0;
    int a1 = -1;
    int a2 = -1;
    while (p1 < v1.size() && p2 < v2.size()){
        a1 = v1[p1].first;
        a2 = v2[p2].first;
        if (a1 == a2){
            TVectorDim p(a1, v1[p1].second + v2[p2].second);
            s.push_back(p);
            p1++;
            p2++;
        }
        else if (a1 > a2){
            s.push_back(v2[p2]);
            p2++;
        }
        else{
            s.push_back(v1[p1]);
            p1++;
        }
    }
    for (; p1 < v1.size(); p1++){
        s.push_back(v1[p1]);
    }
    for (; p2 < v2.size(); p2++){
        s.push_back(v2[p2]);
    }
    return s;
}

int batch_read_sample(ifstream& is, TVectorArray& x_array, TFloatArray& y_array, int& n){
    TString s;
    TVector x;
    float y = 0.0;
    n = 0;
    while (getline(is, s, '\n')){
        read_sample(s, x, y);
        x_array.push_back(x);
        y_array.push_back(y);
        n += 1;
    }
    return 0;
}

int batch_write_sample(ofstream& os, TVectorArray& x_array, TFloatArray& y_array, int& n){
    TString s;
    n = 0;
    for (int i = 0; i < x_array.size(); i++){
        write_sample(s, x_array[i], y_array[i]);
        os << s << endl;
        n += 1;
    }
    return 0;
}

int read_sample(TString& s, TVector& x, float& y){
    int key = -1;
    float val = 0.0;
    TStringArray s_arr;
    split(s, ' ', s_arr);
    
    y = atof(s_arr[0].c_str());
    x.clear();
    for (int i = 1; i < s_arr.size(); i++){
        TStringArray kv;
        split(s_arr[i], ':', kv);
        key = atoi(kv[0].c_str());
        val = atof(kv[1].c_str());
        TVectorDim p(key, val);
        x.push_back(p);
    }
    sort(x.begin(), x.end(), cmp);
    return 0;
}

int write_sample(TString& s, TVector& x, float& y){
    ostringstream oss;
    oss << y;
    for (int i = 0; i < x.size(); i++){
        oss << ' ' << x[i].first << ':' << x[i].second;
    }
    s = oss.str();
    return 0;
}

