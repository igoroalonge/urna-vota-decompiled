// ecourna-lib/ecourna/app/dados/asn/cconversoridentificadorgeradormidia.cpp   (path inferred, see the .h)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
#include "ecourna/app/dados/asn/cconversoridentificadorgeradormidia.h"

namespace ecourna::app::dados::asn {

// wasm func 9225 (vtable slot 2): three string assignments into the GeneralString fields (+8 of each).
ModuloTiposEcoUrna::IdentificadorGeradorMidia
CConversorIdentificadorGeradorMidia::DoConverte(const CIdentificadorGeradorMidia& dado) const
{
    ModuloTiposEcoUrna::IdentificadorGeradorMidia entidade;
    // (u14 declares CIdentificadorGeradorMidia as a struct with public members)
    entidade.set_nome(dado.m_nome);                                      // +0
    entidade.set_serialCertificadoTPM(dado.m_serialCertificadoTPM);      // +12
    entidade.set_serialInstalacao(dado.m_serialInstalacao);              // +24
    return entidade;
}

// wasm func 9224 (vtable slot 3). Observed executing at votaInit, called (through the Deconverte wrapper 3733)
// by comum::asn::CConversorDadoCorrespondencia::DoDesconverte (11400), i.e. while the urna's general state
// (EstadoGeralUrna.DadoCorrespondencia in dinamico/eg.bin) is read back - not through the infomidia
// DadosGeracaoMidia converter (9202, not observed). Each field is copied as a whole GeneralString (func 1671 =
// AbstractString copy constructor, vptr GeneralString @1560588) before its std::string is passed to the
// constructor func 5109.
CIdentificadorGeradorMidia
CConversorIdentificadorGeradorMidia::DoDeconverte(const ModuloTiposEcoUrna::IdentificadorGeradorMidia& entidade) const
{
    const ASN1::GeneralString nome = entidade.get_nome();
    const ASN1::GeneralString serialTPM = entidade.get_serialCertificadoTPM();
    const ASN1::GeneralString serialInstalacao = entidade.get_serialInstalacao();
    return CIdentificadorGeradorMidia(nome.getValue(), serialTPM.getValue(), serialInstalacao.getValue());
}

}  // namespace ecourna::app::dados::asn
