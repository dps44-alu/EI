#ifndef INDEXADORINFORMACION_H_
#define INDEXADORINFORMACION_H_

#include <iostream>
#include <ostream>
#include <string>
#include <list>
#include <unordered_map> // Usar std::unordered_map
#include <vector>
#include <ctime>         // Para struct tm

using namespace std;

// --- Clase Fecha ---
class Fecha
{
public:
    int dia = 0, mes = 0, anyo = 0, hora = 0, min = 0, seg = 0; // Inicialización C++11
    Fecha() = default; // Usar constructor por defecto
    Fecha(struct tm *clock);
    bool operator <(const Fecha& fecha) const;
    Fecha& operator=(const Fecha&) = default; // Usar asignación por defecto
};

// --- Clase InformacionPregunta ---
class InformacionPregunta
{
    friend std::ostream& operator<<(std::ostream& s, const InformacionPregunta& p);
public:
    InformacionPregunta (const InformacionPregunta &) = default; // Usar copia por defecto
    InformacionPregunta (); // Necesita implementación para inicializar a 0
    ~InformacionPregunta () = default; // Destructor por defecto suficiente
    InformacionPregunta& operator= (const InformacionPregunta &) = default; // Usar asignación por defecto

// Miembros públicos según PDF (tipos ajustados)
    int numTotalPal;
    int numTotalPalSinParada;
    int numTotalPalDiferentes;

    int getNumTotalPal () const { return numTotalPal; }
    int getNumTotalPalSinParada () const { return numTotalPalSinParada; }
    int getNumTotalPalDiferentes () const { return numTotalPalDiferentes; }
};

// --- Clase InformacionTerminoPregunta ---
class InformacionTerminoPregunta
{
    friend std::ostream& operator<<(std::ostream& s, const InformacionTerminoPregunta& p);
public:
    InformacionTerminoPregunta (const InformacionTerminoPregunta &) = default;
    InformacionTerminoPregunta (); // Inicializar ft=0
    ~InformacionTerminoPregunta (); // Necesita limpiar lista
    InformacionTerminoPregunta & operator= (const InformacionTerminoPregunta&); // Implementar copia

// Miembros públicos según PDF
    int ft;
    std::list<int> posTerm;
};

// --- Clase InfColeccionDocs ---
class InfColeccionDocs
{
    friend std::ostream& operator<<(std::ostream& s, const InfColeccionDocs& p);
public:
    InfColeccionDocs (const InfColeccionDocs &) = default;
    InfColeccionDocs (); // Inicializar a 0
    ~InfColeccionDocs () = default;
    InfColeccionDocs & operator= (const InfColeccionDocs &) = default;

// Miembros públicos según PDF (tipos verificados con PDF)
    long int numDocs;
    long int numTotalPal;
    long int numTotalPalSinParada;
    long int numTotalPalDiferentes;
    long int tamBytes;

    long int getNumDocs () const { return numDocs; }
    long int getNumTotalPal () const { return numTotalPal; }
    long int getNumTotalPalSinParada () const { return numTotalPalSinParada; }
    long int getNumTotalPalDiferentes () const { return numTotalPalDiferentes; }
    long int getTamBytes () const { return tamBytes; }
};

// --- Clase InfTermDoc ---
class InfTermDoc
{
    friend std::ostream& operator<<(std::ostream& s, const InfTermDoc& p);
public:
    InfTermDoc(const InfTermDoc &) = default;
    InfTermDoc(); // Inicializar ft=0
    ~InfTermDoc (); // Necesita limpiar lista
    InfTermDoc& operator=(const InfTermDoc &); // Implementar copia

// Miembros públicos según PDF
    int ft;
    std::list<int> posTerm;
};

// --- Clase InfDoc ---
class InfDoc
{
    friend std::ostream& operator<<(std::ostream& s, const InfDoc& p);
public:
    InfDoc (const InfDoc &) = default;
    InfDoc (); // Inicializar a 0
    ~InfDoc () = default;
    InfDoc & operator= (const InfDoc &) = default;

// Miembros públicos según PDF (tipos verificados con PDF)
    long int idDoc;
    int numPal;
    int numPalSinParada;
    int numPalDiferentes;
    long int tamBytes;
    Fecha fechaModificacion;

    int getNumPal () const { return numPal; }
    int getNumPalSinParada () const { return numPalSinParada; };
    int getIdDoc () const { return idDoc; }
    long int getTamBytes () const { return tamBytes; }
};

// --- Clase InformacionTermino ---
class InformacionTermino
{
    friend std::ostream& operator<<(std::ostream&, const InformacionTermino&);
public:
    InformacionTermino (const InformacionTermino &); // Implementar copia (por el map)
    InformacionTermino (); // Inicializar ftc=0
    ~InformacionTermino (); // Necesita limpiar map
    InformacionTermino & operator= (const InformacionTermino &); // Implementar asignación

// Miembros públicos según PDF
    int ftc;
    std::unordered_map<long int, InfTermDoc> l_docs; // Usar std::unordered_map

    int getFtc () const { return ftc; }

    // Retorna True si el termino esta indexado en el documento con id pasado
    // En caso afirmativo, se utiliza el infdoc por parametro para retornar la referencia al objeto InformacionTerminoDocumento
    bool IndexedAtDocument (const int& d, InfTermDoc& infTermDoc) const
    {
        auto it = l_docs.find(d);

        if (it != l_docs.end())
        {
            infTermDoc = it->second;
            return true;
        }

        return false;
    }
};

#endif /* INDEXADORINFORMACION_H_ */