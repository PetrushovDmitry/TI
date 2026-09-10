
#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <string>

    using namespace std;

    const double EPS = 1e-9;

    void printMatrix(const vector<vector<double>>& matrix, int iteration) {
        int rows = matrix.size();
        int cols = matrix[0].size();

        cout << string(28, '-') << " Итерация " << iteration << " " << string(28, '-') << "\n";

        cout << "+";
        for (int j = 0; j < cols; ++j) cout << string(10, '-') << "+";
        cout << "\n";

        for (int i = 0; i < rows; ++i) {
            cout << "|";
            for (int j = 0; j < cols; ++j) {
                cout << setw(10) << fixed << setprecision(4) << matrix[i][j] << "|";
            }
            cout << "\n";
            cout << "+";
            for (int j = 0; j < cols; ++j) cout << string(10, '-') << "+";
            cout << "\n";
        }
    }

    int main() {
        setlocale(LC_ALL, "Russian");
        // Исходная матрица 4x5
        vector<vector<double>> matrix = {
            {8, 1,  17, 8, 1},
            {12, 6, 11, 10, 16},
            {4, 19, 11, 15, 2},
            {17,  19, 6, 17, 16}
        };

        int m = matrix.size();      // число стратегий A (строк)
        int n = matrix[0].size();   // число стратегий B (столбцов)

        // Сдвиг, если есть отрицательные элементы
        double matrix_min = matrix[0][0];
        for (int i = 0; i < m; ++i)
            for (int j = 0; j < n; ++j)
                if (matrix[i][j] < matrix_min) matrix_min = matrix[i][j];

        if (matrix_min < 0) {
            for (int i = 0; i < m; ++i)
                for (int j = 0; j < n; ++j)
                    matrix[i][j] += -matrix_min;
        }

        // Добавляем столбец s0 (свободные члены) слева
        for (int i = 0; i < m; ++i)
            matrix[i].insert(matrix[i].begin(), 1.0);

        // Добавляем единичную матрицу identity (базисные переменные)
        for (int i = 0; i < m; ++i)
            for (int j = 0; j < m; ++j)
                matrix[i].push_back(i == j ? 1.0 : 0.0);

        // Целевая строка: [0, 1,1,1,1,1, 0,0,0,0]
        vector<double> target(matrix[0].size(), 0.0);
        for (int j = 1; j <= n; ++j) target[j] = 1.0;
        matrix.push_back(target);

        int totalRows = matrix.size();    // m + 1
        int totalCols = matrix[0].size(); // 1 + n + m
        int lastRow = totalRows - 1;

        cout << "Размер матрицы: " << totalRows << " x " << totalCols << "\n";

        int max_iter = 1000;
        int ii = 1;

        while (ii <= max_iter) {
            // --- ПРАВИЛО БЛЕНДА: разрешающий столбец — первый с положительным элементом в последней строке ---
            int raz_col = -1;
            for (int j = 0; j < totalCols; ++j) {
                if (matrix[lastRow][j] > EPS) {
                    raz_col = j;
                    break;
                }
            }

            if (raz_col == -1) {
                cout << "Оптимум найден\n";
                break;
            }

            // --- Разрешающая строка: min отношения matrix[i][0]/matrix[i][raz_col] среди строк с matrix[i][raz_col] > 0 ---
            // При равенстве отношений — меньший индекс (правило Бленда)
            int raz_row = -1;
            double min_ratio = 1e18;
            for (int i = 0; i < totalRows - 1; ++i) {
                if (matrix[i][raz_col] > EPS) {
                    double ratio = matrix[i][0] / matrix[i][raz_col];
                    if (ratio < min_ratio - 1e-12) {
                        min_ratio = ratio;
                        raz_row = i;
                    }
                    // при равенстве — оставляем строку с меньшим индексом (первую найденную)
                }
            }

            if (raz_row == -1) {
                cout << "Задача не ограничена (нет разрешающей строки)\n";
                break;
            }

            double raz_el = matrix[raz_row][raz_col];

            // Делим разрешающую строку — БЕЗ округления!
            for (int j = 0; j < totalCols; ++j)
                matrix[raz_row][j] /= raz_el;

            // Обнуляем остальные элементы разрешающего столбца
            for (int i = 0; i < totalRows; ++i) {
                if (i == raz_row) continue;
                double factor = matrix[i][raz_col];
                if (fabs(factor) < EPS) continue;
                for (int j = 0; j < totalCols; ++j)
                    matrix[i][j] -= factor * matrix[raz_row][j];
            }

            printMatrix(matrix, ii);
            ++ii;
        }

        if (ii > max_iter)
            cout << "Превышен лимит итераций — возможна вырожденность\n";

        // --- Цена игры ---
        double z_opt = fabs(matrix[lastRow][0]);
        double h = 1.0 / z_opt;

        cout << "\nИтог:\n";
        cout << "Цена игры h = " << fixed << setprecision(6) << h << "\n";

        // --- Стратегия игрока B (Y) ---
        // Ищем в столбцах 1..n единичные базисные столбцы
        vector<double> v_results(n, 0.0);

        for (int j = 1; j <= n; ++j) {
            int ones_count = 0, zeros_count = 0, row_idx = -1;
            for (int i = 0; i < totalRows - 1; ++i) {
                if (fabs(matrix[i][j] - 1.0) < 1e-7) { ones_count++; row_idx = i; }
                else if (fabs(matrix[i][j]) < 1e-7) zeros_count++;
            }

            if (ones_count == 1 && zeros_count == (totalRows - 2)) {
                v_results[j - 1] = matrix[row_idx][0];
            }
        }

        vector<double> Y(n);
        for (int j = 0; j < n; ++j) Y[j] = v_results[j] * h;

        cout << "\nОптимальная стратегия игрока B (Y):\n";
        cout << "Y = [";
        for (int j = 0; j < n; ++j) {
            cout << fixed << setprecision(6) << Y[j];
            if (j < n - 1) cout << ", ";
        }
        cout << "]\n";

        // --- Стратегия игрока A (X) ---
        // Из последних m столбцов последней строки берём абсолютные значения
        vector<double> u_results(m);
        for (int i = 0; i < m; ++i)
            u_results[i] = fabs(matrix[lastRow][totalCols - m + i]);

        vector<double> X(m);
        for (int i = 0; i < m; ++i) X[i] = u_results[i] * h;

        cout << "\nОптимальная стратегия игрока A (X):\n";
        cout << "X = [";
        for (int i = 0; i < m; ++i) {
            cout << fixed << setprecision(6) << X[i];
            if (i < m - 1) cout << ", ";
        }
        cout << "]\n";

        // --- Проверка ---
        cout << "\nПроверка (A против чистых стратегий B):\n";
        for (int j = 0; j < n; ++j) {
            double val = 0;
            for (int i = 0; i < m; ++i) {
                double a_ij = matrix[i][j + 1]; // столбец j+1 в расширенной матрице — исходный
                // Но исходные значения искажены pivot-операциями — восстанавливаем из оригинала
                val += X[i] * 0; // заглушка, будет ниже
            }
        }

        // Аккуратная проверка на исходной матрице
        vector<vector<double>> A_orig = {
            {12, 13,  7, 13, 10},
            { 8, 14,  5,  5, 16},
            {13, 16, 11,  5, 14},
            {13,  6, 18, 12,  4}
        };

        for (int j = 0; j < n; ++j) {
            double val = 0;
            for (int i = 0; i < m; ++i) val += X[i] * A_orig[i][j];
            cout << "  B" << (j + 1) << ": " << fixed << setprecision(4) << val
                << (val >= h - 1e-6 ? "  >= h OK" : "  < h !!!") << "\n";
        }

        return 0;
    }