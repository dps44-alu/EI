#include "../include/buscador.h"   

/****************************************** ResultadoRI ******************************************/

///////////////////////////////////////////////////////
// CONTRUCTORES
///////////////////////////////////////////////////////
ResultadoRI::ResultadoRI (const double& kvSimilitud, const long int& kidDoc, const int& np)
{
    vSimilitud = kvSimilitud;
    idDoc = kidDoc;
    numPregunta = np;
}


///////////////////////////////////////////////////////
// OPERADORES
///////////////////////////////////////////////////////
bool ResultadoRI::operator< (const ResultadoRI& lhs) const
{
    if      (numPregunta == lhs.numPregunta)    return (vSimilitud < lhs.vSimilitud);
    else                                        return (numPregunta > lhs.numPregunta);
}


///////////////////////////////////////////////////////
// FUNCIONES PÚBLICAS
///////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////
// FUNCIONES AMIGAS
///////////////////////////////////////////////////////
ostream& operator<< (ostream &os, const ResultadoRI &res)
{
    os << res.vSimilitud << "\t\t" << res.idDoc << "\t" << res.numPregunta << endl;

    return os;
}



/****************************************** Buscador ******************************************/

///////////////////////////////////////////////////////
// FUNCIONES PRIVADAS
///////////////////////////////////////////////////////
void Buscador::Copia (const Buscador& buscador)
{
    formSimilitud = buscador.formSimilitud;
    c = 2;
    k1 = 1.2;
    b = 0.75;
    avgdl = buscador.avgdl;
    cacheTerminosConsulta = buscador.cacheTerminosConsulta;
}

// NumPregunta FormulaSimilitud NomDocumento Posicion PuntuacionDoc PreguntaIndexada
string Buscador::generarResultados (const int& numDocumentos) const
{
    string str;
    priority_queue<ResultadoRI> docs = docsOrdenados;

    for (int i = 0; i < numDocumentos && !docs.empty(); i++)
    {
        string salida = to_string(docs.top().NumPregunta()) + " ";

        if      (formSimilitud == 0)    salida += "DFR ";
        else                            salida += "BM25 ";

        // Obtener el nombre del fichero a partir del id de documento
        string fich;
        InfDoc infDoc;
        getDocById(docs.top().IdDoc(), fich, infDoc);

        // Quitar la ruta de la carpeta al nombre
        int posBarra = -1;

        for (int j = 0; j < fich.size(); j++)
        {
            if (fich[j] == '/') 
            {
                posBarra = j + 1;
            }
        }

        if (posBarra != -1)
        {
            fich = fich.substr(posBarra, fich.size() - posBarra);
        }

        // Quitar la extension .tim, quitandole los ultimos 4 caracteres
        fich = fich.substr(0, fich.size() - 4);

        salida += fich + " ";

        ostringstream conv;
        conv << i << " " << fixed << setprecision(6) << docs.top().VSimilitud()
            << " " << GetPregunta();

        salida += conv.str();
        str += salida + '\n';

        docs.pop();
    }

    return str;
}

void Buscador::CalcularAvgdl() {
    // avgdl : media de todas las |D| en la colección
    avgdl = 0.0;

    for (const auto& [_, infDoc] : GetIndiceDocs())
    {
        avgdl += infDoc.GetNumPalSinParada();
    }

    int numDocs = GetIndiceDocs().size();
    if (numDocs > 0)
        avgdl = 1.0 * avgdl / numDocs;
    else
        avgdl = 0.0;
}


// w_t,q : peso en la query de un termino de la query
double Buscador::wt_q (const InformacionTerminoPregunta& infTermPreg)
{
    // k : número de términos (no de parada) de la query q
    double k = GetInfPregunta().GetNumTotalPalSinParada();

    if (k == 0) return 0;

    // f_t,q : número de veces que el término aparece en la query
    return 1.0 * infTermPreg.GetFt() / k;
}

// w_t,d : peso en el documento de un termino de la query
double Buscador::wt_d (const InformacionTermino& infTerm, const InfDoc& infDoc)
{
    if (infTerm.GetL_docs_const().size() == 0 || infTerm.GetFtc() == 0 || infDoc.GetTamBytes() == 0) return 0.0;

    // f_t : número total de veces que el término aparece en toda la colección
    double ft = infTerm.GetFtc();

    // lambda_t = ft / N
    // N : cantidad de documentos en la colección 
    double lambda_t = 1.0 * ft / GetInformacionColeccionDocs().GetNumDocs();

    if (lambda_t == 0.0) return 0.0;

    const InfTermDoc* infTermDoc = infTerm.IndexedAtDocument(infDoc.GetIdDoc());
    if (!infTermDoc) return 0;

    // f_t,d : número de veces que el término aparece en el documento
    double ft_d = infTermDoc->GetFt();

    // l_d : longitud en palabras (no de parada) del documento
    double ld = infDoc.GetNumPalSinParada();

    // avr_l_d : media de palabras (no de parada) del tamaño de los documentos
    double avr_ld = 0.0;
    if (GetInformacionColeccionDocs().GetNumDocs() > 0)
        avr_ld = 1.0 * GetInformacionColeccionDocs().GetNumTotalPalSinParada() /
                  GetInformacionColeccionDocs().GetNumDocs();

    // f*_t,d = f_t,d * log2(1 + ((c * avr_l_d) / l_d))
    double s0 = 1.0 + ((c * avr_ld) / ld);
    if      (s0 > 0.0)  s0 = log2(s0);
    else                s0 = log2(1.0);

    double f_t_d = ft_d * s0;

    // n_t : número de documentos en los que aparece el término
    double nt = infTerm.GetL_docs_const().size();

    double s1 = 1.0 + lambda_t;
    if      (s1 > 0.0)  s1 = log2(s1);
    else                s1 = log2(1.0);

    double s2 = (1.0 + lambda_t) / lambda_t;
    if      (s2 > 0.0)  s2 = log2(s2);
    else                s2 = log2(1.0);

    double res = (s1 + f_t_d * s2);
    res *= (ft + 1.0) / (nt * (f_t_d + 1.0));

    return res;
}

/** Devuelve un indice de similitus entre la query indexada y el documento pasado*/
double Buscador::sim (const InfDoc& infDoc)
{
    double sim = 0;

    // k : número de términos (no de parada) de la query q
    for (const auto& [termino, infTermPreg] : GetIndicePregunta())
    {
        auto it = cacheTerminosConsulta.find(termino);
        if (it != cacheTerminosConsulta.end()) 
        {
            sim += wt_q(infTermPreg) * wt_d(it->second, infDoc);
        }
    }

    return sim;
}

double Buscador::IDF (const string& termino)
{
    // N : cantidad de documentos en la colección
    double N = GetInformacionColeccionDocs().GetNumDocs();

    // n(q_i) : número de documentos en los que aparece el término q_i
    double nqi = 0;
   
    auto it = cacheTerminosConsulta.find(termino);
    if (it != cacheTerminosConsulta.end()) 
    {
        nqi = static_cast<double>(it->second.GetL_docs_const().size()); 
    }

    return log2((N - nqi + 0.5) / (nqi + 0.5));
}

// Modelo BM25
double Buscador::score(const InfDoc& infDoc)
{
    double score = 0;

    // n : número de términos (no de parada) de la query
    for (const auto& [termino, infTermPreg] : GetIndicePregunta()) 
    {
        // f(q_i, D) : frecuencia del término q_i en el documento D
        double fqi_D = 0.0;

        auto it = cacheTerminosConsulta.find(termino);
        if (it != cacheTerminosConsulta.end()) 
        {
            const InfTermDoc* infTermDoc = it->second.IndexedAtDocument(infDoc.GetIdDoc());
            if (infTermDoc) 
            {
                fqi_D = static_cast<double>(infTermDoc->GetFt());
            }
        }

        // Si no aparece, suma 0 al score
        if (fqi_D == 0.0) continue;

        // |D| : número de palabras del documento D
        double D_abs = infDoc.GetNumPalSinParada();

        // Sumar al resultado
        double d = (fqi_D + k1 * (1.0 - b + b * (D_abs / avgdl)));
        if (d <= 0.0) d = 1.0;

        score += IDF(termino) * ((fqi_D * (k1 + 1.0)) / d);
    }

    return score;
}


///////////////////////////////////////////////////////
// CONSTRUCTORES
///////////////////////////////////////////////////////
Buscador::Buscador(const string& directorioIndexacion, const int& f): IndexadorHash(directorioIndexacion)
{
    formSimilitud = f;
    c = 2;
    k1 = 1.2;
    b = 0.75;
    CalcularAvgdl();
}

Buscador::Buscador(const Buscador& buscador): IndexadorHash(buscador)
{
    Copia(buscador);
}

Buscador::~Buscador()
{

}


///////////////////////////////////////////////////////
// OPERADORES
///////////////////////////////////////////////////////
Buscador& Buscador::operator= (const Buscador& buscador)
{
    Copia(buscador);
    return *this;
}


///////////////////////////////////////////////////////
// FUNCIONES PÚBLICAS
///////////////////////////////////////////////////////
bool Buscador::Buscar(const int& numDocumentos)
{
    docsOrdenados = {};

    // Cachear términos de la consulta
    unordered_map<string, InformacionTermino> cacheTerminos;
    for (const auto& [termino, _] : GetIndicePregunta()) 
    {
        InformacionTermino infTerm;
        if (Devuelve(termino, infTerm)) 
        {
            cacheTerminos[termino] = infTerm;
        }
    }

    // Guardar caché en atributo temporal
    cacheTerminosConsulta = move(cacheTerminos);

    if (formSimilitud == 0)
    {    
        // DFR
        for (const auto& [_, infDoc] : GetIndiceDocs()) 
        {
            docsOrdenados.push(ResultadoRI(sim(infDoc), infDoc.GetIdDoc(), 0));
        }
    }
    else
    {
        // BM25
        for (const auto& [_, infDoc] : GetIndiceDocs()) 
        {
            docsOrdenados.push(ResultadoRI(score(infDoc), infDoc.GetIdDoc(), 0));
        }
    }
    return true;
}

bool Buscador::Buscar(const string& dirPreguntas, const int& numDocumentos, const int& numPregInicio, const int& numPregFin)
{
    docsOrdenados = {};

    for (int numPreg = numPregInicio; numPreg <= numPregFin; numPreg++)
    {
        string nombre = dirPreguntas + "/" + to_string(numPreg) + ".txt";
        ifstream fichero;

        fichero.open(nombre);
        if (!fichero)
        {
            continue;
        }
        else
        {
            string linea, pregunta;
            while (getline(fichero, linea))
            {
                if (!pregunta.empty()) pregunta += ' ';
                pregunta += linea;
            }
            fichero.close();

            if (!IndexarPregunta(pregunta)) continue;

            priority_queue<ResultadoRI> temporal;

            // Cachear términos de la consulta
            unordered_map<string, InformacionTermino> cacheTerminos;
            for (const auto& [termino, _] : GetIndicePregunta()) 
            {
                InformacionTermino infTerm;
                if (Devuelve(termino, infTerm)) 
                {
                    cacheTerminos[termino] = infTerm;
                }
            }

            // Guardar caché en atributo temporal
            cacheTerminosConsulta = move(cacheTerminos);

            CalcularAvgdl();

            // DFR
            if (formSimilitud == 0)
            {
                for (const auto& [_, infDoc] : GetIndiceDocs()) 
                {
                    temporal.push(ResultadoRI(sim(infDoc), infDoc.GetIdDoc(), numPreg));
                }
            }
            else
            {
                // BM25
                for (const auto& [_, infDoc] : GetIndiceDocs()) 
                {
                    temporal.push(ResultadoRI(score(infDoc), infDoc.GetIdDoc(), numPreg));
                }
            }

            for (int i = 0; i < numDocumentos && !temporal.empty(); i++) 
            {
                docsOrdenados.push(temporal.top());
                temporal.pop();
            }
        }
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
