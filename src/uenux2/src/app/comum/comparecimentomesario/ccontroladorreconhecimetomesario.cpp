// uenux2/src/app/comum/comparecimentomesario/ccontroladorreconhecimetomesario.cpp
// (sic: "reconhecimeto" - the original file name has the typo; the class is ...Reconhecimento...)
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by std::source_location records :65 and
// :76 (ComparaDigitais). Only ComparaDigitais is listed in this unit; the rest of the class is
// described from its uses.
//
// comum::CControladorReconhecimentoMesario (no RTTI; lazy singleton @1909960, 24 bytes, GetInst =
// func 1149):
//   +0  md::CDedo::TipoDedo m_dedo = 0          finger that matched (1, 6, 2 or 7), 0 otherwise
//   +4  int m_limiarScore = 20                  threshold passed to IFingerMatcher::Compara
//   +8  EstadoReconhecimentoBiometrico m_estado = 1 (NAO_COLETADA)
//   +12 std::optional<std::uint32_t> m_idArquivo  (flag +16) id of the stored "me<id>.wsq" image
//   +20 std::size_t m_qtdArquivos               counted at construction (vota_f5374)
// Other members (other units): SalvarBiometriaMesarioRegistrado(digital, titulo) and
// SalvarBiometriaMesarioNaoRegistrado(digital) - both hold a lambda `$_0` returning the file id
// (std::function vtables @1595836 / @1595908); func 5371 writes "me{:06}.wsq" (needs >= 5 MiB free).
#include "comum/comparecimentomesario/ccontroladorreconhecimetomesario.h"

#include <format>

#include "api/hwil/ifingermatcher.h"
#include "api/hwil/ifingerscanner.h"
#include "api/pattern/cpolysingletonlist.h"
#include "comum/log/clogcomum.h"

namespace comum {

namespace {
// Template extraction of this build is an empty stub (u10 §6.4): the call to the configuration
// singleton of the extractor (func 1903, 16-byte object @1908664 = {8, 500, ...}) and two scanner
// calls survive, the output vector is just cleared.                                     ?
void ExtraiTemplate(api::IFingerScanner& leitor, const std::vector<uebyte>& /*digital*/,
                    std::vector<uebyte>& modelo)
{
    (void)ConfiguracaoExtrator::GetInst();          // func 1903
    leitor.AtualizaParametros();                    // IFingerScanner slot 2  (name unknown)
    leitor.Prepara();                               // IFingerScanner slot 1  (name unknown)
    modelo.clear();                                 // func 2224
}
} // namespace

// wasm func 5372. The two vector parameters were removed by dead-argument elimination: the
// extractor stub never reads them, so the wasm function only receives `this`.
// Callers: vota::CRegistraDigitalOperador::BiometriaMesarioPresenteNosEleitores / ProcessTick (u27).
bool CControladorReconhecimentoMesario::ComparaDigitais(const std::vector<uebyte>& digital1,
                                                        const std::vector<uebyte>& digital2)
{
    auto& leitor = api::CPolySingletonList::instance<api::IFingerScanner>();       // :65
    std::vector<uebyte> modelo1;
    std::vector<uebyte> modelo2;
    ExtraiTemplate(leitor, digital1, modelo1);
    ExtraiTemplate(leitor, digital2, modelo2);

    auto& comparador = api::CPolySingletonList::instance<api::IFingerMatcher>();   // :76
    const bool reconhecido = comparador.Compara(modelo1, modelo2, m_limiarScore);  // slot 2
    CLogComum::GetInst().LogaScoreReconhecimentoMesario(comparador.GetScore());   // func 948; slot 4
    // inlined CLogComum::LogaScoreReconhecimentoMesario (clogcomum.cpp:214):
    //   api::CPolySingletonList::instance<IEventosLog>().Loga(
    //       std::format("Batimento de digitais retornou o score {}", score));        (level 1)
    return reconhecido;
}

} // namespace comum
