// Reconstructed from vota_web_wasm.wasm (unit u04). Original: uenux2/src/app/comum/dados/ccargos.cpp
//
// Error type: CErroDados = ecourna::api::exception::CBaseError<comum::EUeComumDadosError>
//   (typeinfo @1528076, vtable @1528096). Every throw site builds it through the thunk comum_f170
//   (code, message, std::source_location). The constructor takes the source_location as a defaulted
//   `std::source_location::current()` argument: that is where the "srcloc line N" of each function
//   comes from.
// Singleton errors: CErroPattern = CBaseError<ecourna::api::pattern::EPatternErr> (typeinfo @1526600),
//   code 1303 "... - instancia nao criada", 1304 "... - instancia ja criada".
//
// Functions of this file that are NOT here because they only exist inlined elsewhere:
//   CCargos::CreateInst (line 25)             -> inlined into 7787 (start-up, unit u02)
//   CCargos::FiltraPorAbrangencia (line 249)  -> inlined into vota::CEleitorVotando::IniciaCiclo (7377)
//   CCargos::GetCurrentEleicaoVersaoPacote (93) -> inlined into the BU QR-code generator (5604)
// Function 5604 (22.8 KB, the BU QR-code generator) and its helpers 5605/5606/5608/5616/5617/6028
// were attributed to this file only because they inline GetCurrentEleicaoVersaoPacote; they are
// reconstructed in ../relatorios/cgeradorbuqrcode.u04-fragment.cpp.
#include "ccargos.h"

#include <algorithm>
#include <format>

#include "cconfiguracaoeleicao.h"
#include "md/processoeleitoral/ccargo.h"
#include "md/processoeleitoral/celeicaope.h"

namespace comum {

using CErroDados   = ecourna::api::exception::CBaseError<EUeComumDadosError>;
using CErroPattern = ecourna::api::exception::CBaseError<ecourna::api::pattern::EPatternErr>;

std::mutex CCargos::s_mutex;
CCargos*   CCargos::s_inst = nullptr;

namespace {
// Helpers that live outside u04 (listed so the code below reads naturally):
//  comum_f3774: one SCargoEleicao per cargo of every CEleicaoPE of CConfiguracaoEleicao's pleito
std::vector<CCargos::SCargoEleicao> MontaCargos(const CConfiguracaoEleicao& cfg);
//  wasm 11559 (table slot 2357): compare CPleito::GetSituacoesEleicoes(eleicao) byte +4, then
//  CCargo::ordemAquisicao (+15)  -> order in which the voter is asked for each cargo
bool ComparaOrdemAquisicao(const CCargos::SCargoEleicao& a, const CCargos::SCargoEleicao& b);
//  wasm 11558 (table slot 2358): same with byte +5 and CCargo::ordemImpressao (+16)
bool ComparaOrdemImpressao(const CCargos::SCargoEleicao& a, const CCargos::SCargoEleicao& b);
}  // namespace

// wasm func 273 (srcloc line 24). The shared body comum_f1406 is used by every GetInst of the
// module (CConfiguracaoEleicao, CEleitores, CHV, CPE, CRdvVota, CRespostas, CValidadorIdentidade...).
CCargos& CCargos::GetInst()
{
    std::lock_guard lock(s_mutex);
    if (s_inst == nullptr)
        throw CErroPattern(ecourna::api::pattern::EPatternErr{1303}, "CCargos - instancia nao criada");
    return *s_inst;
}

// ccargos.cpp:25 - reconstructed from its inlined copy in 7787 (for reference only).
void CCargos::CreateInst()
{
    std::lock_guard lock(s_mutex);
    if (s_inst != nullptr)
        throw CErroPattern(ecourna::api::pattern::EPatternErr{1304}, "CCargos - instancia ja criada");
    auto* novo = new CCargos;
    novo->m_todos  = MontaCargos(CConfiguracaoEleicao::GetInst());   // comum_f3774
    novo->m_cargos = MontaCargos(CConfiguracaoEleicao::GetInst());
    // voting order: CSituacoesEleicoes::ordemAquisicao (+4) then CCargo::ordemAquisicao (+15)
    std::sort(novo->m_cargos.begin(), novo->m_cargos.end(), ComparaOrdemAquisicao);  // wasm 11559
    novo->m_indice = 0;
    delete std::exchange(s_inst, novo);
}

// wasm func 2269 - name inferred. Rewinds the cursor (a[0] = 0). Called before every walk over
// the offices: CEleitorVotando::IniciaCiclo, CParteCargos::Imprime, CGravaResultado, CGeraBU and
// the BU QR-code generator.
void CCargos::First()
{
    m_indice = 0;
}

// shared_f602 (not in u04) - name inferred.
bool CCargos::IsEnd() const
{
    return m_indice >= m_cargos.size();
}

// wasm func 1708 (srcloc line 46)
void CCargos::Next()
{
    if (m_indice == m_cargos.size())
        throw CErroDados(EUeComumDadosError{7815}, "Operação inválida.");
    ++m_indice;
}

// wasm func 332 (srcloc line 59). Observed executing during the recorded votes.
const md::CCargo& CCargos::GetCurrent() const
{
    if (m_indice >= m_cargos.size())
        throw CErroDados(EUeComumDadosError{7816}, "Cargo não posicionado.");
    return CConfiguracaoEleicao::GetInst().GetCargo(m_cargos[m_indice].cargo);
}

// wasm func 1938 (srcloc line 68). Observed executing during the recorded votes.
TCargoID CCargos::GetCurrentCargoID() const
{
    if (m_indice >= m_cargos.size())
        throw CErroDados(EUeComumDadosError{7817}, "Cargo não posicionado.");
    return m_cargos[m_indice].cargo;
}

// wasm func 2836 (srcloc line 76). CConfiguracaoEleicao::GetEleicao (cconfiguracaoeleicao.cpp:293)
// is inlined here.
const md::CEleicaoPE& CCargos::GetCurrentEleicao() const
{
    if (m_indice >= m_cargos.size())
        throw CErroDados(EUeComumDadosError{7818}, "Cargo não posicionado.");
    return CConfiguracaoEleicao::GetInst().GetEleicao(m_cargos[m_indice].eleicao);
}

// wasm func 2835 (srcloc line 85)
TEleicaoID CCargos::GetCurrentEleicaoID() const
{
    if (m_indice >= m_cargos.size())
        throw CErroDados(EUeComumDadosError{7819}, "Cargo não posicionado.");
    return m_cargos[m_indice].eleicao;
}

// ccargos.cpp:93 - only exists inlined (twice) in the BU QR-code generator (wasm 5604), together
// with md::CPleito::GetVersaoPacoteEleicao (cpleito.cpp:158, code 8163
// "Versão de pacote não encontrada", a lower_bound in CPleito's map<TEleicaoID, std::string>).
// Used for the "VERC:" field of a referendum ("consulta") cargo.
std::string CCargos::GetCurrentEleicaoVersaoPacote() const
{
    if (m_indice >= m_cargos.size())
        throw CErroDados(EUeComumDadosError{7820}, "Cargo não posicionado.");
    return CConfiguracaoEleicao::GetInst().GetPleito().GetVersaoPacoteEleicao(m_cargos[m_indice].eleicao);
}

// ccargos.cpp:249 - only exists inlined into vota::CEleitorVotando::IniciaCiclo (7377, unit u06).
// Keeps the cargos the voter may vote for, given the voter's "abrangência" (see
// CEleitores::GetCurrentAbrangenciaEleitor): a voter of this município votes every cargo; a voter
// "em trânsito" from another município of the same UF votes state + federal cargos; a voter from
// another UF votes federal cargos only.
void CCargos::FiltraPorAbrangencia(md::ETipoAbrangencia abr)
{
    std::vector<SCargoEleicao> filtrados;
    if (abr == md::ETipoAbrangencia::MUNICIPAL) {
        filtrados = m_todos;
    } else {
        // The switch sits inside the loop: an invalid value only throws when m_todos is not empty.
        for (const auto& c : m_todos) {
            const md::CCargo& cargo = CConfiguracaoEleicao::GetInst().GetCargo(c.cargo);
            switch (abr) {
            case md::ETipoAbrangencia::ESTADUAL:            // cargo abrangência (CCargo +8) in {1,2}
                if (cargo.GetAbrangencia() == md::ETipoAbrangencia::ESTADUAL ||
                    cargo.GetAbrangencia() == md::ETipoAbrangencia::FEDERAL)
                    filtrados.push_back(c);
                break;
            case md::ETipoAbrangencia::FEDERAL:
                if (cargo.GetAbrangencia() == md::ETipoAbrangencia::FEDERAL)
                    filtrados.push_back(c);
                break;
            default:
                throw CErroDados(EUeComumDadosError{7821}, std::format("Tipo de abrangência inválido: {}", abr));
            }
        }
    }
    m_cargos = std::move(filtrados);
    std::sort(m_cargos.begin(), m_cargos.end(), ComparaOrdemAquisicao);   // wasm 11559 (slot 2357)
    // (the caller then calls First())
}

// comum_f3782 (not in u04) - name inferred. Print/apuração order used by the BU, the RDV writer and
// the QR code: CSituacoesEleicoes::ordemImpressao (+5) then CCargo::ordemImpressao (+16).
void CCargos::OrdenaPorOrdemImpressao()
{
    std::sort(m_cargos.begin(), m_cargos.end(), ComparaOrdemImpressao);   // wasm 11558
    m_indice = 0;
}

}  // namespace comum
