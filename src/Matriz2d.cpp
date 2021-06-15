
#include <cstring>
using namespace std;

#include "Matriz2d.h"

Matriz2d::Matriz2d(int nl, int nc){
    this->n_linhas = nl;
    this->n_cols   = nc;
    int size       = this->n_linhas*this->n_cols;
    mat = new float[size];

    std::memset(mat,0.0,size*sizeof(float));

    //for (int i = 0; i < size; i++){
    //    mat[i] = 0;
    //}
}

Matriz2d::~Matriz2d(){
    delete [] mat;
}


// Modifiquei o acesso ao vetor para melhorar e eficiencia.
float Matriz2d::get(int i, int j)
{
    return mat[i*this->n_linhas + j];
}

void Matriz2d::set(int i, int j, float val){
    mat[i*this->n_linhas + j] = val;
}

float& Matriz2d::operator()(int i, int j)
{
    return  mat[i*this->n_linhas + j];
}
