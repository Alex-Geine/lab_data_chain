#include "QAM32Modulator.h"
#include "filter.hpp"

using namespace std;

// Создание карты созвездия QAM-32
vector<IQPoint> QAM32Modulator:: generate_qam32_constellation()
{
    vector<IQPoint> constellation;
    const double step = 2.0; // Шаг между точками
    const vector<double> levels = { -5, -3, -1, 1, 3, 5 }; // Возможные уровни

    for (double q : levels) {
        for (double i : levels) {

            constellation.push_back({ i, q });
        }
    }
    return constellation; // 6x6 = 36 точек
}

// Функция преобразования 5 бит в индекс символа с Gray-кодированием
uint8_t QAM32Modulator::bits_to_symbol(const bitset<5>& bits) {

    for (size_t i = 0; i < 6; i++)
    {
        for (size_t j = 0; j < 6; j++)
        {
            auto b = bits.to_ulong();
            if (bits.to_ulong() == map[i][j])
            {
                return uint8_t(i * 6 + j);
            }
        }
    }

}

// Преобразование потока битов в символы
vector<IQPoint> QAM32Modulator::map_bits_to_symbols(const vector<uint8_t>& bits) {
    vector<IQPoint> symbols;
    auto constellation = generate_qam32_constellation();

    bitset<5> current_bits;
    size_t count = 0;

    for (uint8_t bit : bits) {
        current_bits.set(count, bit); // Заполняем биты
        count++;

        if (count == 5) {
            uint8_t symbol_index = bits_to_symbol(current_bits);
            symbols.push_back(constellation[symbol_index]);
            count = 0;
            current_bits.reset();
        }
    }

    if (count != 0) {
        throw invalid_argument("Биты не кратные 5!");
    }

    return symbols;
}

void QAM32Modulator::generate_carriers(vector<double>& cos_wave, vector<double>& sin_wave) {
    size_t num_samples = static_cast<size_t>(symbol_duration * sample_rate);
    cos_wave.resize(num_samples);
    sin_wave.resize(num_samples);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> phase_dist(-M_PI, M_PI);
    double phi = phase_dist(gen);

    std::cout<<"phase: " << phi<<std::endl;
    
    for (size_t i = 0; i < num_samples; ++i) {
        double t = i / sample_rate;
        cos_wave[i] = cos(2 * M_PI * carrier_freq * t + phi);
        sin_wave[i] = sin(2 * M_PI * carrier_freq * t + phi);
    }
}

vector<double> QAM32Modulator::modulate_qam32(
    const vector<IQPoint>& symbols,
    const vector<double>& cos_wave,
    const vector<double>& sin_wave
) {
    vector<double> signal;
    size_t samples_per_symbol = cos_wave.size();

    for (const auto& symbol : symbols) {
        for (size_t i = 0; i < samples_per_symbol; ++i) {
            double sample = symbol.I * cos_wave[i] - symbol.Q * sin_wave[i];
            signal.push_back(sample);
        }
    }

    return signal;
}

vector<IQPoint> QAM32Modulator::add_sync_sequence(
    const vector<IQPoint>& data_symbols,
    int num_sync_pairs // Количество пар синхросимволов
) {
    vector<IQPoint> full_frame;

    // Добавляем синхропоследовательность
    for (int i = 0; i < num_sync_pairs; ++i) {
        full_frame.push_back(sync_point_positive);
        full_frame.push_back(sync_point_positive);
        full_frame.push_back(sync_point_negative);
    }

    // Добавляем полезные данные
    full_frame.insert(full_frame.end(), data_symbols.begin(), data_symbols.end());

    return full_frame;
}

vector<double> QAM32Modulator::add_awgn(const vector<double>& signal, double snr_db = 20.0) {
    vector<double> noisy_signal;
    noisy_signal.reserve(signal.size());

    // Рассчитываем мощность сигнала
    double signal_power = 0.0;
    for (const auto& sample : signal) {
        signal_power += sample * sample;
    }
    signal_power /= signal.size();

    // std::cout << "Signal power: " << signal_power << std::endl;

    // Преобразуем SNR из дБ в линейный масштаб
    double snr_linear = pow(10.0, snr_db / 10.0);

    // Мощность шума
    double noise_power = signal_power / snr_linear;

    // std::cout << "snr_linear: " << snr_linear << std::endl;

    // std::cout << "noise_power: " << noise_power << std::endl;

    // signal power
    // noise power (after filer)

    // signal / ( K * noise ) = snr_linear
    // K = sginal / snr / noise
    // for (auto)
    //    noise *= K;

    // Генератор случайных чисел (гауссовский шум)
    random_device rd;
    mt19937 gen(rd());
    normal_distribution<double> dist(0.0, sqrt(noise_power));

    vector<double> noise;
    vector<double> noise_temp;

    // Добавляем шум к сигналу
    for (const auto& sample : signal) {
        noise.push_back(dist(gen));
    }

    noise_temp.resize(noise.size());

    auto filt_high = design_butterworth_high_pass_filter( carrier_freq - 20, sample_rate);
    auto filt_low = design_butterworth_low_pass_filter(carrier_freq + 20, sample_rate);

    filt_high(noise.begin(), noise.end(), noise_temp.begin());
    filt_low(noise_temp.begin(), noise_temp.end(), noise.begin());

    // std::ofstream fstr("noise.txt");
    
    // std::copy(noise.begin(),noise.end(),std::ostream_iterator<double>(fstr, "\n"));
    // fstr.close();

    noise_power = 0;
    for (const auto& sample : noise) {
        noise_power += sample * sample;
    }
    noise_power /= noise.size();

    // std::cout << "Signal power noise noise: " << noise_power << std::endl;
    // std::cout << "Lin: " <<  snr_linear << std::endl;
    //     std::cout << "signal_power: " <<  signal_power << std::endl;

    double K = std::sqrt(signal_power / noise_power / snr_linear) ;
    std::cout << "K: " << K << std::endl;
    for (auto& sample : noise) {
        sample *= K;
    }

        noise_power = 0;
    for (const auto& sample : noise) {
        noise_power += sample * sample;
    }
    noise_power/=noise.size();
    // std::cout << "Signal power noise N N: " << signal_power << std::endl;


    for (int i = 0; i< signal.size(); i++) {
        noise_temp[i] = signal[i] + noise[i];
    }

    noise_power = 0;
    for (const auto& sample : noise_temp) {
        noise_power += sample * sample;
    }
    noise_power/=noise.size();

    // std::cout << "Signal power noise: " << noise_power << std::endl;

    return noise_temp;
}

QAM32Modulator::QAM32Modulator(double carrier_freq, double sample_rate, double symbol_duration)
{
    this->carrier_freq = carrier_freq;
    this->sample_rate = sample_rate;
    this->symbol_duration = symbol_duration;
}

int QAM32Modulator::run(const string& input_data, string& output_data)
{
    // Преобразование строки в биты
    vector<uint8_t> bits;
    for (char c : input_data) {
        if (c != '0' && c != '1') {
            throw invalid_argument("Input data must contain only '0' or '1'");
        }
        bits.push_back(static_cast<uint8_t>(c - '0')); // '0' → 0, '1' → 1
    }

    // Проверка кратности 5
    if (bits.size() % 5 != 0) {
        throw invalid_argument("Input data length must be such that total bits are multiple of 5");
    }

    // Преобразование битов в символы
    auto data_symbols = map_bits_to_symbols(bits);

    // Добавление синхропоследовательности (пример: 10 пар)
    auto symbols_with_sync = add_sync_sequence(data_symbols, 20);

    // Генерация несущих
    vector<double> cos_wave, sin_wave;
    generate_carriers(cos_wave, sin_wave);

    // Модуляция
    auto modulated_signal = modulate_qam32(symbols_with_sync, cos_wave, sin_wave);

    const static double SNR = 20.0;
    static bool flag = 1;

    if (flag)
    {
        std::cout << "SNR level: " << SNR << " dB" << std::endl;
        flag = 0;
    }
    // Добавление шума
    auto noisy_signal = add_awgn(modulated_signal, SNR);

    writeVectorToFile(noisy_signal, "Output_Signal.txt");

    // Конвертация вектора в строку
    ostringstream oss;
    for (const auto& sample : noisy_signal) {
        oss << sample << '\n';
    }
    output_data = oss.str();

    
    return 0;
}

void QAM32Modulator::writeVectorToFile(const std::vector<double>& vec, const std::string& filename) {
    std::ofstream outFile(filename); // Открываем файл для записи
    if (!outFile) {
        std::cerr << "Ошибка: не удалось открыть файл " << filename << std::endl;
        return;
    }

    // Записываем каждое значение из вектора в файл
    for (const auto& value : vec) {
        outFile << value << '\n'; // Значение + перевод строки
    }

    outFile.close(); // Закрываем файл (не обязательно, но рекомендуется)
}
