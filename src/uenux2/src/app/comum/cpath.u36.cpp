// uenux2/src/app/comum/cpath.cpp  --  FRAGMENT written by unit u36 (file owned by u22; see also cpath.u02.cpp
// and CPath::GetPathWsq = wasm 2864 in vota/operador/u27-foreign-fragments.cpp). Reconstructed from
// vota_web_wasm.wasm.
//
// Directories of the fingerprint images (WSQ) on the EXTERNAL flash (the removable card, EFlashOrigem 1)
// for the current turno: <flash externa>/dinamico/trab<turno>/wsq/<subdir>. The internal-flash twins are
// vota_f2298 + thunks 5819 ("habilitado/"), 5818 ("nao-habilitado/"), 3795 ("operador/"), 5815
// ("registrado/"), 3793 ("nao-registrado/") - other units.
//
// In the binary the three external functions are 4-instruction thunks into ONE body (wasm 3898) that takes the
// sub-directory as a (begin, end) pair of a string literal: wasm-opt "merge-similar-functions" folded three
// source functions that differed only in the literal. Written here as the three original-looking functions
// plus the shared body. Class membership and names are inferred.                               (path ?)
//
// Users: CGravadorWSQ::GetCaminhoCorretoExternal (inlined in 5821, packs wsqbio/wsqman/wsqmes.jez),
// vota::CMostraEleitorVotando::SalvaHabilitacaoEleitor (10425, stores the enabling fingerprint of a voter),
// comum::CPedeDigitalMesario (5382, a mesário's fingerprint). None runs in the simulator (no biometrics).
#include "comum/cpath.h"

#include <filesystem>
#include <string_view>

#include "comum/appinfo/cappinfo.h"

namespace comum {

namespace fs = std::filesystem;

// wasm func 3898 (merged body)                                                          // name inferred
fs::path CPath::GetPathWsqExterno(std::string_view subdiretorio)
{
    const auto turno = CAppInfo::GetInst().GetGeral().GetTurno();                  // funcs 185 + 457, eg +32
    return GetPathWsq(EFlashOrigem::EXTERNA, turno) / fs::path(subdiretorio);       // wasm 2864: .../wsq/
}

// wasm func 3794                                                                         // name inferred
fs::path CPath::GetPathWsqOperadorExterno()      { return GetPathWsqExterno("operador/"); }        // @358540

// wasm func 5816                                                                         // name inferred
fs::path CPath::GetPathWsqNaoHabilitadoExterno() { return GetPathWsqExterno("nao-habilitado/"); }  // @358572

// wasm func 5817                                                                         // name inferred
// ("habilitado/" is the tail of the "nao-habilitado/" literal: @358576 = @358572 + 4)
fs::path CPath::GetPathWsqHabilitadoExterno()    { return GetPathWsqExterno("habilitado/"); }

}  // namespace comum
