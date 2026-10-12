# Algoritmos de ordenamiento en Python 3 (para OnlineGDB, elegir lenguaje Python 3)
import sys
sys.setrecursionlimit(10000)

# ---------- a) Bubble sort ----------
def bubble_sort(a):
    n = len(a)
    for i in range(n - 1):
        swapped = False
        for j in range(n - 1 - i):
            if a[j] > a[j + 1]:
                a[j], a[j + 1] = a[j + 1], a[j]
                swapped = True
        if not swapped:
            break

# ---------- b) Mergesort ----------
def merge_sort(a):
    if len(a) < 2:
        return
    tmp = [0.0] * len(a)
    _merge_rec(a, tmp, 0, len(a) - 1)

def _merge_rec(a, tmp, l, r):
    if l >= r:
        return
    m = (l + r) // 2
    _merge_rec(a, tmp, l, m)
    _merge_rec(a, tmp, m + 1, r)
    i, j, k = l, m + 1, l
    while i <= m and j <= r:
        if a[i] <= a[j]:
            tmp[k] = a[i]; i += 1
        else:
            tmp[k] = a[j]; j += 1
        k += 1
    while i <= m:
        tmp[k] = a[i]; i += 1; k += 1
    while j <= r:
        tmp[k] = a[j]; j += 1; k += 1
    a[l:r + 1] = tmp[l:r + 1]

# ---------- c) Tree sort (árbol binario de búsqueda) ----------
class Node:
    __slots__ = ("val", "left", "right")
    def __init__(self, val):
        self.val = val
        self.left = None
        self.right = None

def tree_sort(a):
    if not a:
        return
    root = Node(a[0])
    for x in a[1:]:                      # inserción iterativa
        cur = root
        while True:
            if x < cur.val:
                if cur.left is None:
                    cur.left = Node(x); break
                cur = cur.left
            else:
                if cur.right is None:
                    cur.right = Node(x); break
                cur = cur.right
    stack, cur, idx = [], root, 0        # recorrido inorden iterativo
    while cur is not None or stack:
        while cur is not None:
            stack.append(cur)
            cur = cur.left
        cur = stack.pop()
        a[idx] = cur.val
        idx += 1
        cur = cur.right

# ---------- d) Heapsort ----------
def _heapify(a, n, i):
    while True:
        largest, l, r = i, 2 * i + 1, 2 * i + 2
        if l < n and a[l] > a[largest]:
            largest = l
        if r < n and a[r] > a[largest]:
            largest = r
        if largest == i:
            break
        a[i], a[largest] = a[largest], a[i]
        i = largest

def heap_sort(a):
    n = len(a)
    for i in range(n // 2 - 1, -1, -1):
        _heapify(a, n, i)
    for i in range(n - 1, 0, -1):
        a[0], a[i] = a[i], a[0]
        _heapify(a, i, 0)

# ---------- e) Counting sort para números reales ----------
# Se discretiza: clave = round((x - min) * 10^decimales)
def counting_sort(a, decimales=2):
    if not a:
        return
    factor = 10 ** decimales
    mn, mx = min(a), max(a)
    rango = int(round((mx - mn) * factor)) + 1
    count = [0] * rango
    for x in a:
        count[int(round((x - mn) * factor))] += 1
    idx = 0
    for k in range(rango):
        for _ in range(count[k]):
            a[idx] = mn + k / factor
            idx += 1

# ======================= BENCHMARK (tiempo y memoria) =======================
import os, time, random, resource, pickle

def rss_actual_kb():
    """Memoria residente actual del proceso (KB)."""
    with open("/proc/self/statm") as f:
        paginas = int(f.read().split()[1])
    return paginas * os.sysconf("SC_PAGE_SIZE") // 1024

def medir(fn, n):
    r_fd, w_fd = os.pipe()
    pid = os.fork()
    if pid == 0:                                  
        os.close(r_fd)
        random.seed(12345)
        a = [random.randint(0, 99999) / 100.0 for _ in range(n)]
        base = rss_actual_kb()
        t0 = time.perf_counter()
        fn(a)
        seg = time.perf_counter() - t0
        pico = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss   
        ok = all(a[i] <= a[i + 1] for i in range(len(a) - 1))
        os.write(w_fd, pickle.dumps((seg, base, pico, ok)))
        os._exit(0)
    os.close(w_fd)                                
    datos = b""
    while True:
        chunk = os.read(r_fd, 4096)
        if not chunk:
            break
        datos += chunk
    os.close(r_fd)
    os.waitpid(pid, 0)
    return pickle.loads(datos) if datos else None

if __name__ == "__main__":
    tamanos = [10000, 100000, 1000000, 10000000, 100000000, 1000000000]

  
    algos = [
        ("Bubble sort",   bubble_sort,   10000),
        ("Mergesort",     merge_sort,    1000000),
        ("Tree sort",     tree_sort,     1000000),
        ("Heapsort",      heap_sort,     1000000),
        ("Counting sort", counting_sort, 10000000),
    ]
    print(f"{'Algoritmo':<15}{'n':>13}{'Tiempo (s)':>13}{'Mem. extra (MB)':>17}{'Pico (MB)':>12}")
    print("-" * 70)
    for nombre, fn, max_n in algos:
        for n in tamanos:
            if n > max_n:
                print(f"{nombre:<15}{n:>13}{'omitido':>13}{'-':>17}{'-':>12}")
                continue
            r = medir(fn, n)
            if r is None:
                print(f"{nombre:<15}{n:>13}{'FALLÓ':>13}{'(memoria)':>17}{'-':>12}")
            else:
                seg, base, pico, ok = r
                print(f"{nombre:<15}{n:>13}{seg:>13.3f}{(pico-base)/1024:>17.1f}{pico/1024:>12.1f}"
                      + ("" if ok else "  ¡NO ORDENÓ!"))
            sys.stdout.flush()