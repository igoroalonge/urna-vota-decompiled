// uenux2/src/app/comum/dados/asn/processoeleitoral/cconversororigemconfiguracao.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
//   OrigemConfiguracao ::= ENUMERATED { oficial(1), comunitaria(2) }
// "comunitária" configurations are the ones generated for non-official elections (e.g. community
// elections using urnas); the value comes from EntidadeProcessoEleitoral.origemConfiguracao (-cp.dat).
#include "comum/dados/asn/processoeleitoral/cconversororigemconfiguracao.h"

namespace comum::asn {

// wasm func 11366 (vtable slot 3; srcloc line 38)
// The body is a 1-line thunk into comum_f6030, a wasm-opt "merge-similar-functions" body shared with
// CConversorTipoIdentificadorEleitor::DoDesconverte (it receives srcloc, message, code 7953 and the count 2).
md::EOrigemConfiguracao CConversorOrigemConfiguracao::DoDesconverte(const TEntidade& origem) const
{
    const int valor = origem.asInt();
    if (valor < 1 || valor > 2) {   // (valor - 1) >= 2 as unsigned
        throw CDadosError(7953, "Origem configuração inválida");   // line 38
    }
    return static_cast<md::EOrigemConfiguracao>(valor);
}

// wasm func 11367 (vtable slot 2): anything that is not "comunitária" is written as "oficial".
ModuloProcessoEleitoral::OrigemConfiguracao CConversorOrigemConfiguracao::DoConverte(const TDado& origem) const
{
    return origem == md::EOrigemConfiguracao::Comunitaria
               ? ModuloProcessoEleitoral::OrigemConfiguracao(ModuloProcessoEleitoral::OrigemConfiguracao::comunitaria)
               : ModuloProcessoEleitoral::OrigemConfiguracao(ModuloProcessoEleitoral::OrigemConfiguracao::oficial);
}

} // namespace comum::asn
