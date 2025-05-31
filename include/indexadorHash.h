#ifndef INDEXADORHASH_H_
#define INDEXADORHASH_H_

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <unordered_map> // Usar std::
#include <unordered_set> // Usar std::
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>

#include "indexadorInformacion.h"
#include "stemmer.h"      // Asume que stemmer.h está disponible
#include "tokenizador.h"  // Asume que tokenizador.h está disponible
using namespace std;

const std::string ficheroIndice = "indice";

class IndexadorHash
{
    friend std::ostream& operator<<(std::ostream& s, const IndexadorHash& p);

public:
    // --- Constructores y Destructor ---
    IndexadorHash(const std::string& fichStopWords, const std::string& delimitadores,
                  const bool& detectComp, const bool& minuscSinAcentos,
                  const std::string& dirIndice, const int& tStemmer,
                  const bool& almPosTerm); // almEnDisco eliminado

    IndexadorHash(const std::string& directorioIndexacion);
    IndexadorHash(const IndexadorHash&);
    ~IndexadorHash();
    IndexadorHash& operator=(const IndexadorHash&);

    // --- Métodos de Indexación ---
    bool Indexar(const std::string& ficheroDocumentos);
    bool IndexarDirectorio(const std::string& dirAIndexar);

    // --- Métodos de Persistencia ---
    bool GuardarIndexacion() const;
    bool RecuperarIndexacion(const std::string& directorioIndexacion);

    // --- Métodos de Impresión ---
    void ImprimirIndexacion() const;
    void ImprimirIndexacionPregunta() const;
    void ImprimirPregunta() const;

    // --- Métodos de Consulta (Pregunta) ---
    bool IndexarPregunta(const std::string& preg);
    bool DevuelvePregunta(std::string& preg) const;
    bool DevuelvePregunta(const std::string& word, InformacionTerminoPregunta& inf) const;
    bool DevuelvePregunta(InformacionPregunta& inf) const;

    // --- Métodos de Consulta (Índice Principal) ---
    bool Devuelve(const std::string& word, InformacionTermino& inf) const;
    bool Devuelve(const std::string& word, const std::string& nomDoc, InfTermDoc& InfDoc) const;
    bool Existe(const std::string& word) const;

    // --- Métodos de Borrado ---
    bool BorraDoc(const std::string& nomDoc);
    void VaciarIndiceDocs(); // Añadido según PDF
    void VaciarIndicePreg(); // Añadido según PDF

    // --- Métodos Accesores ---
    int NumPalIndexadas() const;
    std::string DevolverFichPalParada () const;
    int NumPalParada() const;
    std::string DevolverDelimitadores () const;
    bool DevolverCasosEspeciales () const;
    bool DevolverPasarAminuscSinAcentos () const;
    bool DevolverAlmacenarPosTerm () const;
    std::string DevolverDirIndice () const;
    int DevolverTipoStemming () const;
    void ListarPalParada() const;
    void ListarInfColeccDocs() const;
    void ListarTerminos() const;
    bool ListarTerminos(const std::string& nomDoc) const;
    void ListarDocs() const;
    bool ListarDocs(const std::string& nomDoc) const;


    //A partir de un ID de documento, obtiene el nombre del fichero y el objeto infdoc
    //Si la funcion retorna false, indica que el id no esta en la coleccion
    bool getDocById(const long int &, string &, InfDoc &) const;

    void setStopWords(const string &nombreFichero);

    std::unordered_map<std::string, InformacionTerminoPregunta> getIndicePregunta () const;
    std::unordered_map<std::string, InfDoc> getIndiceDocs () const;
    InformacionPregunta getInfPregunta () const;
    std::unordered_map<std::string, InformacionTermino> getIndice () const;
    InfColeccionDocs getInformacionColeccionDocs () const;


private:
    // Miembros privados
    std::string directorioIndice;
    std::unordered_map<std::string, InformacionTermino> indice;
    std::unordered_map<std::string, InfDoc> indiceDocs;
    InfColeccionDocs informacionColeccionDocs;
    unordered_map<string, string> stopWordsMap; // Mapa para palabras de parada

    std::string pregunta;
    std::unordered_map<std::string, InformacionTerminoPregunta> indicePregunta;
    InformacionPregunta infPregunta;

    std::string ficheroStopWords;
    std::unordered_set<std::string> stopWords;

    Tokenizador tok;
    stemmerPorter stemmer;
    int tipoStemmer;
    bool almacenarPosTerm;

    // Método auxiliar privado y const
    void procesarPalabra(std::string& palabra) const;
};

#endif