// uenux2/src/app/comum/dados/ccargods.h
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
// "DS" = data source: small function objects that return a text for the GUI / printed reports, wrapped in
// api::CDataText<DS> (an api::IText whose vtable slot 2 returns DS()()). The "Cargo" data sources read the
// current cargo of the CCargos cursor (and, for candidate-related names, the current candidacy).
#pragma once

#include <string>

#include "comum/dados/md/csexo.h"

namespace comum {

// api::CDataText<CCargoDSNomeSexoCandidato> (RTTI, : api::IText): vtable @1537644, slot 2 = func 12632.
// Layout (2 bytes, passed by value as one 16-bit word): +0 uebyte suplente (0 = titular, 1/2 = 1st/2nd suplente),
// +1 bool abreviaNomeLongo.
struct CCargoDSNomeSexoCandidato
{
    uebyte m_suplente = 0;
    bool m_abreviaNomeLongo = false;   // name inferred

    std::string operator()() const;   // wasm func 2268 (srcloc lines 65, 72, 78; GetNomeAbreviadoCargo line 41 inlined)
};

// Used as api::CDataText<CPadDS<CToUpperDS<CCargoDSNome>>> (func 11247): the cargo name in the neutral/
// masculine/feminine form given by m_sexo, upper-cased and centred.
struct CCargoDSNome
{
    md::CSexo::ESexo m_sexo;
    std::string operator()() const;   // defined in ccargods.cpp; only seen inlined (LTO) into func 11247
};

// Labels for the printed reports (BU, zerésima, boletins) that depend on whether the current cargo is a
// consulta (referendum): "Candidato/Num cand" vs "Resposta/Num resp".
struct CCargoDSLabelRelatorio
{
    static std::string HeaderDetalhe();      // wasm func 5802 (srcloc line 98)
    static std::string HeaderDetalheZE();    // wasm func 11555 (srcloc line 113)
    static std::string TotalVotoNominal();   // wasm func 5801 (srcloc line 128)
};

} // namespace comum
