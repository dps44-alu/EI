import re
import matplotlib.pyplot as plt
from pathlib import Path

# Mapping of result file names to labels
FILES = {
    'fich_salida_trec_eval_DFR_noStemming.res': 'DFR sin stemming',
    'fich_salida_trec_eval_DFR_stemming.res': 'DFR con stemming',
    'fich_salida_trec_eval_BM25_noStemming.res': 'BM25 sin stemming',
    'fich_salida_trec_eval_BM25_stemming.res': 'BM25 con stemming',
}

# Function to extract precision values at recall levels
RECALL_LEVELS = [i/10 for i in range(11)]

PATTERN = re.compile(r"at\s+(\d\.\d\d)\s+(\d+\.\d+)")

def parse_file(path: Path):
    values = []
    with path.open() as f:
        text = f.read()
    # Find the block after "Queryid (Num):  All"
    m = re.search(r"Queryid \(Num\):\s+All.*?Interpolated Recall - Precision Averages:(.*?)Average precision", text, re.S)
    if m:
        block = m.group(1)
        for line in block.strip().splitlines():
            line = line.strip()
            m2 = PATTERN.search(line)
            if m2:
                values.append(float(m2.group(2)))
    return values


def main(out_path="graficaPrecisionCobertura.png"):
    plt.figure(figsize=(8,6))
    for file, label in FILES.items():
        values = parse_file(Path(file))
        if len(values) != len(RECALL_LEVELS):
            print(f"Advertencia: {file} tiene {len(values)} valores, se esperaban {len(RECALL_LEVELS)}")
        plt.plot(RECALL_LEVELS[:len(values)], values, marker='o', label=label)
    plt.xlabel('Cobertura')
    plt.ylabel('Precisión interpolada')
    plt.title('Comparación DFR/BM25 con y sin stemming')
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(out_path)
    print(f"Gráfica guardada en {out_path}")

if __name__ == '__main__':
    main()
