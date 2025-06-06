#include "../include/buscador.h"

int main (int argc, char* argv[]) 
{
    if (argc != 4)
    {
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
        "./StopWordsIngles.txt",                // Stopwords
        ",;:.-/+*\\ '\"{}[]()<>¡!¿?&#=\t@",     // Delimitadores por defecto
        true,                                   // quitarAcentos
        true,                                   // pasarAMinusculas
        dirIndice,                              // Directorio índice
        stemming,                               // Stemming sí/no
        false                                   // almacenarPosTerm = false
    );

    if (!indexador.Indexar("ficherosTimes.txt")) 
    {
        cerr << "❌ Error al indexar ficheros TIME.\n";
        return 1;
    }

    indexador.GuardarIndexacion();

    // Crear buscador con la fórmula indicada
    Buscador buscador(dirIndice, formulaSimilitud);

    // Ejecutar búsqueda sobre las 83 preguntas
    buscador.Buscar("./CorpusTime/Preguntas/", 423, 1, 83);
    // Mostrar todas las respuestas (423 documentos por cada una de las 83 preguntas)
    buscador.ImprimirResultadoBusqueda(423 * 83, nombreSalida);

    cout << "✅ Búsqueda completada con éxito.\n";
    return 0;
}
