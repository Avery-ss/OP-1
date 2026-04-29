#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cmath>

using namespace std;

vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, last - first + 1);
}

int main() {
    ifstream fin("input.txt");
    if (!fin.is_open()) {
        cerr << "Ошибка: не удалось открыть input.txt" << endl;
        return 1;
    }
    
    ofstream fout("output.txt");
    if (!fout.is_open()) {
        cerr << "Ошибка: не удалось открыть output.txt" << endl;
        return 1;
    }
    
    fout << fixed;
    fout.precision(1);
    
    string line;
    if (!getline(fin, line)) {
        cerr << "Ошибка: пустой входной файл" << endl;
        return 1;
    }
    
    line = trim(line);
    istringstream first(line);
    
    int n;
    if (!(first >> n) || n <= 0) {
        cerr << "Ошибка: некорректное количество участников" << endl;
        return 1;
    }
    
    vector<string> names;
    map<string, double> spent;
    map<string, double> owe;
    
    string name;
    while (first >> name) {
        names.push_back(name);
        spent[name] = 0.0;
        owe[name] = 0.0;
    }
    
    if (names.size() != n) {
        cerr << "Ошибка: количество имён не совпадает с заявленным" << endl;
        return 1;
    }
    
    while (getline(fin, line)) {
        line = trim(line);
        if (line.empty()) continue;
        
        size_t pos = line.find('/');
        string left, right;
        
        if (pos != string::npos) {
            left = trim(line.substr(0, pos));
            right = trim(line.substr(pos + 1));
        } else {
            left = line;
        }
        
        istringstream ss(left);
        string who;
        double sum;
        
        if (!(ss >> who >> sum)) {
            cerr << "Ошибка: неверный формат траты в строке: " << line << endl;
            continue;
        }
        
        if (spent.find(who) == spent.end()) {
            cerr << "Ошибка: неизвестный участник '" << who << "'" << endl;
            continue;
        }
        
        set<string> skip;
        if (!right.empty()) {
            vector<string> parts = split(right, ',');
            for (const auto& p : parts) {
                string trimmed = trim(p);
                if (spent.find(trimmed) == spent.end()) {
                    cerr << "Ошибка: неизвестный участник '" << trimmed << "' в списке исключённых" << endl;
                }
                skip.insert(trimmed);
            }
        }
        
        vector<string> share;
        for (const auto& p : names) {
            if (skip.find(p) == skip.end()) {
                share.push_back(p);
            }
        }
        
        if (share.empty()) {
            cerr << "Ошибка: все участники исключены в строке: " << line << endl;
            continue;
        }
        
        double part = sum / share.size();
        spent[who] += sum;
        
        for (const auto& p : share) {
            owe[p] += part;
        }
    }
    
    fin.close();
    
    map<string, double> diff;
    for (const auto& p : names) {
        diff[p] = spent[p] - owe[p];
    }
    
    for (const auto& p : names) {
        fout << p << " " << round(spent[p] * 10) / 10 << " " << round(owe[p] * 10) / 10 << endl;
    }
    
    vector<pair<string, double>> debt, credit;
    
    for (const auto& p : names) {
        if (diff[p] < -0.01) {
            debt.push_back({p, -diff[p]});
        } else if (diff[p] > 0.01) {
            credit.push_back({p, diff[p]});
        }
    }
    
    sort(debt.begin(), debt.end(), 
         [](const pair<string, double>& a, const pair<string, double>& b) {
             return a.second > b.second;
         });
    sort(credit.begin(), credit.end(), 
         [](const pair<string, double>& a, const pair<string, double>& b) {
             return a.second > b.second;
         });
    
    while (!debt.empty() && !credit.empty()) {
        auto& d = debt.back();
        auto& c = credit.back();
        
        double pay = min(d.second, c.second);
        
        if (pay > 0.01) {
            fout << d.first << " " << round(pay * 10) / 10 << " " << c.first << endl;
        }
        
        d.second -= pay;
        c.second -= pay;
        
        if (d.second < 0.01) debt.pop_back();
        if (c.second < 0.01) credit.pop_back();
    }
    
    fout.close();
    return 0;
}