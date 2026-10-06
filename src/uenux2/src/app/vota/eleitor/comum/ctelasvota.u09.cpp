// FRAGMENT reconstructed by unit u09 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp (class reconstructed by unit u07).
// The tools attributed these two functions to iniciovotacao/auxiliares/cvisualizarcandidatos.cpp
// because their only direct caller is CVisualizarCandidatos::StartState. The signature is attested by
// the RTTI of its lambdas: vota::CTelasVota::CriaTelaVisualizacaoCandidato(comum::md::CCandidatura const&,
// unsigned long, unsigned long)::$_0 (photo, api::CDataImage) and ::$_2 (std::function<void(short, int)>,
// func 12272: vice/suplente lines).
//
// Screen "telaVisualizacaoCandidato" (640x480 logical coordinates; fonts are api::SFont constants
// in the data segment: 474888 title, 474896 small, 474992 text, 475032 label).

#include "vota/eleitor/comum/ctelasvota.h"

#include <format>
#include <functional>
#include <string>

#include "api/gui/cinteractiveformbuilder.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/dados/md/candidatura/ccandidatura.h"

namespace vota {

namespace {

struct SContextoTela {                          // captured by reference by the lambdas     name inferred
    api::CInteractiveFormBuilder* builder;      // +0
    const comum::md::CCargo* cargo;             // +4
};

// wasm func 1588 — one "Rótulo: valor" line. The label is written at x = 1 with ": " appended
// (font 474896, align 6); the value goes into a rectangle to the right of the label whose right edge
// depends on how many vices/suplentes the cargo has (vota_f1157) and on whether it is above y = 287
// (photo area: x <= 478). Returns the bottom of the value text (next y).            name inferred
api::TPosition AdicionaCampo(const SContextoTela& ctx, api::TPosition y, const std::string& rotulo,
                             const std::string& valor)
{
    const api::SRect rRotulo = ctx.builder->AddLabel(rotulo + ": ", {1, y}, FONTE_PEQUENA, 0, 6, 1);   // api_f202
    const int suplentes = QtdSuplentes(*ctx.cargo);                                                   // func 1157
    const api::TPosition direita = (y + (rRotulo.bottom - rRotulo.top) * 2 + 4 < 287)
        ? 478 : static_cast<api::TPosition>(644 - 111 * suplentes - 5 * std::min(suplentes, 1));
    const api::SRect rValor = ctx.builder->AddMultiLineLabel(valor, {rRotulo.right, rRotulo.top, direita, rRotulo.bottom /*grows*/},
                                                             FONTE_PEQUENA, 0);                       // func 2782
    return rValor.bottom;
}

}  // namespace

// wasm func 6569
CFormInterativoTelaVota CTelasVota::CriaTelaVisualizacaoCandidato(const comum::md::CCandidatura& candidatura,
                                                                  std::size_t posicao, std::size_t total)
{
    const comum::md::CCargo& cargo = comum::CConfiguracaoEleicao::GetInst().GetCargo(candidatura.GetCargo());  // func 861
    api::CInteractiveFormBuilder b;
    SContextoTela ctx{&b, &cargo};

    // vices / suplentes of the candidate: lambda $_2 (func 12272) calls AdicionaCampo for each suplente
    // of the cargo (CDetalheCandidato::GetSuplente / CCandidatura::GetSuplente) starting at line y
    const std::function<void(short, int)> escreveSuplentes = [&](short y, int indice) { /* $_2 */ };
    b.AddArea({250, 28}, 31);                                            // api_f2245 (?)
    b.AddLabel("VISUALIZAÇÃO", {/*right*/}, FONTE_LABEL /*475032*/, 2, 29, 0);
    const api::SRect rCargo = b.AddLabel(cargo.GetDetalheConsulta(candidatura.GetGenero()),  // func 2796
                                         {1, 30}, FONTE_TITULO /*474888*/, 0, 2, 1);

    api::TPosition y = rCargo.bottom + 10;
    y = AdicionaCampo(ctx, y, "Partido", std::to_string(candidatura.GetPartido()));           // ecourna_f296
    y = AdicionaCampo(ctx, y, "Número",
                      std::format("{:0{}}", candidatura.GetNumero(), cargo.GetQtdDigitos() /*+12*/));
    y = AdicionaCampo(ctx, y, "Nome", candidatura.GetNome());                                 // +20
    const int genero = candidatura.GetGenero();                                               // +48
    AdicionaCampo(ctx, y, "Gênero", genero == 1 ? "masculino" : genero == 2 ? "feminino" : "não informado");

    if (!candidatura.EhInapta()) {                                                            // +52 == 0
        if (TemFoto(cargo)) {                                                                 // func 1546
            // photo in a frame at (639 - 161, 1) .. 161 x 225, from CDadosCandidato{+8, +20, optional +32,
            // +56, +48} (lambda $_0 through api::CDataImage)
            adicionaFotoEmoldurada(b, 639, 1, 161, 225, CDadosCandidato(candidatura));        // func 6561
            // func 1157 is the same QtdSuplentes() used by AdicionaCampo: number of 52-byte entries of
            // the optional block at CCargo +20 (flag +84, vector +72/+76); here only tested for != 0
            if (QtdSuplentes(cargo) != 0)                                                     // func 1157
                b.AddLabel(cargo.GetDetalheConsulta(candidatura.GetGenero()), /*centred under the photo*/
                           posSobFoto, FONTE_PEQUENA, 2, 2, 1);
        }
        escreveSuplentes(static_cast<short>(y + 10), 1);    // std::function call (bad_function_call if empty)
    } else {
        b.AddLabel("Candidato não concorre.", {320, 250}, FONTE_TEXTO /*474992*/, 2, 2, 1);
        b.AddLabel("Não há outras informações.", {320, 280}, FONTE_TEXTO, 2, 2, 1);
    }
    y = AdicionaCampo(ctx, 375, "Situação", candidatura.EhInapta() ? "INAPTO" : "APTO");
    AdicionaCampo(ctx, y, "Página", std::format("{}/{}", posicao, total));

    b.AddLabeledInputControl({{'D', "Retornar"}});                                            // CORRIGE
    if (total >= 2) {
        // key 4 (left) / key 6 (right); the labels change on the first/last page because the
        // navigation wraps around (see CVisualizarCandidatos::StartState)
        b.AddLabeledInput('4', posicao == 1 ? "Última página" : "Página anterior", 0);        // api_f2243
        b.AddLabeledInput('6', posicao == total ? "Primeira página" : "Próxima página", 2);
    }
    return api::CriaFormInterativo(b, "telaVisualizacaoCandidato");                           // func 576
}

}  // namespace vota
