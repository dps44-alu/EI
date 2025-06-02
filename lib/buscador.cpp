#include "../include/buscador.h"   

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//
//  RESULTADORI
//
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////// CONSTRUCTORES ///////////////////////
ResultadoRI::ResultadoRI (const double& kvSimilitud, const long int& kidDoc, const int& np)
{
    vSimilitud = kvSimilitud;
    idDoc = kidDoc;
    numPregunta = np;
}


///////////////////////// OPERADORES /////////////////////////
bool ResultadoRI::operator< (const ResultadoRI& lhs) const
{
    if (numPregunta == lhs.numPregunta) return (vSimilitud < lhs.vSimilitud);
    else                                return (numPregunta > lhs.numPregunta);
}


///////////////////// FUNCIONES PÚBLICAS /////////////////////
double ResultadoRI::VSimilitud () const
{
    return vSimilitud;
}

long int ResultadoRI::IdDoc () const
{
    return idDoc;
}

int ResultadoRI::NumPregunta () const
{
    return numPregunta;
}

////////////////////// FUNCIONES AMIGAS //////////////////////
ostream& operator<< (ostream &os, const ResultadoRI &res)
{
    os << res.vSimilitud << "\t\t" << res.idDoc << "\t" << res.numPregunta << endl;
    return os;
}



//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//
//  BUSCADOR
//
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///////////////////// FUNCIONES PRIVADAS /////////////////////
void Buscador::Copia (const Buscador& buscador)
{
    formSimilitud = buscador.formSimilitud;
    c = 2;
    k1 = 1.2;
    b = 0.75;
    //setStopWords("StopWordsEspanyol.txt");
}

string Buscador::generarResultados (const int& numDocumentos) const
{
    string resultado;
    priority_queue<ResultadoRI> docs = docsOrdenados;

    for (int i = 0; i < numDocumentos && !docs.empty(); i++)
    {
        string salida = to_string(docs.top().NumPregunta()) + " ";

        if      (formSimilitud == 0)    salida += "DFR ";
        else                            salida += "BM25 ";

        //Obtengo el nombre del fichero a partir del id de documento
        string nombreFichero;
        InfDoc inf;
        getDocById(docs.top().IdDoc(), nombreFichero, inf);

        //Al nombre de fichero le quito la ruta de la carpeta
        int ultimaBarra = -1;

        for (int j = 0; j < nombreFichero.size(); j++)
        {
            if (nombreFichero[j] == '/') 
            {
                ultimaBarra = j + 1;
            }
        }

        if (ultimaBarra != -1)
        {
            nombreFichero = nombreFichero.substr(ultimaBarra, nombreFichero.size() - ultimaBarra);
        }

        //Ahora le quito la extension .tim, quitandole los ultimos 4 caracteres
        nombreFichero = nombreFichero.substr(0, nombreFichero.size() - 4);

        salida += nombreFichero + " ";

        ostringstream conv;
        conv << i << " " << docs.top().VSimilitud() << " " << getPregunta();

        salida += conv.str();
        resultado += salida + '\n';

        docs.pop();
    }

    return resultado;
}

/** Retorna un indice de similitus entre la query indexada y el documento pasado*/
double Buscador::sim (const InfDoc& Doc)
{
    double res = 0;

    for (const auto& [str, infPreg] : getIndicePregunta())
    {
        InformacionTermino infTerm;
        if (Devuelve(str, infTerm))
        {
            res += wt_q(infPreg) * wt_d(infTerm, Doc);
        }
    }

    return res;
}

// w_t,q : Devuelve el peso que tiene un termino en la query indexada
double Buscador::wt_q (const InformacionTerminoPregunta& termino)
{
    double k = getInfPregunta().getNumTotalPal();

    if (k == 0) return 0;

    return 1.0 * termino.ft / k;
}

// w_t,d : Devuelve el peso de un termino en el documento pasado por parametro
double Buscador::wt_d (const InformacionTermino& termino, const InfDoc& Doc)
{
    if (termino.l_docs.size() == 0 || termino.ftc == 0 || Doc.getTamBytes() == 0) return 0;

    // lambda_t = ft / N
    double lambda_t = 1.0 * termino.ftc / getInformacionColeccionDocs().getNumDocs();

    if (lambda_t == 0) return 0;

    // f*_t,d
    InfTermDoc infTerm;
    if (!termino.IndexedAtDocument(Doc.getIdDoc(), infTerm)) return 0;

    double ft_d = infTerm.ft;

    double avr_ld = getInformacionColeccionDocs().getTamBytes() / getInformacionColeccionDocs().getNumTotalPal();
    double ld = Doc.getNumPal();

    double f_t_d = ft_d * log2(1 + ((c * avr_ld) / ld));

    // w_t,d
    double ft = termino.ftc;
    double nt = termino.l_docs.size();

    double res = (log2(1 + lambda_t) + f_t_d * log2((1 + lambda_t) / lambda_t));
    res *= (ft + 1) / (nt * (f_t_d + 1));

    return res;
}

/** Funciones utilizadas en el modelo BM25*/
double Buscador::score(const InfDoc& Doc, double& avgdl)
{
    double res = 0;

    for (const auto& [termino, infoPregunta] : getIndicePregunta()) 
    {
        double fqi_D = 0;

        auto it = getIndice().find(termino);
        if (it != getIndice().end())    // El termino se encuentra en el documento
        {
            InfTermDoc infTermDoc;
            if (it->second.IndexedAtDocument(Doc.getIdDoc(), infTermDoc)) 
            {
                fqi_D = infTermDoc.ft;  
            }
        }

        // Si no aparece, no aporta al score
        if (fqi_D == 0) return 0;

        // |D|
        double D_abs = Doc.getNumPal();

        // Sumar al resultado
        res += IDF(termino) * ((fqi_D * (k1 + 1)) / (fqi_D + k1 * (1 - b + b * (D_abs / avgdl))));
    }

    return res;
}

double Buscador::IDF (const string& termino)
{
    double N = getInformacionColeccionDocs().getNumDocs();
    double nqi = 0;

    auto it = getIndice().find(termino);
    if (it != getIndice().end()) 
    {
        nqi = it->second.l_docs.size();  
    }

    return log((N - nqi + 0.5) / (nqi + 0.5));
}


//////////////////////// CONSTRUCTORES ///////////////////////
Buscador::Buscador(const string& directorioIndexacion, const int& f): IndexadorHash(directorioIndexacion)
{
    formSimilitud = f;
    c = 2;
    k1 = 1.2;
    b = 0.75;
}

Buscador::Buscador(const Buscador& buscador): IndexadorHash(buscador)
{
    Copia(buscador);
}

Buscador::~Buscador()
{

}


///////////////////////// OPERADORES /////////////////////////
Buscador& Buscador::operator= (const Buscador& buscador)
{
    Copia(buscador);
    return *this;
}


///////////////////// FUNCIONES PÚBLICAS /////////////////////
bool Buscador::Buscar(const int& numDocumentos)
{
    //Vacio los resultados anteriores
    while (!docsOrdenados.empty())
    {
        docsOrdenados.pop();
    }

    if (formSimilitud == 0)
    {    
        //Formula DFR
        //Añado a la lista la similitud para cada documento de la coleccion
        int numPreg = 0;
        for (const auto& [_, infDoc] : getIndiceDocs()) 
        {
            docsOrdenados.push(ResultadoRI(sim(infDoc), infDoc.getIdDoc(), numPreg));
            numPreg++;
        }
    }
    else
    {
        //Primero necesito calcular el parametro avgdl
        //que es la media del numero de palabras de la coleccion
        double avgdl = 0;
        for (const auto& [_, infDoc] : getIndiceDocs()) 
        {
            avgdl += infDoc.getNumPalSinParada();
        }
        avgdl = 1.0 * avgdl / getIndiceDocs().size();

        //Formula BM25
        int numPreg = 0;
        for (const auto& [_, infDoc] : getIndiceDocs()) 
        {
            docsOrdenados.push(ResultadoRI(score(infDoc, avgdl), infDoc.getIdDoc(), numPreg));
            numPreg++;
        }
    }
    return true;
}

bool Buscador::Buscar(const string& dirPreguntas, const int& numDocumentos, const int& numPregInicio, const int& numPregFin)
{
    double avgdl = 0;

    //Vacio los resultados anteriores
    while (!docsOrdenados.empty()) 
    {
        docsOrdenados.pop();
    }

    //Si se utiliza BM25, calculo ya avgdl que se utilizara para todos los calculos posteriores
    if (formSimilitud == 1)
    {
        unordered_map<string, InfDoc>::const_iterator itd;
        for (itd = getIndiceDocs().begin(); itd != getIndiceDocs().end(); itd++){
            avgdl += itd->second.getNumPalSinParada();
        }
        avgdl = 1.0 * avgdl / getIndiceDocs().size();
    }

    for (int actual = numPregInicio; actual < numPregFin || actual == numPregFin; actual++)
    {
        ifstream input;
        string fichero;
        ostringstream convert;
        convert << actual;
        fichero = dirPreguntas+"/"+convert.str()+".txt";

        //Obtengo la pregunta del fichero numero X
        input.open(fichero.c_str());
        if (!input)
        {
            cerr << "Error, no existe el fichero >" << fichero << "<" << endl;
            return false;
        }
        else
        {
            string pregunta;
            getline(input, pregunta);
            IndexarPregunta(pregunta);
            priority_queue<ResultadoRI> temporal;

            //Formula DFR
            if (formSimilitud == 0){
                //Añado a la lista la similitud para cada documento de la coleccion
                unordered_map<string, InfDoc>::const_iterator it;
                for (it = getIndiceDocs().begin(); it != getIndiceDocs().end(); it++)
                {
                    temporal.push(ResultadoRI(sim(it->second), it->second.idDoc, actual));
                }
            }
            else
            {
                //Formula BM25
                unordered_map<string, InfDoc>::const_iterator it;
                for (it = getIndiceDocs().begin(); it != getIndiceDocs().end(); it++)
                {
                    temporal.push(ResultadoRI(score(it->second, avgdl), it->second.idDoc, actual));
                }
            }

            for (int i = 0; i < numDocumentos; i++)
            {
                docsOrdenados.push(temporal.top());
                temporal.pop();
            }
        }

        input.close();
    }

    return true;
}

void Buscador::ImprimirResultadoBusqueda(const int& numDocumentos) const
{
    cout << generarResultados(numDocumentos);
}

bool Buscador::ImprimirResultadoBusqueda(const int& numDocumentos, const string& nombreFichero) const
{
    ofstream salida;
    salida.open(nombreFichero);

    string res = generarResultados(numDocumentos);

    if (salida)
    {
        salida << res;
        cout << res << endl;

        salida.close();

        return true;
    } 
    else
    {
        cerr << "Error al crear el fichero "<< nombreFichero << endl;
        return false;
    }
}

int Buscador::DevolverFormulaSimilitud() const
{
    return formSimilitud; 
}

bool Buscador::CambiarFormulaSimilitud(const int& f)
{
    if (f == 1 || f == 0)
    {
        formSimilitud = f;
        return true;
    }
    return false;
}

void Buscador::CambiarParametrosDFR(const double& kc)
{
    c = kc;
}

double Buscador::DevolverParametrosDFR() const
{
    return c;
}

void Buscador::CambiarParametrosBM25(const double& kk1, const double& kb)
{
    k1 = kk1;
    b = kb;
}

void Buscador::DevolverParametrosBM25(double& kk1, double& kb) const
{
    kk1 = k1;
    kb = b;
}