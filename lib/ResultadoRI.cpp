#include "../include/ResultadoRI.h"

ResultadoRI::ResultadoRI(const double& kvSimilitud, const long int& kidDoc, const int& np)
{
    vSimilitud = kvSimilitud;
    idDoc = kidDoc;
    numPregunta = np;
}

double ResultadoRI::VSimilitud() const
{
    return vSimilitud;
}

long int ResultadoRI::IdDoc() const
{
    return idDoc;
}

int ResultadoRI::NumPregunta() const
{
    return numPregunta;
}

bool ResultadoRI::operator< (const ResultadoRI& lhs) const
{
    if (numPregunta == lhs.numPregunta) return (vSimilitud < lhs.vSimilitud);
    else                                return (numPregunta > lhs.numPregunta);
}

ostream& operator<<(ostream &os, const ResultadoRI &res)
{
    os << res.vSimilitud << "\t\t" << res.idDoc << "\t" << res.numPregunta << endl;
    return os;
}

/*
int main()
{
    priority_queue<ResultadoRI> mypq;
    mypq.push(ResultadoRI(30, 1, 1));
    mypq.push(ResultadoRI(100, 2, 1));
    mypq.push(ResultadoRI(100, 5, 1));
    mypq.push(ResultadoRI(25, 3, 2));
    mypq.push(ResultadoRI(40, 4, 2));
    mypq.push(ResultadoRI(200, 4, 2));
    mypq.push(ResultadoRI(250, 4, 2));
    mypq.push(ResultadoRI(400, 4, 3));
    mypq.push(ResultadoRI(40, 4, 3));
    cout << "Mostrando la cola de prioridad...\nvSimilitud\tNumDoc\tNumPregunta\n";

    while (!mypq.empty())
    {
        cout << mypq.top();
        mypq.pop();
    }
    cout << endl;

    // SALIDA POR PANTALLA:
    //Mostrando la cola de prioridad...
    //vSimilitud NumDoc NumPregunta
    //100 2 1
    //100 5 1
    //30 1 1
    //250 4 2
    //200 4 2
    //40 4 2
    //25 3 2
    //400 4 3
    //40 4 3
    return 0;
}
*/