#include <iostream>
#include <vector>
#include <random>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <numeric>

using namespace std;

// =========================
// Генератор случайных чисел
// =========================
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<double> dist01(0.0, 1.0);

double rand01() { return dist01(gen); }
double randRange(double a, double b) {
    uniform_real_distribution<double> d(a, b);
    return d(gen);
}
int randInt(int a, int b) {
    uniform_int_distribution<int> d(a, b);
    return d(gen);
}

// =========================
// Красивый вывод таблицы
// =========================
void printTable(const vector<string>& headers,
    const vector<vector<string>>& rows,
    ostream& out) {
    int cols = headers.size();
    vector<int> widths(cols);
    for (int j = 0; j < cols; ++j) widths[j] = (int)headers[j].size();
    for (const auto& row : rows)
        for (int j = 0; j < cols; ++j)
            if ((int)row[j].size() > widths[j]) widths[j] = (int)row[j].size();

    auto printLine = [&]() {
        out << "+";
        for (int j = 0; j < cols; ++j) out << string(widths[j] + 2, '-') << "+";
        out << "\n";
        };

    printLine();
    out << "|";
    for (int j = 0; j < cols; ++j)
        out << " " << setw(widths[j]) << left << headers[j] << " |";
    out << "\n";
    printLine();

    for (const auto& row : rows) {
        out << "|";
        for (int j = 0; j < cols; ++j)
            out << " " << setw(widths[j]) << left << row[j] << " |";
        out << "\n";
    }
    printLine();
}

// =========================
// Вспомогательные функции форматирования
// =========================
string fmt(double x, int prec = 4) {
    ostringstream oss;
    oss << fixed << setprecision(prec) << x;
    return oss.str();
}

string formatOpinions(const vector<double>& opinions) {
    ostringstream oss;
    oss << "(";
    for (size_t i = 0; i < opinions.size(); ++i) {
        oss << fixed << setprecision(3) << opinions[i];
        if (i < opinions.size() - 1) oss << " ";
    }
    oss << ")";
    return oss.str();
}

// =========================
// Случайная матрица n x n
// =========================
vector<vector<double>> create_random_matrix(int n) {
    vector<vector<double>> matrix(n, vector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            matrix[i][j] = rand01();
    return matrix;
}

// =========================
// Нормализация строк
// =========================
vector<vector<double>> normalize_matrix_rows(const vector<vector<double>>& matrix) {
    int n = matrix.size();
    vector<vector<double>> normalized(n, vector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        double row_sum = 0.0;
        for (int j = 0; j < n; ++j) row_sum += matrix[i][j];
        for (int j = 0; j < n; ++j)
            normalized[i][j] = matrix[i][j] / row_sum;
    }
    return normalized;
}

// =========================
// Начальные мнения
// =========================
vector<double> generate_initial_opinions(int n, double min_val, double max_val) {
    vector<double> opinions(n);
    for (int i = 0; i < n; ++i)
        opinions[i] = randRange(min_val, max_val);
    return opinions;
}

// =========================
// Итерация мнений
// =========================
struct IterateResult {
    vector<double> final_opinions;
    int iterations;
    vector<vector<string>> log; // [iter, change, opinions_string]
};

IterateResult iterate_opinions(const vector<vector<double>>& matrix_trust,
    vector<double> opinions,
    double epsilon) {
    IterateResult result;
    int n = matrix_trust.size();
    int iteration = 0;

    vector<double> old_opinions = opinions;

    while (true) {
        vector<double> new_opinions(n, 0.0);
        for (int i = 0; i < n; ++i) {
            double val = 0.0;
            for (int j = 0; j < n; ++j)
                val += matrix_trust[i][j] * old_opinions[j];
            new_opinions[i] = val;
        }

        double max_change = 0.0;
        for (int i = 0; i < n; ++i) {
            double change = fabs(new_opinions[i] - old_opinions[i]);
            if (change > max_change) max_change = change;
        }

        iteration++;

        result.log.push_back({
            to_string(iteration),
            fmt(max_change, 8),
            formatOpinions(new_opinions)
            });

        if (max_change < epsilon) {
            result.final_opinions = new_opinions;
            result.iterations = iteration;
            return result;
        }

        old_opinions = new_opinions;
    }
}

// =========================
// Управляющие агенты
// =========================
struct InfluenceResult {
    vector<int> F;
    vector<int> S;
    vector<double> initial_opinions;
};

InfluenceResult create_influence_agents(int n, int f_count, int s_count,
    double u_value, double v_value,
    double neutral_min, double neutral_max) {
    InfluenceResult res;

    vector<int> all_agents(n);
    iota(all_agents.begin(), all_agents.end(), 0);

    // Случайно выбираем F
    shuffle(all_agents.begin(), all_agents.end(), gen);
    res.F.assign(all_agents.begin(), all_agents.begin() + f_count);

    // Из оставшихся выбираем S
    vector<int> remaining;
    for (int a : all_agents) {
        bool inF = false;
        for (int f : res.F) if (f == a) { inF = true; break; }
        if (!inF) remaining.push_back(a);
    }
    shuffle(remaining.begin(), remaining.end(), gen);
    res.S.assign(remaining.begin(), remaining.begin() + s_count);

    // Начальные мнения
    res.initial_opinions.resize(n);
    for (int agent = 0; agent < n; ++agent) {
        bool inF = false, inS = false;
        for (int f : res.F) if (f == agent) { inF = true; break; }
        for (int s : res.S) if (s == agent) { inS = true; break; }

        if (inF) res.initial_opinions[agent] = u_value;
        else if (inS) res.initial_opinions[agent] = v_value;
        else res.initial_opinions[agent] = randRange(neutral_min, neutral_max);
    }

    return res;
}

// =========================
// Печать матрицы
// =========================
string matrix_to_table(const vector<vector<double>>& matrix, const string& title) {
    int n = matrix.size();

    vector<string> headers;
    headers.push_back("");
    for (int j = 0; j < n; ++j) headers.push_back("A" + to_string(j + 1));

    vector<vector<string>> rows;
    for (int i = 0; i < n; ++i) {
        vector<string> row;
        row.push_back("A" + to_string(i + 1));
        for (int j = 0; j < n; ++j)
            row.push_back(fmt(matrix[i][j], 4));
        rows.push_back(row);
    }

    ostringstream out;
    out << title << "\n\n";
    printTable(headers, rows, out);
    return out.str();
}

// =========================
// Печать мнений
// =========================
string opinions_table(const vector<double>& opinions, const string& title) {
    vector<vector<string>> rows;
    for (size_t i = 0; i < opinions.size(); ++i) {
        rows.push_back({
            "A" + to_string(i + 1),
            fmt(opinions[i], 4)
            });
    }

    ostringstream out;
    out << title << "\n\n";
    printTable({ "Агент", "Мнение" }, rows, out);
    return out.str();
}

// =========================
// Печать лога итераций
// =========================
string iterations_to_table(const vector<vector<string>>& log) {
    ostringstream out;
    printTable({ "Итерация", "Изменение", "Мнения" }, log, out);
    return out.str();
}

// =========================
// main
// =========================
int main() {
    setlocale(LC_ALL, "Russian");
    int n = 10;
    vector<string> full_log;

    // 1. Случайная матрица
    auto random_matrix = create_random_matrix(n);
    string random_matrix_text = matrix_to_table(random_matrix, "Случайная матрица");
    cout << random_matrix_text << "\n";
    full_log.push_back(random_matrix_text);
    full_log.push_back("\n");

    // 2. Нормализованная матрица
    auto A = normalize_matrix_rows(random_matrix);
    string normalized_matrix_text = matrix_to_table(A, "Стохастическая матрица доверия");
    cout << normalized_matrix_text << "\n";
    full_log.push_back(normalized_matrix_text);
    full_log.push_back("\n");

    // 3. Начальные мнения
    auto x0 = generate_initial_opinions(n, 1, 20);
    string initial_opinions_text = opinions_table(x0, "Начальные мнения агентов");
    cout << initial_opinions_text << "\n";
    full_log.push_back(initial_opinions_text);
    full_log.push_back("\n");

    // 4. Итерации без влияния
    double epsilon = 1e-6;
    auto iter_result = iterate_opinions(A, x0, epsilon);

    string iterations_text = iterations_to_table(iter_result.log);
    cout << "\nИзменение мнений\n\n" << iterations_text << "\n";
    full_log.push_back("Изменение мнений");
    full_log.push_back(iterations_text);
    full_log.push_back("\n");

    // 5. Результирующие мнения
    string final_opinions_text = opinions_table(iter_result.final_opinions,
        "Результирующие мнения");
    cout << final_opinions_text << "\n";
    full_log.push_back(final_opinions_text);
    full_log.push_back("\n");

    // 6. Влияющие агенты
    int f_count = randInt(1, 3);
    int s_count = randInt(1, 3);
    double u_value = randRange(0, 100);
    double v_value = randRange(-100, 0);

    auto influence = create_influence_agents(n, f_count, s_count,
        u_value, v_value, 1, 20);

    ostringstream control_info_ss;
    control_info_ss << "Агенты первого игрока F: [";
    for (size_t i = 0; i < influence.F.size(); ++i) {
        control_info_ss << (influence.F[i] + 1);
        if (i + 1 < influence.F.size()) control_info_ss << ", ";
    }
    control_info_ss << "]\n";
    control_info_ss << "Мнение первого игрока: " << fmt(u_value, 4) << "\n";
    control_info_ss << "Агенты второго игрока S: [";
    for (size_t i = 0; i < influence.S.size(); ++i) {
        control_info_ss << (influence.S[i] + 1);
        if (i + 1 < influence.S.size()) control_info_ss << ", ";
    }
    control_info_ss << "]\n";
    control_info_ss << "Мнение второго игрока: " << fmt(v_value, 4);
    string control_text = control_info_ss.str();

    cout << control_text << "\n";
    full_log.push_back(control_text);
    full_log.push_back("\n");

    // 7. Начальные мнения с влиянием
    string control_opinions_text = opinions_table(influence.initial_opinions,
        "Начальные мнения с влиянием");
    cout << control_opinions_text << "\n";
    full_log.push_back(control_opinions_text);
    full_log.push_back("\n");

    // 8. Итерации с влиянием
    auto iter_result2 = iterate_opinions(A, influence.initial_opinions, epsilon);
    string iterations_text_2 = iterations_to_table(iter_result2.log);
    cout << "\nИзменение мнений с влиянием\n\n" << iterations_text_2 << "\n";
    full_log.push_back("Изменение мнений с влиянием");
    full_log.push_back(iterations_text_2);
    full_log.push_back("\n");

    // 9. Результирующие мнения с влиянием
    string final_control_text = opinions_table(iter_result2.final_opinions,
        "Результирующие мнения с влиянием");
    cout << final_control_text << "\n";
    full_log.push_back(final_control_text);

    // 10. Запись в файл
    ofstream file("results.txt");
    if (!file.is_open()) {
        cerr << "Не удалось открыть results.txt\n";
        return 1;
    }

    for (const auto& item : full_log) {
        file << item << "\n";
    }

    file.close();

    cout << "\nРезультаты сохранены в results.txt\n";

    return 0;
}