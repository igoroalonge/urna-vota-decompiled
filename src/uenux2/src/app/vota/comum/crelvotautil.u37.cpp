// uenux2/src/app/vota/comum/crelvotautil.cpp (path inferred, as in unit u09's crelvotautil.u09.cpp)
// -- FRAGMENT written by unit u37.
//
// The "printing in progress" marker: an empty file /dsk/fi/dinamico/imprimindo (internal flash only) that
// exists while a report is being sent to the thermal printer. It is created right before and removed right
// after IPaperRelatorios::ImprimeArquivo in every printing state: CImprimindoBU::ImprimeBU (2890),
// CImprimindoZeresima / CReimprimindoZeresima / CReimprimindoResumoZeresima, CImprimirBJust, CImprimindoBim,
// CImprimindoBEHB. Nothing in this binary reads it: its consumer must be another program of the urna (e.g.
// the start-up/launcher code that detects a print interrupted by a power loss).                         ?
// (The only other reference to the path string is 1487's own existence test before the removal.)
// Both functions sync the file system afterwards (api::CSynchronizer, i.e. sync()).
#include <filesystem>

#include "api/util/csynchronizer.h"
#include "api/util/csystem.h"
#include "comum/cpath.h"
#include "ecourna/api/io/cfile.hpp"
#include "vota/comum/crelvotautil.h"

namespace vota {

namespace {
// Inlined once in 1488 and twice in 1487. In 1487 the existence test has its own stack frame, so it is probably
// a small inlined helper (e.g. "ExisteMarcaImpressao()") of its own.                    name inferred
std::filesystem::path CaminhoMarcaImpressao()
{
    return comum::CPath::GetPathRootSemSA(comum::EFlashOrigem::INTERNA) / "dinamico/imprimindo";   // wasm 634
}
} // namespace

// wasm func 1488 (tools: vota_f1488)                                 name as used by units u08/u09 (inferred)
void CRelVotaUtil::MarcaImpressaoEmAndamento()
{
    ecourna::api::io::CFile marca(CaminhoMarcaImpressao().string(), "wb");            // ecourna_f517
    marca.Close();                                                                    // ecourna 336
    api::CSynchronizer::GetInst().Sync();                                             // wasm 600 + 620
}

// wasm func 1487 (tools: vota_f1487)                                 name as used by units u08/u09 (inferred)
void CRelVotaUtil::RemoveMarcaImpressaoEmAndamento()
{
    if (!api::CSystem::IsRegularFile(CaminhoMarcaImpressao()))                        // api_f412
        return;
    api::CSystem::SecureRemove(CaminhoMarcaImpressao());                              // wasm 2760 (zero-fill + remove)
    api::CSynchronizer::GetInst().Sync();
}

} // namespace vota
