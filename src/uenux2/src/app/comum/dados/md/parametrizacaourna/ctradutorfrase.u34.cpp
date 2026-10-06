// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/comum/dados/md/parametrizacaourna/ctradutorfrase.cpp (attested; owner u05).
//
// How CTradutorFrase::s_labels (std::map<char, ecourna::app::dados::CLabelParametrizado> @1839056) gets its
// contents. The only writer is the assignment below, reached from comum::CRelatorioTesteImpressora slot 10
// (func 11196, "ConfiguraLabels", crelatoriotesteimpressora.cpp) and from the voter start-up routine (7787):
//     CTradutorFrase::SetLabels({{'M', labelMunicipio}, {'Z', labelZona}, {'S', labelSecao}, {'P', labelPartido}});
// i.e. the labels of the urna parametrisation (*-pu.dat, EntidadeParametrizacaoUrna, CConfiguracaoEleicao
// +272/+324/+376/+428) used to expand "<MCSN>", "<ZCSN>", "<S|a|ao|à>" ... in screen and report texts.
// Observed executing (votaInit): 5793, 5795.
//
// The three wasm functions are libc++ template code, written here as the source line that produced them:
//   wasm func 5795  std::map<char, CLabelParametrizado>::map(std::initializer_list<value_type>)
//                   (insert of each 56-byte pair with end() hint; 72-byte tree node; value copy = 5793)
//   wasm func 5794  std::map<char, CLabelParametrizado>::operator=(const map&) applied to the static
//                   s_labels (address constant-propagated into the body): __tree::__assign_multi, which
//                   recycles the old nodes (_DetachedTreeCache) and copy-assigns the key, genero and the four
//                   strings, then allocates nodes for the remaining elements; the leftover cache is freed
//                   with shared_f1706 (__tree::destroy).
//   wasm func 5793  std::pair<const char, CLabelParametrizado>::pair(const pair&):
//                   {char key +0; int genero +4; std::string longo +8, curto +20, longoPlural +32,
//                    curtoPlural +44}  (56 bytes)
#include <initializer_list>
#include <map>
#include <utility>

#include "comum/dados/md/parametrizacaourna/ctradutorfrase.h"

namespace comum::md {

// Inlined into its callers (11196, 7787).                                                  name inferred
void CTradutorFrase::SetLabels(
    std::initializer_list<std::pair<const char, ecourna::app::dados::CLabelParametrizado>> labels)
{
    s_labels = std::map<char, ecourna::app::dados::CLabelParametrizado>(labels);   // funcs 5795, 5794
}

}  // namespace comum::md
