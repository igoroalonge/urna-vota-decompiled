// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred): uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp
//
// 26 of the 37 slots. The others: [5] AtualizaEstadoRegistro 10797 (u20-foreign-fragments.cpp),
// [9] GetEstadoAposRegistroInicial 10794 (operador/u27-foreign-fragments.cpp), [10]/[15]/[17]/[18]
// 10792/10787/10788/10785 (u18-foreign-fragments.cpp), [16] SincronizaBancoDados 10786 (comum/csincronizavota.u02.cpp),
// [28] LogaLimiteMesariosAtingido 10774 (u26-foreign-fragments.cpp), [8] ICF 434.
//
// Log helpers: every record goes to the urna event log (logd.dat, "1|<severity>|<text>") through
// CLogVota (IEventosLog). wasm-opt merged the bodies by shape:
//   func 3888 = "log a 47-byte literal" (the six 8-byte chunks of the literal are its parameters): slots 29, 34, 36
//   func 3887 = "log std::format(fmt, título do mesário)" (fmt passed as [begin, end)): slots 31, 32, 33
//   func 6017 = "log std::format(fmt, título given as argument)": slots 19, 27
// The others build their literal inline and call IEventosLog::Loga (api_f233) or api::CLoga::loga.
//
// WEB BUILD: dead code - the controller is only registered by CAjusteInicial, which the web build skips.
#include "vota/comum/ccontroladorregistramesariosvota.h"

#include <format>
#include <memory>
#include <mutex>

#include "comum/appinfo/cappinfo.h"
#include "comum/comparecimentomesario/cregistradormesario.h"   // comum::CRegistradorMesario (func 815)
#include "comum/dados/celeitores.h"
#include "vota/log/clogvota.h"
#include "vota/operador/cfinalizaoperador.h"
#include "vota/operador/comparecimentomesario/cregistromesarioencerrado.h"
#include "vota/operador/cthreadoperador.h"

namespace vota {

using comum::EPeriodoRegistro;
using comum::md::estadoaplicacao::EEstadoVota;
using ecourna::app::dados::ETipoIdentificadorEleitor;

namespace {

// Periods of comum_comparecimento_mesario.periodo (CComparecimentoMesarioPK): 1 = abertura, 2 = encerramento.
constexpr int PERIODO_ABERTURA = 1;                                                    // name inferred
constexpr int PERIODO_ENCERRAMENTO = 2;                                                // name inferred

EEstadoVota EstadoVotaAtual()
{
    return comum::CAppInfo::GetInst().GetVota(comum::EUrnaTurno::ATUAL).GetEstadoVota();   // funcs 185, 261 ('3')
}

}  // namespace

// =========================================================================================================
// Operator-thread ticks (the comum registration states run on CThreadOperador)

// wasm func 10800 - slot 2
uebyte CControladorRegistraMesariosVota::CriaTick(std::uint32_t ms)
{
    return CThreadOperador::GetInst().CriaTick(ms);           // inlined: m_ticks (+20).AddTick(ms), func 3644
}

// wasm func 10799 - slot 3
void CControladorRegistraMesariosVota::StopTick(uebyte tick)
{
    CThreadOperador::GetInst().StopTick(tick);                // func 422
}

// wasm func 10798 - slot 4
void CControladorRegistraMesariosVota::StartTick(uebyte tick)
{
    CThreadOperador::GetInst().StartTick(tick);               // func 700
}

// =========================================================================================================
// Period and limit

// wasm func 10796 - slot 6. Compiled as a bounds check (estado - '7' <= 3, unsigned) plus the lookup table
// @534720 = {1, 2, 0, 3}.
EPeriodoRegistro CControladorRegistraMesariosVota::GetPeriodoRegistro() const
{
    switch (EstadoVotaAtual()) {
    case EEstadoVota::EAVREGISTROMESARIOINICIAL:              // '7'
        return EPeriodoRegistro::INICIAL;
    case EEstadoVota::EAVVOTAR:                               // '8'
        return EPeriodoRegistro::VOTACAO;
    case EEstadoVota::EAVREGISTROMESARIOFINAL:                // ':'
        return EPeriodoRegistro::FINAL;
    default:                                                  // '9' FIMAQUISICAOVOTOS and everything else
        return EPeriodoRegistro::NENHUM;
    }
}

// wasm func 10795 - slot 7. At most 6 mesários per period; the opening count covers both the registration
// before voting and the one during voting. Any other estadoVota answers "limit reached".
bool CControladorRegistraMesariosVota::LimiteMesariosAtingido() const
{
    const auto& registrador = comum::CRegistradorMesario::GetInst();                    // func 815
    switch (EstadoVotaAtual()) {
    case EEstadoVota::EAVREGISTROMESARIOINICIAL:
    case EEstadoVota::EAVVOTAR:
        return registrador.QuantidadeRegistrados(PERIODO_ABERTURA) >= comum::QTD_MAXIMA_MESARIOS;     // 3606 -> 6007; "> 5"
    case EEstadoVota::EAVREGISTROMESARIOFINAL:
        return registrador.QuantidadeRegistrados(PERIODO_ENCERRAMENTO) >= comum::QTD_MAXIMA_MESARIOS; // 2727 -> 6007
    default:
        return true;
    }
}

// =========================================================================================================
// Where the operator thread goes when a registration round ends

// wasm func 10793 - slot 11: registration DURING voting finished. CRegistroMesarioEncerrado (12 bytes,
// CAppState(1 = messages), vtable @1586776) posts message 14 to the voter terminal in its StartState
// (func 10763, u18). Lazy singleton: s_instancia @1904888, s_mutex @1904864 (static data members of
// cregistromesarioencerrado.cpp); constructor + GetInst inlined here.
comum::CAppState* CControladorRegistraMesariosVota::GetEstadoAposRegistroVotacao()
{
    return &CRegistroMesarioEncerrado::GetInst();
}

// wasm func 10791 - slot 12: registration at the CLOSING finished (the voter terminal was told to generate
// the BU by slot 15).
comum::CAppState* CControladorRegistraMesariosVota::GetEstadoAposRegistroFinal()
{
    return &CFinalizaOperador::GetInst();                     // func 5342 (vota_f764, flags 0)
}

// =========================================================================================================
// The mesário being registered, looked up in the section's roll by the título typed on the MT
// (CThreadOperador +96, slots 17/18).

// wasm func 10790 - slot 13
bool CControladorRegistraMesariosVota::MesarioEhEleitorDaSecao() const
{
    return comum::CEleitores::GetInst().Busca(CThreadOperador::GetInst().m_tituloMesario,       // func 2264
                                              ETipoIdentificadorEleitor::TITULO) != nullptr;    // tipo 1
}

// wasm func 10789 - slot 14
const comum::CEleitorDetalhe* CControladorRegistraMesariosVota::GetEleitorMesario() const
{
    return comum::CEleitores::GetInst().Busca(CThreadOperador::GetInst().m_tituloMesario,
                                              ETipoIdentificadorEleitor::TITULO);
}

// =========================================================================================================
// Log records (all severity 1 unless stated)

// wasm func 10784 - slot 19 (merged body 6017)
void CControladorRegistraMesariosVota::LogaMesarioRegistrado(const std::string& titulo)
{
    CLogVota::GetInst().Loga(std::format("Mesário {} registrado", titulo));
}

// wasm func 10783 - slot 20
void CControladorRegistraMesariosVota::LogaRegistroAntesVotacao()
{
    CLogVota::GetInst().Loga("Registrando mesários antes da votação");
}

// wasm func 10782 - slot 21
void CControladorRegistraMesariosVota::LogaRegistroDuranteVotacao()
{
    CLogVota::GetInst().Loga("Registrando mesários durante a votação");
}

// wasm func 10781 - slot 22
void CControladorRegistraMesariosVota::LogaRegistroAposVotacao()
{
    CLogVota::GetInst().Loga("Registrando mesários após a votação");
}

// wasm func 10779 - slot 23 (api::CLoga::loga inline)
void CControladorRegistraMesariosVota::LogaIndagadoRegistro()
{
    CLogVota::GetInst().Loga(api::ESeveridade{1}, "Operador indagado se ocorrerá registro de mesários");
}

// wasm func 10778 - slot 24
void CControladorRegistraMesariosVota::LogaConfirmouRegistro()
{
    CLogVota::GetInst().Loga("Operador confirmou o registro de mesários");
}

// wasm func 10777 - slot 25
void CControladorRegistraMesariosVota::LogaCancelouRegistro()
{
    CLogVota::GetInst().Loga("Operador cancelou o registro de mesários");
}

// wasm func 10776 - slot 26 (warning, api::CLoga::loga inline)
void CControladorRegistraMesariosVota::LogaTituloInvalido()
{
    CLogVota::GetInst().Loga(api::ESeveridade{2}, "Digitado título inválido para o registro de mesário");
}

// wasm func 10775 - slot 27 (merged body 6017)
void CControladorRegistraMesariosVota::LogaMesarioJaRegistrado(const std::string& titulo)
{
    CLogVota::GetInst().Loga(std::format("Mesário {} já registrado", titulo));
}

// wasm func 10773 - slot 29 (merged body 3888)
void CControladorRegistraMesariosVota::LogaEncerrouRegistro()
{
    CLogVota::GetInst().Loga("Operador encerrou ciclo de registro de mesários");
}

// wasm func 10772 - slot 30. Four int scores (format-arg types 0x18C63 = 4 x int), read with operator[]
// from the vector built by CPedeDigitalMesario (always 4 elements, filled in the order of the fingers the
// mesário HAS in the roll - see the unit doc: the labels below can be misattributed).
void CControladorRegistraMesariosVota::LogaDigitalNaoCorresponde(const std::vector<int>& scores)
{
    CLogVota::GetInst().Loga(std::format(
        "Digital capturada não corresponde a digital do eleitor: Polegar Direito [score {}], "
        "Polegar Esquerdo [score {}], Indicador Direito [score {}], Indicador Esquerdo [score {}]",
        scores[0], scores[1], scores[2], scores[3]));
}

// wasm func 10771 - slot 31 (merged body 3887: the título comes from CThreadOperador +96)
void CControladorRegistraMesariosVota::LogaPedidoLeituraBiometria()
{
    CLogVota::GetInst().Loga(std::format("Pedido de leitura da biometria do mesário {}", GetTituloMesario()));
}

// wasm func 10770 - slot 32 (merged body 3887)
void CControladorRegistraMesariosVota::LogaMesarioEhEleitor()
{
    CLogVota::GetInst().Loga(std::format("Mesário {} é eleitor da seção", GetTituloMesario()));
}

// wasm func 10769 - slot 33 (merged body 3887)
void CControladorRegistraMesariosVota::LogaMesarioNaoEhEleitor()
{
    CLogVota::GetInst().Loga(std::format("Mesário {} não é eleitor da seção", GetTituloMesario()));
}

// wasm func 10768 - slot 34 (merged body 3888)
void CControladorRegistraMesariosVota::LogaConferenciaBiometria()
{
    CLogVota::GetInst().Loga("Realizada a conferência da biometria do mesário");
}

// wasm func 10767 - slot 35
void CControladorRegistraMesariosVota::LogaIndagadoContinuarRegistro()
{
    CLogVota::GetInst().Loga("Operador indagado se continua registrando mesários");
}

// wasm func 10766 - slot 36 (merged body 3888)
void CControladorRegistraMesariosVota::LogaIndagadoFinalizarRegistro()
{
    CLogVota::GetInst().Loga("Operador indagado se finaliza registro mesários");
}

}  // namespace vota
