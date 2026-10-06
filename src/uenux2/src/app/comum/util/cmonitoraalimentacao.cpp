// uenux2/src/app/comum/util/cmonitoraalimentacao.cpp
// Reconstructed from vota_web_wasm.wasm (unit u26). Attested by std::source_location records:
//   :24 static CMonitoraAlimentacao& CMonitoraAlimentacao::GetInst()      (inlined in wasm func 10226)
//   :29 static void CMonitoraAlimentacao::CreateInst(ELogAplicativos)     (inlined in wasm func 1898)
//   :52 void CMonitoraAlimentacao::VerificaAlimentacao()                  (inlined in wasm func 10226)
//
// Every function of this file only exists inlined into vota::CThreadMonitor code (constructor and Run);
// wasm func 1898, which the tools first named after the CreateInst record, is vota::CThreadMonitor::GetInst (see
// src/uenux2/src/app/vota/monitor/cthreadmonitor.cpp).
//
// Web build: the monitor thread is never started (vota::CExecucaoVotaCooperativa does not start threads),
// so none of this code runs in the simulator. The power supply is api::teste::CPowerMock (always mains).
#include "comum/util/cmonitoraalimentacao.h"

#include <source_location>

#include "api/hwil/ipower.h"
#include "api/pattern/cpolysingletonlist.h"
#include "comum/log/clogcomum.h"

namespace comum::util {

std::mutex CMonitoraAlimentacao::s_mutex;
std::unique_ptr<CMonitoraAlimentacao> CMonitoraAlimentacao::s_instancia;

namespace {
// Bits of the status word of api::IPower (the word at IPower+4, refreshed by vtable slot 15).
// Names inferred from the log texts that use them.
uebyte TipoAlimentacao(const api::SStatusEnergia& s)        { return (s.bits >> 1) & 3; }   // 0 rede, 1 bat. int., 2 bat. ext.
uebyte StatusBateriaInterna(const api::SStatusEnergia& s)   { return (s.bits >> 3) & 3; }
uebyte StatusBateriaExterna(const api::SStatusEnergia& s)   { return (s.bits >> 5) & 3; }
} // namespace

// cmonitoraalimentacao.cpp:29 - inlined into wasm func 1898
void CMonitoraAlimentacao::CreateInst(api::ELogAplicativos aplicativo)
{
    std::lock_guard trava(s_mutex);
    if (s_instancia)
        throw CUeComumUtilError(EUeComumUtilError(9156), "Instância já criada");            // :29
    s_instancia = std::make_unique<CMonitoraAlimentacao>(aplicativo);                        // {1, 0xFF, 0xFF, 0xFF, 0}
}

// cmonitoraalimentacao.cpp:24 - inlined into wasm func 10226 (called once, before the monitor loop)
CMonitoraAlimentacao& CMonitoraAlimentacao::GetInst()
{
    std::lock_guard trava(s_mutex);
    if (!s_instancia)   // CErroPattern = CBaseError<ecourna::api::pattern::EPatternErr> (typeinfo @1526600)
        throw ecourna::api::pattern::CErroPattern(1303, "CMonitoraAlimentacao - instancia nao criada");  // :24 (shared_f331)
    return *s_instancia;
}

// cmonitoraalimentacao.cpp:52 - inlined into wasm func 10226, once per iteration of the monitor loop.
// Logs (through comum::CLogComum, i.e. into logd.dat):
//   * every change of power source ("Urna operando na rede elétrica / bateria interna / bateria externa");
//   * every change of the charge state of the battery in use ("Carga da [<fonte>]: [<estado>]").
// After 20 source changes it logs "Suspenso monitoramento de alimentação devido repetição de eventos.
// [ 20 ] eventos" once and stops watching (flapping protection).
void CMonitoraAlimentacao::VerificaAlimentacao()
{
    if (m_qtdEventos >= MAXIMO_EVENTOS)                                                      // (+7 > 19)
        return;

    auto& power = api::CPolySingletonList::instance<api::IPower>();                         // :52 (func 862)
    auto& log = CLogComum::GetInst();                                                        // func 948

    // GetStatus() = inline "AtualizaStatus(m_status) [IPower slot 15]; return m_status;" - the refresh is
    // repeated before every read below, as in the binary.
    const uebyte tipo = TipoAlimentacao(power.GetStatus());
    if (tipo != m_tipoAlimentacao) {
        log.LogaTipoBateria();                    // clogcomum.cpp:144/145/158 (inlined): throws 8850 for tipo 3
        m_statusBateriaInterna = DESCONHECIDO;    // (short store of 0xFFFF at +5)
        m_statusBateriaExterna = DESCONHECIDO;
        m_tipoAlimentacao = tipo;
        ++m_qtdEventos;
    }

    switch (tipo) {
    case 1:   // bateria interna
        if (m_statusBateriaInterna != StatusBateriaInterna(power.GetStatus())) {
            log.LogaNivelBateria(1, StatusBateriaInterna(power.GetStatus()));                // func 5875
            m_statusBateriaInterna = StatusBateriaInterna(power.GetStatus());
        }
        break;
    case 2:   // bateria externa
        if (m_statusBateriaExterna != StatusBateriaExterna(power.GetStatus())) {
            log.LogaNivelBateria(2, StatusBateriaExterna(power.GetStatus()));                // func 5875
            m_statusBateriaExterna = StatusBateriaExterna(power.GetStatus());
        }
        break;
    default:  // rede elétrica: nothing more to log
        break;
    }

    if (m_qtdEventos == MAXIMO_EVENTOS)
        log.LogaSuspensoMonitoramenteRepeticao(MAXIMO_EVENTOS);                             // clogcomum.cpp:136
}

} // namespace comum::util
