// Algoritmos de ordenamiento en C++ (para OnlineGDB, elegir lenguaje C++)
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
using namespace std;

// ---------- a) Bubble sort ----------
void bubbleSort(vector<double>& a) {
    int n = a.size();
    for (int i = 0; i < n - 1; i++) {
        bool swapped = false;
        for (int j = 0; j < n - 1 - i; j++) {
            if (a[j] > a[j + 1]) {
                swap(a[j], a[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) break;
    }
}

// ---------- b) Mergesort ----------
void merge(vector<double>& a, vector<double>& tmp, int l, int m, int r) {
    int i = l, j = m + 1, k = l;
    while (i <= m && j <= r) tmp[k++] = (a[i] <= a[j]) ? a[i++] : a[j++];
    while (i <= m) tmp[k++] = a[i++];
    while (j <= r) tmp[k++] = a[j++];
    for (k = l; k <= r; k++) a[k] = tmp[k];
}
void mergeRec(vector<double>& a, vector<double>& tmp, int l, int r) {
    if (l >= r) return;
    int m = l + (r - l) / 2;
    mergeRec(a, tmp, l, m);
    mergeRec(a, tmp, m + 1, r);
    merge(a, tmp, l, m, r);
}
void mergeSort(vector<double>& a) {
    if (a.size() < 2) return;
    vector<double> tmp(a.size());
    mergeRec(a, tmp, 0, a.size() - 1);
}

// ---------- c) Tree sort (árbol binario de búsqueda) ----------
struct Node {
    double val;
    Node *left, *right;
    Node(double v) : val(v), left(nullptr), right(nullptr) {}
};
void treeSort(vector<double>& a) {
    if (a.empty()) return;
    Node* root = new Node(a[0]);
    // Inserción iterativa (evita desbordar la pila)
    for (size_t i = 1; i < a.size(); i++) {
        Node* cur = root;
        while (true) {
            if (a[i] < cur->val) {
                if (!cur->left) { cur->left = new Node(a[i]); break; }
                cur = cur->left;
            } else {
                if (!cur->right) { cur->right = new Node(a[i]); break; }
                cur = cur->right;
            }
        }
    }
    // Recorrido inorden iterativo + liberación de memoria
    vector<Node*> stack;
    vector<Node*> all;
    Node* cur = root;
    size_t idx = 0;
    while (cur || !stack.empty()) {
        while (cur) { stack.push_back(cur); cur = cur->left; }
        cur = stack.back(); stack.pop_back();
        a[idx++] = cur->val;
        all.push_back(cur);
        cur = cur->right;
    }
    for (Node* n : all) delete n;
}

// ---------- d) Heapsort ----------
void heapify(vector<double>& a, int n, int i) {
    while (true) {
        int largest = i, l = 2 * i + 1, r = 2 * i + 2;
        if (l < n && a[l] > a[largest]) largest = l;
        if (r < n && a[r] > a[largest]) largest = r;
        if (largest == i) break;
        swap(a[i], a[largest]);
        i = largest;
    }
}
void heapSort(vector<double>& a) {
    int n = a.size();
    for (int i = n / 2 - 1; i >= 0; i--) heapify(a, n, i);
    for (int i = n - 1; i > 0; i--) {
        swap(a[0], a[i]);
        heapify(a, i, 0);
    }
}

// ---------- e) Counting sort para números reales ----------
// Counting sort solo funciona con enteros, así que se discretiza:
// clave = round((x - min) * 10^decimales). Se ordena con precisión de 'decimales'.
void countingSort(vector<double>& a, int decimales = 2) {
    if (a.empty()) return;
    double factor = pow(10.0, decimales);
    double mn = *min_element(a.begin(), a.end());
    double mx = *max_element(a.begin(), a.end());
    long long rango = (long long)llround((mx - mn) * factor) + 1;
    vector<int> count(rango, 0);
    for (double x : a) count[(long long)llround((x - mn) * factor)]++;
    size_t idx = 0;
    for (long long k = 0; k < rango; k++)
        for (int c = 0; c < count[k]; c++)
            a[idx++] = mn + k / factor;   // valor reconstruido (redondeado)
}

// ======================= BENCHMARK (tiempo y memoria) =======================
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <chrono>
#include <random>
#include <fstream>
#include <cstdio>

struct Algo {
    string nombre;
    void (*fn)(vector<double>&);
    long long maxN;   
};

struct Resultado { double seg; long baseKB; long picoKB; int ok; };

long rssActualKB() {                       
    long paginas = 0, rss = 0;
    ifstream f("/proc/self/statm");
    f >> paginas >> rss;
    return rss * sysconf(_SC_PAGESIZE) / 1024;
}


bool medir(const Algo& alg, long long n, Resultado& res) {
    int fd[2];
    if (pipe(fd) != 0) return false;
    pid_t pid = fork();
    if (pid == 0) {                        
        close(fd[0]);
        mt19937_64 gen(12345);
        uniform_int_distribution<int> dist(0, 99999);
        vector<double> a(n);
        for (long long i = 0; i < n; i++) a[i] = dist(gen) / 100.0;  
        Resultado r;
        r.baseKB = rssActualKB();          
        auto t0 = chrono::steady_clock::now();
        alg.fn(a);
        auto t1 = chrono::steady_clock::now();
        r.seg = chrono::duration<double>(t1 - t0).count();
        struct rusage u; getrusage(RUSAGE_SELF, &u);
        r.picoKB = u.ru_maxrss;            
        r.ok = is_sorted(a.begin(), a.end());
        write(fd[1], &r, sizeof(r));
        _exit(0);
    }
    close(fd[1]);                          
    ssize_t leidos = read(fd[0], &res, sizeof(res));
    close(fd[0]);
    int estado; waitpid(pid, &estado, 0);
    return leidos == (ssize_t)sizeof(res);
}

int main() {
    
    vector<long long> tamanos = {100000LL, 1000000LL, 10000000LL, 100000000LL, 1000000000LL};

  
    vector<Algo> algos = {
        {"Bubble sort", bubbleSort, 100000LL},       
        {"Mergesort",   mergeSort,  10000000LL},
        {"Tree sort",   treeSort,   10000000LL},
        {"Heapsort",    heapSort,   10000000LL},
        {"Counting sort", [](vector<double>& a) { countingSort(a, 2); }, 10000000LL},
    };

    printf("%-15s %12s %12s %16s %12s\n", "Algoritmo", "n", "Tiempo (s)", "Mem. extra (MB)", "Pico (MB)");
    printf("---------------------------------------------------------------------------\n");
    for (auto& alg : algos) {
        for (long long n : tamanos) {
            if (n > alg.maxN) {
                printf("%-15s %12lld %12s %16s %12s\n", alg.nombre.c_str(), n, "omitido", "-", "-");
                continue;
            }
            Resultado r;
            if (!medir(alg, n, r)) {
                printf("%-15s %12lld %12s %16s %12s\n", alg.nombre.c_str(), n, "FALLÓ", "(memoria)", "-");
            } else {
                printf("%-15s %12lld %12.3f %16.1f %12.1f%s\n", alg.nombre.c_str(), n, r.seg,
                       (r.picoKB - r.baseKB) / 1024.0, r.picoKB / 1024.0, r.ok ? "" : "  ¡NO ORDENÓ!");
            }
            fflush(stdout);
        }
    }
    return 0;
}