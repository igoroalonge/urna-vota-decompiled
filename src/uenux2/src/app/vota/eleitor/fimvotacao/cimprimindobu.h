// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobu.h (path inferred from cimprimindobu.cpp,
// attested by srcloc lines 70, 71, 111, 114).
//
// "Imprimindo BU" = prints the first copy (via) of the Boletim de Urna and asks the mesário whether
// the print is readable ("qualidade do BU"): CONFIRMA = OK, CORRIGE = print it again.
// ImprimeBU() is also used by CImprimirBUOutrasObrigatorias (mandatory copies) and CEmitirMaisBU
// (extra copies). The printed image is the file trab/bu.dat written earlier by CGeraBU; it is
// printed only if its signature trab/bu.vsu verifies (comum::CSigVerifier).
//
// RTTI: api::CState <- comum::CAppState <- vota::CImprimindoBU (typeinfo @1541400, vtable @1541300)
//   [0] 244 dtor (ICF: releases m_tela)  [1] 387 deleting dtor (ICF)  [2] StartState 12078
//   [3] NeedChangeState 7480  [4] GetNextState 1661  [5] nop  [6] nop  [7] ProcessInput 12077  [8] nop
#pragma once

#include <cstdint>

#include "comum/cappstate.h"
#include "vota/eleitor/comum/ctelasvota.h"      // CFormInterativoTelaVota

namespace vota {

using uebyte = std::uint8_t;

/// Second parameter of ImprimeBU (the enum name is attested by the srcloc signature
/// "static void vota::CImprimindoBU::ImprimeBU(uebyte, PrintMessageMode)"; enumerators inferred).
enum class PrintMessageMode : int {
    ComMensagem = 0,   // pass "boletim de urna" / "<n>ª via" to the printer service
    SemMensagem = 1,   // empty texts (mandatory copies printed by CImprimirBUOutrasObrigatorias)
};

class CImprimindoBU final : public comum::CAppState {
public:
    /// Lazy singleton, wasm func 5986 (reconstructed in cquerimprimirbu.cpp).
    static CImprimindoBU& GetInst();

    void StartState() override;       // wasm func 12078 (srcloc 70, 71)
    void ProcessInput() override;     // wasm func 12077

    /// Prints via `numVia` of the BU from trab/bu.dat. wasm func 2890 (srcloc 111, 114).
    static void ImprimeBU(uebyte numVia, PrintMessageMode modo);

private:
    CImprimindoBU();                  // CAppState(2 = keyboard); m_tela = CTelasVota +100/+104

    CFormInterativoTelaVota m_tela;   // +12 (+16 control block): the "qualidade do BU" question
};

}  // namespace vota
