#ifndef _TOKENIZADOR_H_
#define _TOKENIZADOR_H_

#include <iostream>
#include <fstream>
#include <stdlib.h> // Considera <cstdlib>
#include <cstring> // Considera <cctype>
#include <list>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unordered_map>
#include <unordered_set>

// Quitar using namespace std; del header
using namespace std;

class Tokenizador {
    private:
        bool casosEspeciales;
        bool pasarAminuscSinAcentos;
        string delimiters;
        void cambiarAminuscSinAcentos(std::string& str) const;
        string urlDelimiters = "/_:.?&=-@";
        bool isDigit (const std::string &) const;
        bool starts_with(const std::string& str, const std::string& prefix) const;
        bool check_url(const std::string&, char) const;
        bool espacio;
        bool salto;

    public:
        Tokenizador(const std::string& delimitadoresPalabra, const bool& kcasosEspeciales, const bool& minuscSinAcentos);
        Tokenizador(const Tokenizador&);
        Tokenizador();
        ~Tokenizador();
        Tokenizador& operator= (const Tokenizador&);

        void Tokenizar (const std::string& str, std::list<std::string>& tokens) const;
        bool Tokenizar (const std::string& i, const std::string& o) const;
        bool Tokenizar(const std::string& i) const;
        bool TokenizarListaFicheros (const std::string& i) const;
        bool TokenizarDirectorio (const std::string& i) const;

        void DelimitadoresPalabra(const std::string& nuevoDelimiters);
        void AnyadirDelimitadoresPalabra(const std::string& nuevoDelimiters);

        // --- CORRECCIÓN: Solo declaraciones ---
        string DelimitadoresPalabra() const;
        void CasosEspeciales (const bool&);
        bool CasosEspeciales () const;
        void PasarAminuscSinAcentos (const bool& nuevoPasarAminuscSinAcentos);
        bool PasarAminuscSinAcentos () const;
        // --- FIN CORRECCIÓN ---


    friend std::ostream& operator<<(std::ostream&, const Tokenizador&);
};

#endif // _TOKENIZADOR_H_