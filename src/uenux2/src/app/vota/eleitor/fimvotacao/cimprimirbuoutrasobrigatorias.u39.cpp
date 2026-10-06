// FRAGMENT reconstructed by unit u39 from vota_web_wasm.wasm.
// Original file (attested by srclocs :57 etc. of CImprimirBUOutrasObrigatorias::StartState, func 12065):
// uenux2/src/app/vota/eleitor/fimvotacao/cimprimirbuoutrasobrigatorias.cpp   (rest of the file: unit u09)
//
// BOLETIM DE URNA: CImprimirBUOutrasObrigatorias prints the remaining MANDATORY copies ("vias
// obrigatórias") of the BU after the first one was checked by the mesário and the result files were
// recorded. While each copy is printed the voter screen "telaDestinoBUs" shows "Imprimindo vias
// obrigatórias / do Boletim de Urna / Via nº N / Por favor, aguarde..." followed by the configured text
// telling where each copy goes. This is the data source of the "Via nº N" line.
#include <format>
#include <string>

#include "comum/appinfo/cappinfo.h"

namespace vota {

namespace {

// wasm func 13023 (table slot 1101): api_f1191(builder, slot 1101, {320,170}, FONTE_25 @474992, 2) in
// func 12065. Evaluated at draw time: EstadoGeralVota.qtdBU (vota.bin, byte +8 of CEstadoGeralVota) counts the
// copies already printed and is incremented after each copy, so the screen shows the number of the copy
// being printed. uebyte arithmetic (& 255) and an unsigned format argument (arg type 6).
std::string DS_ViaAtualBU()                                                    // name as in cimprimirbuoutrasobrigatorias.cpp (u09)
{
    const uebyte via = static_cast<uebyte>(comum::CAppInfo::GetInst().GetVota().GetQtdBU() + 1);   // funcs 185 + 903
    return std::format("Via nº {}", via);                                      // "Via nº {}" @1712 (Latin-1)
}

}  // namespace

}  // namespace vota
