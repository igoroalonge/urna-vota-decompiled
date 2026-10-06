// FRAGMENT of uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadolocal.cpp (path inferred; file of unit
// u21, which wrote DoConverte = func 11403 and the declaration in cconversordadolocal.h).
// Reconstructed from vota_web_wasm.wasm (unit u35: DoDesconverte = func 11404).
#include "comum/dados/asn/estadoaplicacao/cconversordadolocal.h"

#include "comum/asn/util.h"
#include "comum/dados/asn/estadoaplicacao/cconversorlocalidadeeleitoral.h"   // vtable @1569252

namespace comum::asn {

// wasm func 11404 - vtable slot 3. Observed executing (eg.bin read at votaInit).
//   DadoLocal ::= SEQUENCE { uf SiglaUF, secaoCarga DadoSecao, tipoLocalVotacao TipoLocalVotacao }
// The ENUMERATED is copied (ASN1::ENUMERATED copy ctor, func 752) because Utils::DesconverteTipoLocalVotacao takes
// it by value.
md::estadoaplicacao::CDadoLocal CConversorDadoLocal::DoDesconverte(const ModuloEstadoGeralUrna::DadoLocal& local) const
{
    const std::string uf = local.get_uf();                                                     // field 0
    const ETipoLocalVotacao tipo = Utils::DesconverteTipoLocalVotacao(local.get_tipoLocalVotacao());   // func 3800
    const md::estadoaplicacao::CLocalidadeEleitoral secao =
        CConversorLocalidadeEleitoral().Desconverte(local.get_secaoCarga());                  // func 5697
    return md::estadoaplicacao::CDadoLocal(uf, secao, tipo);                                  // func 5632
}

} // namespace comum::asn
