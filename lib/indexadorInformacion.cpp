#include "../include/indexadorInformacion.h"


/****************************************** InformacionTermino ******************************************/

///////////////////////////////////////////////////////
// FUNCIONES PRIVADAS
///////////////////////////////////////////////////////
void InformacionTermino::Copia (const InformacionTermino& it)
{
    if (this != &it)
    {
        ftc = it.ftc;
        l_docs = it.l_docs;
    }
}


///////////////////////////////////////////////////////
// CONTRUCTORES
///////////////////////////////////////////////////////
InformacionTermino::InformacionTermino ()
{
    ftc = 0;
    l_docs = {};
}

InformacionTermino::InformacionTermino (const InformacionTermino& it)
{
    Copia(it);
}

InformacionTermino::~InformacionTermino ()
{
    ftc = 0;
    l_docs.clear();
}


///////////////////////////////////////////////////////
// OPERADORES
///////////////////////////////////////////////////////
InformacionTermino& InformacionTermino::operator= (const InformacionTermino& it)
{
    Copia(it);
    return *this;
}


///////////////////////////////////////////////////////
// FUNCIONES PÚBLICAS
///////////////////////////////////////////////////////



/****************************************** InfTermDoc ******************************************/

///////////////////////////////////////////////////////
// FUNCIONES PRIVADAS
///////////////////////////////////////////////////////
void InfTermDoc::Copia (const InfTermDoc& itd)
{
    if (this != &itd)
    {
        ft = itd.ft;
        posTerm = itd.posTerm;
    }
}


///////////////////////////////////////////////////////
// CONTRUCTORES
///////////////////////////////////////////////////////
InfTermDoc::InfTermDoc ()
{
    ft = 0;
    posTerm = {};
}

InfTermDoc::InfTermDoc (const InfTermDoc& itd)
{
    Copia(itd);
}

InfTermDoc::~InfTermDoc ()
{
    ft = 0;
    posTerm.clear();
}


///////////////////////////////////////////////////////
// OPERADORES
///////////////////////////////////////////////////////
InfTermDoc& InfTermDoc::operator= (const InfTermDoc& itd)
{
    Copia(itd);
    return *this;
}



/****************************************** InfDoc ******************************************/

///////////////////////////////////////////////////////
// FUNCIONES PRIVADAS
///////////////////////////////////////////////////////
void InfDoc::Copia (const InfDoc& id)
{
    if (this != &id)
    {
        idDoc = id.idDoc;
        numPal = id.numPal;
        numPalSinParada = id.numPalSinParada;
        numPalDiferentes = id.numPalDiferentes;
        tamBytes = id.tamBytes;
        fechaModificacion = id.fechaModificacion;
    }
}


///////////////////////////////////////////////////////
// CONTRUCTORES
///////////////////////////////////////////////////////
InfDoc::InfDoc ()
{
    idDoc = 0;
    numPal = 0;
    numPalSinParada = 0;
    numPalDiferentes = 0;
    tamBytes = 0;
    fechaModificacion = Fecha();
}

InfDoc::InfDoc (const InfDoc& id)
{
    Copia(id);
}

InfDoc::~InfDoc ()
{
    idDoc = 0;
    numPal = 0;
    numPalSinParada = 0;
    numPalDiferentes = 0;
    tamBytes = 0;
}


///////////////////////////////////////////////////////
// OPERADORES
///////////////////////////////////////////////////////
InfDoc& InfDoc::operator= (const InfDoc& id)
{
    Copia(id);
    return *this;
}



/****************************************** InfColeccionDocs ******************************************/

///////////////////////////////////////////////////////
// FUNCIONES PRIVADAS
///////////////////////////////////////////////////////
void InfColeccionDocs::Copia (const InfColeccionDocs& icd)
{
    if (this != &icd)
    {
        numDocs = icd.numDocs;
        numTotalPal = icd.numTotalPal;
        numTotalPalSinParada = icd.numTotalPalSinParada;
        numTotalPalDiferentes = icd.numTotalPalDiferentes;
        tamBytes = icd.tamBytes;
    }
}


///////////////////////////////////////////////////////
// CONTRUCTORES
///////////////////////////////////////////////////////
InfColeccionDocs::InfColeccionDocs ()
{
    numDocs = 0;
    numTotalPal = 0;
    numTotalPalSinParada = 0;
    numTotalPalDiferentes = 0;
    tamBytes = 0;
}

InfColeccionDocs::InfColeccionDocs (const InfColeccionDocs& icd)
{
    Copia(icd);
}

InfColeccionDocs::~InfColeccionDocs ()
{
    numDocs = 0;
    numTotalPal = 0;
    numTotalPalSinParada = 0;
    numTotalPalDiferentes = 0;
    tamBytes = 0;
}


///////////////////////////////////////////////////////
// OPERADORES
///////////////////////////////////////////////////////
InfColeccionDocs& InfColeccionDocs::operator= (const InfColeccionDocs& icd)
{
    Copia(icd);
    return *this;
}



/****************************************** InformacionTerminoPregunta ******************************************/
///////////////////////////////////////////////////////
// FUNCIONES PRIVADAS
///////////////////////////////////////////////////////
void InformacionTerminoPregunta::Copia (const InformacionTerminoPregunta& itp)
{
    if (this != &itp)
    {
        ft = itp.ft;
        posTerm = itp.posTerm;
    }
}


///////////////////////////////////////////////////////
// CONTRUCTORES
///////////////////////////////////////////////////////
InformacionTerminoPregunta::InformacionTerminoPregunta ()
{
    ft = 0;
    posTerm = {};
}

InformacionTerminoPregunta::InformacionTerminoPregunta (const InformacionTerminoPregunta& itp)
{
    Copia(itp);
}

InformacionTerminoPregunta::~InformacionTerminoPregunta ()
{
    ft = 0;
    posTerm.clear();
}


///////////////////////////////////////////////////////
// OPERADORES
///////////////////////////////////////////////////////
InformacionTerminoPregunta& InformacionTerminoPregunta::operator= (const InformacionTerminoPregunta& itp)
{
    Copia(itp);
    return *this;
}



/****************************************** InformacionPregunta ******************************************/
///////////////////////////////////////////////////////
// FUNCIONES PRIVADAS
///////////////////////////////////////////////////////
void InformacionPregunta::Copia (const InformacionPregunta& ip)
{
    if (this != &ip)
    {
        numTotalPal = ip.numTotalPal;
        numTotalPalSinParada = ip.numTotalPalSinParada;
        numTotalPalDiferentes = ip.numTotalPalDiferentes;
    }
}


///////////////////////////////////////////////////////
// CONTRUCTORES
///////////////////////////////////////////////////////
InformacionPregunta::InformacionPregunta() {
    numTotalPal = 0;
    numTotalPalDiferentes = 0;
    numTotalPalSinParada = 0;
}

InformacionPregunta::InformacionPregunta (const InformacionPregunta& ip) {
    Copia(ip);
}

InformacionPregunta::~InformacionPregunta ()
{
    numTotalPal = 0;
    numTotalPalDiferentes = 0;
    numTotalPalSinParada = 0;
}


///////////////////////////////////////////////////////
// OPERADORES
///////////////////////////////////////////////////////
InformacionPregunta& InformacionPregunta::operator= (const InformacionPregunta& ip)
{
    Copia(ip);
    return *this;
}



/****************************************** Fecha ******************************************/
///////////////////////////////////////////////////////
// FUNCIONES PRIVADAS
///////////////////////////////////////////////////////
void Fecha::Copia (const Fecha& f)
{
    if(this != &f)
    {
        d 	= f.d;
        m 	= f.m;
        y 	= f.y;
        h 	= f.h;
        min = f.min;
        s 	= f.s;
    }
}
///////////////////////////////////////////////////////
// CONTRUCTORES
///////////////////////////////////////////////////////
Fecha::Fecha()
{
    d = 0;
    m = 0;
    y = 0;
    h = 0;
    min = 0;
    s = 0;
}

Fecha::Fecha(struct tm *clock)
{
    d = clock->tm_mday;
    m = clock->tm_mon;
    y = clock->tm_year;
    h = clock->tm_hour;
    min = clock->tm_min;
    s = clock->tm_sec;
}

///////////////////////////////////////////////////////
// OPERADORES
///////////////////////////////////////////////////////
bool Fecha::operator< (const Fecha& f) const
{
    return y < f.y ||
           (y == f.y && m < f.m) ||
           (y == f.y && m == f.m && d < f.d) ||
           (y ==f.y && m == f.m && d == f.d && h < f.h) ||
           (y ==f.y && m == f.m && d == f.d && h == f.h && min < f.min) ||
           (y ==f.y && m == f.m && d == f.d && h == f.h && min == f.min && s < f.s);
}

Fecha& Fecha::operator= (const Fecha& f)
{
    Copia(f);
    return *this;
}
