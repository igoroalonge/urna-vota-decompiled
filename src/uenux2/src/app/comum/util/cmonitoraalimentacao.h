// uenux2/src/app/comum/util/cmonitoraalimentacao.h   (path inferred from cmonitoraalimentacao.cpp, attested
// by std::source_location records at lines 24, 29 and 52)
// Reconstructed from vota_web_wasm.wasm (unit u26).
//
// comum::util::CMonitoraAlimentacao ("power-supply monitor") remembers the last power source and battery
// charge levels that were logged, so that the urna log (logd.dat) only gets a record when something
// changes. It is polled by the monitor thread of VOTA (vota::CThreadMonitor::Run, wasm func 10226) and
// created by the CThreadMonitor constructor (wasm func 1898).
//
// No out-of-line function survives in the binary: CreateInst is inlined into func 1898, GetInst and
// VerificaAlimentacao into func 10226. No RTTI (not polymorphic).
#pragma once

#include <cstdint>
#include <memory>
#include <mutex>

#include "api/uelog/cloga.h"          // api::ELogAplicativos (1 = VOTA)
#include "ecourna/api/exception/cbaseerror.hpp"

namespace comum {

/// CBaseError<comum::EUeComumUtilError, SErrorLimits{9150, 9200}> (typeinfo @1600712, vtable @1600748).
enum class EUeComumUtilError : int {};
using CUeComumUtilError =
    ecourna::api::exception::CBaseError<EUeComumUtilError, ecourna::api::exception::SErrorLimits{9150, 9200}>;

namespace util {

using uebyte = std::uint8_t;

class CMonitoraAlimentacao {
public:
    /// cmonitoraalimentacao.cpp:29 - inlined into wasm func 1898 (CThreadMonitor constructor).
    /// Throws CUeComumUtilError 9156 "Instância já criada" when called twice.
    static void CreateInst(api::ELogAplicativos aplicativo);

    /// cmonitoraalimentacao.cpp:24 - inlined into wasm func 10226. Throws
    /// CBaseError<EPatternErr> 1303 "CMonitoraAlimentacao - instancia nao criada" before CreateInst.
    static CMonitoraAlimentacao& GetInst();

    /// cmonitoraalimentacao.cpp:52 (the IPower lookup) - inlined into wasm func 10226, called once per
    /// iteration (every 500 ms) of the monitor thread.
    void VerificaAlimentacao();

    explicit CMonitoraAlimentacao(api::ELogAplicativos aplicativo) : m_aplicativo(aplicativo) {}

private:
    static constexpr uebyte DESCONHECIDO = 0xFF;          // "nothing logged yet"
    static constexpr uebyte MAXIMO_EVENTOS = 20;          // after 20 power-source changes: stop logging

    // 8 bytes (operator new(8) in func 1898; initialised as {1, 0x00FFFFFF})
    api::ELogAplicativos m_aplicativo;                    // +0  = 1 (VOTA); not read by the code seen   ?
    uebyte m_tipoAlimentacao = DESCONHECIDO;              // +4  last logged source (0 rede, 1 bat. interna, 2 bat. externa)
    uebyte m_statusBateriaInterna = DESCONHECIDO;         // +5  last logged charge state of the internal battery
    uebyte m_statusBateriaExterna = DESCONHECIDO;         // +6  last logged charge state of the external battery
    uebyte m_qtdEventos = 0;                              // +7  number of source changes logged (max 20)

    static std::mutex s_mutex;                            // @1911544 (only the unlock residue remains)
    static std::unique_ptr<CMonitoraAlimentacao> s_instancia;   // @1911568
};

} // namespace util
} // namespace comum
