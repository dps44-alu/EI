#include "../include/tokenizador.h"

#include <fstream>
#include <algorithm>
#include <cctype> 
#include <filesystem>

////////////////////////////////////////////////////////////////////////////////////////////////////////
//  TODOS PRIVADOS
////////////////////////////////////////////////////////////////////////////////////////////////////////
void Tokenizador::Copia (const Tokenizador& tok)
{
    delimiters = tok.delimiters;
    delimitersSet = tok.delimitersSet;
    casosEspeciales = tok.casosEspeciales;
    pasarAminuscSinAcentos = tok.pasarAminuscSinAcentos;
    extraEspacio = tok.extraEspacio;
}

void Tokenizador::AmpliaDelimitadoresPalabra (const string& nuevosDelimitadoresPalabra)
{
    for (const char& c : nuevosDelimitadoresPalabra)
    {
        if (delimitersSet.find(c) == delimitersSet.end())
        {
            delimiters += c;
            delimitersSet.insert(c);
        }
    }

    if (casosEspeciales && delimitersSet.find(' ') == delimitersSet.end())    // && delimiters.find(' ') == string::npos
    {
        delimiters += " ";
        delimitersSet.insert(' ');
        extraEspacio = true;
    }
}

string Tokenizador::ToLowerNoAccents(const string& str) {
    string result(str.length(), ' ');
    unsigned char c;
    for (unsigned int i  = 0; i < str.length(); i++)
    {
        c = static_cast<unsigned char>(str[i]);
        if ((c > 191 && c < 198) || (c > 223 && c < 230))   // A a
        {
            c = 97;     // a (97)
        } 
        else if ((c > 199 && c < 204) || (c > 231 && c < 236))   // E e
        {
            c = 101;    // e (101)
        } 
        else if ((c > 203 && c < 208) || (c > 235 && c < 240))   // I i
        {
            c = 105;    // i (105)
        } 
        else if ((c > 209 && c < 215) || (c > 241 && c < 247))   // O o
        {
            c = 111;    // o (111)
        } 
        else if ((c > 216 && c < 221) || (c > 248 && c < 253))   // U u
        {
            c = 117;    // u (117)
        } 
        else if (c == 209 || c == 241)  // Ñ ñ
        {
            c = 'n';
            //c = 110;    // n (110)
        } 
        else if (c == 199 || c == 231)   // C c
        {
            c = 99;    // c (99)
        } 
        else if (c == 221 || c == 253 || c == 255)   // Y y
        {
            c = 121;    // y (121)
        } 
        else if (c > 64 && c < 91)   // A (65) - Z (90)
        {
            c += 32;
        }
        result[i] = c;
    }
    return result;
}

bool Tokenizador::EsDelimitador (const char& c, const unordered_set<char>& excluidosSet = {}) const
{
    // Es true si c está en delimiters pero no en excluidos
    return delimitersSet.count(c) && !excluidosSet.count(c);
}

void Tokenizador::CompruebaTokenEspecial (const string& str, string& token, string::size_type& delimiter_pos, const char& c) const
{
    if (str[delimiter_pos] == c)  // Comprueba que el delimitador sea c (punto/guión)
    {
        string word = token;
        string::size_type pos = delimiter_pos;  // Posición del delimitador
        while (str[pos] == c)   // Vuelve simpre con pos = la posición del siguiente delimitador
        {
            word += c;
            pos++;
            if (isalpha(str[pos]) || isdigit(str[pos]))   // Comprueba si al punto/guión le sigue una letra o un numero
            {
                while (pos != str.length() && !EsDelimitador(str[pos]))     // Acaba cuando encuentra un delimitador
                {
                    word += str[pos];
                    pos++;
                }

                // Comprueba si a la letra/dígito le sigue un delimitador que no sea '.'/'-', o es el fin del string
                if (pos == str.length() || EsDelimitador(str[pos], {c}))
                {
                    token = word;
                    delimiter_pos = pos;
                    return;
                }
                else    // Le sigue un punto '.'/'-'
                {
                    if (!isalpha(str[pos + 1]) && !isdigit(str[pos + 1]))   // Termina si nos en número/letra
                    {
                        token = word;
                        delimiter_pos = pos;
                        return;
                    }
                }
            }
            else
            {
                return;
            }
        }
    }
}

void Tokenizador::CompruebaCasoEspecial (const string& str, string& token, string::size_type& delimiter_pos) const
{
    string word;
    string::size_type pos;
    // URL
    if ((token == "http" || token == "https" || token == "ftp") && str[delimiter_pos] == ':' && delimiter_pos != str.length() - 1)  // Cabecera
    {
        word = token;
        word += ':';
        pos = delimiter_pos;
        pos++;
        // Mientras no se encuentre un delimitador no excluido continue
        while (pos != str.length() && !EsDelimitador(str[pos], {'_', ':', '/', '.', '?', '&', '-', '=', '#', '@'}))
        {
            word += str[pos];
            pos++;
        }
        token = word;
        delimiter_pos = pos;
        return;
    }

    // Número decimal
    // Todo números o un punto (o coma) al principio + todo números
    if (all_of(token.begin(), token.end(), ::isdigit) 
    || ((token.front() == '.' || token.front() == ',') && all_of(token.begin() + 1, token.end(), ::isdigit)))
    {
        bool cero = false;
        if (token.front() == '.' || token.front() == ',')   // El punto/coma está al principio
        {
            cero = true;
            word = "0";
        }
        else 
        {
            // Si el punto/coma es delimitador, puede estar delante del token pero que no lo hayamos recogido
            int delimiter_anterior_pos = delimiter_pos - token.length() - 1;
            if (delimiter_anterior_pos > -1 && (str[delimiter_anterior_pos] == '.' || str[delimiter_anterior_pos] == ','))
            {
                cero = true;
                word = "0";
                word += str[delimiter_anterior_pos];
            }
        }
        word += token;
        pos = delimiter_pos;
        if (pos == str.length() || pos + 1 == str.length() || EsDelimitador(str[pos + 1]))  // Si no hay más
        {
            token = word;
            delimiter_pos = pos;
            return;
        }
        int it = 0;     // Para contar cuantas iteraciones del bucle llevamos
        while (true)
        {
            it++;
            // Si lo que sigue no es dígito, se comprueba, sino se añade sin más comprobaciones
            if (!isdigit(str[pos]))
            {
                // Si no hay más o es un delimitardor que no sea punto/coma
                if (pos == str.length() || EsDelimitador(str[pos], {'.', ','}))
                {
                    if (it == 1 && !cero)   break;
                    token = word;
                    delimiter_pos = pos;
                    return;
                }
                else
                {
                    // Si al punto/coma le sigue algo que no sea un delimitador, continua
                    if (str[pos] == '.' || str[pos] == ',')
                    {
                        if (pos + 1 == str.length() || EsDelimitador(str[pos + 1]))
                        {
                            token = word;
                            delimiter_pos = pos;
                            return;
                        }
                        word += str[pos];
                        pos++;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else    // Es dígito
            {
                word += str[pos];
                pos++;
            }
        }
    }

    // E-mail
    if (str[delimiter_pos] == '@')  // Comprueba que el delimitador sea '@'
    {
        word = token;
        pos = delimiter_pos;    // Posición del delimitador
        word += str[pos];
        pos++;
        while (true)
        {
            // Sólo se permiten los delimitadores que no sean un espacio o un @
            if (EsDelimitador(str[pos]) || pos == str.length())
            {
                if (pos == str.length() || str[pos] == ' ')
                {
                    token = word;
                    delimiter_pos = pos;
                    return;
                }
                else if (str[pos] == '@')
                {
                    break;
                }

            }
            word += str[pos];
            pos++;
        }
    }

    // Acrónimo
    CompruebaTokenEspecial(str, token, delimiter_pos, '.');

    // Multipalabra
    CompruebaTokenEspecial(str, token, delimiter_pos, '-');

}

void Tokenizador::TokenizarAux (const string& str, list<string>& tokens) const
{
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);   // Inicio del primer token
    string::size_type pos = str.find_first_of(delimiters, lastPos);     // Final del primer token
    if (casosEspeciales)
    {
        while(string::npos != pos || string::npos != lastPos)
        {
            if (pos == string::npos)    pos = str.length();
            string token = str.substr(lastPos, pos - lastPos);
            CompruebaCasoEspecial(str, token, pos);
            tokens.push_back(token);
            lastPos = str.find_first_not_of(delimiters, pos);
            pos = str.find_first_of(delimiters, lastPos);
        }
    }
    else
    {
        while(string::npos != pos || string::npos != lastPos)
        {
            if (pos == string::npos)    pos = str.length();
            string token = str.substr(lastPos, pos - lastPos);
            tokens.push_back(token);
            lastPos = str.find_first_not_of(delimiters, pos);
            pos = str.find_first_of(delimiters, lastPos);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//  CONTRUCTORES Y DESTRUCTOR
////////////////////////////////////////////////////////////////////////////////////////////////////////
Tokenizador::Tokenizador (const string& delimitadoresPalabra, const bool& kcasosEspeciales, const bool& minuscSinAcentos) {
    casosEspeciales = kcasosEspeciales;
    pasarAminuscSinAcentos = minuscSinAcentos;
    extraEspacio = false;
    AmpliaDelimitadoresPalabra(delimitadoresPalabra);
    delimitersSet = unordered_set<char>(delimiters.begin(), delimiters.end());
}

Tokenizador::Tokenizador (const Tokenizador& tok)
{
    Copia(tok);
}

Tokenizador::Tokenizador ()
{
    delimiters = ",;:.-/+*\\ '\"{}[]()<>¡!¿?&#=\t@";
    casosEspeciales = true;
    pasarAminuscSinAcentos = false;
    extraEspacio = false;
}

Tokenizador::~Tokenizador ()
{
    delimiters = "";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//  OPERADORES
////////////////////////////////////////////////////////////////////////////////////////////////////////
Tokenizador& Tokenizador::operator= (const Tokenizador& tok)
{
    Copia(tok);
    return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//  MÉTODOS PÚBLICOS
////////////////////////////////////////////////////////////////////////////////////////////////////////
void Tokenizador::Tokenizar (const string& str, list<string>& tokens) const
{
    tokens.clear();
    if (pasarAminuscSinAcentos)
    {
        string strMod = ToLowerNoAccents(str);

        TokenizarAux(strMod, tokens);
    }
    else
    {
        TokenizarAux(str, tokens);
    }
}

bool Tokenizador::Tokenizar (const string& NomFichEntr, const string& NomFichSal) const
{
    string cadena;
    list<string> tokens;

    ifstream i(NomFichEntr.c_str());
    if (!i) 
    {
        cerr << "ERROR: No existe el archivo: " << NomFichEntr << endl;
        return false;
    }

    ofstream f(NomFichSal, ios::out | ios::trunc);
    if (!f) {
        cerr << "ERROR: No se puede crear el archivo de salida: " << NomFichSal << endl;
        return false;
    }

    while (getline(i, cadena))
    {
        if (!cadena.empty())
        {
            Tokenizar(cadena, tokens);

            for (const auto& token : tokens)
            {
                f << token << '\n';
            }
        }
            
    }
    
    i.close();
    f.close();

    return true;
}

bool Tokenizador::Tokenizar (const string& NomFichEntr) const
{
    return Tokenizar(NomFichEntr, NomFichEntr + ".tk");
}

bool Tokenizador::TokenizarListaFicheros (const string& NomFichEntr) const
{
    string cadena;
    
    ifstream i(NomFichEntr.c_str());
    if (!i) 
    {
        cerr << "ERROR: No existe el archivo: " << NomFichEntr << endl;
        return false;
    }

    while (getline(i, cadena))
    {
        if(!cadena.empty())
        {  
            Tokenizar(cadena);
        }
    }

    i.close();

    return true;
}

bool Tokenizador::TokenizarDirectorio (const string& dirAIndexar) const
{

    if (!std::filesystem::is_directory(dirAIndexar))  // Verifica que es un directorio válido
    {
        return false;
    }

    bool exito = true;

    // Recorremos el directorio recursivamente
    for (const auto& entry : std::filesystem::recursive_directory_iterator(dirAIndexar))
    {
        if (std::filesystem::is_regular_file(entry)) 
        {
            string archivo = entry.path().string();

            // Llamamos a la función Tokenizar para cada archivo encontrado
            if (!Tokenizar(archivo))
            {
                cerr << "ERROR: No se pudo tokenizar el archivo: " << archivo << endl;
                exito = false;
            }
        }
    }

    return exito;
}

void Tokenizador::DelimitadoresPalabra (const string& nuevoDelimiters)
{
    delimiters = "";
    delimitersSet.clear();
    extraEspacio = false;
    AmpliaDelimitadoresPalabra(nuevoDelimiters);
}

void Tokenizador::AnyadirDelimitadoresPalabra (const string& nuevoDelimiters)
{
    AmpliaDelimitadoresPalabra(nuevoDelimiters);
}

string Tokenizador::DelimitadoresPalabra () const
{
    return delimiters;
}

void Tokenizador::CasosEspeciales (const bool& nuevoCasosEspeciales)
{
    if (casosEspeciales && !nuevoCasosEspeciales && extraEspacio)
    {
        delimiters.pop_back();
    }
    casosEspeciales = nuevoCasosEspeciales;
}

bool Tokenizador::CasosEspeciales () const
{
    return casosEspeciales;
}

void Tokenizador::PasarAminuscSinAcentos (const bool& nuevoPasarAminuscSinAcentos)
{
    pasarAminuscSinAcentos = nuevoPasarAminuscSinAcentos;
}

bool Tokenizador::PasarAminuscSinAcentos () const
{
    return pasarAminuscSinAcentos;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//  FUNCIONES AMIGAS
////////////////////////////////////////////////////////////////////////////////////////////////////////
ostream& operator<< (ostream& s, const Tokenizador& tok)
{
    s << "DELIMITADORES: " << tok.delimiters << " TRATA CASOS ESPECIALES: " << tok.casosEspeciales 
        << " PASAR A MINUSCULAS Y SIN ACENTOS: " << tok.pasarAminuscSinAcentos;

    return s;
}