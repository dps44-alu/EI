#include "indexadorInformacion.h" // Ajustar ruta si es necesario
#include <ostream>
#include <list>
#include <unordered_map>
#include <string>
#include <cctype> // Para isspace (si se necesita en algún operador)

using namespace std;

// --- Clase Fecha ---
Fecha::Fecha(struct tm *clock) {
    if (clock) {
        dia = clock->tm_mday;
        mes = clock->tm_mon;
        anyo = clock->tm_year;
        hora = clock->tm_hour;
        min = clock->tm_min;
        seg = clock->tm_sec;
    } else {
        dia = mes = anyo = hora = min = seg = 0;
    }
}

bool Fecha::operator<(const Fecha& f) const {
    if (anyo != f.anyo) return anyo < f.anyo;
    if (mes != f.mes) return mes < f.mes;
    if (dia != f.dia) return dia < f.dia;
    if (hora != f.hora) return hora < f.hora;
    if (min != f.min) return min < f.min;
    return seg < f.seg;
}
// El operator= y constructor por defecto se usan los generados (ver .h)

// --- Clase InformacionPregunta ---
InformacionPregunta::InformacionPregunta() :
    numTotalPal(0),
    numTotalPalSinParada(0),
    numTotalPalDiferentes(0)
{}
// El resto de métodos especiales usan los generados por defecto (ver .h)

// --- operator<< para InformacionPregunta (CORREGIDO) ---
ostream& operator<<(ostream& s, const InformacionPregunta& p) {
    // Formato con etiquetas según diff indexador09/10.cpp
    s << "numTotalPal: " << p.numTotalPal
      << "\tnumTotalPalSinParada: " << p.numTotalPalSinParada
      << "\tnumTotalPalDiferentes: " << p.numTotalPalDiferentes;
    // No añadir salto de línea aquí si la llamada lo añade
    return s;
}

// --- Clase InformacionTerminoPregunta ---
InformacionTerminoPregunta::InformacionTerminoPregunta() : ft(0) {}

InformacionTerminoPregunta::~InformacionTerminoPregunta() {
    posTerm.clear();
}

InformacionTerminoPregunta& InformacionTerminoPregunta::operator=(const InformacionTerminoPregunta& other) {
    if (this != &other) {
        ft = other.ft;
        posTerm = other.posTerm;
    }
    return *this;
}
// El constructor de copia usa el generado por defecto (ver .h)

// --- operator<< para InformacionTerminoPregunta (CORREGIDO) ---
ostream& operator<<(ostream& s, const InformacionTerminoPregunta& p) {
    // Formato con etiqueta ft: según diff indexador09/10.cpp
    s << "ft: " << p.ft;
    // Imprimir posiciones si existen (separadas por espacio según diff)
    for (const auto& pos : p.posTerm) {
        s << " " << pos; // Espacio antes de cada posición
    }
    return s;
}

// --- Clase InfColeccionDocs ---
InfColeccionDocs::InfColeccionDocs() :
    numDocs(0),
    numTotalPal(0),
    numTotalPalSinParada(0),
    numTotalPalDiferentes(0),
    tamBytes(0)
{}
// El resto de métodos especiales usan los generados por defecto (ver .h)

// --- operator<< para InfColeccionDocs (CORREGIDO) ---
ostream& operator<<(ostream& s, const InfColeccionDocs& p) {
    // Formato con etiquetas y tabs según diff indexador01/03/05.cpp
    s << "numDocs: " << p.numDocs
      << "\tnumTotalPal: " << p.numTotalPal // Usar tab como separador
      << "\tnumTotalPalSinParada: " << p.numTotalPalSinParada
      << "\tnumTotalPalDiferentes: " << p.numTotalPalDiferentes
      << "\ttamBytes: " << p.tamBytes;
    // No añadir salto de línea aquí
    return s;
}

// --- Clase InfTermDoc ---
InfTermDoc::InfTermDoc() : ft(0) {}

InfTermDoc::~InfTermDoc() {
    posTerm.clear();
}

InfTermDoc& InfTermDoc::operator=(const InfTermDoc& other) {
    if (this != &other) {
        ft = other.ft;
        posTerm = other.posTerm;
    }
    return *this;
}
// El constructor de copia usa el generado por defecto (ver .h)

// --- operator<< para InfTermDoc (CORREGIDO) ---
ostream& operator<<(ostream& s, const InfTermDoc& p) {
    // Formato con etiqueta ft: y posiciones con espacio según diff indexador04/07/08.cpp
    s << "ft: " << p.ft;
    // Imprimir posiciones si existen (separadas por espacio)
    for (const auto& pos : p.posTerm) {
        s << " " << pos; // Espacio antes de cada posición
    }
    return s;
}

// --- Clase InfDoc ---
InfDoc::InfDoc() :
    idDoc(0),
    numPal(0),
    numPalSinParada(0),
    numPalDiferentes(0),
    tamBytes(0),
    fechaModificacion()
{}
// El resto de métodos especiales usan los generados por defecto (ver .h)

// --- operator<< para InfDoc (CORREGIDO - etiqueta y SIN FECHA) ---
ostream& operator<<(ostream& s, const InfDoc& p) {
    // Formato con etiquetas y tabs según diff indexador03/05.cpp
    // ¡SIN FECHA! La salida esperada no la incluye.
     s << "idDoc: " << p.idDoc
       << "\tnumPal: " << p.numPal
       << "\tnumPalSinParada: " << p.numPalSinParada // Etiqueta corregida
       << "\tnumPalDiferentes: " << p.numPalDiferentes
       << "\ttamBytes: " << p.tamBytes;
    // No añadir fecha ni salto de línea
    return s;
}


// --- Clase InformacionTermino ---
InformacionTermino::InformacionTermino() : ftc(0) {}

InformacionTermino::InformacionTermino(const InformacionTermino &other) :
    ftc(other.ftc),
    l_docs(other.l_docs)
{}

InformacionTermino::~InformacionTermino() {
    l_docs.clear();
}

InformacionTermino& InformacionTermino::operator=(const InformacionTermino& other) {
    if (this != &other) {
        ftc = other.ftc;
        l_docs = other.l_docs;
    }
    return *this;
}

// --- operator<< para InformacionTermino (CORREGIDO) ---
ostream& operator<<(ostream& s, const InformacionTermino& p) {
    // Formato con etiquetas según diff indexador04/07/08.cpp
    s << "Frecuencia total: " << p.ftc << "\tfd: " << p.l_docs.size();
    // Imprimir detalles de cada documento si l_docs no está vacío
    for (const auto& pair : p.l_docs) {
        // Formato: <tab>Id.Doc: <id><tab>(Salida de InfTermDoc)
        s << "\tId.Doc: " << pair.first << "\t" << pair.second; // pair.second llama a InfTermDoc::operator<< corregido
    }
    return s;
}