#ifndef BUSCADOR_H_
#define BUSCADOR_H_

#include "indexadorHash.h"

#include <queue>
#include <cmath>
#include <sstream>
#include <fstream>
#include <iomanip>

using namespace std;

class ResultadoRI 
{
    friend ostream& operator<< (ostream&, const ResultadoRI&);

    private:
        double vSimilitud;
        long int idDoc;
        int numPregunta;

    public:
        ResultadoRI (const double& kvSimilitud, const long int& kidDoc, const int& np);
        bool operator< (const ResultadoRI& lhs) const;
        double VSimilitud () const;
        long int IdDoc () const;
        int NumPregunta () const;
};


class Buscador: public IndexadorHash {

    friend ostream& operator<<(ostream& s, const Buscador& p) {
        string preg;
        s << "Buscador: " << endl;
        if(p.DevuelvePregunta(preg))
        s << "\tPregunta indexada: " << preg << endl;
        else
        s << "\tNo hay ninguna pregunta indexada" << endl;
        s << "\tDatos del indexador: " << endl << (IndexadorHash) p;		// Invoca a la sobrecarga de la salida del Indexador

        return s;
    }

    private:	
        Buscador();	
        
        priority_queue<ResultadoRI> docsOrdenados;	
        
        int formSimilitud;
        
        double c;
        
        double k1;
        
        double b;

        double avgdl = 0.0;

        unordered_map<string, InformacionTermino> cacheTerminosConsulta;
        
        void Copia(const Buscador&);

        string generarResultados (const int&) const;

        void CalcularAvgdl();

        double sim (const InfDoc&);

        double wt_q (const InformacionTerminoPregunta&);

        double wt_d (const InformacionTermino&, const InfDoc&);

        double score (const InfDoc&);

        double IDF (const string&);

    public:
        Buscador(const string& directorioIndexacion, const int& f);
        
        Buscador(const Buscador&);

        ~Buscador();

        Buscador& operator= (const Buscador&);

        bool Buscar(const int& numDocumentos = 99999);
        
        bool Buscar(const string& dirPreguntas, const int& numDocumentos, const int& numPregInicio, const int& numPregFin);
        
        void ImprimirResultadoBusqueda(const int& numDocumentos = 99999) const;
        
        bool ImprimirResultadoBusqueda(const int& numDocumentos, const string& nombreFichero) const;
        
        int DevolverFormulaSimilitud() const;
        
        bool CambiarFormulaSimilitud(const int& f);
        
        void CambiarParametrosDFR(const double& kc);
        
        double DevolverParametrosDFR() const;
        
        void CambiarParametrosBM25(const double& kk1, const double& kb);
        
        void DevolverParametrosBM25(double& kk1, double& kb) const;
};

#endif
