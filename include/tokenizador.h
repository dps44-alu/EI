#ifndef TOKENIZADOR_H
#define TOKENIZADOR_H

#include <iostream>
#include <list>
#include <unordered_set>

using namespace std;

class Tokenizador {
    friend ostream& operator<< (ostream&, const Tokenizador&);
    // cout << "DELIMITADORES: " << delimiters << " TRATA CASOS ESPECIALES: " << casosEspeciales << " PASAR A MINUSCULAS Y SIN ACENTOS: " << pasarAminuscSinAcentos;
    // Aunque se modifique el almacenamiento de los delimitadores por temas de eficiencia, 
    // el campo delimiters se imprimirá con el string leído en el tokenizador 
    // (tras las modificaciones y eliminación de los caracteres repetidos correspondientes)

    private:
        string delimiters;
        // Delimitadores de terminos. Aunque se modifique la forma de almacenamiento interna para mejorar la eficiencia, 
        // este campo debe permanecer para indicar el orden en que se introdujeron los delimitadores

        unordered_set<char> delimitersSet;
        // Para poder realizar búsquedas de O(1) en él

        bool casosEspeciales;
        // Si true detectará palabras compuestas y casos especiales. Sino, trabajará al igual que el algoritmo propuesto 
        // en la sección "Versión del tokenizador vista en clase"

        bool pasarAminuscSinAcentos;
        // Si true pasará el token a minúsculas y quitará acentos, antes de realizar la tokenización

        bool extraEspacio;
        // Si está a true significa que se ha añadido a delimiters el caracter ' ' artificialmente

        void Copia (const Tokenizador&);
        // Copia un objeto Tokenizador en otro

        void TokenizarAux (const string& str, list<string>& tokens) const;
        // Auxiliar de Tokenizar para evitar repetir código

        void AmpliaDelimitadoresPalabra (const string&);    
        // Añade nuevos delimitadores a la variable delimiters

        static string ToLowerNoAccents (const string&) ;
        // Modifica un string poniéndolo en minúsuclas y quitándole los acentos

        bool EsDelimitador (const char&, const unordered_set<char>&) const;
        // Comprueba si un caracter forma parte de los delimitadores, excluyendo a los que se pasen por parámetro

        void CompruebaTokenEspecial (const string&, string&, string::size_type&, const char&) const;
        // Comprueba que el token sea un acrónimo (c = '.') o una multipalabra (c == '-')

        void CompruebaCasoEspecial (const string&, string&, string::size_type&) const;
        // En caso de que casosEspeciales = true, comprueba que el token sea un caso especial o no

    public:
        Tokenizador (const string& delimitadoresPalabra, const bool& kcasosEspeciales, const bool& minuscSinAcentos);	
        // Inicializa delimiters a delimitadoresPalabra filtrando que no se introduzcan delimitadores repetidos (de izquierda a derecha, 
        // en cuyo caso se eliminarían los que hayan sido repetidos por la derecha); casosEspeciales a kcasosEspeciales; 
        // pasarAminuscSinAcentos a minuscSinAcentos

        Tokenizador (const Tokenizador&);

        Tokenizador ();	
        // Inicializa delimiters=",;:.-/+*\\ '\"{}[]()<>¡!¿?&#=\t@"; casosEspeciales a true; pasarAminuscSinAcentos a false

        ~Tokenizador ();
        // Pone delimiters=""

        Tokenizador& operator= (const Tokenizador&);

        void Tokenizar (const string& str, list<string>& tokens) const;
        // Tokeniza str devolviendo el resultado en tokens. La lista tokens se vaciará antes de almacenar el resultado de la tokenización. 

        bool Tokenizar (const string& i, const string& f) const; 
        // Tokeniza el fichero i guardando la salida en el fichero f (una palabra en cada línea del fichero). 
        // Devolverá true si se realiza la tokenización de forma correcta; false en caso contrario enviando a cerr el mensaje correspondiente 
        // (p.ej. que no exista el archivo i)

        bool Tokenizar (const string& i) const;
        // Tokeniza el fichero i guardando la salida en un fichero de nombre i añadiéndole extensión .tk 
        // (sin eliminar previamente la extensión de i por ejemplo, del archivo pp.txt se generaría el resultado en pp.txt.tk), 
        // y que contendrá una palabra en cada línea del fichero. Devolverá true si se realiza la tokenización de forma correcta; 
        // false en caso contrario enviando a cerr el mensaje correspondiente (p.ej. que no exista el archivo i)

        bool TokenizarListaFicheros (const string& i) const; 
        // Tokeniza el fichero i que contiene un nombre de fichero por línea guardando la salida en ficheros (uno por cada línea de i) 
        // cuyo nombre será el leído en i añadiéndole extensión .tk, y que contendrá una palabra en cada línea del fichero leído en i. 
        // Devolverá true si se realiza la tokenización de forma correcta de todos los archivos que contiene i; 
        // devolverá false en caso contrario enviando a cerr el mensaje correspondiente (p.ej. que no exista el archivo i, 
        // o que se trate de un directorio, enviando a "cerr" los archivos de i que no existan o que sean directorios; 
        // luego no se ha de interrumpir la ejecución si hay algún archivo en i que no exista)

        bool TokenizarDirectorio (const string& i) const; 
        // Tokeniza todos los archivos que contenga el directorio i, incluyendo los de los subdirectorios, 
        // guardando la salida en ficheros cuyo nombre será el de entrada añadiéndole extensión .tk, 
        // y que contendrá una palabra en cada línea del fichero. Devolverá true si se realiza la tokenización 
        // de forma correcta de todos los archivos; devolverá false en caso contrario enviando a cerr el mensaje correspondiente 
        // (p.ej. que no exista el directorio i, o los ficheros que no se hayan podido tokenizar)

        void DelimitadoresPalabra(const string& nuevoDelimiters); 
        // Inicializa delimiters a nuevoDelimiters, filtrando que no se introduzcan delimitadores repetidos 
        // (de izquierda a derecha, en cuyo caso se eliminarían los que hayan sido repetidos por la derecha)

        void AnyadirDelimitadoresPalabra(const string& nuevoDelimiters);
        // Añade al final de "delimiters" los nuevos delimitadores que aparezcan en "nuevoDelimiters" (no se almacenarán caracteres repetidos)

        string DelimitadoresPalabra() const;
        // Devuelve "delimiters" 

        void CasosEspeciales (const bool& nuevoCasosEspeciales);
        // Cambia la variable privada "casosEspeciales" 

        bool CasosEspeciales () const;
        // Devuelve el contenido de la variable privada "casosEspeciales" 

        void PasarAminuscSinAcentos (const bool& nuevoPasarAminuscSinAcentos);
        // Cambia la variable privada "pasarAminuscSinAcentos". Atención al formato de codificación del corpus (comando "file" de Linux). 
        // Para la corrección de la práctica se utilizará el formato actual (ISO-8859). 

        bool PasarAminuscSinAcentos () const;
        // Devuelve el contenido de la variable privada "pasarAminuscSinAcentos"
};

#endif