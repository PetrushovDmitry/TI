#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>

using namespace std;

// Ядро игры H(x, y) = a*x^2 + b*y^2 + c*x*y + d*x + e*y
double calculate_H(double x, double y, double a, double b, double c, double d, double e) {
    return a * x * x + b * y * y + c * x * y + d * x + e * y;
}

// ---------- Аналитическое решение ----------

struct AnalyticalResult {
    bool found;
    double x_star;
    double y_star;
    double h;
};

AnalyticalResult solve_analytical(double a, double b, double c, double d, double e) {
    AnalyticalResult res;

    double H_xx = 2 * a;
    double H_yy = 2 * b;

    // Проверка выпукло-вогнутости
    if (H_xx >= 0 || H_yy <= 0) {
        res.found = false;
        res.x_star = 0;
        res.y_star = 0;
        res.h = 0;
        return res;
    }

    // Определитель = 4ab - c^2
    double det = 4 * a * b - c * c;

    // x* = (-2bd + ce) / det
    double x_star = (-2 * b * d + c * e) / det;
    // y* = (-2ae + cd) / det
    double y_star = (-2 * a * e + c * d) / det;
    // Цена игры
    double h = calculate_H(x_star, y_star, a, b, c, d, e);

    res.found = true;
    res.x_star = x_star;
    res.y_star = y_star;
    res.h = h;
    return res;
}

// ---------- Метод Брауна-Робинсона ----------

struct BRResult {
    double price;
    int best_row_idx;
    int best_col_idx;
};

BRResult brown_robinson(const vector<vector<double>>& matrix, int num_iter = 1000) {
    int n = matrix.size();       // строки
    int m = matrix[0].size();    // столбцы

    vector<double> row_sums(n, 0.0);
    vector<double> col_sums(m, 0.0);

    vector<int> row_counts(n, 0);
    vector<int> col_counts(m, 0);

    int current_row = 0;
    vector<double> min_v;
    vector<double> max_v;
    min_v.reserve(num_iter);
    max_v.reserve(num_iter);

    for (int i = 1; i <= num_iter; ++i) {
        // Игрок B прибавляет текущую строку игрока A
        for (int j = 0; j < m; ++j) col_sums[j] += matrix[current_row][j];
        // Выбор минимального проигрыша B
        int current_col = 0;
        for (int j = 1; j < m; ++j)
            if (col_sums[j] < col_sums[current_col]) current_col = j;
        col_counts[current_col]++;

        // Игрок A прибавляет столбец B
        for (int r = 0; r < n; ++r) row_sums[r] += matrix[r][current_col];
        // Выбор максимального выигрыша A
        current_row = 0;
        for (int r = 1; r < n; ++r)
            if (row_sums[r] > row_sums[current_row]) current_row = r;
        row_counts[current_row]++;

        // Нижняя цена игры — min(col_sums)/i
        double min_col = col_sums[0];
        for (int j = 1; j < m; ++j) if (col_sums[j] < min_col) min_col = col_sums[j];
        min_v.push_back(min_col / i);

        // Верхняя цена игры — max(row_sums)/i
        double max_row = row_sums[0];
        for (int r = 1; r < n; ++r) if (row_sums[r] > max_row) max_row = row_sums[r];
        max_v.push_back(max_row / i);
    }

    // price = (max(min_v) + min(max_v)) / 2
    double max_of_min = *max_element(min_v.begin(), min_v.end());
    double min_of_max = *min_element(max_v.begin(), max_v.end());
    double price = (max_of_min + min_of_max) / 2.0;

    int best_row_idx = 0;
    for (int r = 1; r < n; ++r) if (row_counts[r] > row_counts[best_row_idx]) best_row_idx = r;

    int best_col_idx = 0;
    for (int j = 1; j < m; ++j) if (col_counts[j] > col_counts[best_col_idx]) best_col_idx = j;

    return { price, best_row_idx, best_col_idx };
}

// ---------- Численное решение ----------

// Вспомогательная функция: вывод матрицы в строку
string matrixToString(const vector<vector<double>>& matrix, int precision = 3) {
    ostringstream oss;
    oss << fixed << setprecision(precision);
    oss << "[";
    for (size_t i = 0; i < matrix.size(); ++i) {
        oss << "[";
        for (size_t j = 0; j < matrix[i].size(); ++j) {
            oss << matrix[i][j];
            if (j < matrix[i].size() - 1) oss << " ";
        }
        oss << "]";
        if (i < matrix.size() - 1) oss << "\n ";
    }
    oss << "]";
    return oss.str();
}

// Возвращает пару: цена игры и лог
pair<double, vector<string>> solve_numerical(int N, double a, double b, double c, double d, double e) {
    vector<string> log;

    // Сетка из N+1 точек от 0 до 1
    vector<double> steps(N + 1);
    for (int i = 0; i <= N; ++i) steps[i] = (double)i / N;

    // Матрица (N+1) x (N+1)
    vector<vector<double>> matrix(N + 1, vector<double>(N + 1, 0.0));
    for (int i = 0; i <= N; ++i)
        for (int j = 0; j <= N; ++j)
            matrix[i][j] = calculate_H(steps[i], steps[j], a, b, c, d, e);

    // Проверка на седловую точку
    // row_mins[i] = min по строке i
    vector<double> row_mins(N + 1);
    for (int i = 0; i <= N; ++i) {
        double mn = matrix[i][0];
        for (int j = 1; j <= N; ++j) if (matrix[i][j] < mn) mn = matrix[i][j];
        row_mins[i] = mn;
    }
    // col_maxs[j] = max по столбцу j
    vector<double> col_maxs(N + 1);
    for (int j = 0; j <= N; ++j) {
        double mx = matrix[0][j];
        for (int i = 1; i <= N; ++i) if (matrix[i][j] > mx) mx = matrix[i][j];
        col_maxs[j] = mx;
    }

    // lower_price = max(row_mins)
    double lower_price = row_mins[0];
    int lm_idx = 0;
    for (int i = 1; i <= N; ++i) if (row_mins[i] > lower_price) { lower_price = row_mins[i]; lm_idx = i; }

    // upper_price = min(col_maxs)
    double upper_price = col_maxs[0];
    int um_idx = 0;
    for (int j = 1; j <= N; ++j) if (col_maxs[j] < upper_price) { upper_price = col_maxs[j]; um_idx = j; }

    // Формируем лог
    {
        ostringstream oss;
        oss << "N = " << N;
        log.push_back(oss.str());
    }
    cout << "N = " << N << "\n";

    log.push_back(matrixToString(matrix, 3));

    if (fabs(lower_price - upper_price) < 1e-12) {
        // Седловая точка
        int x_idx = lm_idx;
        int y_idx = um_idx;
        double x_val = steps[x_idx];
        double y_val = steps[y_idx];

        ostringstream oss;
        oss << fixed << setprecision(3)
            << "Есть седловая точка: x=" << x_val
            << ", y=" << y_val
            << ", H=" << setprecision(4) << lower_price;
        log.push_back(oss.str());
        cout << oss.str() << "\n";

        return { lower_price, log };
    }
    else {
        // Метод Брауна-Робинсона
        BRResult br = brown_robinson(matrix);
        double x_val = steps[br.best_row_idx];
        double y_val = steps[br.best_col_idx];

        ostringstream oss;
        oss << fixed << setprecision(3)
            << "Седловой точки нет, метод Брауна-Робинсона: x=" << x_val
            << ", y=" << y_val
            << ", H=" << setprecision(4) << br.price;
        log.push_back(oss.str());
        cout << oss.str() << "\n";

        return { br.price, log };
    }
}

// ---------- Основная программа ----------

int main() {
    setlocale(LC_ALL, "Russian");
    double a = -3;
    double b = 9;
    double c = 18;
    double d = -9.0 / 5.0;
    double e = -81.0 / 5.0;

    // Аналитическое решение
    AnalyticalResult ar = solve_analytical(a, b, c, d, e);

    double h_analytical = 0.0;
    if (ar.found) {
        cout << "--- Аналитическое решение ---\n";
        cout << fixed << setprecision(3);
        cout << "x* = " << ar.x_star << "\n";
        cout << "y* = " << ar.y_star << "\n";
        cout << "h* = " << ar.h << "\n\n";
        h_analytical = ar.h;
    }
    else {
        cout << "\nНе удалось найти аналитическое решение.\n";
    }

    // Решение при различном шаге сетки
    ofstream file("game_log.txt");
    if (!file.is_open()) {
        cerr << "Не удалось открыть файл game_log.txt\n";
        return 1;
    }

    for (int n = 2; n <= 10; ++n) {
        auto result = solve_numerical(n, a, b, c, d, e);
        double h_num = result.first;
        vector<string> iteration_log = result.second;

        double error = fabs(h_analytical - h_num);

        for (const auto& line : iteration_log) file << line << "\n";
        file << fixed << setprecision(6) << "Погрешность: " << error << "\n";
        file << string(40, '-') << "\n\n";
    }

    file.close();
    cout << "\nЛог записан в файл game_log.txt\n";

    return 0;
}