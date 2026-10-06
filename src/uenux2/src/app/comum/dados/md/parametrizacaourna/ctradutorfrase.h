// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/parametrizacaourna/ctradutorfrase.h
//
// CTradutorFrase ("phrase translator") expands the placeholders that the urna's texts (screens,
// reports) contain, using the labels of the urna parametrisation (*-pu.dat,
// ModuloParametrizacaoUrna::LabelParametrizado {genero, longo, curto, longoPlural, curtoPlural}):
//   <KFTC>        label K (one char, e.g. S=Seção, Z=Zona, M=Município, P=Partido) in form
//                 F = C(urto)/L(ongo), T = S(ingular)/P(lural), case C = A(lta)/B(aixa)/N(ormal)
//                 e.g. "<PLSN>" -> "Partido", "<SCSA>" -> "SEÇÃO"
//   <K|n|m|f>     gender agreement: picks n/m/f by the gender of label K
//                 e.g. "não pertence <S|a|ao|à> <SCSN>"
//   <DT+N>/<DT-N> reference date +/- N days, formatted DD/MM/YYYY
// All members are static.
#pragma once

#include <map>
#include <string>

#include "api/util/cdate.h"
#include "ecourna/app/dados/clabelparametrizado.h"

namespace comum::md {

class CTradutorFrase {
public:
    // func 654 - name inferred (the analysis tools call it TraduzLabel after the inlined srclocs)
    static std::string Traduz(const std::string& frase);

private:
    static const ecourna::app::dados::CLabelParametrizado& RecuperaLabel(const std::string& token); // func 5658 (:95)
    static std::string TraduzLabel(const std::string& conteudo);   // :104/:119/:132 (inlined in 654)
    static std::string TraduzTexto(const std::string& conteudo);   // :141/:156      (inlined in 654)
    static std::string TraduzData(const std::string& conteudo);    // name inferred  (inlined in 654)

    // std::map<char, CLabelParametrizado> @1839056 (begin) / @1839060 (end node) / @1839064 (size);
    // filled when the parametrisation is loaded (not in this unit).
    static std::map<char, ecourna::app::dados::CLabelParametrizado> s_labels;
    static api::CDate s_dataReferencia;   // @1839068 (the <DT±N> base date)
};

// Latin-1 aware case conversion used by TraduzLabel (also referenced through function pointers:
// table slots 2920 and 6425; ToUpper is also used by CCargos::GetCurrentEleicaoVersaoPacote).
void ConverteMaiusculas(std::string& texto);   // func 3509  name inferred
void ConverteMinusculas(std::string& texto);   // func 5158  name inferred

}  // namespace comum::md
