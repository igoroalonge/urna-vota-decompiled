// FRAGMENT reconstructed by unit u26 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp (owner: unit u07). Merge into it.
// Functions of that file that the tools filed in unit u26:
//   func 6580, 6581 (tools: cprezeresima.cpp), func 13012 (no file), and the inlined GetKeyboardLayout
//   (ctelasvota.cpp:1301, only inside func 11805 CTesteTeclado::StartState).
#include "vota/eleitor/comum/ctelasvota.h"

#include <format>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "api/hwil/iurna.h"
#include "api/pattern/cpolysingletonlist.h"
#include "api/util/cdatetime.h"

namespace vota {

// @1833360: names of the 13 keys of the voter keypad, in keypad order (built by __wasm_call_ctors, 156 bytes).
// Used by the keyboard test (CTesteTeclado) as the list of keys to test and as the labels of the boxes.
const std::vector<std::string> CTelasVota::ms_nomesTeclas{                                // name inferred
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "BRANCO", "CORRIGE", "CONFIRMA"};

// Layout entry of one key (24-byte initializer_list element: key string + 12-byte value).  name inferred
struct SLayoutTecla {
    api::SPoint centro;          // +12
    api::SPoint tamanho;         // +16
    api::TFontSize fonte;        // +20 (short): 25 digits, 15 for BRANCO/CORRIGE/CONFIRMA
    char tecla;                  // +22: character reported by the keypad driver
};

// ctelasvota.cpp:1301 (anonymous namespace in the original; inlined into func 11805). The keypad drawing
// depends on the urna model (api::IUrna slot 0): models up to 2019 have the keys further right.
// The std::map is built by func 6579 (std::map<std::string, SLayoutTecla>(initializer_list)).
std::map<std::string, SLayoutTecla> CTelasVota::GetKeyboardLayout()
{
    const auto& n = ms_nomesTeclas;
    if (api::CPolySingletonList::instance<api::IUrna>().GetModelo() <= 2019) {            // :1301 (func 923)
        return {
            {n[0], {{245, 140}, {41, 36}, 25, '1'}}, {n[1], {{320, 140}, {41, 36}, 25, '2'}},
            {n[2], {{395, 140}, {41, 36}, 25, '3'}}, {n[3], {{245, 195}, {41, 36}, 25, '4'}},
            {n[4], {{320, 195}, {41, 36}, 25, '5'}}, {n[5], {{395, 195}, {41, 36}, 25, '6'}},
            {n[6], {{245, 250}, {41, 36}, 25, '7'}}, {n[7], {{320, 250}, {41, 36}, 25, '8'}},
            {n[8], {{395, 250}, {41, 36}, 25, '9'}}, {n[9], {{320, 305}, {41, 36}, 25, '0'}},
            {n[10], {{201, 360}, {85, 36}, 15, 'B'}}, {n[11], {{320, 360}, {85, 36}, 15, 'D'}},
            {n[12], {{439, 345}, {85, 51}, 15, 'C'}},
        };
    }
    return {                                                                               // UE2020 and later
        {n[0], {{185, 140}, {41, 36}, 25, '1'}}, {n[1], {{260, 140}, {41, 36}, 25, '2'}},
        {n[2], {{335, 140}, {41, 36}, 25, '3'}}, {n[3], {{185, 195}, {41, 36}, 25, '4'}},
        {n[4], {{260, 195}, {41, 36}, 25, '5'}}, {n[5], {{335, 195}, {41, 36}, 25, '6'}},
        {n[6], {{185, 250}, {41, 36}, 25, '7'}}, {n[7], {{260, 250}, {41, 36}, 25, '8'}},
        {n[8], {{335, 250}, {41, 36}, 25, '9'}}, {n[9], {{260, 305}, {41, 36}, 25, '0'}},
        {n[10], {{433, 140}, {85, 36}, 15, 'B'}}, {n[11], {{433, 195}, {85, 36}, 15, 'D'}},
        {n[12], {{433, 250}, {85, 91}, 15, 'C'}},
    };
    // (UE2020 values of B/D/C decoded from the i64 constants 10133464242979249, 10133464246583729 and
    //  25614587969274289 as four little-endian int16 {x, y, w, h}.)
}

// wasm func 6580 (tools: vota_f6580 in cprezeresima.cpp). Mandatory keyboard test question.   name as in u20
CFormInterativoTelaVota CTelasVota::CriaTelaTesteTeclado()
{
    return CriaTelaTextoConfirmaCorrige("Por favor, teste o teclado", std::string("Testar"), std::nullopt);   // func 2376
}

// wasm func 6581 (tools: vota_f6581). Optional keyboard test question (CORRIGE = "Não testar").  name as in u20
CFormInterativoTelaVota CTelasVota::CriaTelaTesteTecladoOpcional()
{
    return CriaTelaTextoConfirmaCorrige("Quer testar o teclado?", std::string("Testar"), std::string("Não testar"));
}

// wasm func 13012 (table slot 1102; referenced by the CTelasVota constructor, inlined into func 7787). Text source of
// the automatic power-off warning: seconds left until g_dataHoraDesligamento (@1833312, set by
// CEstadoComDesligamentoAutomatico), never negative. Format argument type: long long.   name inferred
std::string TextoSegundosParaDesligamento(const std::string& formato)
{
    const api::CDateTime agora;                                                           // func 479
    const long long segundos = api::CDateTime::DiferencaSegundos(g_dataHoraDesligamento, agora);   // func 5471
    return std::vformat(formato, std::make_format_args(segundos > 0 ? segundos : 0LL));
}

} // namespace vota
