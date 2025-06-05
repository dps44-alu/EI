#include "../include/buscador.h"

int main (int argc, char* argv[]) 
{
    if (argc != 4) 
    {
        cerr << "Uso: " << argv[0] << " <DFR|BM25> <stemming|no> <fichero_salida>\n";
        return 1;
    }

    string modelo = argv[1];
    string stemFlag = argv[2];
    string nombreSalida = argv[3];

    bool stemming = (stemFlag == "stemming");
    int formulaSimilitud = (modelo == "BM25") ? 1 : 0;

    string dirIndice = "./indiceTIME";

    // Crear indexador y procesar colección
    IndexadorHash indexador(
        "./StopWordsIngles.txt",    // Stopwords
        ". ,:",                     // Delimitadores por defecto
        true,                       // quitarAcentos
        true,                       // pasarAMinusculas
        dirIndice,                  // Directorio índice
        stemming,                   // Stemming sí/no
        false                       // almacenarPosTerm = false
    );

    if (!indexador.Indexar("ficherosTimes.txt")) 
    {
        cerr << "❌ Error al indexar ficheros TIME.\n";
        return 1;
    }

    indexador.GuardarIndexacion();

    // Crear buscador con la fórmula indicada
    Buscador buscador(dirIndice, formulaSimilitud);

    cout << buscador << endl;

    // Ejecutar búsqueda sobre las 83 preguntas
    buscador.Buscar("./CorpusTime/Preguntas/", 423, 1, 83);
    buscador.ImprimirResultadoBusqueda(423, nombreSalida);

    cout << "✅ Búsqueda completada con éxito.\n";
    return 0;
}


/*
./busqueda DFR no fich_dfr_no.txt
./busqueda DFR stemming fich_dfr_stem.txt
./busqueda BM25 no fich_bm25_no.txt
./busqueda BM25 stemming fich_bm25_stem.txt
*/