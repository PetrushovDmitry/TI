#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <numeric>

using namespace std;

// Глобальные данные
const int N = 4;
vector<int> players_list = { 1, 2, 3, 4 };

map<vector<int>, double> v_table;

// Все коалиции (в виде отсортированных векторов)
vector<vector<int>> all_coalitions;

// Получение v(S)
double get_v(const vector<int>& coalition) {
    vector<int> key = coalition;
    sort(key.begin(), key.end());
    auto it = v_table.find(key);
    if (it == v_table.end()) return 0;
    return it->second;
}

// Красивый вывод коалиции
string format_coalition(const vector<int>& c) {
    if (c.empty()) return "{}";
    string s = "{";
    for (size_t i = 0; i < c.size(); ++i) {
        s += to_string(c[i]);
        if (i < c.size() - 1) s += ", ";
    }
    s += "}";
    return s;
}

// Вспомогательные операции с коалициями
vector<int> set_union_vec(const vector<int>& a, const vector<int>& b) {
    set<int> s(a.begin(), a.end());
    s.insert(b.begin(), b.end());
    return vector<int>(s.begin(), s.end());
}

vector<int> set_intersection_vec(const vector<int>& a, const vector<int>& b) {
    set<int> sa(a.begin(), a.end());
    vector<int> result;
    for (int x : b) if (sa.count(x)) result.push_back(x);
    return result;
}

bool is_disjoint(const vector<int>& a, const vector<int>& b) {
    set<int> sa(a.begin(), a.end());
    for (int x : b) if (sa.count(x)) return false;
    return true;
}

// Красивый вывод таблицы
// col_widths — желаемая ширина столбцов
void printTable(const vector<string>& headers,
    const vector<vector<string>>& rows,
    ostream& out) {
    int cols = headers.size();
    vector<int> widths(cols);
    for (int j = 0; j < cols; ++j) widths[j] = (int)headers[j].size();
    for (const auto& row : rows)
        for (int j = 0; j < cols; ++j)
            if ((int)row[j].size() > widths[j]) widths[j] = (int)row[j].size();

    // верхняя граница
    auto printLine = [&]() {
        out << "+";
        for (int j = 0; j < cols; ++j) {
            out << string(widths[j] + 2, '-') << "+";
        }
        out << "\n";
        };

    printLine();
    // заголовки
    out << "|";
    for (int j = 0; j < cols; ++j) {
        out << " " << setw(widths[j]) << left << headers[j] << " |";
    }
    out << "\n";
    printLine();

    // строки
    for (const auto& row : rows) {
        out << "|";
        for (int j = 0; j < cols; ++j) {
            out << " " << setw(widths[j]) << left << row[j] << " |";
        }
        out << "\n";
    }
    printLine();
}

// Проверка супераддитивности
string check_superadditivity() {
    vector<vector<string>> rows;
    bool is_super = true;

    for (const auto& s : all_coalitions) {
        for (const auto& t : all_coalitions) {
            if (is_disjoint(s, t)) {
                double v_s = get_v(s);
                double v_t = get_v(t);
                vector<int> u = set_union_vec(s, t);
                double v_union = get_v(u);
                double sum_v = v_s + v_t;

                string status = (v_union >= sum_v) ? "+" : "-";
                if (v_union < sum_v) is_super = false;

                auto fmt = [](double x) {
                    ostringstream oss;
                    oss << setprecision(6) << x;
                    return oss.str();
                    };

                rows.push_back({
                    format_coalition(s),
                    fmt(v_s),
                    format_coalition(t),
                    fmt(v_t),
                    fmt(sum_v),
                    fmt(v_union),
                    status
                    });
            }
        }
    }

    vector<string> headers = { "S", "v(S)", "T", "v(T)", "v(S)+v(T)", "v(S∪T)", "Статус" };

    ostringstream out;
    out << "\nПРОВЕРКА НА СУПЕРАДДИТИВНОСТЬ\n\n";
    printTable(headers, rows, out);
    out << "\n";
    if (is_super) out << "Игра супераддитивна";
    else          out << "Игра НЕ супераддитивна";
    return out.str();
}

// Проверка выпуклости
string check_convexity() {
    vector<vector<string>> rows;
    bool is_convex = true;

    auto fmt = [](double x) {
        ostringstream oss;
        oss << setprecision(6) << x;
        return oss.str();
        };

    for (const auto& s : all_coalitions) {
        for (const auto& t : all_coalitions) {
            double v_s = get_v(s);
            double v_t = get_v(t);
            vector<int> u = set_union_vec(s, t);
            vector<int> inter = set_intersection_vec(s, t);
            double v_union = get_v(u);
            double v_intersection = get_v(inter);

            double left = v_union + v_intersection;
            double right = v_s + v_t;

            string status = (left >= right) ? "+" : "-";
            if (left < right) is_convex = false;

            rows.push_back({
                format_coalition(s),
                fmt(v_s),
                format_coalition(t),
                fmt(v_t),
                fmt(v_intersection),
                fmt(v_union),
                fmt(right),
                fmt(left),
                status
                });
        }
    }

    vector<string> headers = {
        "S", "v(S)", "T", "v(T)", "v(S∩T)", "v(S∪T)",
        "v(S)+v(T)", "v(S∪T)+v(S∩T)", "Статус"
    };

    ostringstream out;
    out << "\n\nПРОВЕРКА НА ВЫПУКЛОСТЬ\n\n";
    printTable(headers, rows, out);
    out << "\n";
    if (is_convex) out << "Игра выпуклая";
    else           out << "Игра НЕ выпуклая";
    return out.str();
}

// Факториал
long long factorial(int n) {
    long long r = 1;
    for (int i = 2; i <= n; ++i) r *= i;
    return r;
}
// Вектор Шепли
vector<double> shapley_value() {
    vector<double> shapley(N, 0.0);

    for (int i = 1; i <= N; ++i) {
        int idx = i - 1;

        for (const auto& coalition : all_coalitions) {
            // i должна входить в коалицию
            bool has_i = false;
            for (int x : coalition) if (x == i) { has_i = true; break; }
            if (!has_i) continue;

            int s_size = coalition.size();

            // coalition без i
            vector<int> without_i;
            for (int x : coalition) if (x != i) without_i.push_back(x);

            double contribution = get_v(coalition) - get_v(without_i);

            double weight = (double)(factorial(s_size - 1) * factorial(N - s_size))
                / (double)factorial(N);

            shapley[idx] += weight * contribution;
        }
    }

    return shapley;
}

// main
int main() {
    setlocale(LC_ALL, "Russian");
    v_table = {
        {{1}, 4},
        {{2}, 1},
        {{3}, 3},
        {{4}, 1},

        {{1, 2}, 6},
        {{1, 3}, 8},
        {{1, 4}, 6},

        {{2, 3}, 5},
        {{2, 4}, 3},
        {{3, 4}, 5},

        {{1, 2, 3}, 9},
        {{1, 2, 4}, 8},
        {{1, 3, 4}, 10},
        {{2, 3, 4}, 7},

        {{1, 2, 3, 4}, 11}
    };

    // Все коалиции: подмножества {1,2,3,4} размером от 1 до N
    for (int mask = 1; mask < (1 << N); ++mask) {
        vector<int> coalition;
        for (int i = 0; i < N; ++i) {
            if (mask & (1 << i)) coalition.push_back(i + 1);
        }
        all_coalitions.push_back(coalition);
    }

    // Проверки
    string super_text = check_superadditivity();
    string convex_text = check_convexity();

    cout << super_text << "\n";
    cout << convex_text << "\n";

    // Вектор Шепли
    vector<double> result = shapley_value();

    vector<vector<string>> shapley_rows;
    for (int i = 0; i < N; ++i) {
        ostringstream oss;
        oss << fixed << setprecision(4) << result[i];
        shapley_rows.push_back({ "x" + to_string(i + 1), oss.str() });
    }

    ostringstream shapley_ss;
    shapley_ss << "\n\nВЕКТОР ШЕПЛИ\n\n";
    printTable({ "Игрок", "Значение" }, shapley_rows, shapley_ss);

    string shapley_text = shapley_ss.str();
    cout << shapley_text << "\n";

    // Групповая рационализация
    double sum_shapley = accumulate(result.begin(), result.end(), 0.0);
    double v_N = get_v({ 1, 2, 3, 4 });

    ostringstream group_ss;
    group_ss << fixed << setprecision(4) << sum_shapley;
    ostringstream vN_ss;
    vN_ss << fixed << setprecision(4) << v_N;

    ostringstream group_text_ss;
    group_text_ss << "\n\nГРУППОВАЯ РАЦИОНАЛИЗАЦИЯ\n\n";
    printTable({ "Сумма Шепли", "v(N)" },
        { {group_ss.str(), vN_ss.str()} },
        group_text_ss);
    string group_text = group_text_ss.str();
    cout << group_text << "\n";

    // Индивидуальная рационализация
    vector<vector<string>> indiv_rows;
    for (int i = 0; i < N; ++i) {
        int player = i + 1;
        double solo = get_v({ player });

        string status = (result[i] >= solo) ? "+" : "-";

        ostringstream oss_sh, oss_solo;
        oss_sh << fixed << setprecision(4) << result[i];
        oss_solo << fixed << setprecision(4) << solo;

        indiv_rows.push_back({
            to_string(player),
            oss_sh.str(),
            oss_solo.str(),
            status
            });
    }

    ostringstream indiv_ss;
    indiv_ss << "\n\nИНДИВИДУАЛЬНАЯ РАЦИОНАЛИЗАЦИЯ\n\n";
    printTable({ "Игрок", "Шепли", "v({i})", "Статус" }, indiv_rows, indiv_ss);

    string individual_text = indiv_ss.str();
    cout << individual_text << "\n";

    // Сохранение в файл
    ofstream file("result.txt");
    if (!file.is_open()) {
        cerr << "Не удалось открыть result.txt\n";
        return 1;
    }

    file << super_text << "\n\n";
    file << convex_text << "\n\n";
    file << shapley_text << "\n\n";
    file << group_text << "\n\n";
    file << individual_text;

    file.close();

    cout << "\nРезультат сохранён в result.txt\n";

    return 0;
}