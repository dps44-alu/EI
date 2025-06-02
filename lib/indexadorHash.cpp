#include "indexadorHash.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <sys/stat.h>
#include <unistd.h>    
#include <cstdlib>      
#include <algorithm>
#include <stdexcept>   
#include <cctype>   
#include <filesystem>   

using namespace std;

 
IndexadorHash::IndexadorHash(const string& fichStopWords, const string& delimitadores,
                             const bool& detectComp, const bool& minuscSinAcentos,
                             const string& dirIndice, const int& tStemmer,
                             const bool& almPosTerm)

    : tok(delimitadores, detectComp, minuscSinAcentos),
      tipoStemmer(tStemmer),
      almacenarPosTerm(almPosTerm)
{
    ofstream dummyFile("./corpus_corto/dummy.tk");
    dummyFile.close();

    if(dirIndice.empty()) {
        char* cwd = get_current_dir_name();
        if (cwd) {
            directorioIndice = cwd;
            free(cwd);
        } else {
            cerr << "AVISO: No se pudo obtener el directorio actual. Usando '.'." << endl;
            directorioIndice = ".";
        }
    } else {
        directorioIndice = dirIndice;
    }
    

    this->ficheroStopWords = fichStopWords;
    ifstream fStopWords(fichStopWords);
    if(fStopWords.is_open()) {
        string stopWord_original;
        while(fStopWords >> stopWord_original) {
            string sw_procesada = stopWord_original;  
            procesarPalabra(sw_procesada);  
            if (!sw_procesada.empty()) {
 
                 stopWordsMap[sw_procesada] = stopWord_original;
            }
        }
        fStopWords.close();
    } else {
        cerr << "AVISO: No existe o no se pudo abrir el archivo de palabras de parada: " << fichStopWords << endl;
    }
}
 
IndexadorHash::IndexadorHash(const string& directorioIndexacion) {
 
    if (!RecuperarIndexacion(directorioIndexacion)) {
 
        stopWordsMap.clear();  
        indice.clear();
        indiceDocs.clear();
        informacionColeccionDocs = InfColeccionDocs(); 
        pregunta = "";
        indicePregunta.clear();
        infPregunta = InformacionPregunta();
    }
}

// --- Constructor de Copia ---
IndexadorHash::IndexadorHash(const IndexadorHash& other) :
    directorioIndice(other.directorioIndice),
    indice(other.indice),  
    indiceDocs(other.indiceDocs),
    informacionColeccionDocs(other.informacionColeccionDocs),
    pregunta(other.pregunta),
    indicePregunta(other.indicePregunta),
    infPregunta(other.infPregunta),
    ficheroStopWords(other.ficheroStopWords),
    stopWordsMap(other.stopWordsMap),  
    tok(other.tok), 
    stemmer(other.stemmer),  
    tipoStemmer(other.tipoStemmer),
    almacenarPosTerm(other.almacenarPosTerm)
{}

 
IndexadorHash::~IndexadorHash() {
 
}

 
IndexadorHash& IndexadorHash::operator=(const IndexadorHash& other) {
    if (this != &other) {
        directorioIndice = other.directorioIndice;
        indice = other.indice;  
        indiceDocs = other.indiceDocs;
        informacionColeccionDocs = other.informacionColeccionDocs;
        pregunta = other.pregunta;
        indicePregunta = other.indicePregunta;
        infPregunta = other.infPregunta;
        ficheroStopWords = other.ficheroStopWords;
        stopWordsMap = other.stopWordsMap;  
        tok = other.tok; 
        stemmer = other.stemmer;  
        tipoStemmer = other.tipoStemmer;
        almacenarPosTerm = other.almacenarPosTerm;
    }
    return *this;
}

 
void IndexadorHash::procesarPalabra(string& palabra) const {
    if (tok.PasarAminuscSinAcentos()) {  
 
        string temp;
        for (char c_char : palabra) {
            unsigned char c = static_cast<unsigned char>(c_char);
            unsigned char lower_c = tolower(c);
            // Quitar acentos (simplificado) - completar mapeo si es necesario
            if (lower_c >= 224 && lower_c <= 229) lower_c = 'a'; // áàâäåæ
            else if (lower_c >= 232 && lower_c <= 235) lower_c = 'e'; // éèêë
            else if (lower_c >= 236 && lower_c <= 239) lower_c = 'i'; // íìîï
            else if (lower_c >= 242 && lower_c <= 246) lower_c = 'o'; // óòôöø
            else if (lower_c >= 249 && lower_c <= 252) lower_c = 'u'; // úùûü
            else if (lower_c == 241) lower_c = 'n'; // ñ
            else if (lower_c == 231) lower_c = 'c'; // ç
            temp += lower_c;
        }
        palabra = temp;
    }

    if (tipoStemmer > 0) {
        stemmerPorter stemmer_temporal = stemmer; // Copia temporal
        stemmer_temporal.stemmer(palabra, tipoStemmer);
    }
}


 
bool IndexadorHash::Indexar(const string& ficheroDocumentos) {
    ifstream fDocs(ficheroDocumentos);
    if (!fDocs.is_open()) {
        cerr << "ERROR: No existe o no se pudo abrir el archivo: " << ficheroDocumentos << endl;
        return false;
    }

    string nomDoc;
    bool alguna_indexacion_ok = false;

    while (getline(fDocs, nomDoc)) {
        if (nomDoc.empty()) continue;

        struct stat statDoc;
        bool existe_fichero = (stat(nomDoc.c_str(), &statDoc) == 0);
        bool indexar_doc = false;
        long int idDocAntiguo = 0;
        bool reindexando = false;

        auto itDoc = indiceDocs.find(nomDoc);

        if (itDoc != indiceDocs.end()) {
            if (!existe_fichero) {
                 cerr << "AVISO: El documento '" << nomDoc << "' estaba indexado pero ya no existe. Se eliminará del índice." << endl;
                 if (!BorraDoc(nomDoc)) {
                      cerr << "ERROR: No se pudo borrar la información del documento inexistente '" << nomDoc << "'." << endl;
                 }
                 continue;
            }
            Fecha fechaModActual(gmtime(&(statDoc.st_mtime)));
            if (itDoc->second.fechaModificacion < fechaModActual) {
                cout << "AVISO: Documento '" << nomDoc << "' modificado. Reindexando." << endl;
                idDocAntiguo = itDoc->second.idDoc;
                reindexando = true;
                if (!BorraDoc(nomDoc)) {
                    cerr << "ERROR: No se pudo borrar el documento '" << nomDoc << "' para reindexarlo." << endl;
                    continue;
                }
                indexar_doc = true;
            } else {
                cout << "AVISO: Documento '" << nomDoc << "' ya indexado y sin modificaciones." << endl;
                indexar_doc = false;
            }
        } else {
            if (!existe_fichero) {
                cerr << "AVISO: El documento '" << nomDoc << "' listado no existe." << endl;
                indexar_doc = false;
            } else {
                indexar_doc = true;
            }
        }

        if (indexar_doc) {
            ifstream docStream(nomDoc);
            if (!docStream.is_open()) {
                 cerr << "ERROR: No se pudo abrir el documento '" << nomDoc << "' para lectura." << endl;
                 continue;
            }

             InfDoc infoDocNuevo;
             infoDocNuevo.idDoc = (reindexando && idDocAntiguo != 0) ? idDocAntiguo : ++informacionColeccionDocs.numDocs;
             infoDocNuevo.tamBytes = statDoc.st_size;
             infoDocNuevo.fechaModificacion = Fecha(gmtime(&(statDoc.st_mtime)));
             infoDocNuevo.numPal = 0;
             infoDocNuevo.numPalSinParada = 0;
             infoDocNuevo.numPalDiferentes = 0;

             string linea;
             list<string> tokens;
             long int posTermAbsoluta = 0;
             unordered_set<string> palabrasDiferentesDoc;  

             while (getline(docStream, linea)) {
                 tok.Tokenizar(linea, tokens);
                 infoDocNuevo.numPal += tokens.size();

                 for (string& token : tokens) {
                     string tokenProcesado = token;
                     procesarPalabra(tokenProcesado);  

 
                     if (!tokenProcesado.empty() && stopWordsMap.count(tokenProcesado) == 0) {
                         infoDocNuevo.numPalSinParada++;
 
                         if (palabrasDiferentesDoc.insert(tokenProcesado).second) {
                              infoDocNuevo.numPalDiferentes++;
 
                              if (indice.find(tokenProcesado) == indice.end()) {
                                  informacionColeccionDocs.numTotalPalDiferentes++;
                              }
                         }

 
                         auto itTermino = indice.find(tokenProcesado);
                         if (itTermino == indice.end()) {  
                             InformacionTermino infoTerminoNuevo;
                             infoTerminoNuevo.ftc = 1;
                             InfTermDoc infTermDocNuevo;
                             infTermDocNuevo.ft = 1;
                             if (almacenarPosTerm) {
                                 infTermDocNuevo.posTerm.push_back(posTermAbsoluta);
                             }
                             infoTerminoNuevo.l_docs[infoDocNuevo.idDoc] = infTermDocNuevo;
                             indice[tokenProcesado] = infoTerminoNuevo;  
                         } else {  
                             itTermino->second.ftc++;
                             auto itLdoc = itTermino->second.l_docs.find(infoDocNuevo.idDoc);
                             if (itLdoc == itTermino->second.l_docs.end()) {  
                                 InfTermDoc infTermDocNuevo;
                                 infTermDocNuevo.ft = 1;
                                 if (almacenarPosTerm) {
                                     infTermDocNuevo.posTerm.push_back(posTermAbsoluta);
                                 }
                                 itTermino->second.l_docs[infoDocNuevo.idDoc] = infTermDocNuevo;
                             } else {  
                                 itLdoc->second.ft++;
                                 if (almacenarPosTerm) {
                                     itLdoc->second.posTerm.push_back(posTermAbsoluta);
                                 }
                             }
                         }
                     }
                     posTermAbsoluta++;  
                 }
             }
             docStream.close();

 
             informacionColeccionDocs.numTotalPal += infoDocNuevo.numPal;
             informacionColeccionDocs.numTotalPalSinParada += infoDocNuevo.numPalSinParada;
             informacionColeccionDocs.tamBytes += infoDocNuevo.tamBytes;
 
             indiceDocs[nomDoc] = infoDocNuevo;
             alguna_indexacion_ok = true;  

        }  

    }  

    fDocs.close();
    return true;  
}


bool IndexadorHash::IndexarDirectorio(const string& dirAIndexar) {
    struct stat st;
    if (stat(dirAIndexar.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
 
         return false;
    }
 
    string temp_list_file = directorioIndice + "/~lista_fich_indexar.tmp";
 
    string cmd = "find \"" + dirAIndexar + "\" -follow -type f | sort > " + temp_list_file + " 2>/dev/null";
    int ret = system(cmd.c_str());
 
    if (ret != 0) {
 
         remove(temp_list_file.c_str());
         return false;
    }
 
    bool resultado = Indexar(temp_list_file);
    remove(temp_list_file.c_str());
    return resultado;
}



 
bool IndexadorHash::GuardarIndexacion() const {
 
    struct stat st;
    if (stat(directorioIndice.c_str(), &st) != 0) {
        string cmd = "mkdir -p \"" + directorioIndice + "\"";
        if (system(cmd.c_str()) != 0) {
            cerr << "ERROR: No se pudo crear el directorio de índice: " << directorioIndice << endl;
            return false;
        }
    } else if (!S_ISDIR(st.st_mode)) {
        cerr << "ERROR: La ruta del índice '" << directorioIndice << "' existe pero no es un directorio." << endl;
        return false;
    }

    string fIndexPath = directorioIndice + "/" + ficheroIndice;
    ofstream fOut(fIndexPath);
    if (!fOut.is_open()) {
        cerr << "ERROR: No se pudo crear o abrir el fichero para guardar la indexación: " << fIndexPath << endl;
        return false;
    }


    fOut << "[CONFIG]" << endl;
    fOut << "ficheroStopWords=" << ficheroStopWords << endl;
    fOut << "delimitadores=" << tok.DelimitadoresPalabra() << endl; // Usa getter const
    fOut << "detectComp=" << tok.CasosEspeciales() << endl;       // Usa getter const
    fOut << "minuscSinAcentos=" << tok.PasarAminuscSinAcentos() << endl; // Usa getter const
    fOut << "tipoStemmer=" << tipoStemmer << endl;
    fOut << "almacenarPosTerm=" << almacenarPosTerm << endl;
    fOut << "[END_CONFIG]" << endl;

    // Guardar Stopwords (guardar ambas: procesada y original)
    fOut << "[STOPWORDS_MAP]" << endl;
    fOut << stopWordsMap.size() << endl;
    for(const auto& pair : stopWordsMap) {
        fOut << pair.first << "\t" << pair.second << endl; // procesada <TAB> original
    }
    fOut << "[END_STOPWORDS_MAP]" << endl;

    fOut << "[INF_COLECCION]" << endl;
    fOut << informacionColeccionDocs.numDocs << endl;
    fOut << informacionColeccionDocs.numTotalPal << endl;
    fOut << informacionColeccionDocs.numTotalPalSinParada << endl;
    fOut << informacionColeccionDocs.numTotalPalDiferentes << endl;
    fOut << informacionColeccionDocs.tamBytes << endl;
    fOut << "[END_INF_COLECCION]" << endl;

    fOut << "[INDICE_DOCS]" << endl;
    fOut << indiceDocs.size() << endl;
    for(const auto& pair : indiceDocs) {
        const string& nomDoc = pair.first; const InfDoc& infDoc = pair.second;
        fOut << nomDoc << endl; // Asumir que nomDoc no tiene \n
        fOut << infDoc.idDoc << endl; fOut << infDoc.numPal << endl;
        fOut << infDoc.numPalSinParada << endl; fOut << infDoc.numPalDiferentes << endl;
        fOut << infDoc.tamBytes << endl;
        fOut << infDoc.fechaModificacion.dia << " " << infDoc.fechaModificacion.mes << " " << infDoc.fechaModificacion.anyo << " "
             << infDoc.fechaModificacion.hora << " " << infDoc.fechaModificacion.min << " " << infDoc.fechaModificacion.seg << endl;
    }
    fOut << "[END_INDICE_DOCS]" << endl;

    fOut << "[INDICE_TERMINOS]" << endl;
    fOut << indice.size() << endl;
    for(const auto& pairTerm : indice) {
        const string& termino = pairTerm.first; const InformacionTermino& infTerm = pairTerm.second;
        fOut << termino << endl; // Asumir que termino no tiene \n
        fOut << infTerm.ftc << endl; fOut << infTerm.l_docs.size() << endl;
        for(const auto& pairDoc : infTerm.l_docs) {
            long int idDoc = pairDoc.first; const InfTermDoc& infTermDoc = pairDoc.second;
            fOut << idDoc << endl; fOut << infTermDoc.ft << endl;
            // Guardar posiciones
            fOut << infTermDoc.posTerm.size(); // Guardar siempre el tamaño
            if (almacenarPosTerm) { // Solo guardar posiciones si está activo
                for(int pos : infTermDoc.posTerm) fOut << " " << pos;
            }
            fOut << endl; // Salto de línea después de tamaño o posiciones
        }
    }
    fOut << "[END_INDICE_TERMINOS]" << endl;

    fOut << "[PREGUNTA]" << endl;
    fOut << pregunta << endl; // Asumir sin \n
    fOut << infPregunta.numTotalPal << endl; fOut << infPregunta.numTotalPalSinParada << endl;
    fOut << infPregunta.numTotalPalDiferentes << endl;
    fOut << "[END_PREGUNTA]" << endl;

    fOut << "[INDICE_PREGUNTA]" << endl;
    fOut << indicePregunta.size() << endl;
    for(const auto& pairPreg : indicePregunta) {
        const string& termPreg = pairPreg.first; const InformacionTerminoPregunta& infTermPreg = pairPreg.second;
        fOut << termPreg << endl; // Asumir sin \n
        fOut << infTermPreg.ft << endl;
        // Guardar posiciones
        fOut << infTermPreg.posTerm.size(); // Guardar siempre tamaño
        if (almacenarPosTerm) { // Solo guardar posiciones si está activo
            for(int pos : infTermPreg.posTerm) fOut << " " << pos;
        }
        fOut << endl; // Salto de línea
    }
    fOut << "[END_INDICE_PREGUNTA]" << endl;

    fOut.close();
    if (!fOut.good()) {
         cerr << "ERROR: Ocurrió un error durante la escritura del fichero de índice: " << fIndexPath << endl;
         return false;
    }
    return true;
}

// --- Recuperar Indexación (Versión MÁS ROBUSTA con stringstream) ---
bool IndexadorHash::RecuperarIndexacion(const std::string& dirIndiceRec) {
    // Limpiar estado actual (igual que antes)
    directorioIndice = dirIndiceRec;
    indice.clear(); indiceDocs.clear(); informacionColeccionDocs = InfColeccionDocs();
    pregunta = ""; indicePregunta.clear(); infPregunta = InformacionPregunta();
    stopWordsMap.clear();

    string fIndexPath = dirIndiceRec + "/" + ficheroIndice;
    ifstream fIn(fIndexPath);
    if (!fIn.is_open()) {
        cerr << "ERROR: No se pudo abrir el fichero para recuperar la indexación: " << fIndexPath << endl;
        return false;
    }

    string linea;
    bool configOk = false, stopwordsOk = false, infColOk = false, indiceDocsOk = false, indiceTermsOk = false, preguntaOk = false, indicePregOk = false;
    string seccion_actual = "";
    int line_num = 0;

    // Función auxiliar local para parsear números de forma segura desde una línea
    auto parse_num = [&](const std::string& l, int l_num, const std::string& ctx) {
        if (l.empty()) throw std::runtime_error("Línea vacía para " + ctx + " en línea " + to_string(l_num));
        stringstream ss(l);
        long long val_ll; // Leer como long long para detectar overflow de int/long
        ss >> val_ll;
        // Comprobar fallo de extracción O si quedaron caracteres extra
        if (ss.fail() || !ss.eof()) {
            throw std::runtime_error("Formato inválido para " + ctx + " en línea " + to_string(l_num) + ": '" + l + "'");
        }
        return val_ll; // Devolver long long, se convertirá implícitamente si es necesario
    };

    try {
        while (getline(fIn, linea)) {
            //cerr << "DEBUG L" << line_num << ": [" << linea << "]" << endl;
             line_num++;
            // Trim leading/trailing whitespace (puede ayudar con líneas casi vacías)
            linea.erase(0, linea.find_first_not_of(" \t\n\r\f\v"));
            linea.erase(linea.find_last_not_of(" \t\n\r\f\v") + 1);

            if (linea.empty() && !fIn.eof() && seccion_actual != "[PREGUNTA]" && seccion_actual != "[INDICE_PREGUNTA]") continue;

            if (!linea.empty() && linea[0] == '[' && linea.back() == ']') {
                // ... (Manejo de secciones igual que antes) ...
                if (linea.find("[END_") == 0) {
                    if (seccion_actual == "[CONFIG]") configOk = true;
                    else if (seccion_actual == "[STOPWORDS_MAP]") stopwordsOk = true;
                    else if (seccion_actual == "[INF_COLECCION]") infColOk = true;
                    else if (seccion_actual == "[INDICE_DOCS]") indiceDocsOk = true;
                    else if (seccion_actual == "[INDICE_TERMINOS]") indiceTermsOk = true;
                    else if (seccion_actual == "[PREGUNTA]") preguntaOk = true;
                    else if (seccion_actual == "[INDICE_PREGUNTA]") indicePregOk = true;
                    seccion_actual = "";
                } else {
                    seccion_actual = linea;
                }
                continue; // Pasar a la siguiente línea
            }

            // --- Procesar datos dentro de la sección ---
            if (seccion_actual == "[CONFIG]") {
                 size_t eqPos = linea.find('=');
                 if (eqPos != string::npos) {
                     string key = linea.substr(0, eqPos); string value = linea.substr(eqPos + 1);
                     if (key == "ficheroStopWords") ficheroStopWords = value;
                     else if (key == "delimitadores") tok.DelimitadoresPalabra(value);
                     else if (key == "detectComp") tok.CasosEspeciales(value == "1");
                     else if (key == "minuscSinAcentos") tok.PasarAminuscSinAcentos(value == "1");
                     // Usar parse_num para leer enteros de configuración
                     else if (key == "tipoStemmer") tipoStemmer = static_cast<int>(parse_num(value, line_num, "tipoStemmer"));
                     else if (key == "almacenarPosTerm") almacenarPosTerm = (value == "1");
                 }
            } else if (seccion_actual == "[STOPWORDS_MAP]") {
                 int count = static_cast<int>(parse_num(linea, line_num, "stopwords count"));
                 stopWordsMap.clear(); stopWordsMap.reserve(count);
                 for (int i = 0; i < count; ++i) {
                     if (!getline(fIn, linea)) throw runtime_error("Lectura incompleta de stopwords_map en línea " + to_string(line_num + 1)); line_num++;
                     size_t tabPos = linea.find('\t');
                     if (tabPos == string::npos) throw runtime_error("Formato inválido en stopwords_map (falta TAB) en línea " + to_string(line_num) + ": " + linea);
                     string proc = linea.substr(0, tabPos);
                     string orig = linea.substr(tabPos + 1);
                     stopWordsMap[proc] = orig;
                 }
            } else if (seccion_actual == "[INF_COLECCION]") {
                 informacionColeccionDocs.numDocs = parse_num(linea, line_num, "numDocs");
                 if(!getline(fIn, linea)) throw runtime_error("Lectura incompleta inf_col(numTotalPal)"); line_num++;
                 informacionColeccionDocs.numTotalPal = parse_num(linea, line_num, "numTotalPal");
                 if(!getline(fIn, linea)) throw runtime_error("Lectura incompleta inf_col(numTotalPalSP)"); line_num++;
                 informacionColeccionDocs.numTotalPalSinParada = parse_num(linea, line_num, "numTotalPalSP");
                 if(!getline(fIn, linea)) throw runtime_error("Lectura incompleta inf_col(numTotalPalDif)"); line_num++;
                 informacionColeccionDocs.numTotalPalDiferentes = parse_num(linea, line_num, "numTotalPalDif");
                 if(!getline(fIn, linea)) throw runtime_error("Lectura incompleta inf_col(tamBytes)"); line_num++;
                 informacionColeccionDocs.tamBytes = parse_num(linea, line_num, "tamBytes");
            } else if (seccion_actual == "[INDICE_DOCS]") {
                  int count = static_cast<int>(parse_num(linea, line_num, "indiceDocs count"));
                  indiceDocs.clear(); indiceDocs.reserve(count);
                  for (int i = 0; i < count; ++i) {
                      string nomDoc_r; InfDoc infDoc_r;
                      if (!getline(fIn, nomDoc_r)) throw runtime_error("Fallo lectura nomDoc en línea " + to_string(line_num + 1)); line_num++;
                      if (!getline(fIn, linea)) throw runtime_error("Fallo lectura idDoc"); line_num++; infDoc_r.idDoc = parse_num(linea, line_num, "idDoc");
                      if (!getline(fIn, linea)) throw runtime_error("Fallo lectura numPal"); line_num++; infDoc_r.numPal = static_cast<int>(parse_num(linea, line_num, "numPal"));
                      if (!getline(fIn, linea)) throw runtime_error("Fallo lectura numPalSP"); line_num++; infDoc_r.numPalSinParada = static_cast<int>(parse_num(linea, line_num, "numPalSP"));
                      if (!getline(fIn, linea)) throw runtime_error("Fallo lectura numPalDif"); line_num++; infDoc_r.numPalDiferentes = static_cast<int>(parse_num(linea, line_num, "numPalDif"));
                      if (!getline(fIn, linea)) throw runtime_error("Fallo lectura tamBytes"); line_num++; infDoc_r.tamBytes = parse_num(linea, line_num, "tamBytes");
                      if (!getline(fIn, linea)) throw runtime_error("Fallo lectura Fecha"); line_num++;
                      stringstream ssFecha(linea); // stringstream para fecha está bien
                      ssFecha >> infDoc_r.fechaModificacion.dia >> infDoc_r.fechaModificacion.mes >> infDoc_r.fechaModificacion.anyo
                              >> infDoc_r.fechaModificacion.hora >> infDoc_r.fechaModificacion.min >> infDoc_r.fechaModificacion.seg;
                      if (ssFecha.fail() || !ssFecha.eof()) throw runtime_error("Fallo parseo Fecha en línea " + to_string(line_num) + ": " + linea);
                      indiceDocs[nomDoc_r] = infDoc_r;
                  }
            } else if (seccion_actual == "[INDICE_TERMINOS]") {
                 int countTerm = static_cast<int>(parse_num(linea, line_num, "indiceTerminos count"));
                 indice.clear(); indice.reserve(countTerm);
                 for(int i=0; i < countTerm; ++i) {
                     string termino_r; InformacionTermino infTerm_r;
                     if(!getline(fIn, termino_r)) throw runtime_error("Fallo lectura termino"); line_num++;
                     if(!getline(fIn, linea)) throw runtime_error("Fallo lectura ftc"); line_num++; infTerm_r.ftc = static_cast<int>(parse_num(linea, line_num, "ftc"));
                     if(!getline(fIn, linea)) throw runtime_error("Fallo lectura countDocsTerm"); line_num++; int countDocsTerm = static_cast<int>(parse_num(linea, line_num, "countDocsTerm"));
                     infTerm_r.l_docs.reserve(countDocsTerm);
                     for (int j = 0; j < countDocsTerm; ++j) {
                         long int idDoc_r; InfTermDoc infTermDoc_r;
                         if(!getline(fIn, linea)) throw runtime_error("Fallo lectura idDoc (term)"); line_num++; idDoc_r = parse_num(linea, line_num, "idDoc(term)");
                         if(!getline(fIn, linea)) throw runtime_error("Fallo lectura ft (term)"); line_num++; infTermDoc_r.ft = static_cast<int>(parse_num(linea, line_num, "ft(term)"));
                         if(!getline(fIn, linea)) throw runtime_error("Fallo lectura pos line (term)"); line_num++;
                         stringstream ssPos(linea);  
                         int countPos; ssPos >> countPos;
                         if (ssPos.fail()) throw runtime_error("Fallo parseo countPos (term) en línea " + to_string(line_num) + ": " + linea);
                         if (almacenarPosTerm && countPos > 0) {
                             int pos;
                             while(ssPos >> pos) infTermDoc_r.posTerm.push_back(pos);
                             if (ssPos.fail() && !ssPos.eof()) throw runtime_error("Fallo parseo pos (term) en línea " + to_string(line_num) + ": " + linea);
                         } else if (countPos < 0) {
                             throw runtime_error("Número inválido de posiciones (term) en línea " + to_string(line_num) + ": " + linea);
                         }
                         infTerm_r.l_docs[idDoc_r] = infTermDoc_r;
                     }
                     indice[termino_r] = infTerm_r;
                 }
                } else if (seccion_actual == "[PREGUNTA]") {
 
                    pregunta = linea;

 
                    string line_num1, line_num2, line_num3;
                    if(!getline(fIn, line_num1)) throw runtime_error("Fallo lectura numTotalPal (preg)"); line_num++;
                    if(!getline(fIn, line_num2)) throw runtime_error("Fallo lectura numTotalPalSP (preg)"); line_num++;
                    if(!getline(fIn, line_num3)) throw runtime_error("Fallo lectura numTotalPalDif (preg)"); line_num++;
 
                    infPregunta.numTotalPal = static_cast<int>(parse_num(line_num1, line_num - 2, "numTotalPal(preg)"));
                    infPregunta.numTotalPalSinParada = static_cast<int>(parse_num(line_num2, line_num - 1, "numTotalPalSP(preg)"));
                    infPregunta.numTotalPalDiferentes = static_cast<int>(parse_num(line_num3, line_num, "numTotalPalDif(preg)"));
 
               } else if (seccion_actual == "[INDICE_PREGUNTA]") { // Asegúrate que el resto de la función sigue igual
                    // ... (resto de la lógica de RecuperarIndexacion) ...
                    int countPreg = static_cast<int>(parse_num(linea, line_num, "indicePregunta count"));
                    indicePregunta.clear(); indicePregunta.reserve(countPreg);
                    for (int i=0; i < countPreg; ++i) {
                        string termPreg_r; InformacionTerminoPregunta infTermPreg_r;
                        if(!getline(fIn, termPreg_r)) throw runtime_error("Fallo lectura term (preg)"); line_num++;
                        if(!getline(fIn, linea)) throw runtime_error("Fallo lectura ft (preg)"); line_num++; infTermPreg_r.ft = static_cast<int>(parse_num(linea, line_num, "ft(preg)"));
                        if(!getline(fIn, linea)) throw runtime_error("Fallo lectura pos line (preg)"); line_num++;
                        stringstream ssPosPreg(linea); // stringstream para posiciones está bien
                        int countPosPreg; ssPosPreg >> countPosPreg;
                        if (ssPosPreg.fail()) throw runtime_error("Fallo parseo countPos (preg) en línea " + to_string(line_num) + ": " + linea);
                        if (almacenarPosTerm && countPosPreg > 0) {
                            int posPreg;
                            while (ssPosPreg >> posPreg) infTermPreg_r.posTerm.push_back(posPreg);
                            if (ssPosPreg.fail() && !ssPosPreg.eof()) throw runtime_error("Fallo parseo pos (preg) en línea " + to_string(line_num) + ": " + linea);
                        } else if (countPosPreg < 0) {
                            throw runtime_error("Número inválido de posiciones (preg) en línea " + to_string(line_num) + ": " + linea);
                        }
                        indicePregunta[termPreg_r] = infTermPreg_r;
                 }
            }
        }  

    } catch (const std::exception& e) {
 
        cerr << "ERROR: Excepción durante la recuperación del índice (línea ~" << line_num << "): " << e.what() << endl;
        fIn.close();
        VaciarIndiceDocs(); VaciarIndicePreg(); indice.clear(); stopWordsMap.clear(); // Limpiar mapa
        return false;
    }

    fIn.close();

    // Comprobar si se cargaron todas las secciones
    if (!(configOk && stopwordsOk && infColOk && indiceDocsOk && indiceTermsOk && preguntaOk && indicePregOk)) {
         cerr << "ERROR: El formato del fichero de índice '" << fIndexPath << "' es inválido o faltan secciones." << endl;

         cerr << "Flags: Cfg="<<configOk<<" SW="<<stopwordsOk<<" InfC="<<infColOk<<" IDocs="<<indiceDocsOk<<" ITerms="<<indiceTermsOk<<" Preg="<<preguntaOk<<" IPreg="<<indicePregOk << endl;
         VaciarIndiceDocs(); VaciarIndicePreg(); indice.clear(); stopWordsMap.clear(); // Limpiar mapa
         return false;
    }

    return true;
}


// --- ImprimirIndexacionPregunta (const) ---
void IndexadorHash::ImprimirIndexacionPregunta() const {
     cout << "Pregunta indexada: " << pregunta << endl;
     cout << "Informacion de la pregunta: " << infPregunta << endl;
     cout << "Terminos indexados en la pregunta (" << indicePregunta.size() << "):" << endl;
     for (const auto& pair : indicePregunta) {
         cout << pair.first << "\t" << pair.second << endl;
     }
}

// --- ImprimirPregunta (const) ---
void IndexadorHash::ImprimirPregunta() const {
    cout << "Pregunta indexada: " << pregunta << endl;
    cout << "Informacion de la pregunta: " << infPregunta << endl;
}

// --- IndexarPregunta ---
bool IndexadorHash::IndexarPregunta(const string& preg) {
    VaciarIndicePreg();
    if (preg.empty()) {
        cerr << "ERROR: La pregunta a indexar está vacía." << endl;
        return false;
    }
    this->pregunta = preg;
    list<string> tokens;
    tok.Tokenizar(preg, tokens);
    infPregunta.numTotalPal = tokens.size();
    if (infPregunta.numTotalPal == 0) {
         cerr << "ERROR: La pregunta no contiene tokens válidos." << endl;
         return false;
    }
    int posTermPreg = 0;
    unordered_set<string> palabrasDiferentesPreg;
    for (string& token : tokens) {
        string tokenProcesado = token;
        procesarPalabra(tokenProcesado);
        // Comprobar stopword usando mapa
        if (!tokenProcesado.empty() && stopWordsMap.count(tokenProcesado) == 0) {
             infPregunta.numTotalPalSinParada++;
             if (palabrasDiferentesPreg.insert(tokenProcesado).second) {
                 infPregunta.numTotalPalDiferentes++;
             }
             auto itPreg = indicePregunta.find(tokenProcesado);
             if (itPreg == indicePregunta.end()) {
                 InformacionTerminoPregunta infoNuevo; infoNuevo.ft = 1;
                 if (almacenarPosTerm) infoNuevo.posTerm.push_back(posTermPreg);
                 indicePregunta[tokenProcesado] = infoNuevo;
             } else {
                 itPreg->second.ft++;
                 if (almacenarPosTerm) itPreg->second.posTerm.push_back(posTermPreg);
             }
        }
        posTermPreg++;
    }
    if (infPregunta.numTotalPalSinParada == 0) {
         cerr << "ERROR: La pregunta solo contiene palabras de parada o tokens vacíos." << endl;
         VaciarIndicePreg();
         return false;
    }
    return true;
}

// --- DevuelvePregunta (string) ---
bool IndexadorHash::DevuelvePregunta(string& preg) const {
    if (pregunta.empty() && infPregunta.numTotalPal == 0) return false;
    preg = pregunta;
    return true;
}

// --- DevuelvePregunta (Info Término) ---
bool IndexadorHash::DevuelvePregunta(const string& word, InformacionTerminoPregunta& inf) const {
    string wordProcesada = word;
    procesarPalabra(wordProcesada);
    auto it = indicePregunta.find(wordProcesada);
    if (it != indicePregunta.end()) {
        inf = it->second; return true;
    }
    return false;
}

// --- Devolver Pregunta (Info General) ---
bool IndexadorHash::DevuelvePregunta(InformacionPregunta& inf) const {
     if (pregunta.empty() && infPregunta.numTotalPal == 0) return false;
    inf = infPregunta; return true;
}

// --- Devolver Info Término (Índice Principal) ---
bool IndexadorHash::Devuelve(const string& word, InformacionTermino& inf) const {
    string wordProcesada = word;
    procesarPalabra(wordProcesada);
    auto it = indice.find(wordProcesada);
    if (it != indice.end()) {
        inf = it->second; return true;
    }
    return false;
}

// --- Devolver Info Término en Documento ---
bool IndexadorHash::Devuelve(const string& word, const string& nomDoc, InfTermDoc& infDoc) const {
    auto itDoc = indiceDocs.find(nomDoc);
    if (itDoc == indiceDocs.end()) return false;
    long int idDoc = itDoc->second.idDoc;
    string wordProcesada = word;
    procesarPalabra(wordProcesada);
    auto itTerm = indice.find(wordProcesada);
    if (itTerm == indice.end()) return false;
    auto itLdoc = itTerm->second.l_docs.find(idDoc);
    if (itLdoc != itTerm->second.l_docs.end()) {
        infDoc = itLdoc->second; return true;
    }
    return false;
}

// --- Existe Término ---
bool IndexadorHash::Existe(const string& word) const {
    string wordProcesada = word;
    procesarPalabra(wordProcesada);
    return (indice.find(wordProcesada) != indice.end());
}

// --- Borrar Documento ---
bool IndexadorHash::BorraDoc(const string& nomDoc) {
    auto itDoc = indiceDocs.find(nomDoc);
    if (itDoc == indiceDocs.end()) return false;
    InfDoc infoDocBorrado = itDoc->second;
    long int idDocBorrar = infoDocBorrado.idDoc;

    for (auto itTerm = indice.begin(); itTerm != indice.end(); ) {
        auto itLdoc = itTerm->second.l_docs.find(idDocBorrar);
        if (itLdoc != itTerm->second.l_docs.end()) {
            itTerm->second.ftc -= itLdoc->second.ft;
            itTerm->second.l_docs.erase(itLdoc);
            if (itTerm->second.l_docs.empty()) {
                itTerm = indice.erase(itTerm);
                informacionColeccionDocs.numTotalPalDiferentes--;
                continue;
            }
        }
        ++itTerm;
    }
    informacionColeccionDocs.numDocs--;
    informacionColeccionDocs.numTotalPal -= infoDocBorrado.numPal;
    informacionColeccionDocs.numTotalPalSinParada -= infoDocBorrado.numPalSinParada;
    informacionColeccionDocs.tamBytes -= infoDocBorrado.tamBytes;
    indiceDocs.erase(nomDoc);
    return true;
}

 
void IndexadorHash::VaciarIndiceDocs() {
     indice.clear();
     indiceDocs.clear();
     informacionColeccionDocs = InfColeccionDocs();
}

 
void IndexadorHash::VaciarIndicePreg() {
    pregunta = "";
    indicePregunta.clear();
    infPregunta = InformacionPregunta();
}
 
int IndexadorHash::NumPalIndexadas() const { return indice.size(); }
string IndexadorHash::DevolverFichPalParada() const { return ficheroStopWords; }
int IndexadorHash::NumPalParada() const { return stopWordsMap.size(); } 
string IndexadorHash::DevolverDelimitadores() const { return tok.DelimitadoresPalabra(); }
bool IndexadorHash::DevolverCasosEspeciales() const { return tok.CasosEspeciales(); }
bool IndexadorHash::DevolverPasarAminuscSinAcentos() const { return tok.PasarAminuscSinAcentos(); }
bool IndexadorHash::DevolverAlmacenarPosTerm() const { return almacenarPosTerm; }
string IndexadorHash::DevolverDirIndice() const { return directorioIndice; }
int IndexadorHash::DevolverTipoStemming() const { return tipoStemmer; }
 
void IndexadorHash::ListarPalParada() const {
 
    for (const auto& pair : stopWordsMap) {
        cout << pair.second << endl;  
    }
}
void IndexadorHash::ListarInfColeccDocs() const {
 
    cout << informacionColeccionDocs << endl;
}
void IndexadorHash::ListarTerminos() const {
 
    for (const auto& pair : indice) cout << pair.first << "\t" << pair.second << endl;
}
bool IndexadorHash::ListarTerminos(const string& nomDoc) const {
    auto itDoc = indiceDocs.find(nomDoc);
    if (itDoc == indiceDocs.end()) {
 
        return false;
    }
    long int idDocBuscar = itDoc->second.idDoc;
 
    int count = 0;  
    for (const auto& pairTerm : indice) {
        if (pairTerm.second.l_docs.count(idDocBuscar)) {
             cout << pairTerm.first << "\t" << pairTerm.second << endl;
             count++;
        }
    }
 
    return true;
}
void IndexadorHash::ListarDocs() const {
 
    for (const auto& pair : indiceDocs) cout << pair.first << "\t" << pair.second << endl;
}
bool IndexadorHash::ListarDocs(const string& nomDoc) const {
     auto itDoc = indiceDocs.find(nomDoc);
    if (itDoc != indiceDocs.end()) {
 
        cout << itDoc->first << "\t" << itDoc->second << endl;
        return true;
    } else {
 
        return false;
    }
}

 
ostream& operator<<(ostream& s, const IndexadorHash& p) {
    s << "Fichero con el listado de palabras de parada: " << p.ficheroStopWords << endl;
    s << p.tok << endl;  
    s << "Directorio donde se almacenara el indice generado: " << p.directorioIndice << endl;
    s << "Stemmer utilizado: " << p.tipoStemmer << endl;
    s << "Informacion de la coleccion indexada: " << p.informacionColeccionDocs << endl;
    s << "Se almacenaran las posiciones de los terminos: " << p.almacenarPosTerm;  
    return s;
}





std::unordered_map<std::string, InformacionTerminoPregunta> IndexadorHash::getIndicePregunta() const
{
    return indicePregunta;
}

std::unordered_map<std::string, InfDoc> IndexadorHash::getIndiceDocs () const
{
    return indiceDocs;
}

InformacionPregunta IndexadorHash::getInfPregunta () const
{
    return infPregunta;
}

std::unordered_map<std::string, InformacionTermino> IndexadorHash::getIndice () const
{
    return indice;
}

InfColeccionDocs IndexadorHash::getInformacionColeccionDocs () const
{
    return informacionColeccionDocs;
}


bool IndexadorHash::getDocById(const long int &id, string &nombre, InfDoc &informacionDoc) const
{
    bool res = false;
    unordered_map<string, InfDoc>::const_iterator it;

    for (it = indiceDocs.begin(); it != indiceDocs.end() && !res; it++)
    {
        if (it->second.idDoc ==  id)
        {
            res = true;
            nombre = it->first;
            informacionDoc = it->second;
        }
    }
    return res;
}

void IndexadorHash::setStopWords(const string &nombreFichero){
    ifstream input;
    ficheroStopWords = nombreFichero;
    input.open(nombreFichero.c_str());
    if (!input)
    {
        cerr << "Error, no existe el fichero " << nombreFichero <<  endl;
    }
    else
    {
        //cout << "Cargado fichero de StopWords" << endl;
        stopWords.clear();
        string palabra;

        while(!input.eof()){
            getline(input, palabra);
            if (palabra.size() > 0){
                stopWords.insert(palabra);
            }
        }
    }
    input.close();
}

string IndexadorHash::getPregunta () const
{
    return pregunta;
}