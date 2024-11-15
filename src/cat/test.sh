#!/bin/bash

filename="test2.txt"

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
    
    if diff -u s21_cat.txt cat.txt; then
        echo "Файлы s21_cat.txt и cat.txt идентичны"
    else
        echo -e "\033[1;31mФайлы s21_cat.txt и cat.txt отличаются\033[0m"
    fi

    
    rm s21_cat.txt cat.txt
}

check_files ""
#for linux
# flags=("-b" "-n" "-e" "-s" "-t" "-E" "-T" "--number-nonblank" "--number" "--squeeze-blank") 
#for mac
flags=("-v" "-b" "-n" "-e" "-s" "-t" )

echo -e "\033[1;33mПроверка с одиночными флагами:\033[0m"
for flag in "${flags[@]}"; do
    check_files "$flag"
done


# if 
# echo -e "\033[1;33mПроверка всех комбинаций из двух флагов:\033[0m"
# for flag1 in "${flags[@]}"; do
#     for flag2 in "${flags[@]}"; do
#         if [ "$flag1" != "$flag2" ]; then
#             check_files "$flag1 $flag2"
#         fi
#     done
# done