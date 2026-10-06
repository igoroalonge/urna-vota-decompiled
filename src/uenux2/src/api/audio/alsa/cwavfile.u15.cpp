// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/audio/alsa/cwavfile.cpp (the constructor
// CWavFile(const CWavAudio&, const void*) - srcloc cwavfile.cpp:47, func 10841 - belongs to another unit).
// Only the two destructors are in u15. Merge into cwavfile.cpp.
#include <cstdlib>

namespace api {

// CWavFile (vtable @1600484). Layout from the destructor:
//   +48 void* m_dados     (PCM buffer, malloc'ed; freed and reset)          name inferred
//   +52 void* m_cabecalho (second malloc'ed block)                          name inferred
class CWavFile {
public:
    virtual ~CWavFile();
    // ... (constructor and accessors: other unit)
private:
    unsigned char m_resto[44];   // +4..+48 (see cwavfile.cpp constructor)
    void* m_dados = nullptr;     // +48
    void* m_cabecalho = nullptr; // +52
};

// wasm func 5345 (vtable slot 0)                                                   // name inferred
CWavFile::~CWavFile()
{
    if (m_dados) {
        std::free(m_dados);
        m_dados = nullptr;
    }
    if (m_cabecalho)
        std::free(m_cabecalho);
}

// wasm func 10238 (vtable slot 1): deleting destructor = ~CWavFile() + operator delete(this).

} // namespace api
