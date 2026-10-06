// uenux2/src/app/comum/dados/asn/candidatura/cconversordadoscandidato.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u35). The inlined md constructors are attested by srclocs
// cdadoscandidato.cpp:35 and :65; Utils::DesconverteSexo by util.cpp:483 (see comum/asn/util.u35.cpp).
#include "comum/dados/asn/candidatura/cconversordadoscandidato.h"

#include <string>

#include "comum/asn/util.h"

// The md constructors inlined here (cdadoscandidato.cpp:35 / :65 and the two without ordem) are reconstructed in
// src/uenux2/src/app/comum/dados/md/candidatura/cdadoscandidato.u35.cpp.

namespace comum::asn {

// wasm func 11450 - vtable slot 3. Observed executing (every candidate and running mate at votaInit).
md::CDadosCandidato CConversorDadosCandidato::DoDesconverte(const ModuloCandidatos::DadosCandidato& dados) const
{
    const std::string codigo = dados.get_codigo();                                   // field 0
    const std::string nomeUrna = dados.get_nomeUrna();                               // field 3
    const md::CSexo::ESexo sexo = Utils::DesconverteSexo(dados.get_sexo());          // field 7, util.cpp:483
    const auto situacao = m_apto ? md::CDadosCandidato::ESituacao(0)                 // apto   name inferred
                                 : md::CDadosCandidato::ESituacao(1);                // inapto (= apto ^ 1)

    const bool temFonetico = dados.hasOptionalField(2);                             // nomeFonetico [3]
    if (dados.hasOptionalField(4)) {                                                 // ordemSuplencia [5]
        const auto ordem = static_cast<uebyte>(dados.get_ordemSuplencia());
        if (temFonetico)
            return md::CDadosCandidato(codigo, nomeUrna, dados.get_nomeFonetico(), sexo, situacao, ordem);   // :65
        return md::CDadosCandidato(codigo, nomeUrna, sexo, situacao, ordem);                                // :35
    }
    if (temFonetico)
        return md::CDadosCandidato(codigo, nomeUrna, dados.get_nomeFonetico(), sexo, situacao);
    return md::CDadosCandidato(codigo, nomeUrna, sexo, situacao);
}

} // namespace comum::asn
