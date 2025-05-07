/*───────────────────────────────────────────────────────────────*
 *  crear_array.cpp   — fast, cross‑platform generator
 *  Keeps the same CrearArray interface; no other code changes
 *  required in your project or Docker setup.
 *───────────────────────────────────────────────────────────────*/

 #include "crear_array.h"

 #include <cstdint>
 #include <cstdlib>
 #include <cstring>
 #include <fstream>
 #include <iostream>
 #include <random>
 #include <vector>
 
 #ifndef _WIN32
 #  include <sys/mman.h>
 #  include <sys/stat.h>
 #  include <fcntl.h>
 #  include <unistd.h>
 #  include <thread>
 #endif
 
/*────────  constructor & simple accessors (unchanged)  ───────*/
CrearArray::CrearArray(const char* fname, int M, int X)
    : filename(fname), M(M), X(X) {}

const char* CrearArray::getFileName() const { return filename; }
int  CrearArray::getM()  const { return M; }
int  CrearArray::getX()  const { return X; }
void CrearArray::setX(int x)   { X = x;   }

/*──── helper: fill a slice with fresh random uint64_t values ──*/
static void fill_slice(uint64_t* ptr, uint64_t len)
{
    std::mt19937_64 rng{ std::random_device{}() };
    std::uniform_int_distribution<uint64_t> dist;
    for (uint64_t i = 0; i < len; ++i) ptr[i] = dist(rng);
}

/*───────────────────────────────────────────────────────────────*
 *  CrearArray::crearArrayN  —  generates the .bin file          *
 *  No new arguments, same behaviour, faster implementation.     *
 *───────────────────────────────────────────────────────────────*/
int CrearArray::crearArrayN() const
{

    if (!prepareFile()) {
        std::cerr << "Fallo al preparar el archivo.\n";
        return 1;
    }
    
    const uint64_t bytes_ram  = static_cast<uint64_t>(M) * 1024 * 1024;
    const uint64_t total_nums = (bytes_ram * static_cast<uint64_t>(X))
                                / sizeof(uint64_t);
    if (total_nums == 0) {
        std::cerr << "M*X produce tamaño 0.\n";
        return 1;
    }
    const uint64_t total_bytes = total_nums * sizeof(uint64_t);

/*================================================================
 *  Build path selection:
 *    • On Linux/macOS (POSIX) → mmap + parallel fill (fastest)
 *    • On Windows           → buffered stream fallback
 *    • Force fallback by defining CREARARRAY_FORCE_STREAM
 *================================================================*/
#if defined(_WIN32) || defined(CREARARRAY_FORCE_STREAM)
 /*──────────────  Portable buffered‑stream version  ─────────────*/
    constexpr uint64_t CHUNK = 1ULL << 20;                 // 8 MiB
    std::vector<uint64_t> buf( std::min<uint64_t>(CHUNK, total_nums) );

    std::ofstream out(filename,
                    std::ios::binary |
                    std::ios::trunc  |
                    std::ios::out);
    if (!out) {
        std::cerr << "No se pudo abrir " << filename << '\n';
        return 1;
    }

    std::mt19937_64 rng{ std::random_device{}() };
    std::uniform_int_distribution<uint64_t> dist;

    uint64_t done = 0;
    while (done < total_nums) {
        const uint64_t n = std::min<uint64_t>(buf.size(), total_nums - done);
        for (uint64_t i = 0; i < n; ++i) buf[i] = dist(rng);
        out.write(reinterpret_cast<char*>(buf.data()),
                static_cast<std::streamsize>(n * sizeof(uint64_t)));
        if (!out) { std::cerr << "Error al escribir.\n"; return 1; }
        done += n;
    }
    out.close();
    return 0;

#else   /*──────────  POSIX mmap + multi‑thread version  ────────*/

    /* 1. Create/size the file exactly */
    int fd = ::open(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) { perror("open"); return 1; }
    if (ftruncate(fd, total_bytes) != 0) { perror("ftruncate"); return 1; }

    /* 2. Map it into memory */
    void* addr = mmap(nullptr, total_bytes,
                    PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) { perror("mmap"); return 1; }

    /* 3. Fill in parallel */
    const unsigned nThreads =
        std::max(1u, std::thread::hardware_concurrency());
    const uint64_t perT = total_nums / nThreads;
    uint64_t* base = static_cast<uint64_t*>(addr);
    std::vector<std::thread> pool;

    for (unsigned t = 0; t < nThreads; ++t) {
        uint64_t* slice = base + t * perT;
        uint64_t  len   = (t == nThreads - 1)
                            ? total_nums - t * perT
                            : perT;
        pool.emplace_back(fill_slice, slice, len);
    }
    for (auto& th : pool) th.join();

    /* 4. Flush & close */
    if (munmap(addr, total_bytes) != 0) { perror("munmap"); return 1; }
    close(fd);
    return 0;
#endif
} 

bool CrearArray::prepareFile() const {
#if defined(_WIN32) || defined(CREARARRAY_FORCE_STREAM)
    std::ifstream infile(filename, std::ios::binary);
    bool exists = infile.good();
    infile.close();

    std::ofstream out(filename,
                        std::ios::binary |
                        std::ios::trunc |
                        std::ios::out);
    if (!out) {
        std::cerr << "No se pudo preparar " << filename << '\n';
        return false;
    }
    out.close();
    return true;

#else
    int flags = O_RDWR | O_CREAT;
    if (access(filename, F_OK) == 0)
        flags |= O_TRUNC;  // Si existe, trúncalo
    int fd = ::open(filename, flags, 0666);
    if (fd < 0) { perror("open"); return false; }
    close(fd);
    return true;
#endif
}
    