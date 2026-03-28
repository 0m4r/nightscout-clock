#!/bin/bash

if [[ $PWD == *"scripts"* ]]; then
    PROJECTDIR=$(dirname $PWD)
else
    PROJECTDIR=$PWD
fi

UPLOAD_PORT=$(awk -F '=' '/^upload_port/ {gsub(/ /, "", $2); print $2; exit}' "$PROJECTDIR/platformio.ini")
UPLOAD_SPEED=$(awk -F '=' '/^upload_speed/ {gsub(/ /, "", $2); print $2; exit}' "$PROJECTDIR/platformio.ini")

if [[ -z "$UPLOAD_PORT" ]]; then
    echo "upload_port not found in platformio.ini"
    exit 1
fi

if [[ -z "$UPLOAD_SPEED" ]]; then
    echo "upload_speed not found in platformio.ini"
    exit 1
fi

while true; do
    python ~/.platformio/packages/tool-esptoolpy/esptool.py \
        --chip esp32 --port "$UPLOAD_PORT" --baud "$UPLOAD_SPEED" --before default_reset --after hard_reset \
        run
    if [ $? -eq 0 ]; then
        break
    fi
done

echo "Reset done"
