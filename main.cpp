#define _USE_MATH_DEFINES
#define STENCIL 3 

#include <iostream>
#include <fstream>
#include <cmath>
#include "math.h"
#include <iomanip>
#include <string>

// O efeito de ter colocado  o comando
#include "Matriz2d.h"

using namespace std;

// 1. Melhorar a estrutura do Codigo
//   1.a: Utilize estrutura de diretorio do projeto:
//        src/            --> todos os arquivos .cpp ✔
//        include/        --> todos os arquivos .h ✔
//        main.cpp ✔
//        CMakeLists.txt  --> Arquivo de configuração de projeto qye usa Cmake. ✔
// 2. Estudar https://cmake.org/ e gerar um scrip CMakeLists.txt para compilar ✔
//       o código considerando a estrutura acima.
// 3. Modificar o código para trabalhar apenas com dois/tres vetores. ✔
// 4. Considerar o tamanho do stencil nos loops i,j ✔
// 5. Implementar a camada de atenuação por borda. (implementado porem nao esta funcionando corretamente)
// 6: Remover os ifs internos nas funções get de matriz3D e atenuação ✔
// 7. Criar arquivo com funções de escrita e chamar a função dentro do laco do tempo.
// 8. Implementar uma estrutura ou classe para armazenar os parametros do problema


float fonte(int x, int z, float t, float fcorte, float xs, float zs){

    float td = t - ((2*sqrt(M_PI))/fcorte);
    float fc = (fcorte/(3*sqrt(M_PI)));

    if (x != xs || z != zs){
        return 0;
    } 

    float eq1 = (1.0 - 2.0 * M_PI * pow(M_PI * fc * td, 2));

    float eq2 = pow(M_E, M_PI*pow((M_PI*fc*td), 2));

    return eq1/eq2;

}

float atenuacao(float d){
    return pow(M_E, -1*pow(0.098*d, 2));
}

int main() {

    float X = 3000;     // Largura do dominio em m
    float Z = 3000;     // Altura do dominio em m
    float T = 1;        // Tempo total em s
    float c = 2200;     // Celeridade da onda em m/s
    float fcorte = 40;  // frequencia de pico em Hz
    float dx = 10;      // Passos em x
    float dz = dx;      // Passos em z
    float dt = 0.00025; // Passos no tempo

    int Nx = X/dx;      // Iteracoes em x
    int Nz = Z/dz;      // Iteracoes em z
    int Nt = 3500;      // T/dt; //Iteracoes no tempo

    int xs = 150;       // posicao da fonte em x
    int zs = 150;       // posicao da fonte em z

    float cou = c*dt/dx;  // numero de courant, para dx = dz
    float c1 = (pow(cou, 2)/12.0); 
    float c2 = pow(c*dt, 2);
    int borda = 20;       // largura da borda que sofrera a atenuacao
    int d;                // distancia entre o no e o contorno
    float atenuacoes[borda];  

    float val; // variavel auxiliar 

    // para calcular o mdf basta utilizar duas matrizes, uma para armazenar o valor atual
    // e outra para calcular o proximo valor 
    // como essa forma discreta da equacao da onda requer o valor dois passos atras no tempo (i, j, k - 2)
    // basta realizar a troca dos valores de u_current para a u_next, assim na proxima iteracao os valores
    // no tempo k - 2 ficam armazenados na mesma matriz que armazenara os valores futuros
    Matriz2d u_current(Nx, Nz);
    Matriz2d u_next(Nx, Nz);

    ofstream myfile;    
    string base(".dat");

    // imprime os parametros 
    cout << "Nx = " << Nx << endl;
    cout << "Nz = " << Nz << endl;
    cout << "Nt = " << Nt << endl;
    cout << "Numero de Courant = " << cou << endl;

    // calcula o vetor de atenuacoes para evitar que o calculo seja realizada a cada iteracao na borda
    // CHECK: verificar se o parametro de 
    //   entrada dever ser multiplicado por dx
    for(int i = 0; i < borda; i++){
        atenuacoes[i] = atenuacao(i);
    }

    // MDF 
    // o algoritmo segue a seguinte ordem:
    //      1. inicializa duas matrizes zeradas u_current e u_next
    //      2. calcula u_next a partir dos valores da u_current pelo MDF
    //      3. percorre as bordas aplicando uma funcao de atenuacao do valor para evitar a reflexao
    //      4. u_current passa a ser u_next e u_next recebe os valores anteriores de u_current
    //      5. gera um arquivo de dados para plot a cada x iteracoes no tempo
    for (int k = 0; k < Nt; k++){

        // calcula u_next
        for (int i = STENCIL; i < Nx - STENCIL; i++){
            for (int j = STENCIL; j < Nz - STENCIL; j++){

                val = 
                c1 *
                (
                    -1*(u_current(i - 2, j) + u_current(i, j - 2)) + 
                    16*(u_current(i - 1, j) + u_current(i, j - 1)) - 
                    60* u_current(i,     j) +
                    16*(u_current(i + 1, j) + u_current(i, j + 1)) -
                       (u_current(i + 2, j) + u_current(i, j + 2)) 
                ) 
                + 2*u_current(i, j) - u_next(i, j) - c2 * fonte(i, j, k*dt, fcorte, xs, zs);

                //u_next.set(i, j, val);

                u_next(i,j) = val;
            }
        }

        // percorre as bordas do modelo aplicando a atenuacao
        // 
        //     +-------------------+
        //     |                   |
        //     |                   |
        //     |                   |
        //     |                   |
        //     |                   |
        //     |                   |
        //     +-------------------+
        //
        // for (int i = 0; i < borda; i++){
        //     for(int j = 0; j < Nz; j++){
        //         u_next(i, j) =  u_next(i, j) * atenuacao(j*dx);
        //     }
        // }

        // Reynolds NR BC
        // du/dx = (u(x+dx,t) - u(x,t))/dx
        // du/dt = (u(x,t+dt) - u(x,t))/dt
        // na direçao X
        //   du/dt - vel*du/dx = 0
        // (u(x,t+dt) - u(x,t))/dt + vel*(u(x+dx,t) - u(x,t))/dx
        // (u(x,t+dt) - u(x,t)) =   dt*(-vel*(u(x+dx,t) - u(x,t))/dx
        // u(x, t+dt) = u(x,t) - cou * (u(x+dx,t) - u(x,t) )
        //    onde cou = dt*vel/dx
        for (int i = 0; i < STENCIL; i++) {
            for(int j = 0; j < Nz; j++) {
                u_next(i,j) = u_current(i,j) - cou*(u_current(i+1,j) - u_current(i,j));
            }

       for (int i = Nx-STENCIL; i < Nx; i++) {
            for(int j = 0; j < Nz; j++) {
                u_next(i,j) = u_current(i,j) - cou*(u_current(i+1,j) - u_current(i,j));
            }
        //
        //
        /* TODO: Essa troca de vetores pode ser evitada
         
         */
        // troca u_next <--> u_current
        for (int i = STENCIL; i < Nx - STENCIL; i++){
            for (int j = STENCIL; j < Nz - STENCIL; j++){
                val = u_next(i, j);
                u_next(i, j)    = u_current(i, j);
                u_current(i, j)=  val;
            }
        }

        // gera arquivo de dados a cada 500 iteracoes em k
        if (k % 200 == 0){
            myfile.open(".././data" + to_string(k/200) + base);
            for (int i = 0; i < Nx; i++){
                for (int j = 0; j < Nz; j++)
                {
                    myfile << i << " " << j << " " << u_current(i, j) << "\n";
                }
                myfile << "\n\n";
            }
            myfile.close();
        }

    }
}

