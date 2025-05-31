#ifndef RESULTADORI_H_
#define RESULTADORI_H_

#include <iostream>
#include <queue>

using namespace std;

class ResultadoRI 
{
    friend ostream& operator<<(ostream&, const ResultadoRI&);

    private:
        double vSimilitud;
        long int idDoc;
        int numPregunta;

    public:
        ResultadoRI(const double& kvSimilitud, const long int& kidDoc, const int& np);
        double VSimilitud() const;
        long int IdDoc() const;
        int NumPregunta() const;
        bool operator< (const ResultadoRI& lhs) const;
};

#endif