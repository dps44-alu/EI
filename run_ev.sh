#!/bin/bash

# Compila el main
g++ src/main_time.cpp lib/buscador.cpp lib/indexadorHash.cpp lib/indexadorInformacion.cpp lib/stemmer.cpp lib/tokenizador.cpp -o ev_time

# Ejecuta el buscador para las 4 combinaciones
./ev_time DFR no fich_salida_DFR_noStemming.txt
./ev_time DFR stemming fich_salida_DFR_stemming.txt
./ev_time BM25 no fich_salida_BM25_noStemming.txt
./ev_time BM25 stemming fich_salida_BM25_stemming.txt

# Ejecuta trec_eval para cada resultado
./trec_eval -q -o frelevancia_trec_eval_TIME.txt fich_salida_DFR_noStemming.txt > fich_salida_trec_eval_DFR_noStemming.res
./trec_eval -q -o frelevancia_trec_eval_TIME.txt fich_salida_DFR_stemming.txt > fich_salida_trec_eval_DFR_stemming.res
./trec_eval -q -o frelevancia_trec_eval_TIME.txt fich_salida_BM25_noStemming.txt > fich_salida_trec_eval_BM25_noStemming.res
./trec_eval -q -o frelevancia_trec_eval_TIME.txt fich_salida_BM25_stemming.txt > fich_salida_trec_eval_BM25_stemming.res

echo "✅ Evaluación completa. Resultados generados:"
ls fich_salida_trec_eval_*.res
