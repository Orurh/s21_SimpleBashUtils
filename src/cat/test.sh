#!/bin/bash

filename="bytes.txt"
filename2="test3.txt"

check_files() {
    flags="$1"
    
    echo "Проверка с флагами: ${flags:-без флагов}"
    
    if [ -z "$flags" ]; then
        # Без флагов 
        ./s21_cat "$filename" "$filename2" >> s21_cat.txt
        cat "$filename" "$filename2" >> cat.txt
    else
        # С флагами
        ./s21_cat $flags "$filename" "$filename2" >> s21_cat.txt
        cat $flags "$filename" "$filename2" >> cat.txt
    fi
    
    if diff -s s21_cat.txt cat.txt; then
        echo -e "\033[1;32mФайлы s21_cat.txt и cat.txt идентичны\033[0m"
    else
        echo -e "\033[1;31mФайлы s21_cat.txt и cat.txt отличаются\033[0m"
    fi

    
    rm s21_cat.txt cat.txt
}

check_files ""
#for linux
flags=("-b" "-n" "-e" "-s" "-t" "-E" "-T" "--number-nonblank" "--number" "--squeeze-blank") 
# #for mac
# flags=("-v" "-b" "-n" "-e" "-s" "-t" )

echo -e "\033[1;33mПроверка с одиночными флагами:\033[0m"
for flag in "${flags[@]}"; do
    check_files "$flag"
done


 
echo -e "\033[1;33mПроверка всех комбинаций из двух флагов:\033[0m"
for flag1 in "${flags[@]}"; do
    for flag2 in "${flags[@]}"; do
        if [ "$flag1" != "$flag2" ]; then
            check_files "$flag1 $flag2"
        fi
    done
done