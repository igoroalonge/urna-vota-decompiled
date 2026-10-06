// uenux2/src/app/comum/cpath.h  (path inferred from cpath.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u22).
//
// comum::CPath: every directory of the urna's storage, built from two statics initialised by
// __wasm_call_ctors (func 14478):
//   ms_raizesFlash[2] @1838576 = { "/dsk/fi/", "/dsk/fe/" }   (EFlashOrigem 0 = flash interna / MI,
//                                                              1 = flash externa / cartão de memória MV)
//   ms_raiz          @1838600 = "/"                          (prefix of every absolute path; lets tests
//                                                              relocate the tree)
// atexit destructors: func 11649 (ms_raizesFlash), func 11648 (ms_raiz).
//
// Resulting tree (see docs/06-filesystem-and-election-data.md):
//   <flash>/dinamico/          eg.bin, eg.vsu, log/logd.dat          GetPathDinamico   (func 1082)
//   <flash>/dinamico/trab1|2/  vota.bin rdv.dat uenux.db gap.bin ... GetPathTrab       (func 358)
//   <flash>/dinamico/res1|2/   result files (BU, RDV, ...)           GetPathResult     (func 1399)
//   <flash>/estatico/          election data (read only)             GetPathEstatico   (func 762)
//   /dsk/fi/ , /dsk/fe/        roots without the SA sub-tree         GetPathRootSemSA  (func 634)
//   /dsk/mr/                   memória de resultado (pen drive)     GetPathMR         (func 949)
#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace comum {

enum class EFlashOrigem : int { INTERNA = 0, EXTERNA = 1 };   // names inferred (u02 uses the same)
enum class EUrnaTurno : char { PRIMEIRO = '1', SEGUNDO = '2' }; // values from the checks '1'/'2'

class CPath
{
public:
    static std::filesystem::path GetPathRootSemSA(EFlashOrigem origem);                 // func 634  (:153)
    static std::filesystem::path GetPathResult(EFlashOrigem origem, EUrnaTurno turno);  // func 1399 (:189)
    static std::filesystem::path GetPathTrab(EFlashOrigem origem, EUrnaTurno turno);    // func 358  (:216)

    // Members defined in cpath.cpp but listed in other units (names inferred):
    static std::filesystem::path GetPathTrab(EFlashOrigem origem);                      // func 436 (u02)
    static std::filesystem::path GetPathDinamico(EFlashOrigem origem);                  // func 1082
    static std::filesystem::path GetPathEstatico(EFlashOrigem origem);                  // func 762
    static std::filesystem::path GetPathLog();                                          // func 5899
    static std::filesystem::path GetPathMR();                                           // func 949
    static std::filesystem::path GetPathRoot(std::string_view caminhoAbsoluto);         // func 6048
    static std::filesystem::path GetRaiz();                                             // func 3834

private:
    static const std::string ms_raizesFlash[2];   // @1838576 (12 bytes each)
    static const std::string ms_raiz;             // @1838600
};

} // namespace comum
