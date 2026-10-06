// uenux2/src/app/comum/cpath.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by std::source_location records
// :153 (GetPathRootSemSA), :189 (GetPathResult), :216 (GetPathTrab).
// Functions of this file listed in other units are included (marked) so that the file is complete;
// see also the u02 fragment cpath.u02.cpp.
#include "comum/cpath.h"

#include <format>

#include "comum/comumdefs.h"   // CUeComumError = CBaseError<EUeComumError, SErrorLimits{7200, 7600}> (factory func 480)

namespace comum {

namespace fs = std::filesystem;

// static initialisation (inlined into __wasm_call_ctors, func 14478)
const std::string CPath::ms_raizesFlash[2] = {"/dsk/fi/", "/dsk/fe/"};
const std::string CPath::ms_raiz = "/";

// wasm func 6048 (other unit) - the absolute path re-rooted under ms_raiz.
fs::path CPath::GetPathRoot(std::string_view caminhoAbsoluto)                     // name inferred
{
    return fs::path(ms_raiz) / fs::path(caminhoAbsoluto).relative_path();       // func 1651 = path::__relative_path
}

// wasm func 634 (srcloc :153). Observed executing (votaInit and CArquivosSavd).
// "SemSA": the root of a flash without the "sa/<secao>/" sub-tree used by the SA application.
fs::path CPath::GetPathRootSemSA(EFlashOrigem origem)
{
    switch (origem) {
    case EFlashOrigem::INTERNA:
        return GetPathRoot("/dsk/fi/");                                          // inlined (func 6048)
    case EFlashOrigem::EXTERNA:
        return GetPathRoot("/dsk/fe/");
    }
    throw CUeComumError(EUeComumError{7224}, std::format("Mídia inválida: {}", origem));   // :153
}

// wasm func 1399 (srcloc :189): <flash>/dinamico/res<turno>/
// NOTE: `origem` indexes ms_raizesFlash without a range check (only the turno is validated).
fs::path CPath::GetPathResult(EFlashOrigem origem, EUrnaTurno turno)
{
    if (turno != EUrnaTurno::PRIMEIRO && turno != EUrnaTurno::SEGUNDO)       // compiled as `turno - '1' <= 1`
        throw CUeComumError(EUeComumError{7226}, std::format("Turno invalido: {}", turno));   // :189
    fs::path caminho = fs::path(ms_raizesFlash[static_cast<int>(origem)]) / "dinamico/res";
    caminho += std::to_string(static_cast<char>(turno) - '0');              // func 296 = std::to_string(int)
    caminho /= "";                                                            // trailing separator
    return caminho;
}

// wasm func 358 (srcloc :216): <flash>/dinamico/trab<turno>/  - Observed executing.
fs::path CPath::GetPathTrab(EFlashOrigem origem, EUrnaTurno turno)
{
    if (turno != EUrnaTurno::PRIMEIRO && turno != EUrnaTurno::SEGUNDO)
        throw CUeComumError(EUeComumError{7227}, std::format("Turno invalido: {}", turno));   // :216
    fs::path caminho = fs::path(ms_raizesFlash[static_cast<int>(origem)]) / "dinamico" / "trab";
    caminho += std::to_string(static_cast<char>(turno) - '0');
    caminho /= "";
    return caminho;
}

// ---- listed in other units (names inferred) --------------------------------------------------------

// wasm func 1082 (via the merged body func 6047(out, origem, begin, end))
fs::path CPath::GetPathDinamico(EFlashOrigem origem)
{
    return fs::path(ms_raizesFlash[static_cast<int>(origem)]) / "dinamico/";
}

// wasm func 762 (same merged body 6047)
fs::path CPath::GetPathEstatico(EFlashOrigem origem)
{
    return fs::path(ms_raizesFlash[static_cast<int>(origem)]) / "estatico/";
}

// wasm func 5899: always the internal flash
fs::path CPath::GetPathLog()
{
    return fs::path(ms_raizesFlash[static_cast<int>(EFlashOrigem::INTERNA)]) / "dinamico/log/";
}

// wasm func 949
fs::path CPath::GetPathMR()
{
    return GetPathRoot("/dsk/mr/");
}

// wasm func 3834
fs::path CPath::GetRaiz()
{
    return ms_raiz;
}

} // namespace comum
