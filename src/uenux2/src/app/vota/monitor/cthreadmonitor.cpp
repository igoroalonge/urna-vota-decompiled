// uenux2/src/app/vota/monitor/cthreadmonitor.cpp
// Reconstructed from vota_web_wasm.wasm (unit u26). std::source_location records of this file:
//   :52  CThreadMonitor::CThreadMonitor()                  (inlined in func 1898)
//   :93  bool CThreadMonitor::VotacaoSuspensa()            (inlined in func 10226)
//   :103 SaiPorVotacaoSuspensa()::(lambda)                 (func 10224, reconstructed by unit u19)
//   :117 void CThreadMonitor::MostraMensagemDesligamento() (record only: no code references it)
//   :127 void CThreadMonitor::MonitorFoneOuvido()          (inlined in func 10226)
//   :153/:156 void CThreadMonitor::VerificaErroCriticoFlash()
//   :165/:167 void CThreadMonitor::VerificaErroCriticoKbdTE()
//   :225 void CThreadMonitor::LogaEspaco() const
//   :258/:260 void CThreadMonitor::LogaStatusRedeAcBateria() const
// The CThreadVota methods that the tools also filed here (funcs 2126, 7709, 7710 and their helpers
// 4633, 5569) are in vota/comum/cthreadvota.cpp; CApplicationContextStack::Top (1695) and
// CApplication::ShowExceptionMsg (5568) in src/uenux2/src/api/u26-foreign-fragments.cpp.
//
// Error class of the throws below: vota::CUeVotaError = CBaseError<vota::EUeVotaError, SErrorLimits{9300,
// 9500}> (typeinfo @1532388), built through the merged thunk func 253.
#include "vota/monitor/cthreadmonitor.h"

#include <chrono>
#include <cstdint>
#include <fcntl.h>
#include <filesystem>
#include <format>
#include <future>
#include <memory>
#include <source_location>
#include <string>
#include <system_error>
#include <thread>
#include <unistd.h>
#include <utility>

#include "api/hwil/ibeep.h"
#include "api/hwil/ipower.h"
#include "api/hwil/iurna.h"
#include "api/pattern/cpolysingletonlist.h"
#include "api/util/isystemdatetime.h"
#include "comum/appinfo/cappinfo.h"
#include "comum/cpath.h"
#include "comum/iinterfaceinit.h"
#include "comum/util/cmonitoraalimentacao.h"
#include "comum/util/formatatamanho.h"          // comum::util::FormataTamanho (func 1947, path inferred)
#include "ecourna/api/util/cstringutils.h"
#include "vota/comum/csincronizavota.h"
#include "vota/comum/votadefs.h"                // vota::CUeVotaError
#include "vota/eleitor/cthreadeleitor.h"
#include "vota/log/clogvota.h"
#include "vota/operador/cthreadoperador.h"

namespace vota {

std::mutex CThreadMonitor::s_mutex;
std::unique_ptr<CThreadMonitor> CThreadMonitor::s_instancia;

namespace {

using comum::EFlashOrigem;

// wasm func 5454 (name inferred). Free bytes of the file system holding `caminho` (space_info::free =
// f_bfree * f_frsize). The THROWING overload std::filesystem::space(path) is used: func 4677 (libc++ __space)
// has no error_code parameter left (wasm-opt removed it because both callers pass nullptr) and stores
// ec = 0 in its ErrorHandler, so a statfs failure throws std::filesystem::filesystem_error("space")
// (shared_f4794) out of CThreadMonitor::Run.
std::uintmax_t EspacoLivre(const std::string& caminho)
{
    return std::filesystem::space(caminho).free;                // func 4677 = libc++ __space(p, nullptr)
}

// wasm func 5453 (name inferred). capacity - free (same throwing overload).
std::uintmax_t EspacoUtilizado(const std::string& caminho)
{
    const auto espaco = std::filesystem::space(caminho);
    return espaco.capacity - espaco.free;
}

// wasm func 5463 (name inferred). Value (in kB) of one line of /proc/meminfo, e.g. "MemTotal:  16384 kB".
// Returns 0 when the file cannot be opened, the key is absent or the number is not followed by a
// non-digit on its line. In the browser MEMFS has no /proc/meminfo, so this always returns 0.
unsigned LeValorMemInfo(const std::string& chave)
{
    unsigned valor = 0;
    auto buffer = std::make_unique<char[]>(4096);                // operator new(4096) + memset 0
    const int fd = ::open("/proc/meminfo", O_RDONLY);
    if (fd < 0)
        return valor;
    const ssize_t lidos = ::read(fd, buffer.get(), 4096);
    ::close(fd);

    std::string conteudo(buffer.get());
    conteudo.resize(std::min<std::size_t>(conteudo.size(), static_cast<std::size_t>(lidos)));

    const auto posicao = conteudo.find(chave);
    if (posicao == std::string::npos)
        return valor;
    const std::string linha = conteudo.substr(posicao, conteudo.find('\n', posicao) - posicao);
    const auto inicio = linha.find_first_of("0123456789");
    if (inicio == std::string::npos)
        return valor;
    const auto fim = linha.find_first_not_of("0123456789", inicio);
    if (fim == std::string::npos)
        return valor;
    valor = ecourna::api::util::CStringUtils::ToDWord(linha.substr(inicio, fim - inicio));
    return valor;
}

// Status word of api::IPower (IPower+4, refreshed by vtable slot 15 = AtualizaStatus). Bit names inferred
// from the messages printed when they are set.
constexpr std::uint32_t CHAVE_DESLIGADA       = 0x4000;   // int +4, bit 14 -> "URNA DESLIGADA"
constexpr std::uint32_t MIDIA_EXTERNA_AUSENTE = 0x0200;   // int +4, bit 9 (byte +5 & 2)
constexpr std::uint32_t REDE_CA_AUSENTE       = 0x0006;   // int +4, bits 1..2 = tipo de alimentação != 0
constexpr std::uint32_t FONE_DESCONECTADO     = 0x0002;   // int +8, bit 1
constexpr std::uint8_t  TECLADO_DESCONECTADO  = 0x01;     // byte +19, bit 0

} // namespace

// ------------------------------------------------------------------------------------------------------
// wasm func 1898 (GetInst with the constructor, and CMonitoraAlimentacao::CreateInst, inlined)  name inferred
CThreadMonitor& CThreadMonitor::GetInst()
{
    std::lock_guard trava(s_mutex);
    if (!s_instancia)
        s_instancia.reset(new CThreadMonitor());
    return *s_instancia;
}

// cthreadmonitor.cpp:52 - inlined into func 1898
CThreadMonitor::CThreadMonitor()
    : CThreadVota()                                                        // api::CThread() = func 3598
{
    auto& power = api::CPolySingletonList::instance<api::IPower>();       // :52 (func 862)
    m_bFoneConectado = !(power.GetStatus().flags & FONE_DESCONECTADO);
    m_proximoLog = api::CPolySingletonList::instance<api::ISystemDateTime>().GetDataHora();   // func 1155, slot 0
    comum::util::CMonitoraAlimentacao::CreateInst(api::ELogAplicativos(1));                   // cmonitoraalimentacao.cpp:29
}

// wasm func 10227 (vtable slot 1): deleting destructor = CThreadVota::~CThreadVota (2126) + operator delete.
// wasm func 10229: exit-time reset of s_instancia (s_instancia.reset()).

// ------------------------------------------------------------------------------------------------------
// wasm func 10226 (vtable slot 2). The thread body.
void CThreadMonitor::Run()
{
    auto& alimentacao = comum::util::CMonitoraAlimentacao::GetInst();     // cmonitoraalimentacao.cpp:24
    do {
        alimentacao.VerificaAlimentacao();                                 // cmonitoraalimentacao.cpp:52
        MonitorFoneOuvido();
        if (VotacaoSuspensa())
            SaiPorVotacaoSuspensa();       // never returns normally in this build (see below)
        VerificaErroCriticoFlash();
        VerificaErroCriticoKbdTE();

        const auto agora = api::CPolySingletonList::instance<api::ISystemDateTime>().GetDataHora();
        if (agora >= m_proximoLog) {
            LogaEspaco();
            LogaMemoria();
            LogaStatusRedeAcBateria();
            m_proximoLog = agora + INTERVALO_LOG_S;
        }

        // Compiled as `if (byte@1584624 == 1) emscripten_sleep(500)`: aborts in this build (no Asyncify).
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    } while (!m_bParar);
}

// wasm func 10225 (vtable slot 5). Called by CThreadVota's exception handlers (7709/7710) before the
// fatal-error screen: stops the two state-machine threads (their loops test m_bParar, CThread +8).
void CThreadMonitor::FinalizaExecucao()
{
    CThreadOperador::GetInst().Parar();        // func 270, +8 = true
    CThreadEleitor::GetInst().Parar();         // func 316, +8 = true
}

// ------------------------------------------------------------------------------------------------------
// :127 (inlined into Run)
void CThreadMonitor::MonitorFoneOuvido()
{
    auto& power = api::CPolySingletonList::instance<api::IPower>();       // :127
    const bool desconectado = power.GetStatus().flags & FONE_DESCONECTADO;
    if (!desconectado != m_bFoneConectado) {
        CLogVota::GetInst().Loga(desconectado ? "Fone de ouvido desconectado" : "Fone de ouvido conectado");
        m_bFoneConectado = !desconectado;
    }
}

// :93 (inlined into Run). The power key was turned to "off".
bool CThreadMonitor::VotacaoSuspensa()
{
    return api::CPolySingletonList::instance<api::IPower>().GetStatus().bits & CHAVE_DESLIGADA;   // :93
}

// Inlined into Run. The lambda (srcloc :103) is func 10224, reconstructed by unit u19
// (src/uenux2/src/app/vota/monitor/cthreadmonitor.u19.cpp): it plays an IBeep pattern (slot 4).
void CThreadMonitor::SaiPorVotacaoSuspensa()
{
    CLogVota::GetInst().Loga("Mudança do estado da chave: URNA DESLIGADA");  // CLoga::loga(app, 1, ...)
    CSincronizaVota::MarcaUrnaDesligando();                                   // func 3336: byte @1832936 = 1
    CThreadOperador::GetInst().Parar();
    CThreadEleitor::GetInst().Parar();
    auto aviso = std::async(std::launch::async, [] {                          // :103
        api::CPolySingletonList::instance<api::IBeep>().vf4();
    });
    // In this build std::async cannot create a thread: the compiler reduced the call to
    // "allocate the shared state + std::__thread_struct (func 2541) + throw std::system_error(
    // 'thread constructor failed')" (shared_f1223), so whatever followed here is not in the binary.   ?
}

// :153 / :156 (inlined into Run). The external flash (MV) disappeared: fatal error, except in
// "treinamento" phase (CEstadoGeral fase '3').
void CThreadMonitor::VerificaErroCriticoFlash()
{
    auto& power = api::CPolySingletonList::instance<api::IPower>();                         // :153
    if (comum::CAppInfo::GetInst().GetGeral().GetFase() == comum::EFase::Treinamento)        // +48 == '3'
        return;
    if (power.GetStatus().bits & MIDIA_EXTERNA_AUSENTE)
        throw CUeVotaError(9390, "Erro na Mídia Externa - Mídia não está presente");         // :156
}

// :165 / :167 (inlined into Run). The voter terminal keyboard ("teclado do TE") is reported through the
// same power/controller status block.
void CThreadMonitor::VerificaErroCriticoKbdTE()
{
    auto& power = api::CPolySingletonList::instance<api::IPower>();                         // :165
    if (power.GetStatus().teclado & TECLADO_DESCONECTADO)                                     // byte +19
        throw CUeVotaError(9391, "Erro no teclado do eleitor - dispositivo desconectado");   // :167
}

// :225 (inlined into Run). Sizes are formatted by FormataTamanho(size_t): on wasm32 size_t is 32 bits, so
// the 64-bit byte counts are truncated modulo 4 GiB before formatting.
void CThreadMonitor::LogaEspaco() const
{
    const auto livreMI  = EspacoLivre(comum::CPath::GetPathDinamico(EFlashOrigem::INTERNA));      // func 1082 (0)
    const auto livreMV  = EspacoLivre(comum::CPath::GetPathDinamico(EFlashOrigem::EXTERNA));      // func 1082 (1)
    const auto usadoMI  = EspacoUtilizado(comum::CPath::GetPathDinamico(EFlashOrigem::INTERNA));
    const auto usadoMV  = EspacoUtilizado(comum::CPath::GetPathDinamico(EFlashOrigem::EXTERNA));

    auto& log = CLogVota::GetInst();
    log.Loga(std::format("Espaço livre na MI [{}]", comum::util::FormataTamanho(livreMI)));
    log.Loga(std::format("Espaço livre na MV [{}]", comum::util::FormataTamanho(livreMV)));
    log.Loga(std::format("Espaço utilizado na MI [{}]", comum::util::FormataTamanho(usadoMI)));
    log.Loga(std::format("Espaço utilizado na MV [{}]", comum::util::FormataTamanho(usadoMV)));

    // IInterfaceInit::LogDiskInfo() (iinterfaceinit.cpp:285, inlined): EnviarMensagem(19) (slot 5);
    // empty answer -> CUeComumError 7277 "Resposta nula"; answer != 0 -> DebugUenux(
    // "%s: erro [%d] ao tentar logar informações sobre as partições", "LogDiskInfo", resposta[0]).
    comum::IInterfaceInit::GetInst().LogDiskInfo();                                          // :225 (func 611)
}

// Inlined into Run (name inferred). /proc/meminfo is in kB; the log gets bytes.
void CThreadMonitor::LogaMemoria() const
{
    const int total = static_cast<int>(LeValorMemInfo("MemTotal"));
    const int livre = static_cast<int>(LeValorMemInfo("MemFree"));
    auto& log = CLogVota::GetInst();
    log.LogaQuantidade({"memória total", std::int64_t{total} * 1024});                       // func 3268
    log.LogaQuantidade({"memória livre", std::int64_t{livre} * 1024});
    log.LogaQuantidade({"memória usada", (std::int64_t{total} - livre) * 1024});
}

// :258 / :260 (inlined into Run). Only urnas of model 2020 or later report voltages. Values of the IPower
// getters are in hundredths (V, A). IPower slot names inferred from this use:
//   slot 1 tensão da bateria interna, slot 2 corrente, slot 3 tensão da bateria externa, slot 4 tensão da rede CA.
void CThreadMonitor::LogaStatusRedeAcBateria() const
{
    if (api::CPolySingletonList::instance<api::IUrna>().GetModelo() < 2020)                  // :258 (func 923)
        return;

    auto& power = api::CPolySingletonList::instance<api::IPower>();                          // :260
    const auto bits            = power.GetStatus().bits;
    const int tensaoRede       = power.GetTensaoRedeCA();              // slot 4
    const int tensaoBatExterna = power.GetTensaoBateriaExterna();      // slot 3
    const int tensaoBateria    = tensaoBatExterna <= 0 ? power.GetTensaoBateriaInterna()      // slot 1
                                                       : power.GetTensaoBateriaExterna();     // slot 3
    const int corrente         = power.GetCorrenteBateria();           // slot 2

    std::string texto = "Urna ligada ";
    texto += std::format("{} rede CA em [{:3.2f}V] e ",
                         (bits & REDE_CA_AUSENTE) ? "desconectada da" : "conectada na", tensaoRede / 100.0);
    if (tensaoBatExterna <= 0)
        texto += std::format("na Bateria Interna com [{:2.2f}V/{}A]", tensaoBateria / 100.0, corrente / 100.0);
    else
        texto += std::format("na Bateria Externa com [{:2.2f}V/{}A]", tensaoBateria / 100.0, corrente / 100.0);
    CLogVota::GetInst().Loga(texto);
}

// :117 - the record exists in the data segment but no function references it. The routine was probably
// only called from the part of SaiPorVotacaoSuspensa that follows std::async, which the compiler removed
// (thread creation always throws in this build).                                                       ?
void CThreadMonitor::MostraMensagemDesligamento()
{
}

// Library code the tools filed in this file:
//   func 4677 = std::filesystem::__space(const path&, error_code*) with ec constant-propagated to nullptr
//              (libc++, statfs; on error ErrorHandler::report throws filesystem_error "space")
//   func 2541 = std::__thread_struct::__thread_struct() (new __thread_struct_imp: two empty vectors)

} // namespace vota
