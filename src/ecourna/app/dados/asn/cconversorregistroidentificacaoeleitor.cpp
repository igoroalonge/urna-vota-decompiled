// ecourna-lib/ecourna/app/dados/asn/cconversorregistroidentificacaoeleitor.cpp   (path inferred, see the .h)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40. No srcloc of its own (nothing thrown here); the
// IConversorASN<IdentificadorEleitor, CIdentificadorEleitor> wrappers it calls are funcs 9113 / 9111.
#include "ecourna/app/dados/asn/cconversorregistroidentificacaoeleitor.h"

#include "ecourna/app/dados/asn/cconversores.h"   // CConversorIdentificadorEleitor (unit u14)

namespace ecourna::app::dados::asn {

// wasm func 9114 (vtable slot 2)
// SEQUENCE::includeOptionalField (func 515) / removeOptionalField (shared_f432) are what the generated
// set_/omit_ accessors expand to; comum_f2188 is the IdentificadorEleitor (CHOICE) assignment.
ModuloTiposEcoUrna::RegistroIdentificacaoEleitor
CConversorRegistroIdentificacaoEleitor::DoConverte(const CRegistroIdentificacaoEleitor& registro) const
{
    const CConversorIdentificadorEleitor conversorIdentificador;   // vptr @1131700

    ModuloTiposEcoUrna::RegistroIdentificacaoEleitor entidade;
    // GetIdentificadorHabilitacao (func 2669) throws CDadosError 2028 if the pointer is empty.
    entidade.set_identificacaoUtilizada(
        conversorIdentificador.Converte(CIdentificadorEleitor(registro.GetIdentificadorHabilitacao())));   // 5172, 9113

    if (registro.PossuiIdentificadorPrincipal()) {                                    // optional flag +16
        entidade.set_identificacaoPrincipal(
            conversorIdentificador.Converte(CIdentificadorEleitor(registro.GetIdentificadorPrincipal())));  // 9260
    } else {
        entidade.omit_identificacaoPrincipal();
    }
    return entidade;
}

// wasm func 9112 (vtable slot 3)
CRegistroIdentificacaoEleitor
CConversorRegistroIdentificacaoEleitor::DoDeconverte(const ModuloTiposEcoUrna::RegistroIdentificacaoEleitor& entidade) const
{
    const CConversorIdentificadorEleitor conversorIdentificador;

    const CIdentificadorEleitor utilizada = conversorIdentificador.Deconverte(entidade.get_identificacaoUtilizada());
    if (entidade.identificacaoPrincipal_isPresent()) {                               // SEQUENCE::hasOptionalField(0)
        const CIdentificadorEleitor principal = conversorIdentificador.Deconverte(entidade.get_identificacaoPrincipal());
        return CRegistroIdentificacaoEleitor(utilizada, principal);                  // func 1878
    }
    return CRegistroIdentificacaoEleitor(utilizada);                                 // func 1675
}

}  // namespace ecourna::app::dados::asn
