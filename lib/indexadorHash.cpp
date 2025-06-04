#include "../include/indexadorHash.h"

#include <fstream>
#include <unistd.h>
#include <sys/stat.h>

///////////////////////////////////////////////////////
// FUNCIONES PRIVADAS
///////////////////////////////////////////////////////
void IndexadorHash::Copia (const IndexadorHash &ih)
{
    if (this != &ih)
    {
        indice = ih.indice;
        indiceDocs = ih.indiceDocs;
        informacionColeccionDocs = ih.informacionColeccionDocs;
        pregunta = ih.pregunta;
        indicePregunta = ih.indicePregunta;
        infPregunta = ih.infPregunta;
        stopWords = ih.stopWords;
        ficheroStopWords = ih.ficheroStopWords;
        tok = ih.tok;
        directorioIndice = ih.directorioIndice;
        tipoStemmer = ih.tipoStemmer;
        almacenarPosTerm = ih.almacenarPosTerm;
    }
}

void IndexadorHash::VaciarIndices ()
{
    indice.clear();
    VaciarIndiceDocs();
    VaciarIndicePreg();
}

void IndexadorHash::LimpiarIndexador ()
{
    VaciarIndices();
    pregunta = "";
    ficheroStopWords = "";
    directorioIndice = "";
    tipoStemmer = 0;
    almacenarPosTerm = false;
}

string IndexadorHash::ToLowerNoAccents(const string& str) const
{
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


///////////////////////////////////////////////////////
// CONTRUCTORES
///////////////////////////////////////////////////////
IndexadorHash::IndexadorHash ()
{
    indice = {};
    indiceDocs = {};
    informacionColeccionDocs = InfColeccionDocs();
    pregunta = "";
    indicePregunta = {};
    infPregunta = InformacionPregunta();
    stopWords = {};
    ficheroStopWords = "";
    tok = Tokenizador();
    directorioIndice = get_current_dir_name();
    tipoStemmer = 0;
    almacenarPosTerm = false;
}

IndexadorHash::IndexadorHash (const string &fichStopWords, const string &delimitadores, const bool &detectComp, const bool &minuscSinAcentos,
              const string &dirIndice, const int &tStemmer, const bool &almPosTerm)
{
    ficheroStopWords = fichStopWords;

    ifstream f(ficheroStopWords, ifstream::in);
    if (f.good())
    {
        string cadena;
        while (f >> cadena)
        {
            stopWords.insert(cadena);
        }

        f.close();
    }
    else
    {
        cerr << "ERROR: No existe el archivo: " << ficheroStopWords << "\n";
    }

    tok = Tokenizador(delimitadores, detectComp, minuscSinAcentos);

    if (dirIndice == "")
    {
        directorioIndice = get_current_dir_name();
    }
    else
    {
        directorioIndice = dirIndice;
    }

    tipoStemmer = tStemmer;
    almacenarPosTerm = almPosTerm;

    indice = {};
    indiceDocs = {};
    informacionColeccionDocs = InfColeccionDocs();
    pregunta = "";
    indicePregunta = {};
    infPregunta = InformacionPregunta();
}

IndexadorHash::IndexadorHash (const string &directorioIndexacion)
{
    RecuperarIndexacion(directorioIndexacion);
}

IndexadorHash::IndexadorHash (const IndexadorHash &ih)
{
    Copia(ih);
}

IndexadorHash::~IndexadorHash ()
{
    LimpiarIndexador();
}


///////////////////////////////////////////////////////
// OPERADORES
///////////////////////////////////////////////////////
IndexadorHash& IndexadorHash::operator= (const IndexadorHash& ih)
{
    Copia(ih);
    return *this;
}


///////////////////////////////////////////////////////
// FUNCIONES PÚBLICAS
///////////////////////////////////////////////////////
bool IndexadorHash::Indexar (const string &ficheroDocumentos)
{
    ifstream fichDocumentos(ficheroDocumentos, ifstream::in);

    string nomDoc;		        // Nombre documento a indexar
    string lineaDoc;	        // Linea a tokenizar

    bool indexar = false;	    // True/False, sí/no se ha indexado
    struct stat statDocumento;
    int idDocAIndexar;
    list<string> tokens;
    int posTerm;		        // Posición del término en el documento

    if (fichDocumentos.good())
    {
        ifstream documento;
        nomDoc = "";

        while (getline(fichDocumentos, nomDoc))
        {
            auto itIndiceDocs = indiceDocs.find(nomDoc);
            idDocAIndexar = 0;
            indexar = false;
            posTerm = 0;

            stat(nomDoc.c_str(), &statDocumento);
            Fecha fechaDoc(gmtime(&(statDocumento.st_mtime)));

            // Comprobar si ya existe el documento
            if (itIndiceDocs != indiceDocs.end())
            {
                // Guardar el id del documento indexado antes
                if (itIndiceDocs->second.GetFechaModicifacion() < fechaDoc)
                {
                    idDocAIndexar = itIndiceDocs->second.GetIdDoc();
                    indexar = true;
                }

                // Borrar el documento antiguo.
                if (!BorraDoc(nomDoc))
                {
                    return false;
                }
            }
            else
            {
                indexar = true;     // Si no existe, se indexa
            }

            if (indexar)
            {
                documento.open(nomDoc.c_str(), ifstream::in);
                if (documento.good())
                {
                    InfDoc informacionDocumento;

                    if (idDocAIndexar != 0)
                    {
                        informacionDocumento.SetIdDoc(idDocAIndexar);
                    }
                    else
                    {
                        informacionColeccionDocs.SetNumDocs(informacionColeccionDocs.GetNumDocs() + 1);
                        informacionDocumento.SetIdDoc(informacionColeccionDocs.GetNumDocs());
                    }

                    informacionDocumento.SetTamBytes(statDocumento.st_size);
                    informacionDocumento.SetFechaModificacion(fechaDoc);

                    while (getline(documento, lineaDoc))
                    {
                        tok.Tokenizar(lineaDoc, tokens);        // Obtener los tokens de la linea

                        // Acumular el número de palabras de la línea
                        informacionDocumento.SetNumPal(informacionDocumento.GetNumPal() + tokens.size());

                        // Recorrer los tokens de la línea leida
                        for (auto itTokens = tokens.begin(); itTokens != tokens.end(); ++itTokens)
                        {
                            stemmer.stemmer((*itTokens), tipoStemmer);

                            auto itStopWords = stopWords.find((*itTokens));
                            if (itStopWords == stopWords.end())     // No es una stop word
                            {
                                // Incrementa palabras sin stop words
                                informacionDocumento.SetNumPalSinParada(informacionDocumento.GetNumPalSinParada() + 1);
                                auto itIndice = indice.find((*itTokens));

                                if (itIndice == indice.end())					// Si el término no existe
                                {
                                    InformacionTermino informacionTerminoGlobal;
                                    InfTermDoc informacionTerminoDocumento;

                                    // Incrementa palabras diferentes
                                    informacionDocumento.SetNumPalDiferentes(informacionDocumento.GetNumPalDiferentes() + 1);
                                    informacionColeccionDocs.SetNumTotalPalDiferentes(informacionColeccionDocs.GetNumTotalPalDiferentes() + 1);

                                    // Inicializa término en documento
                                    informacionTerminoDocumento.SetFt(1);
                                    informacionTerminoDocumento.AddPos(posTerm);

                                    // Inserta el término como global
                                    // informacionTerminoGlobal.~InformacionTermino();
                                    informacionTerminoGlobal.SetFtc(1);
                                    informacionTerminoGlobal.AddInf(informacionDocumento.GetIdDoc(), informacionTerminoDocumento);
                                    string termino = *itTokens;
                                    indice.insert({termino, informacionTerminoGlobal});
                                }
                                else						// Si el término ya existe
                                {
                                    auto itLdocs = itIndice->second.GetL_docs_ref().find(informacionDocumento.GetIdDoc());

                                    if (itLdocs != itIndice->second.GetL_docs().end())  // Si ya está en el documento
                                    {
                                        // Incrementa frecuencia en el documento
                                        itLdocs->second.SetFt(itLdocs->second.GetFt() + 1);
                                        itLdocs->second.AddPos(posTerm);
                                    }
                                    else		// Si existe pero no en el documento actual
                                    {
                                        // Inserta un nuevo registro documento - InfoTermDoc
                                        informacionDocumento.SetNumPalDiferentes(informacionDocumento.GetNumPalDiferentes() + 1);
                                        InfTermDoc informacionTerminoDocumento;
                                        informacionTerminoDocumento.SetFt(1);
                                        informacionTerminoDocumento.AddPos(posTerm);
                                        itIndice->second.AddInf(informacionDocumento.GetIdDoc(), informacionTerminoDocumento);
                                    }

                                    // Incrementa frecuencia del término global
                                    itIndice->second.SetFtc(itIndice->second.GetFtc() + 1);
                                }
                            }

                            ++posTerm;	// Incrementamos posición del término
                        }

                        lineaDoc = "";
                    }

                    // Actualizamos la informacion de la coleccion de documentos
                    informacionColeccionDocs.SetNumTotalPal(informacionColeccionDocs.GetNumTotalPal() + informacionDocumento.GetNumPal());
                    informacionColeccionDocs.SetNumTotalPalSinParada(informacionColeccionDocs.GetNumTotalPalSinParada() + informacionDocumento.GetNumPalSinParada());
                    informacionColeccionDocs.SetTamBytes(informacionColeccionDocs.GetTamBytes() + informacionDocumento.GetTamBytes());
                    indiceDocs.insert({nomDoc, informacionDocumento});		// Añadimos el documento como indexado

                    documento.close();
                }
                else
                {
                    return false;
                }
            }
        }

        fichDocumentos.close();
    }
    else
    {
        return false;
    }

    return true;
}

bool IndexadorHash::IndexarDirectorio (const string& dirAIndexar)
{
    struct stat dir;
    int err = stat(dirAIndexar.c_str(), &dir);

    if (err == -1 || !S_ISDIR(dir.st_mode))
    {
        return false;
    }
    else
    {
        string cmd = "find " + dirAIndexar + " -follow -type f -not -name \"*.tk\" | sort > lista_fich_indexar";
        system(cmd.c_str());
        return Indexar("lista_fich_indexar");
    }
}

bool IndexadorHash::GuardarIndexacion () const
{
    struct stat statDir;

    stat(directorioIndice.c_str(), &statDir);

    // Comprueba si existe el directorio, si no lo crea
    if (stat(directorioIndice.c_str(), &statDir) != 0 || !S_ISDIR(statDir.st_mode))
    {
        string cmd = "mkdir -p " + directorioIndice;
        system(cmd.c_str());
    }

    string indexFile = directorioIndice + "/indices.txt";
    ofstream f(indexFile.c_str(), ofstream::out);

    if (f.good())
    {
        f << almacenarPosTerm << "\n";
        f << ficheroStopWords << "\n";

        for (auto itStopWords = stopWords.begin(); itStopWords != stopWords.end(); ++itStopWords)
        {
            f << (*itStopWords) << " ";
        }

        f << "\n";
        f << infPregunta.GetNumTotalPal() << "\n";
        f << infPregunta.GetNumTotalPalDiferentes() << "\n";
        f << infPregunta.GetNumTotalPalSinParada() << "\n";
        f << pregunta << "\n";
        f << indicePregunta.size() << "\n";		// Número de elementos de la pregunta

        // Escribe todos los InformacionTerminoPregunta
        for (auto itIndicePregunta = indicePregunta.begin(); itIndicePregunta != indicePregunta.end(); ++itIndicePregunta)
        {
            f << itIndicePregunta->first << "\n";
            f << itIndicePregunta->second.GetFt() << "\n";

            for (auto itPosTerm = itIndicePregunta->second.GetPosTerm_const().begin();
            itPosTerm != itIndicePregunta->second.GetPosTerm_const().end(); ++itPosTerm)
            {
                f << (*itPosTerm) << " ";
            }

            f << "\n";
        }

        f << informacionColeccionDocs.GetNumDocs() << "\n";
        f << informacionColeccionDocs.GetNumTotalPal() << "\n";
        f << informacionColeccionDocs.GetNumTotalPalDiferentes() << "\n";
        f << informacionColeccionDocs.GetNumTotalPalSinParada() << "\n";
        f << informacionColeccionDocs.GetTamBytes() << "\n";

        f << indice.size() << "\n";
        // Escribe índice --> colección de términos
        for (auto itIndice = indice.begin(); itIndice != indice.end(); ++itIndice)
        {
            f << itIndice->first << "\n";
            f << itIndice->second.GetFtc() << "\n";
            f << itIndice->second.GetL_docs().size() << "\n";

            // Escribimos l_docs --> InfTermDocs de cada termino
            for (auto itLdocs = itIndice->second.GetL_docs_const().begin();
            itLdocs != itIndice->second.GetL_docs_const().end(); ++itLdocs)
            {
                f << itLdocs->first << "\n";
                f << itLdocs->second.GetFt() << "\n";

                for (auto itPosTerm = itLdocs->second.GetPosTerm_const().begin(); itPosTerm != itLdocs->second.GetPosTerm_const().end(); ++itPosTerm)
                {
                    f << (*itPosTerm) << " ";
                }

                f << "\n";
            }
        }

        f << indiceDocs.size() << "\n";

        // Escribimos indiceDocs --> coleccion de documentos indexados
        for (auto itIndiceDocs = indiceDocs.begin(); itIndiceDocs != indiceDocs.end(); ++itIndiceDocs)
        {
            f << itIndiceDocs->first << "\n";
            f << itIndiceDocs->second.GetIdDoc() << "\n";
            f << itIndiceDocs->second.GetNumPal() << "\n";
            f << itIndiceDocs->second.GetNumPalDiferentes() << "\n";
            f << itIndiceDocs->second.GetNumPalSinParada() << "\n";
            f << itIndiceDocs->second.GetTamBytes() << "\n";

            f << itIndiceDocs->second.GetAnyo() << " ";
            f << itIndiceDocs->second.GetMes() << " ";
            f << itIndiceDocs->second.GetDia() << " ";
            f << itIndiceDocs->second.GetHora() << " ";
            f << itIndiceDocs->second.GetMin() << " ";
            f << itIndiceDocs->second.GetSeg() << "\n";
        }

        f << tok.CasosEspeciales() << "\n";
        f << tok.PasarAminuscSinAcentos() << "\n";
        f << tok.DelimitadoresPalabra() << "\n";

        f.close();
        return true;
    }

    return false;
}

bool IndexadorHash::RecuperarIndexacion (const string& directorioIndexacion)
{
    LimpiarIndexador();
    directorioIndice = directorioIndexacion;

    string indexFile = directorioIndice + "/indices.txt";
    ifstream f(indexFile.c_str(), ifstream::in);

    Tokenizador tokAux(" ", false, false);

    if (f.good())
    {
        string dato = "";
        getline(f, dato);
        almacenarPosTerm = atoi(dato.c_str());
        getline(f, ficheroStopWords);

        // Leer stop words
        getline(f, dato);
        list<string> tokens;
        tokAux.Tokenizar(dato, tokens);

        for (auto itStopWords = tokens.begin(); itStopWords != tokens.end(); ++itStopWords)
        {
            stopWords.insert((*itStopWords));
        }

        getline(f, dato);
        infPregunta.SetNumTotalPal(atoi(dato.c_str()));
        getline(f, dato);
        infPregunta.SetNumTotalPalDiferentes(atoi(dato.c_str()));
        getline(f, dato);
        infPregunta.SetNumTotalPalSinParada(atoi(dato.c_str()));
        getline(f, pregunta);

        // Indice pregunta
        getline(f, dato);			// Tamaño del indicePregunta
        for (int i = atoi(dato.c_str()); i != 0; i--)
        {
            string termino;
            InformacionTerminoPregunta infTermPreg;
            getline(f, termino);

            getline(f, dato);
            infTermPreg.SetFt(atoi(dato.c_str()));

            getline(f, dato);
            tokAux.Tokenizar(dato, tokens);
            for (auto itTokens = tokens.begin(); itTokens != tokens.end(); ++itTokens)
            {
                infTermPreg.AddPos(atoi((*itTokens).c_str()));
            }

            indicePregunta.insert({termino, infTermPreg});
        }

        getline(f, dato);
        informacionColeccionDocs.SetNumDocs(atoi(dato.c_str()));
        getline(f, dato);
        informacionColeccionDocs.SetNumTotalPal(atoi(dato.c_str()));
        getline(f, dato);
        informacionColeccionDocs.SetNumTotalPalDiferentes(atoi(dato.c_str()));
        getline(f, dato);
        informacionColeccionDocs.SetNumTotalPalSinParada(atoi(dato.c_str()));
        getline(f, dato);
        informacionColeccionDocs.SetTamBytes(atoi(dato.c_str()));

        getline(f, dato);
        for (int i = atoi(dato.c_str()); i != 0; i--)
        {
            string termino;
            getline(f, termino);

            InformacionTermino infoTermino;
            getline(f, dato);
            infoTermino.SetFtc(atoi(dato.c_str()));

            getline(f, dato);
            for (int j = atoi(dato.c_str()); j != 0; j--)
            {
                InfTermDoc infTermDoc;
                string idDoc;
                getline(f, idDoc);

                getline(f, dato);
                infTermDoc.SetFt(atoi(dato.c_str()));
                getline(f, dato);
                tokAux.Tokenizar(dato, tokens);
                for (auto itTokens = tokens.begin(); itTokens != tokens.end(); ++itTokens)
                {
                    infTermDoc.AddPos(atoi((*itTokens).c_str()));
                }

                // Añade registro a infoTermino.ldocs
                infoTermino.AddInf(atoi(idDoc.c_str()), infTermDoc);
            }

            indice.insert({termino, infoTermino});
        }

        getline(f, dato);
        for (int i = atoi(dato.c_str()); i != 0; i--)
        {
            InfDoc infDoc;
            string nomDoc;
            getline(f, nomDoc);

            getline(f, dato);
            infDoc.SetIdDoc(atoi(dato.c_str()));
            getline(f, dato);
            infDoc.SetNumPal(atoi(dato.c_str()));
            getline(f, dato);
            infDoc.SetNumPalDiferentes(atoi(dato.c_str()));
            getline(f, dato);
            infDoc.SetNumPalSinParada(atoi(dato.c_str()));
            getline(f, dato);
            infDoc.SetTamBytes(atoi(dato.c_str()));

            getline(f, dato);

            tokAux.Tokenizar(dato, tokens);
            auto itTokens = tokens.begin();

            infDoc.SetAnyo(atoi((*itTokens).c_str()));
            ++itTokens;
            infDoc.SetMes(atoi((*itTokens).c_str()));
            ++itTokens;
            infDoc.SetDia(atoi((*itTokens).c_str()));
            ++itTokens;
            infDoc.SetHora(atoi((*itTokens).c_str()));
            ++itTokens;
            infDoc.SetMin(atoi((*itTokens).c_str()));
            ++itTokens;
            infDoc.SetSeg(atoi((*itTokens).c_str()));

            indiceDocs.insert({nomDoc, infDoc});
        }

        getline(f, dato);
        tok.CasosEspeciales(atoi(dato.c_str()));
        getline(f, dato);
        tok.PasarAminuscSinAcentos(atoi(dato.c_str()));
        getline(f, dato);
        tok.DelimitadoresPalabra(dato);

        f.close();
    }
    else
    {
        return false;
    }

    return true;
}

bool IndexadorHash::IndexarPregunta (const string& preg)
{
    int posTerm = 0;
    indicePregunta.clear();
    infPregunta.SetNumTotalPal(0);
    infPregunta.SetNumTotalPalDiferentes(0);
    infPregunta.SetNumTotalPalSinParada(0);

    pregunta = preg;

    list<string> tokensPregunta;
    tok.Tokenizar(preg, tokensPregunta);

    infPregunta.SetNumTotalPal(tokensPregunta.size());

    if (infPregunta.GetNumTotalPal() == 0)
    {
        return false;
    }

    for (auto itTokens = tokensPregunta.begin(); itTokens != tokensPregunta.end(); ++itTokens)
    {
        stemmer.stemmer((*itTokens), tipoStemmer);
        auto itStopWords = stopWords.find((*itTokens));

        if (itStopWords == stopWords.end())	// No es una stop word
        {
            string palabra = *itTokens;
            auto itIndicePregunta = indicePregunta.find((*itTokens));

            if (itIndicePregunta != indicePregunta.end())   // El término ya está indexado en la pregunta
            {
                itIndicePregunta->second.SetFt(itIndicePregunta->second.GetFt() + 1);
                itIndicePregunta->second.AddPos(posTerm);
            }
            else    // El término no está indexado en la pregunta
            {
                InformacionTerminoPregunta infoTerminoPreg;
                infoTerminoPreg.SetFt(1);
                infoTerminoPreg.AddPos(posTerm);

                indicePregunta.insert({(*itTokens), infoTerminoPreg});

                infPregunta.SetNumTotalPalDiferentes(infPregunta.GetNumTotalPalDiferentes() + 1);
            }

            infPregunta.SetNumTotalPalSinParada(infPregunta.GetNumTotalPalSinParada() + 1);
        }

        ++posTerm;
    }

    if (infPregunta.GetNumTotalPalSinParada() == 0)
    {
        return false;
    }

    return true;
}

bool IndexadorHash::DevuelvePregunta (string& preg) const
{
    if (!pregunta.empty())
    {
        preg = pregunta;
        return true;
    }

    return false;
}

bool IndexadorHash::DevuelvePregunta (const string& word, InformacionTerminoPregunta& inf) const
{
    auto it = indicePregunta.find(ToLowerNoAccents(word));

    if (it != indicePregunta.end())
    {
        inf = it->second;
        return true;
    }

    return false;
}

bool IndexadorHash::DevuelvePregunta (InformacionPregunta& inf) const
{
    if (!pregunta.empty())
    {
        inf = infPregunta;
        return true;
    }

    return false;
}

bool IndexadorHash::Devuelve (const string& word, InformacionTermino& inf) const
{
    auto temp = indice.find(ToLowerNoAccents(word));

    if (temp != indice.end())
    {
        inf = temp->second;
        return true;
    }

    inf = InformacionTermino();
    return false;
}

bool IndexadorHash::Devuelve (const string& word, const string& nomDoc, InfTermDoc& InfDoc) const
{
    auto itIndiceDocs = indiceDocs.find(nomDoc);

    if (itIndiceDocs != indiceDocs.end())
    {
        auto itIndice = indice.find(word);
        if (itIndice != indice.end())
        {
            long int idDoc = itIndiceDocs->second.GetIdDoc();
            auto itLdocs = itIndice->second.GetL_docs_const().find(idDoc);

            if (itLdocs != itIndice->second.GetL_docs().end())
            {
                InfDoc = itLdocs->second;
                return true;
            }
        }
    }

    return false;
}

bool IndexadorHash::Existe (const string& word) const
{
    return (indice.find(word) != indice.end());
}

bool IndexadorHash::BorraDoc(const string& nomDoc)
{
    auto itIndiceDocs = indiceDocs.find(nomDoc);

    if (itIndiceDocs != indiceDocs.end())  // Si existe el documento
    {
        int idDoc = itIndiceDocs->second.GetIdDoc();

        // Usamos un enfoque diferente para recorrer y posiblemente eliminar elementos
        auto itIndice = indice.begin();
        while (itIndice != indice.end())
        {
            auto itLdocs = itIndice->second.GetL_docs_const().find(idDoc);
            if (itLdocs != itIndice->second.GetL_docs().end())
            {
                // Resta la frecuencia del término
                itIndice->second.SetFtc(itIndice->second.GetFtc() - itLdocs->second.GetFt());
                itIndice->second.DeleteInf(idDoc);

                // Si este término solo aparecía en nomDoc, se elimina del índice de términos
                if (itIndice->second.GetL_docs().empty())
                {
                    // Importante: erase() devuelve el siguiente iterador válido
                    itIndice = indice.erase(itIndice);
                    informacionColeccionDocs.SetNumTotalPalDiferentes(informacionColeccionDocs.GetNumTotalPalDiferentes() - 1);
                }
                else
                {
                    ++itIndice;
                }
            }
            else
            {
                ++itIndice;
            }
        }

        // Resta totales de la información de la colección
        informacionColeccionDocs.SetNumDocs(informacionColeccionDocs.GetNumDocs() - 1);
        informacionColeccionDocs.SetNumTotalPal(informacionColeccionDocs.GetNumTotalPal() - itIndiceDocs->second.GetNumPal());
        informacionColeccionDocs.SetNumTotalPalSinParada(informacionColeccionDocs.GetNumTotalPalSinParada() - itIndiceDocs->second.GetNumPalSinParada());
        informacionColeccionDocs.SetTamBytes(informacionColeccionDocs.GetTamBytes() - itIndiceDocs->second.GetTamBytes());

        indiceDocs.erase(itIndiceDocs);
        return true;
    }

    return false;
}

void IndexadorHash::VaciarIndiceDocs ()
{
    indice.clear();
    indiceDocs.clear();
    informacionColeccionDocs = InfColeccionDocs();
}

void IndexadorHash::VaciarIndicePreg ()
{
    pregunta = "";
    indicePregunta.clear();
    infPregunta = InformacionPregunta();
}

int IndexadorHash::NumPalIndexadas () const
{
    return indice.size();
}

string IndexadorHash::DevolverFichPalParada () const
{
    return ficheroStopWords;
}

void IndexadorHash::ListarPalParada () const
{
    for (auto it = stopWords.begin(); it != stopWords.end(); ++it)
    {
        cout << (*it) << "\n";
    }
}

int IndexadorHash::NumPalParada () const
{
    return stopWords.size();
}

string IndexadorHash::DevolverDelimitadores () const
{
    return tok.DelimitadoresPalabra();
}

bool IndexadorHash::DevolverCasosEspeciales () const
{
    return tok.CasosEspeciales();
}

bool IndexadorHash::DevolverPasarAminuscSinAcentos () const
{
    return tok.PasarAminuscSinAcentos();
}

bool IndexadorHash::DevolverAlmacenarPosTerm () const
{
    return almacenarPosTerm;
}

string IndexadorHash::DevolverDirIndice () const
{
    return directorioIndice;
}

int IndexadorHash::DevolverTipoStemming () const
{
    return tipoStemmer;
}

void IndexadorHash::ListarInfColeccDocs() const
{
    cout << informacionColeccionDocs << "\n";
}

void IndexadorHash::ListarTerminos () const
{
    for (auto it = indice.begin(); it != indice.end(); ++it)
    {
        cout << it->first << '\t' << it->second << "\n";
    }
}

bool IndexadorHash::ListarTerminos (const string& nomDoc) const
{
    auto itIndiceDocs = indiceDocs.find(nomDoc);

    if (itIndiceDocs != indiceDocs.end())	// Comprueba que exista el documento
    {
        // Recorre lista de terminos indexados
        for (auto itIndice = indice.begin(); itIndice != indice.end(); ++itIndice)
        {
            auto itLdocs = itIndice->second.GetL_docs_const().find(itIndiceDocs->second.GetIdDoc());
            if (itLdocs != itIndice->second.GetL_docs().end())
            {
                cout << itLdocs->first << '\t' << itIndice->second << "\n";
            }
        }

        return true;
    }

    return false;
}

void IndexadorHash::ListarDocs () const
{
    for (auto itIndiceDocs = indiceDocs.begin(); itIndiceDocs != indiceDocs.end(); ++itIndiceDocs)
    {
        cout << itIndiceDocs->first << '\t' << itIndiceDocs->second << endl;
    }
}

bool IndexadorHash::ListarDocs (const string& nomDoc) const
{
    auto itIndiceDocs = indiceDocs.find(nomDoc);

    if(itIndiceDocs != indiceDocs.end())
    {
        cout << itIndiceDocs->first << '\t' << itIndiceDocs->second << "\n";
        return true;
    }

    return false;
}


/****************************************** Añadidos para Buscador ******************************************/

bool IndexadorHash::getDocById(const long int &id, string &nombre, InfDoc &informacionDoc) const
{
    for (const auto& iD : indiceDocs)
    {
        if (iD.second.GetIdDoc() ==  id)
        {
            nombre = iD.first;
            informacionDoc = iD.second;

            return true;
        }
    }

    return false;
}

string IndexadorHash::GetPregunta () const 
{
    return pregunta;
}

unordered_map<string, InformacionTerminoPregunta> IndexadorHash::GetIndicePregunta() const
{
    return indicePregunta;
}

unordered_map<string, InfDoc> IndexadorHash::GetIndiceDocs () const
{
    return indiceDocs;
}

InformacionPregunta IndexadorHash::GetInfPregunta () const
{
    return infPregunta;
}

unordered_map<string, InformacionTermino> IndexadorHash::GetIndice () const
{
    return indice;
}

InfColeccionDocs IndexadorHash::GetInformacionColeccionDocs () const
{
    return informacionColeccionDocs;
}
