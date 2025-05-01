#pragma once
#include <vector>
#include <cmath>
#include <bitset>
#include <stdexcept>
#include <random>
#include <fstream>
#include <iostream>
#include <sstream>
#define M_PI 3.1415

// Структура для хранения I/Q компонент
struct IQPoint {
    double I;
    double Q;
};

class QAM32Modulator
{

    // Точки синхронизации (I, Q)
    IQPoint sync_point_positive{ 1.0, 1.0 };  // (1, 1)
    IQPoint sync_point_negative{ -1.0, -1.0 }; // (-1, -1)

    const char map[6][6] =  //  Таблица кода Грея
    {
        {-1,2,3,19,18,-1},
        {6,14,10,26,30,22},
        {7,15,11,27,31,23},
        {5,13,9,25,29,21},
        {4,12,8,24,28,20},
        {-1,0,1,17,16,-1}
    };

    // Параметры сигнала
    double carrier_freq; // Частота несущей (Гц)
    double sample_rate; // Частота дискретизации (Гц)
    double symbol_duration; // Длительность символа (сек)

    std::vector<IQPoint> generate_qam32_constellation(); // Создание карты созвездия QAM-32

    uint8_t bits_to_symbol(const std::bitset<5>& bits); // Функция преобразования 5 бит в индекс символа с Грей-кодированием

    std::vector<IQPoint> map_bits_to_symbols(const std::vector<uint8_t>& bits); // Преобразование потока битов в символы

    void generate_carriers(std::vector<double>& cos_wave, std::vector<double>& sin_wave);   // Генерация квадратурных несущих

    std::vector<double> modulate_qam32( // Модуляция QAM32
        const std::vector<IQPoint>& symbols,
        const std::vector<double>& cos_wave,
        const std::vector<double>& sin_wave
    );

    std::vector<IQPoint> add_sync_sequence( //  Добавление приамбулы
        const std::vector<IQPoint>& data_symbols,
        int num_sync_pairs = 10 // Количество пар синхросимволов
    );

    std::vector<double> add_awgn(const std::vector<double>& signal, double snr_db); //  Добавление шума в дБ

    void writeVectorToFile(const std::vector<double>& vec, const std::string& filename);    //  Запись в файл

public:

    QAM32Modulator(double carrier_freq = 100.0, double sample_rate = 1000.0, double symbol_duration = 1.0);
    int run(const std::string& input_data, std::string& output_data);
};
