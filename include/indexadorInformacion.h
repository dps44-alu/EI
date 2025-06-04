#ifndef INDEXADORINFORMACION_H
#define INDEXADORINFORMACION_H

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <list>

using namespace std;


class InfTermDoc
{
    friend ostream& operator<<(ostream& s, const InfTermDoc& p)
    {
        s << "ft: " << p.ft;

        // A continuaci�n se mostrar�an todos los elementos de p.posTerm
        // (?posicion TAB posicion TAB ... posicion, es decir nunca finalizar� en un TAB?):
        // s << ?\t? << posicion;
        for (const auto& itPosTerm : p.posTerm)
        {
            s << "\t" << itPosTerm;
        }

        return s;
    }

    private:
        int ft;	            // Frecuencia del t�rmino en el documento
        list<int> posTerm;
        // Solo se almacenar� esta informaci�n si el campo privado del indexador almacenarPosTerm == true
        // Lista de n�meros de palabra en los que aparece el t�rmino en el documento. Los n�meros de palabra
        // comenzar�n desde cero (la primera palabra del documento). Se numerar�n las palabras de parada.
        // Estar� ordenada de menor a mayor posici�n.

        void Copia (const InfTermDoc &);

    public:
        InfTermDoc ();		// Inicializa ft = 0
        InfTermDoc (const InfTermDoc &);
        ~InfTermDoc ();		// Pone ft = 0
        InfTermDoc & operator= (const InfTermDoc &);

        int                 GetFt               () const    { return ft;        }
        list<int>           GetPosTerm          () const    { return posTerm;   }
        list<int>&          GetPosTerm_ref      ()          { return posTerm;   }   // Para iterar y modificar
        const list<int>&    GetPosTerm_const    () const    { return posTerm;   }   // Para iterar sin modificar

        void SetFt      (const int& f)          { ft        = f; }
        void SetPosTerm (const list<int>& l)    { posTerm   = l; }
        void AddPos     (const int& p)          { posTerm.push_back(p); }
};


class Fecha
{
    private:
        int d, m, y, h, min, s;     // d�a, mes, a�o, hora, minutos, segundos

        void Copia (const Fecha &);

    public:
        Fecha ();
        Fecha (struct tm *);
        bool operator< (const Fecha &) const;
        Fecha& operator= (const Fecha &);

        int GetDia  () const { return d;    }
        int GetMes  () const { return m;    }
        int GetAnyo () const { return y;    }
        int GetHora () const { return h;    }
        int GetMin  () const { return min;  }
        int GetSeg  () const { return s;    }

        void SetDia     (const int& x)  { d     = x; }
        void SetMes     (const int& x)  { m     = x; }
        void SetAnyo    (const int& x)  { y     = x; }
        void SetHora    (const int& x)  { h     = x; }
        void SetMin     (const int& x)  { min   = x; }
        void SetSeg     (const int& x)  { s     = x; }
};


class InformacionTermino
{
    friend ostream& operator<<(ostream& s, const InformacionTermino& p)
    {
        s << "Frecuencia total: " << p.ftc << "\tfd: " << p.l_docs.size();

        // A continuaci�n se mostrar�an todos los elementos de p.l_docs:
        // s << ?\tId.Doc: ? << idDoc << ?\t? << InfTermDoc;
        for (const auto& itLdocs : p.l_docs)
        {
            s << "\tId.Doc: " << itLdocs.first << "\t" << itLdocs.second;
        }

        return s;
    }

    private:
        int ftc;	        // Frecuencia total del t�rmino en la colecci�n
        unordered_map<int, InfTermDoc> l_docs;
        // Tabla Hash que se acceder� por el id del documento, devolviendo un objeto de la clase InfTermDoc
        // que contiene toda la informaci�n de aparici�n del t�rmino en el documento

        void Copia (const InformacionTermino &);

    public:
        InformacionTermino ();		// Inicializa ftc = 0
        InformacionTermino (const InformacionTermino &);
        ~InformacionTermino ();		// Pone ftc = 0 y vac�a l_docs
        InformacionTermino & operator= (const InformacionTermino &);

        int                                     GetFtc          () const    { return ftc;       }
        unordered_map<int, InfTermDoc>          GetL_docs       () const    { return l_docs;    }
        unordered_map<int, InfTermDoc>&         GetL_docs_ref   ()          { return l_docs;    }  // Para iterar y modificar
        const unordered_map<int, InfTermDoc>&   GetL_docs_const () const    { return l_docs;    }  // Para iterar sin modificar


        void SetFtc     (const int& f)                              { ftc       = f; }
        void SetL_docs  (const unordered_map<int, InfTermDoc>& l)   { l_docs    = l; }
        void AddInf     (const int& id, const InfTermDoc& inf)      { l_docs.insert({id, inf}); }
        void DeleteInf  (const int& i)                              { l_docs.erase(i); }

        bool IndexedAtDocument (const int& d, InfTermDoc& infTermDoc) const
        {
            auto it = l_docs.find(d);

            if (it != l_docs.end())
            {
                infTermDoc = it->second;
                return true;
            }

            return false;
        }
};


class InfDoc
{
    friend ostream& operator<<(ostream& s, const InfDoc& p)
    {
        s << "idDoc: " << p.idDoc << "\tnumPal: " << p.numPal << "\tnumPalSinParada: " << p.numPalSinParada << "\tnumPalDiferentes: "
          << p.numPalDiferentes << "\ttamBytes: " << p.tamBytes;

        return s;
    }

    private:
        int idDoc;
        // Identificador del documento. El primer documento indexado en la colecci�n ser� el identificador 1
        int numPal;	            // N� total de palabras del documento
        int numPalSinParada;	// N� total de palabras sin stop-words del documento
        int numPalDiferentes;
        // N� total de palabras diferentes que no sean stop-words (sin acumular la frecuencia de cada una de ellas)
        int tamBytes;	        // Tama�o en bytes del documento
        Fecha fechaModificacion;
        // Atributo correspondiente a la fecha y hora (completa) de modificaci�n del documento. El tipo "Fecha/hora" lo elegir�/implementar� el alumno

        void Copia (const InfDoc &);

    public:
        InfDoc ();
        InfDoc (const InfDoc &);
        ~InfDoc ();
        InfDoc & operator= (const InfDoc &);

        int     GetIdDoc                () const { return idDoc;                }
        int     GetNumPal               () const { return numPal;               }
        int     GetNumPalSinParada      () const { return numPalSinParada;      }
        int     GetNumPalDiferentes     () const { return numPalDiferentes;     }
        int     GetTamBytes             () const { return tamBytes;             }
        Fecha   GetFechaModicifacion    () const { return fechaModificacion;    }

        int GetDia  () const { return fechaModificacion.GetDia();   }
        int GetMes  () const { return fechaModificacion.GetMes();   }
        int GetAnyo () const { return fechaModificacion.GetAnyo();  }
        int GetHora () const { return fechaModificacion.GetHora();  }
        int GetMin  () const { return fechaModificacion.GetMin();   }
        int GetSeg  () const { return fechaModificacion.GetSeg();   }

        void SetIdDoc               (const int& i)      { idDoc             = i; }
        void SetNumPal              (const int& n)      { numPal            = n; }
        void SetNumPalSinParada     (const int& n)      { numPalSinParada   = n; }
        void SetNumPalDiferentes    (const int& n)      { numPalDiferentes  = n; }
        void SetTamBytes            (const int& n)      { tamBytes          = n; }
        void SetFechaModificacion   (const Fecha& f)    { fechaModificacion = f; }

        void SetDia     (const int& x)  { fechaModificacion.SetDia(x);  }
        void SetMes     (const int& x)  { fechaModificacion.SetMes(x);  }
        void SetAnyo    (const int& x)  { fechaModificacion.SetAnyo(x); }
        void SetHora    (const int& x)  { fechaModificacion.SetHora(x); }
        void SetMin     (const int& x)  { fechaModificacion.SetMin(x);  }
        void SetSeg     (const int& x)  { fechaModificacion.SetSeg(x);  }
};


class InfColeccionDocs
{
    friend ostream& operator<<(ostream& s, const InfColeccionDocs& p)
    {
        s << "numDocs: " << p.numDocs << "\tnumTotalPal: " << p.numTotalPal << "\tnumTotalPalSinParada: " << p.numTotalPalSinParada
          << "\tnumTotalPalDiferentes: " << p.numTotalPalDiferentes << "\ttamBytes: " << p.tamBytes;

        return s;
    }

    private:
        int numDocs;	            // N� total de documentos en la colecci�n
        int numTotalPal;            // N� total de palabras en la colecci�n
        int numTotalPalSinParada;   // N� total de palabras sin stop-words en la colecci�n
        int numTotalPalDiferentes;
        // N� total de palabras diferentes en la colecci�n que no sean stop-words (sin acumular la frecuencia de cada una de ellas)
        int tamBytes;	            // Tama�o total en bytes de la colecci�n

        void Copia (const InfColeccionDocs &);

    public:
        InfColeccionDocs ();
        InfColeccionDocs (const InfColeccionDocs &);
        ~InfColeccionDocs ();
        InfColeccionDocs & operator= (const InfColeccionDocs &);

        int GetNumDocs                  () const { return numDocs;                  }
        int GetNumTotalPal              () const { return numTotalPal;              }
        int GetNumTotalPalSinParada     () const { return numTotalPalSinParada;     }
        int GetNumTotalPalDiferentes    () const { return numTotalPalDiferentes;    }
        int GetTamBytes                 () const { return tamBytes;                 }

        void SetNumDocs                 (const int& n)  { numDocs               = n; }
        void SetNumTotalPal             (const int& n)  { numTotalPal           = n; }
        void SetNumTotalPalSinParada    (const int& n)  { numTotalPalSinParada  = n; }
        void SetNumTotalPalDiferentes   (const int& n)  { numTotalPalDiferentes = n; }
        void SetTamBytes                (const int& n)  { tamBytes              = n; }

};


class InformacionTerminoPregunta
{
    friend ostream& operator<<(ostream& s, const InformacionTerminoPregunta& p)
    {
        s << "ft: " << p.ft;

        // A continuaci�n se mostrar�an todos los elementos de p.posTerm
        // (?posicion TAB posicion TAB ... posicion, es decir nunca finalizar� en un TAB?):
        // s << ?\t? << posicion;
        for (const auto& itPosTerm : p.posTerm)
        {
            s << "\t" << itPosTerm;
        }

        return s;
    }

    private:
        int ft;	            // Frecuencia total del t�rmino en la pregunta
        list<int> posTerm;
        // Solo se almacenar� esta informaci�n si el campo privado del indexador almacenarPosTerm == true
        // Lista de n�meros de palabra en los que aparece el t�rmino en la pregunta. Los n�meros de palabra
        // comenzar�n desde cero (la primera palabra de la pregunta). Se numerar�n las palabras de parada.
        // Estar� ordenada de menor a mayor posici�n.

        void Copia (const InformacionTerminoPregunta &);

    public:
        InformacionTerminoPregunta ();
        InformacionTerminoPregunta (const InformacionTerminoPregunta &);
        ~InformacionTerminoPregunta ();
        InformacionTerminoPregunta & operator= (const InformacionTerminoPregunta &);

        int                 GetFt               () const    { return ft;       }
        list<int>           GetPosTerm          () const    { return posTerm;  }
        list<int>&          GetPosTerm_ref      ()          { return posTerm;  }    // Para iterar y modificar
        const list<int>&    GetPosTerm_const    () const    { return posTerm;  }    // Para iterar sin modificar


        void SetFt      (const int& f)          { ft        = f; }
        void SetPosTerm (const list<int>& l)    { posTerm   = l; }
        void AddPos     (const int& p)          { posTerm.push_back(p); }
};


class InformacionPregunta
{
    friend ostream& operator<<(ostream& s, const InformacionPregunta& p)
    {
        s << "numTotalPal: " << p.numTotalPal << "\tnumTotalPalSinParada: "<< p.numTotalPalSinParada << "\tnumTotalPalDiferentes: " << p.numTotalPalDiferentes;

        return s;
    }

    private:
        int numTotalPal;            // N� total de palabras en la pregunta
        int numTotalPalSinParada;   // N� total de palabras sin stop-words en la pregunta
        int numTotalPalDiferentes;
        // N� total de palabras diferentes en la pregunta que no sean stop-words
        // (sin acumular la frecuencia de cada una de ellas)

        void Copia (const InformacionPregunta &);

    public:
        InformacionPregunta ();
        InformacionPregunta (const InformacionPregunta &);
        ~InformacionPregunta ();
        InformacionPregunta & operator= (const InformacionPregunta &);

        int GetNumTotalPal              () const { return numTotalPal;           }
        int GetNumTotalPalSinParada     () const { return numTotalPalSinParada;  }
        int GetNumTotalPalDiferentes    () const { return numTotalPalDiferentes; }

        void SetNumTotalPal             (const int& n)  { numTotalPal           = n; }
        void SetNumTotalPalSinParada    (const int& n)  { numTotalPalSinParada  = n; }
        void SetNumTotalPalDiferentes   (const int& n)  { numTotalPalDiferentes = n; }
};

#endif