// FRAGMENTS reconstructed from vota_web_wasm.wasm by unit u38 ("app:vota: classes without known file").
// Original files: voter-side files of uenux2/src/app/vota/eleitor/..., one section each. "(attested)" =
// the path appears in a std::source_location record; "(path inferred)" = TSE naming convention.
//
// Contents:
//   * 29 exit-time destructor stubs ("__dtor_<variable>", never executed in this build) of the statics
//     behind the voter states' lazy singletons (pattern and evidence: top of
//     src/uenux2/src/app/vota/u38-foreign-fragments.cpp);
//   * wasm func 11240: the column header "Sequencial ... <identifier type>" of the end-of-day report
//     "Eleitores com habilitação biográfica" (behb.dat), written out at the end of this file.

#include <cstddef>
#include <memory>
#include <mutex>
#include <string>

#include "comum/dados/cconfiguracaoeleicao.h"                    // GetTipoIdentificadorPrincipal (+668)
#include "comum/dados/md/eleitor/celeitoridentidade.h"           // ETipoIdentificadorEleitor
#include "vota/eleitor/cconferevotoemcargo.h"
#include "vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.h"
#include "vota/eleitor/votaproporcional/cpedeproporcional.h"
// (+ the headers of the other state classes named below)

namespace vota {

using ETipoIdentificadorEleitor = ecourna::app::dados::ETipoIdentificadorEleitor;

// ======================================================================================================
// uenux2/src/app/vota/eleitor/cconferevotoemcargo.h   (path inferred; cconferevotoemcargo.cpp is attested)
// ======================================================================================================
// CConfereVotoEmCargo<PROXIMO, TELA> is the "conferência" state shown after the voter's CONFIRMA; its
// GetProximoEstado() returns PROXIMO::GetInst() (unit u06). Its own lazy singleton is NOT built from plain
// statics like the other states: the mutex and the unique_ptr are TEMPLATE static data members. Evidence:
// every one of them has a guard word next to it (guard after the 24-byte mutex, guard after the 4-byte
// unique_ptr, so the pair spans 36 bytes and the unique_ptr is at mutex + 28, not + 24), and
// __wasm_call_ctors (func 14478) still sets those 20 guards (10 instantiations x 2) to 1. What is left of
// the linkonce initialiser is "if (!(guard & 1)) guard = 1;" once the __cxa_atexit call is removed.
template <class PROXIMO, ETelaVotacao TELA>
std::mutex CConfereVotoEmCargo<PROXIMO, TELA>::s_mutex;                                         // name inferred
template <class PROXIMO, ETelaVotacao TELA>
std::unique_ptr<CConfereVotoEmCargo<PROXIMO, TELA>> CConfereVotoEmCargo<PROXIMO, TELA>::s_instancia;   // name inferred
//
// The GetInst() of each instantiation is inlined in the voting states:
//   CConfereVotoEmCargo<CMajoritarioValido, (ETelaVotacao)2>   in CPedeMajoritario::ProcessInputAudio (func 11683)
//       s_mutex @1838452 (guard @1838476), s_instancia @1838480 (guard @1838484)
// wasm func 11675 (table slot 2094) - __dtor_CConfereVotoEmCargo<CMajoritarioValido,2>::s_mutex -> ~mutex()   [body = func 150]
// wasm func 11674 (table slot 2095) - __dtor_CConfereVotoEmCargo<CMajoritarioValido,2>::s_instancia -> ~unique_ptr()
//                                     [thunk -> func 1286: ~IConfereVotoEmCargo (1717) + free]
//   CConfereVotoEmCargo<CProporcionalBranco, (ETelaVotacao)4>  in CPedeProporcional::ProcessInputAudio (func 11711)
//       s_mutex @1838112 (guard @1838136), s_instancia @1838140 (guard @1838144)
// wasm func 11709 (table slot 2043) - __dtor_CConfereVotoEmCargo<CProporcionalBranco,4>::s_mutex -> ~mutex()   [body = func 150]
// wasm func 11708 (table slot 2044) - __dtor_CConfereVotoEmCargo<CProporcionalBranco,4>::s_instancia -> ~unique_ptr()
//                                     [thunk -> func 1286]
//   CConfereVotoEmCargo<CConfirmaVotoLegenda, (ETelaVotacao)10> in CPedeNominal::GetProximoEstado (vtable slot 16, func 11724)
//       s_mutex @1837976 (guard @1838000; its stub is func 11723, not u38), s_instancia @1838004 (guard @1838008)
// wasm func 11722 (table slot 2026) - __dtor_CConfereVotoEmCargo<CConfirmaVotoLegenda,10>::s_instancia -> ~unique_ptr()
//                                     [thunk -> func 1286]
// The stubs of the other 7 instantiations were identified by unit u30 (src/uenux2/src/app/vota/u30-foreign-fragments.cpp).
// Where these stubs were emitted: a template static is instantiated (and its destroy helper emitted) in each
// TU that uses it, not in the header. The function order agrees: 11674/11675 sit between the
// CConfereVotoEmCargo<CMajoritario*>::vf16 bodies (11670-11673) and CPedeMajoritario (11682-11684), i.e. in
// votamajoritario/cpedemajoritario.cpp; 11708/11709 sit among the CPedeProporcional functions (11707-11714),
// i.e. in votaproporcional/cpedeproporcional.cpp; 11722 sits next to CPedeNominal (11724/11725), i.e. in
// cpedenominal.cpp (path inferred). Only the definition text above belongs to cconferevotoemcargo.h.

// ======================================================================================================
// uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp   (attested)
// ======================================================================================================
// CPedeProporcional: CPedeProporcional::GetInst (func 3849 -> merged body 6051).
std::mutex CPedeProporcional::s_mutex;   // @1838084
std::unique_ptr<CPedeProporcional> CPedeProporcional::s_instancia;   // @1838108   (its stub: func 11714, not u38)
// wasm func 11713 (table slot 2046) - __dtor_CPedeProporcional::s_mutex -> s_mutex.~mutex()   [body = func 150]

// ======================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp   (attested)
// ======================================================================================================
// testeteclado::impl::IGeradorTeclas: IGeradorTeclas::GetInst (srcloc ctesteteclado.cpp:121) inlined in CTesteTeclado::StartState (func 11805).
std::mutex testeteclado::impl::IGeradorTeclas::s_mutex;   // @1837656
// wasm func 11802 (table slot 1898) - __dtor_IGeradorTeclas::s_mutex -> s_mutex.~mutex()   [body = func 150]

// testeteclado::CTesteTeclado: CTesteTeclado::GetInst (func 3855).
std::mutex testeteclado::CTesteTeclado::s_mutex;   // @1835124
std::unique_ptr<testeteclado::CTesteTeclado> testeteclado::CTesteTeclado::s_instancia;   // @1835148   (its stub: func 11808, not u38)
// wasm func 11807 (table slot 1897) - __dtor_CTesteTeclado::s_mutex -> s_mutex.~mutex()   [body = func 150]

// ======================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.cpp   (path inferred)
// ======================================================================================================
// testeteclado::CEsperaRetestar: CEsperaRetestar::GetInst (func 5936).
std::mutex testeteclado::CEsperaRetestar::s_mutex;   // @1835012
std::unique_ptr<testeteclado::CEsperaRetestar> testeteclado::CEsperaRetestar::s_instancia;   // @1835036
// wasm func 11827 (table slot 1864) - __dtor_CEsperaRetestar::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 11828 (table slot 1863) - __dtor_CEsperaRetestar::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 1564: ~ (ICF 785) + free]

// ======================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cretomada.cpp   (path inferred)
// ======================================================================================================
// testeteclado::CRetomada: GetInst inlined in CAjusteInicial::ValidaTemposDesligamento (func 7160).
std::mutex testeteclado::CRetomada::s_mutex;   // @1834972
std::unique_ptr<testeteclado::CRetomada> testeteclado::CRetomada::s_instancia;   // @1834996
// wasm func 11832 (table slot 1856) - __dtor_CRetomada::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 11833 (table slot 1855) - __dtor_CRetomada::s_instancia -> s_instancia.~unique_ptr()   [inline: testeteclado::CBase::~CBase (1559) + free]

// ======================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cprezeresima.cpp   (attested)
// ======================================================================================================
// testeteclado::CPreZeresima: GetInst inlined in CVerificaEleicaoPassou::StartState (func 11914).
std::mutex testeteclado::CPreZeresima::s_mutex;   // @1834720
std::unique_ptr<testeteclado::CPreZeresima> testeteclado::CPreZeresima::s_instancia;   // @1834744
// wasm func 11863 (table slot 1802) - __dtor_CPreZeresima::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 11864 (table slot 1801) - __dtor_CPreZeresima::s_instancia -> s_instancia.~unique_ptr()   [inline: testeteclado::CBase::~CBase (1559) + free]

// ======================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/cgeradadosdinamicos.cpp   (attested)
// ======================================================================================================
// CGeraDadosDinamicos: GetInst inlined in testeteclado::CPreZeresima::GetEstadoPassouNoTeste (func 5945).
std::mutex CGeraDadosDinamicos::s_mutex;   // @1834692
std::unique_ptr<CGeraDadosDinamicos> CGeraDadosDinamicos::s_instancia;   // @1834716
// wasm func 11866 (table slot 1797) - __dtor_CGeraDadosDinamicos::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 11867 (table slot 1796) - __dtor_CGeraDadosDinamicos::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 349: free only (empty destructor body)]

// ======================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp   (attested)
// ======================================================================================================
// CVerificaHorarioZeresima: CVerificaHorarioZeresima::GetInst (func 5947).
std::mutex CVerificaHorarioZeresima::s_mutex;   // @1834664
std::unique_ptr<CVerificaHorarioZeresima> CVerificaHorarioZeresima::s_instancia;   // @1834688
// wasm func 11872 (table slot 1789) - __dtor_CVerificaHorarioZeresima::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 11873 (table slot 1788) - __dtor_CVerificaHorarioZeresima::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 1564: ~ (ICF 785) + free]

// ======================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/cregerarzeresima.cpp   (path inferred)
// ======================================================================================================
// CRegerarZeresima: GetInst inlined in CConfirmaRegerarZeresima::ProcessInput (func 11838).
std::mutex CRegerarZeresima::s_mutex;   // @1834888   (its stub: func 11843, not u38)
std::unique_ptr<CRegerarZeresima> CRegerarZeresima::s_instancia;   // @1834912
// wasm func 11844 (table slot 1837) - __dtor_CRegerarZeresima::s_instancia -> s_instancia.~unique_ptr()   [inline: CGeraZeresimaBase::~CGeraZeresimaBase (1720) + free]

// ======================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.cpp   (path inferred)
// ======================================================================================================
// CGeraZeresima: CGeraZeresima::GetInst (func 5962).
std::mutex CGeraZeresima::s_mutex;   // @1834188   (its stub: func 11937, not u38)
std::unique_ptr<CGeraZeresima> CGeraZeresima::s_instancia;   // @1834212
// wasm func 11938 (table slot 1690) - __dtor_CGeraZeresima::s_instancia -> s_instancia.~unique_ptr()   [inline: CGeraZeresimaBase::~CGeraZeresimaBase (1720) + free]

// ======================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp   (attested)
// ======================================================================================================
// CVisualizarCandidatos: CVisualizarCandidatos::GetInst (func 1279).
std::mutex CVisualizarCandidatos::s_mutex;   // @1834636
std::unique_ptr<CVisualizarCandidatos> CVisualizarCandidatos::s_instancia;   // @1834660   (its stub: func 11880, not u38)
// wasm func 11878 (table slot 1780) - __dtor_CVisualizarCandidatos::s_mutex -> s_mutex.~mutex()   [body = func 150]

// ======================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporcargo.cpp   (attested)
// ======================================================================================================
// CMenuFiltrarCandidatosPorCargo: GetInst inlined in CMenuVisualizarCandidatos::StartState (func 11881).
std::mutex CMenuFiltrarCandidatosPorCargo::s_mutex;   // @1834552
std::unique_ptr<CMenuFiltrarCandidatosPorCargo> CMenuFiltrarCandidatosPorCargo::s_instancia;   // @1834576
// wasm func 11889 (table slot 1764) - __dtor_CMenuFiltrarCandidatosPorCargo::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 11890 (table slot 1763) - __dtor_CMenuFiltrarCandidatosPorCargo::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 349: free only (empty destructor body)]

// ======================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatospornumero.cpp   (path inferred)
// ======================================================================================================
// CMenuFiltrarCandidatosPorNumero: CMenuFiltrarCandidatosPorNumero::GetInst (func 5954).
std::mutex CMenuFiltrarCandidatosPorNumero::s_mutex;   // @1834524
std::unique_ptr<CMenuFiltrarCandidatosPorNumero> CMenuFiltrarCandidatosPorNumero::s_instancia;   // @1834548
// wasm func 11893 (table slot 1758) - __dtor_CMenuFiltrarCandidatosPorNumero::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 11894 (table slot 1757) - __dtor_CMenuFiltrarCandidatosPorNumero::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 389: ~ (ICF 244) + free]

// ======================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/cmaisinformacoes.cpp   (path inferred)
// ======================================================================================================
// CMaisInformacoes: CMaisInformacoes::GetInst (func 1280).
std::mutex CMaisInformacoes::s_mutex;   // @1834496
std::unique_ptr<CMaisInformacoes> CMaisInformacoes::s_instancia;   // @1834520
// wasm func 11898 (table slot 1752) - __dtor_CMaisInformacoes::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 11899 (table slot 1751) - __dtor_CMaisInformacoes::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 389: ~ (ICF 244) + free]

// ======================================================================================================
// uenux2/src/app/vota/eleitor/creiniciocomparecimentomesario.cpp   (path inferred)
// ======================================================================================================
// CReinicioComparecimentoMesario: CReinicioComparecimentoMesario::GetInst (func 3864 -> merged body 764).
std::mutex CReinicioComparecimentoMesario::s_mutex;   // @1834328
std::unique_ptr<CReinicioComparecimentoMesario> CReinicioComparecimentoMesario::s_instancia;   // @1834352
// wasm func 11918 (table slot 1720) - __dtor_CReinicioComparecimentoMesario::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 11919 (table slot 1719) - __dtor_CReinicioComparecimentoMesario::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 349: free only (empty destructor body)]

// ======================================================================================================
// uenux2/src/app/vota/eleitor/fimvotacao/cgerarelatorios.cpp   (attested: srcloc :51 in StartState)
// ======================================================================================================
// "Eleitores com habilitação biográfica" (relatório BEHB, file behb.dat) is printed at the end of the day
// by CGeraRelatorios::StartState (func 12105), when the urna is biometric and not in demo mode. For each
// section that has voters released by biographic data (habilitação biográfica = the voter was identified
// by the mesário without a fingerprint match), it prints the section heading, then this header line
// (comum_f604(form, 2961, ESQUERDA) = CTextFieldPaper(CDataText<std::string (*)()>(ESQUERDA, &f))), then
// one line per voter: AlinhaEmColunas(std::format("{:04}", sequencial), identidade), where `identidade`
// is CEleitorDecorator::GetIdentidadePorTipo(eleitor, eleitor.tipo principal (+104)) formatted by
// ecourna_f1924. Both the header and the rows use the same inlined padding code with width 38.
namespace {

// Characters in one printed line (normal font). The same total is used by the report helpers:
// CRelUtil::CompletaDireita(rótulo, 28) + "DD/MM/YYYY" = 38, and the "=====" separators are 38 wide.
constexpr std::size_t LARGURA_LINHA = 38;                                             // name inferred

// Inlined into func 11240 and into the row loop of func 12105.                     // name inferred
// The left text, spaces, then the right text flush with column 38. No padding when they do not fit.
// The padding is a NAMED string: the first '+' is libc++'s operator+(const string&, const string&) (fresh
// buffer, memcpy of the left text then of the spaces, inlined). A temporary `std::string(n, ' ')` would
// select operator+(const string&, string&&) = rhs.insert(0, lhs), which does not appear. The second '+'
// is operator+(string&&, const string&) = append (func 160).
std::string AlinhaEmColunas(const std::string& esquerda, const std::string& direita)
{
    const std::size_t ocupado = esquerda.size() + direita.size();
    const std::string espacos(ocupado < LARGURA_LINHA ? LARGURA_LINHA - ocupado : 0, ' ');
    return esquerda + espacos + direita;                         // length_error above max_size (func 161)
}

// Inlined into func 11240. Labels 1 and 2 are the same as in vota::TipoToStr (func 10586,
// cinformacaothreadoperador.cpp:40), but NOT label 3: TipoToStr says "Identificador" (13 chars @86243),
// this function "Número livre" (assigned through string::assign, func 276). An unknown value gives ""
// here instead of throwing 9409 "Tipo de identidade do eleitor inválido".
std::string NomeTipoIdentificador(const ETipoIdentificadorEleitor tipo)               // name inferred
{
    std::string nome;
    switch (tipo) {
    case ETipoIdentificadorEleitor::TITULO: nome = "Título";       break;   // 6 chars (Latin-1)
    case ETipoIdentificadorEleitor::CPF:    nome = "CPF";          break;
    case ETipoIdentificadorEleitor::LIVRE:  nome = "Número livre"; break;   // string::assign (func 276)
    }
    return nome;
}

// wasm func 11240 (table slot 2961; not observed: the recorded end-of-day run printed "Nenhum eleitor passou
// por habilitação biográfica", so the header was not drawn).                         // name inferred
// e.g. "Sequencial                      Título"  (10 + 22 spaces + 6 = 38 columns)
std::string CabecalhoSequencialIdentificador()
{
    const std::string identificador =
        NomeTipoIdentificador(comum::CConfiguracaoEleicao::GetInst().GetTipoIdentificadorPrincipal());   // func 187, +668
    return AlinhaEmColunas("Sequencial", identificador);
}

}  // namespace

}  // namespace vota
