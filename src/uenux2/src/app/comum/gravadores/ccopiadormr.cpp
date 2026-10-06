// uenux2/src/app/comum/gravadores/ccopiadormr.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u35). Constructors 1276 / 3827: unit u08.
// None of these functions ran in the recorded sessions (the web page never reaches the encerramento).
#include "comum/gravadores/ccopiadormr.h"

#include <filesystem>

#include "api/util/csystem.h"     // CopyFile (func 378)
#include "comum/cpath.h"          // GetPathResult (func 1274), GetPathMR (func 949)

namespace comum {

// wasm func 11638 - vtable slot 0. Body = shared_f1723(this, vtable @1553348): vptr = CCopiadorMR; ~m_nome.
// (shared_f1723 is a merged "set vptr + destroy the std::string at +4" body also used by RHVoice classes.)
CCopiadorMR::~CCopiadorMR() = default;

// wasm func 5872 - vtable slot 1 (deleting destructor) of CCopiadorMR AND CCopiadorWSQMR:
// shared_f1722(this, vtable @1553348) = the same + operator delete(this).

// wasm func 5874 - vtable slot 2 (also CCopiadorWSQMR slot 2).                              name inferred
// Copies the final result file of the internal memory to the result stick. The destination is not checked here;
// vota::CCopiaResultadoParaMR afterwards compares only the BU copy with its source (CSystem::AreFilesEqual).
void CCopiadorMR::Copia() const
{
    const std::filesystem::path origem  = CPath::GetPathResult(EFlashOrigem::INTERNA) / m_nome;
    const std::filesystem::path destino = CPath::GetPathMR() / m_nome;             // "/dsk/mr/<nome>"
    api::CSystem::CopyFile(origem.string(), destino.string(), false);
}

// wasm func 5873 - vtable slot 3 (also CCopiadorWSQMR slot 3).                              name inferred
// Same, taking the file from the MV's result directory. No call of slot 3 exists in the VOTA code of this binary
// (CCopiaResultadoParaMR uses slot 2 only); probably used by another application (recovery of results from the
// MV of a broken urna).                                                                         ?
void CCopiadorMR::CopiaDaMV() const
{
    const std::filesystem::path origem  = CPath::GetPathResult(EFlashOrigem::EXTERNA) / m_nome;
    const std::filesystem::path destino = CPath::GetPathMR() / m_nome;
    api::CSystem::CopyFile(origem.string(), destino.string(), false);
}

// CCopiadorWSQMR's overrides: in the binary they are the very same functions 5874 / 5873 (identical bodies folded).
void CCopiadorWSQMR::Copia() const { CCopiadorMR::Copia(); }                      // ? (folded into 5874)
void CCopiadorWSQMR::CopiaDaMV() const { CCopiadorMR::CopiaDaMV(); }              // ? (folded into 5873)

} // namespace comum
