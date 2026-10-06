// uenux2/mock/api/hwil/cpowermock.h   (path inferred, like curnamock.h: namespace api::teste, interface in
//                                      uenux2/src/api/hwil/ipower.h)
//
// Reconstructed from vota_web_wasm.wasm (unit u33).
//
// api::teste::CPowerMock : api::IPower     typeinfo @1531020, vtable @1530952 (17 slots), 48 bytes.
// Registered by the simulator bootstrap (wasm func 8302):
//     CPolySingletonList::push<api::IPower>(std::make_unique<api::teste::CPowerMock>(), info);
// with the constructor inlined: IPower::m_status zeroed (+4..+19), the simulated status
// {0x840, 2, 0, 100} at +20..+35 and std::chrono::steady_clock::now() at +40.
//
// Despite its name, IPower's status word is the urna's general hardware status (power, headphones, media,
// voter keyboard, key switch). Users and the slots they call:
//   slot 15 (refresh) + inline bit tests: the status header / battery icon (CFormBuilder, CPowerInformation),
//       CMonitoraAlimentacao, CExibeAlertaDesligamento, CEstadoComDesligamentoAutomatico, CRelatorioTesteImpressora,
//       vota::CThreadMonitor (headphones, media, keyboard, key switch - see the status layout below);
//   slot 7: the "NNN%" text of the status header;
//   slots 1, 2, 3, 4: CThreadMonitor::LogaStatusRedeAcBateria (cthreadmonitor.cpp:258/260, models >= 2020 only):
//       "Urna ligada {conectada na|desconectada da} rede CA em [{:3.2f}V] e na Bateria {Interna|Externa} com
//        [{:2.2f}V/{}A]" (values in hundredths). The monitor thread never runs in the web build.
//   slots 0, 5, 6, 16: no caller in the binary.
//
// Which class owns slots 0..6? Their bodies touch only IPower::m_status (+4..+19) after refreshing it through
// slot 15, never a CPowerMock member, so they may just as well be default bodies written in ipower.h (like
// slots 8..12, which carry ipower.h srclocs). A vtable cannot tell the two apart. They are written here, under
// the tools' class name; unit u18 declares them pure in ipower.h.                                       // ?
//
// CROSS-UNIT MISMATCH (to reconcile when merging): u18's ipower.h (a) declares NO data member, although the
// 16-byte status at +4 is part of IPower (the mock's own fields start at +20, and slots 0..6 read +4..+19);
// it needs `protected: SStatusEnergia m_status;` after the vptr; and (b) names slots 1, 3, 4 `vf1`,
// `GetTensaoBateria`, `GetTensaoCarregador`, where this file says GetTensaoBateriaInterna /
// GetTensaoBateriaExterna / GetTensaoRedeCA (from CThreadMonitor::LogaStatusRedeAcBateria: slot 4 is printed as
// "rede CA", slot 3 is used when > 0 and otherwise slot 1, labelled Externa / Interna). As written, the three
// `override`s below do not match a base declaration.
#pragma once

#include <chrono>
#include <cmath>
#include <cstdint>

#include "api/hwil/ipower.h"

namespace api {

// The refreshed copy lives in IPower at +4 (called m_status here; NOT yet declared in u18's ipower.h, see above).
// 16 bytes.
// Meanings from the code that tests them (CPowerInformation u19, CThreadMonitor u06/u09, CFormBuilder u15):
struct SStatusEnergia {                         //                                           name inferred
    std::uint32_t flags = 0;                    // +0
    //   bits 1-2  (& 0x06)  fonte: 0 rede elétrica (AC), 2 bateria interna, 4 bateria externa
    //   bits 3-4  (& 0x18)  bateria interna: 0 cheia, 0x08 parcial, 0x10 crítica, 0x18 ausente
    //   bits 5-6  (& 0x60)  bateria externa: 0 cheia, 0x20 parcial, 0x40 crítica, 0x60 ausente
    //   bit 9     (0x0200)  mídia externa ausente   ("Erro na Mídia Externa - Mídia não está presente", 9390)
    //   bit 11    (0x0800)  ? (set by the mock)
    //   bit 13    (0x2000)  tested by slot 0 (no caller)
    //   bit 14    (0x4000)  chave em "URNA DESLIGADA" ("Mudança do estado da chave: URNA DESLIGADA")
    std::uint16_t perifericos = 0;              // +4   bit 1: fone de ouvido DESconectado (CThreadMonitor logs
                                                //      "Fone de ouvido desconectado"/"... conectado" on change)
    std::uint16_t tensaoBateriaInterna = 0;     // +6   V x 100 (slot 1)
    std::uint16_t tensaoBateriaExterna = 0;     // +8   V x 100 (slot 3)
    std::uint16_t tensaoRedeCA = 0;             // +10  V x 100 (slot 4; 0 = on battery, see slot 2)
    std::uint16_t corrente = 0;                 // +12  A x 100 for slot 2 / CThreadMonitor; the mock's slot 7
                                                //      reads the same field as the battery percentage (100)
    std::uint8_t  temperatura = 0;              // +14  ? (slot 6)
    std::uint8_t  estadoTeclado = 0;            // +15  bit 0: teclado do eleitor desconectado (CThreadMonitor, 9391)
};

namespace teste {

class CPowerMock : public IPower {
public:
    CPowerMock() : m_criacao(std::chrono::steady_clock::now()) {}   // inlined in wasm func 8302

    // slot 0 - wasm func 8128                                                        // name from u18, ?
    bool EhAlimentacaoExterna() override
    {
        AtualizaStatus(m_status);
        return (m_status.flags & 0x2000) == 0;
    }

    // slot 1 - wasm func 8120 (u18: "vf1"; CThreadMonitor: tensão da bateria interna) // name inferred
    int GetTensaoBateriaInterna() override
    {
        AtualizaStatus(m_status);
        return m_status.tensaoBateriaInterna;
    }

    // slot 2 - wasm func 8116                                                        // name from u18/u06
    // Signed: negative (discharging) while the AC voltage is 0. 0 when the internal battery is absent (0x18).
    // (The test is "u32 at +8 < 65536", i.e. the high half tensaoRedeCA == 0.)
    int GetCorrenteBateria() override
    {
        AtualizaStatus(m_status);
        if ((m_status.flags & 0x18) == 0x18)
            return 0;
        return m_status.tensaoRedeCA == 0 ? -static_cast<int>(m_status.corrente)
                                          : static_cast<int>(m_status.corrente);
    }

    // slot 3 - wasm func 8113 (CThreadMonitor: tensão da bateria externa; u18: GetTensaoBateria)
    int GetTensaoBateriaExterna() override
    {
        AtualizaStatus(m_status);
        return m_status.tensaoBateriaExterna;
    }

    // slot 4 - wasm func 8111 (CThreadMonitor: tensão da rede CA; u18: GetTensaoCarregador)
    int GetTensaoRedeCA() override
    {
        AtualizaStatus(m_status);
        return m_status.tensaoRedeCA;
    }

    // slot 5 - wasm func 8109                                                        // name from u18, ?
    // Estimate = round(fator(tensão) x m_status.corrente), with tensão = external battery if > 0, else internal
    // (the same choice as CThreadMonitor), in hundredths of a volt (1300 = 13.00 V, a 12 V lead-acid battery).
    // BUG (constants as compiled): the factors for 8.20-9.39 V and 9.40-10.19 V are 0.0133 and 0.0208, ten
    // times too small for the otherwise geometric progression 0.035, 0.055, 0.086, [0.133], [0.208], 0.29 ...
    // (ratio ~1.56). Those two bands report ~1-2 % instead of ~13-21 %. Below 2.81 V ("no reading") the factor is
    // 1.0. No caller in this binary; in the simulator both voltages are 0 (factor 1.0).
    int GetCargaEstimada() override
    {
        AtualizaStatus(m_status);
        const unsigned tensao = m_status.tensaoBateriaExterna != 0 ? m_status.tensaoBateriaExterna
                                                                   : m_status.tensaoBateriaInterna;
        double fator;
        if      (tensao < 281)  fator = 1.0;
        else if (tensao < 480)  fator = 0.035;
        else if (tensao < 680)  fator = 0.055;
        else if (tensao < 820)  fator = 0.086;
        else if (tensao < 940)  fator = 0.0133;     // sic (0.133 expected)
        else if (tensao < 1020) fator = 0.0208;     // sic (0.208 expected)
        else if (tensao < 1060) fator = 0.29;
        else if (tensao < 1080) fator = 0.357;
        else if (tensao < 1100) fator = 0.392;
        else if (tensao < 1120) fator = 0.416;
        else if (tensao < 1140) fator = 0.439;
        else if (tensao < 1160) fator = 0.467;
        else if (tensao < 1180) fator = 0.502;
        else if (tensao < 1200) fator = 0.537;
        else if (tensao < 1220) fator = 0.584;
        else if (tensao < 1240) fator = 0.635;
        else if (tensao < 1260) fator = 0.694;
        else if (tensao < 1280) fator = 0.769;
        else if (tensao < 1300) fator = 0.863;
        else                    fator = 0.98;
        return static_cast<int>(std::round(fator * m_status.corrente));    // i32.trunc_sat_f64_s
    }

    // slot 6 - wasm func 8108                                                        // name from u18, ?
    int GetTemperatura() override
    {
        AtualizaStatus(m_status);
        return m_status.temperatura;
    }

    // slot 7 - wasm func 8107                                                        // name from u18/u15
    // Reads the mock's own configured status (not the refreshed copy) and clamps it to 100 in place.
    int GetPercentualBateria() override
    {
        if (m_simulado.corrente > 100)
            m_simulado.corrente = 100;
        return static_cast<std::uint8_t>(m_simulado.corrente);
    }

    // slots 8..12: IPower defaults ("Not supported", ipower.h:354..424) - not overridden.
    // slots 13 / 14: trivial destructors (ICF 174 / 144).

    // slot 15 - wasm func 8071: two i64 copies (+20..+27 and +28..+35 -> out[0..15])   // name from u18
    void AtualizaStatus(SStatusEnergia& status) override { status = m_simulado; }

    // slot 16 - ICF 434 ("return 1")                                                  // ?
    bool vf16() override { return true; }

private:
    // {0x840, 2, 0, 0, 0, 100}: mains power, internal battery full, external battery "crítica" (0x40), headphones
    // reported as disconnected (bit 1 of +4), no voltages, 100 -> icon 0 ("img-ac-bateria-full"), "100%" in the header.
    SStatusEnergia m_simulado{0x840, 2, 0, 0, 0, 100, 0, 0};         // +20..+35
    //                                                                   +36 padding
    std::chrono::steady_clock::time_point m_criacao;                  // +40 (never read in this build)
};

} // namespace teste
} // namespace api
