#!/bin/bash

set -e  # Sai em caso de erro

ROOT_DIR="."
DEST_DIR="lv_port_linux"
SRC_DEST_DIR="$DEST_DIR/src"

# Verifica pastas
if [ ! -d "$DEST_DIR" ]; then
    echo "Erro: Pasta '$DEST_DIR' não encontrada."
    exit 1
fi

if [ ! -d "$SRC_DEST_DIR" ]; then
    echo "Erro: Pasta '$SRC_DEST_DIR' não encontrada."
    exit 1
fi

#Copia o arquivo main para a pasta src
cp -v main.c "$SRC_DEST_DIR/"

# 1. Copia arquivos principais
echo "Copiando lv_conf.h e CMakeLists.txt para $DEST_DIR..."
cp -v lv_conf.h "$DEST_DIR/"
cp -v CMakeLists.txt "$DEST_DIR/"

# 2. Copia comm/ e screens/ (sem jsmn.c/.h ainda, vamos gerar novos)
echo "Copiando pasta 'comm' para $SRC_DEST_DIR..."
cp -rv comm "$SRC_DEST_DIR/"

echo "Preparando pasta 'screens' em $SRC_DEST_DIR/screens..."

# Cria diretório de destino para screens
mkdir -p "$SRC_DEST_DIR/screens"

# Copia todos os arquivos de screens/, exceto jsmn.h e jsmn.c (vamos recriá-los)
for file in screens/*; do
    basename_file=$(basename "$file")
    if [[ "$basename_file" != "jsmn.h" && "$basename_file" != "jsmn.c" ]]; then
        cp -v "$file" "$SRC_DEST_DIR/screens/"
    fi
done

# 3. Processa jsmn.h e jsmn.c
# Extrai linhas 120-180 → jsmn_parse
sed -n '103,454p' jsmn/jsmn.h > /tmp/jsmn_parse.c

# Extrai linhas 182-186 → jsmn_init
sed -n '454,464p' jsmn/jsmn.h > /tmp/jsmn_init.c

# Monta jsmn.c
{
  echo '#include "jsmn.h"'
  echo
  cat /tmp/jsmn_parse.c
  echo
  cat /tmp/jsmn_init.c
} > lv_port_linux/src/screens/jsmn.c

# Cria jsmn.h sem essas linhas
awk 'NR < 102 || NR > 465' jsmn/jsmn.h > lv_port_linux/src/screens/jsmn.h

echo "✅ Arquivos jsmn.h e jsmn.c processados e salvos em $SRC_DEST_DIR/screens/"
echo "✅ Setup concluído com sucesso!"