// Reconstructed from vota_web_wasm.wasm (unit u31).
// Original file UNKNOWN - path inferred: uenux2/mock/app/simulador/wasm/cwasmnullpaper.cpp
//
// simulador::CWasmNullPaper : api::IPaperRelatorios : api::IPaper - the "paper" (report output) of the web
// build. Every report of the urna (zerésima, boletim de urna vias, BJE, boletim de mesários, ...) goes through
// api::IPaperRelatorios; this implementation discards all of it.
// RTTI typeinfo @1530632; vtable @1530572 (15 slots), slot names from their callers (units u09, u16, u17):
//   [0] ~ (ICF 174)  [1] deleting (ICF 144)
//   [2] Print(const IText&, EStyle)                               nop (ICF 1528)
//   [3] nop (218): paper cut (?)   [4] nop (218): line feed (?)   (roles as recorded by tools/bu/operator_harness.mjs)
//   [5] Abre(const std::string& arquivo)                          nop (ICF 425)    creport.cpp:31
//   [6] Fecha()                                                   nop (218)        creport.cpp:37
//   [7] ImprimeArquivo(arquivo, verificador, titulo, via)         8372 (empty)
//   [8] ImprimeArquivo(arquivo, verificador, cabecalho, mensagem, via)   8371 (only releases `cabecalho`)
//   [9] ret 0 (ICF 340)  [10] AguardaFimImpressao() nop (218)  [11] nop (1870)
//   [12] QR-code image (vector<uebyte>, modules) (?) nop (1870)  [13] ret 1 (371)  [14] nop (1528)
//
// Reports are first COMPOSED through this interface: Abre(<trab>/bu.dat) (slot 5), text/line-feed/cut/QR
// (slots 2/4/3/12), Fecha() (slot 6) - on the urna, presumably what writes the printable report file. Only then does
// CImprimindoBU print it with slot 8 + slot 10. Here the composition is discarded too, so the file handed to
// slot 8 (<trab>/bu.dat, and buj/bim/behb.dat) is never written in the web build. Verified with the harness
// (flow treino): 4 reports composed (bu, buj, bim, behb: 124/69/42/73 calls between slots 5 and 6), then one
// slot-8 call and one slot-10 call.
// Registered as api::IPaperRelatorios by func 8302 (4-byte object).
//
// BU note: vota::CImprimindoBU::ImprimeBU passes trab/bu.dat and a comum::CSigVerifier(trab, "bu.dat",
// "bu.vsu") to slot 8 - on the urna the printer service checks the signature before printing each via. Here
// the verifier is never evaluated and nothing is printed; AguardaFimImpressao returns at once.
#include <filesystem>
#include <memory>
#include <string>

#include "api/hwil/ipaperrelatorios.h"
#include "comum/relatorios/csigverifier.h"

namespace simulador {

class CWasmNullPaper : public api::IPaperRelatorios {
public:
    void ImprimeArquivo(const std::filesystem::path& arquivo, const comum::CSigVerifier& verificador,
                        const std::string& titulo, const std::string& via) override;                  // slot 7 (8372)
    void ImprimeArquivo(const std::filesystem::path& arquivo, const comum::CSigVerifier& verificador,
                        api::SharedPaperForm cabecalho, const std::string& mensagem,
                        const std::string& via) override;                                              // slot 8 (8371)
    // every other slot: the ICF no-op bodies listed above
};

// wasm func 8372 - slot 7 (zerésima, BJE, boletim de mesários)
void CWasmNullPaper::ImprimeArquivo(const std::filesystem::path&, const comum::CSigVerifier&, const std::string&,
                                    const std::string&)
{
}

// wasm func 8371 - slot 8 (boletim de urna, one call per via). The header form is taken by value; the only
// code is the release of that shared_ptr (callee-destroyed: libc++ ABI v2 makes shared_ptr trivial_abi).
void CWasmNullPaper::ImprimeArquivo(const std::filesystem::path&, const comum::CSigVerifier&,
                                    api::SharedPaperForm /*cabecalho*/, const std::string&, const std::string&)
{
}

}  // namespace simulador
