// uenux2/src/app/comum/cpath.cpp (+ helpers of unknown file)  --  FRAGMENT written by unit u02
// (cpath.cpp is owned by u22). Paths of the work directories:
//   <root of the flash>/dinamico/trab<turno>, root = /dsk/fi (flash interna, EFlashOrigem 0) or
//   /dsk/fe (flash externa, 1; table of roots @1838576, 12 bytes per flash), turno '1' or '2'
// in the web build (see analysis/runtime/README.md). CPath::GetPathTrab(EFlashOrigem, EUrnaTurno) is
// wasm func 358 (srcloc cpath.cpp:216).

#include "comum/cpath.h"

namespace comum {

// wasm func 436                                                        // name inferred
// Overload taking the turn from the loaded general state (CEstadoGeral +32).
std::filesystem::path CPath::GetPathTrab(EFlashOrigem origem)
{
    return GetPathTrab(origem, CAppInfo::GetInst().GetGeral().GetTurno());
}

namespace {
// wasm func 6036 (flash interna) and wasm func 6035 (flash externa): two copies of the same inlined
// helper, specialised by the constant flash; each builds "<trab>/<nome>" from a [begin, end) literal.
std::filesystem::path ArquivoTrabInterno(std::string_view nome)
{
    return CPath::GetPathTrab(EFlashOrigem::INTERNA) / std::string(nome);
}
std::filesystem::path ArquivoTrabExterno(std::string_view nome)
{
    return CPath::GetPathTrab(EFlashOrigem::EXTERNA) / std::string(nome);
}
} // namespace

// Callers: CEleitores::CompleteLoad, vota::impl::CSincronismoVotoEleitor::vf2, wasm 5736/5737/6737/7787.
std::filesystem::path ArquivoRdvInterno()   { return ArquivoTrabInterno("rdv.dat"); }    // wasm func 1551
std::filesystem::path ArquivoRdvExterno()   { return ArquivoTrabExterno("rdv.dat"); }    // wasm func 1701
std::filesystem::path ArquivoBancoInterno() { return ArquivoTrabInterno("uenux.db"); }   // wasm func 2826
std::filesystem::path ArquivoBancoExterno() { return ArquivoTrabExterno("uenux.db"); }   // wasm func 5762

} // namespace comum
