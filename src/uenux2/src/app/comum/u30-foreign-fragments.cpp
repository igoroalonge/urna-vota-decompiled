// Functions of uenux2/src/app/comum/... that the tools attributed to the web platform (unit u30), because
// their only caller is votaInit or because they sit next to web-entry functions in the function table
// ("data-table neighbours"). Reconstructed by unit u30; the owning files are reconstructed by other units
// (appinfo: u20, dados/ccargos: u04, relatorios/crelutil: u24?, md/estadoaplicacao: u03/u05).

#include <memory>
#include <mutex>
#include <optional>

#include "comum/appinfo/cappinfo.h"
#include "comum/appinfo/servicos/cservicoestadogeral.h"
#include "comum/appinfo/servicos/cservicoestadogeralgap.h"
#include "comum/appinfo/servicos/cservicoestadogeralsa.h"
#include "comum/cconfiguracaoeleicao.h"
#include "comum/dados/ccandidaturas.h"
#include "comum/dados/ccargos.h"
#include "comum/dados/md/estadoaplicacao/cestadogeralvota.h"
#include "comum/dados/md/processoeleitoral/ccargo.h"

namespace comum {

// =========================================================================================================
// uenux2/src/app/comum/appinfo/cappinfo.cpp (path inferred; class reconstructed by u20)
// wasm func 11572 (table slot 34) - observed executing. 1,550 bytes, almost all of it the inlined
// move-assignment / move-construction of std::optional<CEstadoGeral> (180 bytes: 7 strings/vectors).
// Called by votaInit right after the fixture: loads <MI>/dinamico/eg.bin into the cache.     name inferred
// (unit u29 calls it comum::teste::CarregaAppInfo(CAppInfo&); it writes CAppInfo's private members, so a
//  member function is the more natural reading.)                                                        // ?
// =========================================================================================================
void CAppInfo::CarregaGeral()
{
    m_campo528 = 0;                        // +528 cleared first (the last 4 bytes of the 532-byte object) ?
    m_geral = CServicoEstadoGeral(EFlashOrigem::INTERNA).Carrega();   // service {vptr @1558024, 0} (func 1941);
                                                                      // Carrega = CFileASN read + BER decode
                                                                      // (func 3791); flag of the optional at +180
}

// =========================================================================================================
// uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeralvota.cpp (path inferred; class: u03)
// wasm func 11266 (table slot 51). A plain setter kept out of line (called through invoke from votaInit:
// GetVota(Atual).SetEstadoVota('8') = EAVVOTAR, the "voting open" state).                   name inferred
// =========================================================================================================
void md::estadoaplicacao::CEstadoGeralVota::SetEstadoVota(EEstadoVota estado)
{
    m_estadoVota = estado;                 // +0
}

// =========================================================================================================
// uenux2/src/app/comum/appinfo/servicos/ - instantiations of the IServicoEstado<T, Conversor> template
// (iservicoestado.h, path inferred; see unit u20/u18). The common body is func 2894 (merge-similar-functions:
// the conversor's Converte and CFileASN::WriteToFile are passed as table slots, the conversor vtable as a
// constant):
//     template <class T, class C> void IServicoEstado<T, C>::Salva(const T& estado)
//     {
//         const std::string arquivo = GetPathArquivo().string();       // vtable slot 2 (e.g. .../trab1/sa.bin)
//         api::CFileASN::WriteToFile(arquivo, C().Converte(estado));   // BER; ~ASN1::SEQUENCE afterwards
//     }
// =========================================================================================================

// wasm func 10089 (table slot 436) - observed executing: 2894(this, estado, slot 457 = WriteToFile<EstadoGeralSA>,
// slot 456 = IConversorASN<EstadoGeralSA, CEstadoGeralSA>::Converte, vtable CConversorEstadoGeralSA @1568732)
template <>
void IServicoEstado<md::estadoaplicacao::CEstadoGeralSA, asn::CConversorEstadoGeralSA>::Salva(
    const md::estadoaplicacao::CEstadoGeralSA& estado);

// wasm func 10112 (table slot 433) - observed executing: same with 455/454 and CConversorEstadoGeralGap @1568496
template <>
void IServicoEstado<md::estadoaplicacao::CEstadoGeralGap, asn::CConversorEstadoGeralGap>::Salva(
    const md::estadoaplicacao::CEstadoGeralGap& estado);

// wasm func 11566 (table slot 434): constructor, thunk of the shared service constructor func 3897 with the
// vtable of CServicoEstadoGeralSA (@1558176):
//     {vptr, m_midia = midia, m_turno = turno}; if turno == '3' ("turno atual") the real turno is read from
//     eg.bin of the same medium: m_turno = CServicoEstadoGeral(midia).Carrega().GetTurno()   (+32)
CServicoEstadoGeralSA::CServicoEstadoGeralSA(EFlashOrigem midia, EUrnaTurno turno)
    : m_midia(midia), m_turno(turno)
{
    if (m_turno == EUrnaTurno::Atual)
        m_turno = CServicoEstadoGeral(midia).Carrega().GetTurno();
}

// =========================================================================================================
// Sort comparators (captureless lambdas passed as function pointers; each has its own table slot).
// =========================================================================================================

// wasm func 11559 (table slot 2357) - observed executing.                                  name inferred
// CCargos, ACQUISITION order = the order in which the voter is asked: first the eleição's ordemAquisicao
// (CSituacoesEleicoes record byte +4), then the cargo's ordemAquisicao (CCargo +15). Used when CCargos is
// created (inlined into start-up func 7787) and by vota::CEleitorVotando::IniciaCiclo.
const auto PorOrdemAquisicao = [](const CCargos::SCargoEleicao& a, const CCargos::SCargoEleicao& b) {
    const auto& cfg = CConfiguracaoEleicao::GetInst();                                  // func 187
    const auto ea = cfg.GetSituacoesEleicoes(a.eleicao).GetOrdemAquisicao();            // func 271, +4
    const auto eb = cfg.GetSituacoesEleicoes(b.eleicao).GetOrdemAquisicao();
    if (ea != eb)
        return ea < eb;
    return cfg.GetCargo(a.cargo).GetOrdemAquisicao()                                     // func 861, +15
           < cfg.GetCargo(b.cargo).GetOrdemAquisicao();
};

// wasm func 11558 (table slot 2358).                                                       name inferred
// CCargos::OrdenaPorOrdemImpressao (comum_f3782), PRINT order: eleição ordemImpressao (+5), then the cargo's
// ordemImpressao (CCargo +16). This is the order of the cargos in the BOLETIM DE URNA, the RDV/result files
// and the BU QR codes (callers of 3782: vota::CGeraBU::StartState 12110, CGravaResultado 12098,
// CCargos::GetCurrentEleicaoVersaoPacote 5604).
const auto PorOrdemImpressao = [](const CCargos::SCargoEleicao& a, const CCargos::SCargoEleicao& b) {
    const auto& cfg = CConfiguracaoEleicao::GetInst();
    const auto ea = cfg.GetSituacoesEleicoes(a.eleicao).GetOrdemImpressao();            // +5
    const auto eb = cfg.GetSituacoesEleicoes(b.eleicao).GetOrdemImpressao();
    if (ea != eb)
        return ea < eb;
    return cfg.GetCargo(a.cargo).GetOrdemImpressao() < cfg.GetCargo(b.cargo).GetOrdemImpressao();   // +16
};

// wasm func 11547 (table slot 2363).                                                       name inferred
// CConfiguracaoEleicao::GetCargos(eleicao, ordenado) (comum_f3775; cconfiguracaoeleicao.cpp, path inferred):
// copies the eleição's 140-byte md::CCargo vector and, when `ordenado`, std::sort's it by ordemImpressao.
// Used by comum::CGravadorBU (slot 7, u23: "cargos = cfg.GetCargos(eleição, true)") - the per-eleição cargo
// order inside the BU file - and by vota::CInformacaoEleitor::Inicializar (func 7787, formerly shown as
// CHKDFSeed::GetSeed) and func 5735.
const auto PorOrdemImpressaoCargo = [](const md::CCargo& a, const md::CCargo& b) {
    return a.GetOrdemImpressao() < b.GetOrdemImpressao();                                // byte +16
};

// =========================================================================================================
// atexit destructors of singleton storage (compiler-generated "__dtor_" functions). Emscripten never runs
// atexit handlers here (noExitRuntime), so these are dead code kept only because their addresses sit in the
// function table. Each pair is {std::mutex (pthread_mutex_destroy stub: the folded body func 150),
// std::unique_ptr<T> (reset + ~T + free)}:
//   func 11550 ~std::mutex        @1838756 \  CConfiguracaoEleicao::GetInst (func 187)
//   func 11551 ~unique_ptr        @1838752 /    (~CConfiguracaoEleicao = func 5792)
//   func 11552 ~std::mutex        @1838724 \  singleton of func 2832 (named "CFederacoes"; ~ = func 5800)
//   func 11553 ~unique_ptr        @1838748 /
//   func 11560 ~std::mutex        @1838696 \  CCargos::s_mutex / s_inst (ccargos.h, u04)
//   func 11561 ~unique_ptr        @1838720 /    (~CCargos = func 5805)
//   func 11563 ~std::mutex        @1838668 \  CCandidaturas (GetCandidaturaAtual, ccandidaturas.cpp:261,
//   func 11564 ~unique_ptr        @1838692 /    44 bytes, ctor 5811, ~ = func 3786)
// =========================================================================================================

}  // namespace comum
