#!/bin/bash

# Имя файла для тестирования
test_filename="s21_grep.c"

check_grep_files() {
    flags="$1"
    pattern="$2"
    echo "Проверка grep с флагами: ${flags:-без флагов} с паттерном: '$pattern'"

    # Создание или очистка временных файлов
    > grep_output.txt
    > s21_grep_output.txt

    # Без флагов
    if [ -z "$flags" ]; then
        grep "$pattern" "$test_filename" >> grep_output.txt
        ./s21_grep "$pattern" "$test_filename" >> s21_grep_output.txt
    else
        # С флагами
        grep $flags "$pattern" "$test_filename" >> grep_output.txt
        ./s21_grep $flags "$pattern" "$test_filename" >> s21_grep_output.txt
    fi

    if diff -s s21_grep_output.txt grep_output.txt; then
        echo -e "\033[1;32mФайлы s21_grep_output.txt и grep_output.txt идентичны\033[0m"
    else
        echo -e "\033[1;31mФайлы s21_grep_output.txt и grep_output.txt отличаются\033[0m"
    fi
}

# Основные флаги для тестирования
single_flags=("-e" "-i" "-v" "-c" "-l" "-n" "-h" "-s" "-o")
pairs=()

# Генерация пар флагов (без -f)
for ((i=0; i<${#single_flags[@]}; i++)); do
    for ((j=i; j<${#single_flags[@]}; j++)); do
        pairs+=("${single_flags[i]} ${single_flags[j]}")
    done
done

# Проверка с одиночными флагами
echo -e "\033[1;33mПроверка с одиночными флагами:\033[0m"
for flag in "${single_flags[@]}"; do
    check_grep_files "$flag" "void"
done

# Проверка с парными флагами
echo -e "\033[1;33mПроверка с парными флагами:\033[0m"
for pair in "${pairs[@]}"; do
    check_grep_files "$pair" "void"
done

# Проверка с флагом -f
echo -e "\033[1;33mПроверка с флагом -f:\033[0m"
check_grep_files "-f patterns.txt" "$test_filename"

# Генерация пар флагов (с -f)
f_pairs=()
for flag in "${single_flags[@]}"; do
    f_pairs+=("-f patterns.txt $flag")
done

# Проверка парных флагов с -f
echo -e "\033[1;33mПроверка с флагом -f в паре с другими флагами:\033[0m"
for pair in "${f_pairs[@]}"; do
    check_grep_files "$pair" "void"
done

# Очищение временных файлов
rm grep_output.txt s21_grep_output.txt