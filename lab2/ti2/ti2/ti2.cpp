#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <string>
#include <algorithm>

using namespace std;

// ---------- Работа с матрицами ----------

// Умножение вектора (строка) на матрицу: result = u * A
vector<double> vecMulMat(const vector<double>& u, const vector<vector<double>>& A) {
    int n = A[0].size();
    int m = A.size();
    vector<double> result(n, 0.0);
    for (int j = 0; j < n; ++j)
        for (int i = 0; i < m; ++i)
            result[j] += u[i] * A[i][j];
    return result;
}

// Умножение матрицы на вектор (столбец): result = A * u
vector<double> matMulVec(const vector<vector<double>>& A, const vector<double>& u) {
    int m = A.size();
    int n = A[0].size();
    vector<double> result(m, 0.0);
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            result[i] += A[i][j] * u[j];
    return result;
}

// Скалярное произведение
double dot(const vector<double>& a, const vector<double>& b) {
    double s = 0.0;
    for (size_t i = 0; i < a.size(); ++i) s += a[i] * b[i];
    return s;
}

// Обращение матрицы методом Гаусса-Жордана
// Возвращает true, если обратная матрица найдена
bool invertMatrix(const vector<vector<double>>& A_in,
    vector<vector<double>>& A_inv) {
    int n = A_in.size();
    // Создаём расширенную матрицу [A | I]
    vector<vector<double>> aug(n, vector<double>(2 * n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) aug[i][j] = A_in[i][j];
        aug[i][n + i] = 1.0;
    }

    for (int col = 0; col < n; ++col) {
        // Ищем опорный элемент
        int pivot = -1;
        double maxAbs = 0.0;
        for (int row = col; row < n; ++row) {
            if (fabs(aug[row][col]) > maxAbs) {
                maxAbs = fabs(aug[row][col]);
                pivot = row;
            }
        }
        if (maxAbs < 1e-12) return false; // вырожденная

        swap(aug[col], aug[pivot]);

        double div = aug[col][col];
        for (int j = 0; j < 2 * n; ++j) aug[col][j] /= div;

        for (int row = 0; row < n; ++row) {
            if (row == col) continue;
            double factor = aug[row][col];
            if (fabs(factor) < 1e-15) continue;
            for (int j = 0; j < 2 * n; ++j)
                aug[row][j] -= factor * aug[col][j];
        }
    }

    A_inv.assign(n, vector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            A_inv[i][j] = aug[i][n + j];
    return true;
}

// ---------- Основная программа ----------

int main() {
    setlocale(LC_ALL, "Russian");
    // Исходная матрица 3x3
    vector<vector<double>> matrix = {
        { 1, 11, 11},
        { 7,  5,  8},
        {16,  6,  2}
    };

    int m = matrix.size();
    int n = matrix[0].size();

    vector<double> u(m, 1.0); // вектор из единиц

    // ============== АНАЛИТИЧЕСКОЕ РЕШЕНИЕ ==============
    cout << "=== Аналитическое решение ===\n";

    vector<vector<double>> A_inv;
    if (invertMatrix(matrix, A_inv)) {
        // denom = u * A_inv * u^T
        vector<double> uAinv = vecMulMat(u, A_inv); // u * A_inv (строка)
        double denom = dot(uAinv, u);                // (u * A_inv) * u^T

        // x_ = (u * A_inv) / denom
        vector<double> x_(n);
        for (int j = 0; j < n; ++j) x_[j] = uAinv[j] / denom;

        // y_ = (A_inv * u^T) / denom
        vector<double> AinvU = matMulVec(A_inv, u); // A_inv * u
        vector<double> y_(m);
        for (int i = 0; i < m; ++i) y_[i] = AinvU[i] / denom;

        double v = 1.0 / denom;

        cout << fixed << setprecision(4);
        cout << "Стратегия A: [";
        for (int j = 0; j < n; ++j) {
            cout << x_[j];
            if (j < n - 1) cout << " ";
        }
        cout << "]\n";

        cout << "Стратегия B: [";
        for (int i = 0; i < m; ++i) {
            cout << y_[i];
            if (i < m - 1) cout << " ";
        }
        cout << "]\n";

        cout << "Цена игры: " << v << "\n";
    }
    else {
        cout << "Матрица вырожденная — аналитическое решение невозможно\n";
    }

    // ============== МЕТОД БРАУНА-РОБИНСОНА ==============

    int num_iterations = 1000;
    double epsilon_limit = 0.01;

    vector<double> acc_gain_a(m, 0.0); // накопленный выигрыш A по строкам
    vector<double> acc_loss_b(n, 0.0); // накопленный проигрыш B по столбцам

    vector<int> counts_a(m, 0);
    vector<int> counts_b(n, 0);

    int idx_a = 0;
    int idx_b = 0;

    double v_upper_min = -1e18;
    double v_lower_max = 1e18;

    // Для истории шагов (первые 14)
    struct Step {
        int k;
        int a;
        int b;
        vector<double> winA;
        vector<double> loseB;
        double v_low;
        double v_up;
        double eps;
    };
    vector<Step> history;

    vector<double> last_winA, last_loseB;

    double v_lower = 0.0, v_upper = 0.0;
    double epsilon = 0.0;
    int k_final = 0;

    for (int k = 1; k <= num_iterations; ++k) {
        counts_a[idx_a]++;
        counts_b[idx_b]++;

        // acc_gain_a += matrix[:, idx_a]  — столбец idx_a матрицы
        for (int i = 0; i < m; ++i) acc_gain_a[i] += matrix[i][idx_a];

        // acc_loss_b += matrix[idx_b, :]  — строка idx_b матрицы
        for (int j = 0; j < n; ++j) acc_loss_b[j] += matrix[idx_b][j];

        v_lower = *max_element(acc_gain_a.begin(), acc_gain_a.end()) / k;
        v_upper = *min_element(acc_loss_b.begin(), acc_loss_b.end()) / k;

        if (v_upper > v_upper_min) v_upper_min = v_upper;
        if (v_lower < v_lower_max) v_lower_max = v_lower;

        epsilon = fabs(v_lower_max) - fabs(v_upper_min);

        // a, b для записи в историю
        vector<double> a(m), b(n);
        if (last_winA.empty()) {
            for (int i = 0; i < m; ++i) a[i] = matrix[i][idx_a];
            for (int j = 0; j < n; ++j) b[j] = matrix[idx_b][j];
        }
        else {
            for (int i = 0; i < m; ++i) a[i] = matrix[i][idx_a] + last_winA[i];
            for (int j = 0; j < n; ++j) b[j] = matrix[idx_b][j] + last_loseB[j];
        }

        if ((int)history.size() < 14) {
            Step s;
            s.k = k;
            s.a = idx_a + 1;
            s.b = idx_b + 1;
            s.winA = a;
            s.loseB = b;
            s.v_low = round(v_lower * 100) / 100;
            s.v_up = round(v_upper * 100) / 100;
            s.eps = round(epsilon * 100) / 100;
            history.push_back(s);
        }

        last_winA = a;
        last_loseB = b;

        // Выбор следующих индексов
        idx_a = max_element(acc_gain_a.begin(), acc_gain_a.end()) - acc_gain_a.begin();
        idx_b = min_element(acc_loss_b.begin(), acc_loss_b.end()) - acc_loss_b.begin();

        k_final = k;

        if (epsilon <= epsilon_limit && k > 10) break;
    }

    // Оценки
    double price_estimate = (v_lower + v_upper) / 2.0;

    double sum_a = 0.0, sum_b = 0.0;
    for (int i = 0; i < m; ++i) sum_a += counts_a[i];
    for (int j = 0; j < n; ++j) sum_b += counts_b[j];

    vector<double> strat_a(m), strat_b(n);
    for (int i = 0; i < m; ++i) strat_a[i] = counts_a[i] / sum_a;
    for (int j = 0; j < n; ++j) strat_b[j] = counts_b[j] / sum_b;

    cout << "\n=== Метод Брауна-Робинсона ===\n";
    cout << "Итераций: " << k_final << "\n";
    cout << fixed << setprecision(4);
    cout << "Цена игры (нижняя/верхняя): " << v_lower << " / " << v_upper << "\n";
    cout << "Средняя оценка: " << price_estimate << "\n";

    cout << "Стратегия A: [";
    for (int i = 0; i < m; ++i) {
        cout << strat_a[i];
        if (i < m - 1) cout << " ";
    }
    cout << "]\n";

    cout << "Стратегия B: [";
    for (int j = 0; j < n; ++j) {
        cout << strat_b[j];
        if (j < n - 1) cout << " ";
    }
    cout << "]\n";

    cout << "Погрешность: " << epsilon << "\n";

    // ============== ПЕРВЫЕ ШАГИ ==============
    cout << "\n=== Первые шаги ===\n";
    cout << left
        << setw(5) << "k"
        << setw(5) << "A"
        << setw(5) << "B"
        << setw(22) << "win A"
        << setw(22) << "Lose B"
        << setw(8) << "v_low"
        << setw(8) << "v_up"
        << setw(8) << "eps"
        << "\n";

    for (const auto& s : history) {
        string winA = "[";
        for (size_t i = 0; i < s.winA.size(); ++i) {
            winA += to_string((int)s.winA[i]);
            if (i < s.winA.size() - 1) winA += " ";
        }
        winA += "]";

        string loseB = "[";
        for (size_t i = 0; i < s.loseB.size(); ++i) {
            loseB += to_string((int)s.loseB[i]);
            if (i < s.loseB.size() - 1) loseB += " ";
        }
        loseB += "]";

        cout << left
            << setw(5) << s.k
            << setw(5) << ("A" + to_string(s.a))
            << setw(5) << ("B" + to_string(s.b))
            << setw(22) << winA
            << setw(22) << loseB
            << setw(8) << fixed << setprecision(2) << s.v_low
            << setw(8) << s.v_up
            << setw(8) << s.eps
            << "\n";
    }

    return 0;
}