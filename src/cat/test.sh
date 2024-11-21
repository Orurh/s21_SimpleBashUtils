#!/bin/bash

filename="s21_cat.h"

check_files() {
    flags="$1"
    echo "Проверка с флагами: ${flags:-без флагов}"
    if [ -z "$flags" ]; then
        # Без флагов 
        ./s21_cat "$filename" >> s21_cat.txt
        cat "$filename" >> cat.txt
    else
        # С флагами
        ./s21_cat $flags "$filename" >> s21_cat.txt
        cat $flags "$filename" >> cat.txt
    fi
    
    if diff -s s21_cat.txt cat.txt; then
        echo -e "\033[1;32mФайлы s21_cat.txt и cat.txt идентичны\033[0m"
    else
        echo -e "\033[1;31mФайлы s21_cat.txt и cat.txt отличаются\033[0m"
    fi
    rm s21_cat.txt cat.txt
}

check_files ""
flags=("-b" "-n" "-e" "-s" "-t" "-E" "-T" "--number-nonblank" "--number" "--squeeze-blank") 

echo -e "\033[1;33mПроверка с одиночными флагами:\033[0m"
for flag in "${flags[@]}"; do
    check_files "$flag"
done

