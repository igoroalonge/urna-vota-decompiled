// FRAGMENT of ecourna-lib/ecourna/api/compression/clzmacompress.cpp (srcloc-attested; class reconstructed by
// unit u12 in clzmacompress.hpp/.cpp). Reconstructed by unit u40 from vota_web_wasm.wasm.
//
// DEAD CODE in this binary: no function stores the CLzmaCompress vtable, so neither function below can run
// (see clzmacompress.hpp).
#include "ecourna/api/compression/clzmacompress.hpp"

#include <format>

namespace ecourna::api::compression {

// wasm func 9585 (table slot 6056). Progress callback bound into a std::function by DoAdd (functor vtable
// @1109952) and called by the 7-Zip update callback. Emits the IObservableProgressWithDescription signal
// (signal_impl at this+8, operator() = func 9583) with total 100: the first value is a percentage, truncated
// from 64 to 32 bits (unsigned long on wasm32).                                          // name inferred
void CLzmaCompress::OnProgress(std::uint64_t percentual, const std::string& nome)
{
    m_progresso(static_cast<unsigned long>(percentual), 100, std::format("Compactando arquivo {}...", nome));
}

// wasm func 8378: atexit destructor of the file-static `std::mutex s_mutex` (@1923244) used by DoAdd.
// std::mutex::~mutex -> pthread_mutex_destroy is a stub in this single-threaded build, so the body is the
// ICF'd "noexcept pthread residue" func 150 applied to the mutex address.

} // namespace ecourna::api::compression
