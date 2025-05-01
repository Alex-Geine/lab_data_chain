// Modulyator.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include "QAM32Modulator.h"
#include <fstream>

using namespace std;

int main()
{
    // Генерация тестовых битов (пример)
    string input_bits = "101100110110011011000111010110";
    QAM32Modulator modulyator;
    string output_signal;

    modulyator.run(input_bits, output_signal);

    
    
    return 0;
}

