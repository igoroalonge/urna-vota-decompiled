// Reconstructed from vota_web_wasm.wasm (unit u05). Original: uenux2/src/app/comum/dados/crespostas.cpp
//
// CRespostas = the answers ("respostas") of a consulta popular (plebiscite/referendum question on the
// ballot). It is an api::CDataMap<unsigned, comum::md::CRespostaConsulta> singleton: a keyed table with
// a "current record" cursor (GetCurrent/Next) that the report and screen data sources walk.
// CRespostaConsulta layout (from its users): +0 unsigned numero, +4 std::string nome.
//
// Only GetInst survives as a function of its own. Three other wasm functions carry the srcloc of
// crespostas.cpp:28 because GetInst was inlined into them; they belong to other files:
//   func 11478  zeresima line provider for a consulta answer  (comum/relatorios, see u05 doc)
//   func 11479  "name of the current answer" text provider    (vota/eleitor/comum/ctelasvota.cpp)
//   func 12621  api::CDataText<comum::CRespostasDSNumero>::vf2 (not in this unit)
#include "crespostas.h"

#include "api/pattern/csingleton.h"

namespace comum {

// wasm func 2812 (srcloc line 28)
CRespostas& CRespostas::GetInst()
{
    // comum_f1406(mutex @1838960, srcloc, msg, &s_instancia @1838984); throws
    // CBaseError<EPatternError>(1303, "CRespostas - instancia nao criada") when not created.
    return api::CSingleton<CRespostas>::GetInst("CRespostas - instancia nao criada");
}

// static void CRespostas::CreateInst()  (srcloc line 33) - inlined into the start-up function 7787.

}  // namespace comum
