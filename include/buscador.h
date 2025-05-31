#ifndef BUSCADOR_H_
#define BUSCADOR_H_

#include "indexadorHash.h"
#include "ResultadoRI.h"

#include <queue>
#include <cmath>

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

public:
    Buscador(const string& directorioIndexacion, const int& f);
    // Constructor para inicializar Buscador a partir de la indexaci�n generada previamente y almacenada en "directorioIndexacion". En caso que no exista el directorio o que no contenga los datos de la indexaci�n se enviar� a cerr la excepci�n correspondiente y se continuar� la ejecuci�n del programa manteniendo los �ndices vac�os
    // Inicializar� la variable privada "formSimilitud" a "f" y las constantes de cada modelo: "c = 2;  k1 = 1.2;  b = 0.75;"

    Buscador(const Buscador&);

    ~Buscador();

    Buscador& operator= (const Buscador&);

    // En los m�todos de "Buscar" solo se evaluar�n TODOS los documentos que contengan alguno de los t�rminos de la pregunta (tras eliminar las palabras de parada). 
    bool Buscar(const int& numDocumentos = 99999);
    // Devuelve true si en IndexadorHash.pregunta hay indexada una pregunta no vac�a con alg�n t�rmino con contenido, y si sobre esa pregunta se finaliza la b�squeda correctamente con la f�rmula de similitud indicada en la variable privada "formSimilitud".
    // Por ejemplo, devuelve falso si no finaliza la b�squeda por falta de memoria, mostrando el mensaje de error correspondiente, e indicando el documento y t�rmino en el que se ha quedado.
    // Se guardar�n los primeros "numDocumentos" documentos m�s relevantes en la variable privada "docsOrdenados" en orden decreciente seg�n la relevancia sobre la pregunta (se vaciar� previamente el contenido de esta variable antes de realizar la b�squeda). Se almacenar�n solo los documentos que compartan alg�n t�rmino (no de parada) con la query (aunque ese n�mero de documentos sea inferior a "numDocumentos"). Como n�mero de pregunta en "ResultadoRI.numPregunta" se almacenar� el valor 0. En caso de que no se introduzca el par�metro "numDocumentos", entonces dicho par�metro se inicializar� a 99999)

    bool Buscar(const string& dirPreguntas, const int& numDocumentos, const int& numPregInicio, const int& numPregFin);
    // Realizar� la b�squeda entre el n�mero de pregunta "numPregInicio" y "numPregFin", ambas preguntas incluidas. El corpus de preguntas estar� en el directorio "dirPreguntas", y tendr� la estructura de cada pregunta en un fichero independiente, de nombre el n�mero de pregunta, y extensi�n ".txt" (p.ej. 1.txt 2.txt 3.txt ... 83.txt). Esto significa que habr� que indexar cada pregunta por separado y ejecutar una b�squeda por cada pregunta a�adiendo los resultados de cada pregunta (junto con su n�mero de pregunta) en la variable privada "docsOrdenados". Asimismo, se supone que previamente se mantendr� la indexaci�n del corpus.
    // Se guardar�n los primeros "numDocumentos" documentos m�s relevantes para cada pregunta en la variable privada "docsOrdenados". Se almacenar�n solo los documentos que compartan alg�n t�rmino (no de parada) con la query (aunque ese n�mero de documentos sea inferior a "numDocumentos").
    // La b�squeda se realiza con la f�rmula de similitud indicada en la variable privada "formSimilitud".
    // Devuelve falso si no finaliza la b�squeda (p.ej. por falta de memoria), mostrando el mensaje de error correspondiente, indicando el documento, pregunta y t�rmino en el que se ha quedado.

    void ImprimirResultadoBusqueda(const int& numDocumentos = 99999) const;
    // Imprimir� por pantalla los resultados de la �ltima b�squeda (un n�mero M�XIMO de "numDocumentos" (en caso de que no se introduzca el par�metro "numDocumentos", entonces dicho par�metro se inicializar� a 99999) por cada pregunta, entre los que tienen alg�n t�rmino de la pregunta), los cuales estar�n almacenados en la variable privada "docsOrdenados" en orden decreciente seg�n la relevancia sobre la pregunta, en el siguiente formato (una l�nea por cada documento): 
   
    bool ImprimirResultadoBusqueda(const int& numDocumentos, const string& nombreFichero) const;
    // Lo mismo que "ImprimirResultadoBusqueda()" pero guardando la salida en el fichero de nombre "nombreFichero"
    // Devolver� false si no consigue crear correctamente el archivo

    int DevolverFormulaSimilitud() const;
    // Devuelve el valor del campo privado "formSimilitud"

    bool CambiarFormulaSimilitud(const int& f);
    // Cambia el valor de "formSimilitud" a "f" si contiene un valor correcto (f == 0 || f == 1);
    // Devolver� false si "f" no contiene un valor correcto, en cuyo caso no cambiar�a el valor anterior de "formSimilitud"

    void CambiarParametrosDFR(const double& kc);
    // Cambia el valor de "c = kc"

    double DevolverParametrosDFR() const;
    // Devuelve el valor de "c"

    void CambiarParametrosBM25(const double& kk1, const double& kb);
    // Cambia el valor de "k1 = kk1; b = kb;"

    void DevolverParametrosBM25(double& kk1, double& kb) const;
    // Devuelve el valor de "k1" y "b"

private:	
    Buscador();	
    // Este constructor se pone en la parte privada porque no se permitir� crear un buscador sin inicializarlo convenientemente a partir de una indexaci�n. 
    // Se inicializar� con todos los campos vac�os y la variable privada "formSimilitud" con valor 0 y las constantes de cada modelo: "c = 2;  k1 = 1.2; b = 0.75"

    priority_queue< ResultadoRI > docsOrdenados;	
    // Contendr� los resultados de la �ltima b�squeda realizada en orden decreciente seg�n la relevancia sobre la pregunta. El tipo "priority_queue" podr� modificarse por cuestiones de eficiencia. La clase "ResultadoRI" aparece en la secci�n "Ejemplo de modo de uso de la cola de prioridad de STL"

    int formSimilitud;
    // 0: DFR, 1: BM25

    double c;
    // Constante del modelo DFR

    double k1;
    // Constante del modelo BM25

    double b;
    // Constante del modelo BM25

    void Copia(const Buscador&);

    string generarResultados (const int&) const;

    double sim (const InfDoc&);

    double pesoTerminoQuery (const InformacionTerminoPregunta&);

    double pesoTerminoDocumento (const InformacionTermino&, const InfDoc&);

    double lambda (const InformacionTermino&);

    double fPrimaTerminoDocumento (const InformacionTermino&, const InfDoc&) const;

    double score (const InfDoc&, double&);

    double IDF (const string&);
};

#endif
