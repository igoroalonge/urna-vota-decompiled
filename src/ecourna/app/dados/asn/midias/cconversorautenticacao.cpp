// ecourna-lib/ecourna/app/dados/asn/midias/cconversorautenticacao.cpp   (path inferred: siblings
//   cconversoraplicativo.cpp and cconversorinformacaomidia.cpp are srcloc-attested in asn/midias/)
// Reconstructed from vota_web_wasm.wasm (unit u11). Only DoDeconverte is in this unit.
//
// "Autenticação" of an application on a medium (InformacaoMidia.aplicativos[i].autenticacao): the hash of
// the password that unlocks it, its length, how many attempts are allowed and an optional validity window.
#include <vector>

#include "ecourna/api/asn/iconversorasn.hpp"
#include "ecourna/api/util/datahora.hpp"              // ConverteDataHoraJE (func 1877, unit u40), header/name inferred
#include "ecourna/app/dados/midias/cinformacaomidia.h"    // CAutenticacao (unit u14)
#include "ModuloInformacaoMidia.h"

namespace ecourna::app::dados::asn {

// RTTI: CConversorAutenticacao : IConversorASN<ModuloInformacaoMidia::Autenticacao, CAutenticacao>
// vtable @1125420: [0] 174  [1] 144  [2] 9201 DoConverte (unit u40)  [3] 9200 DoDeconverte
//
//   Autenticacao ::= SEQUENCE { dataInicial [1] EXPLICIT DataHoraJE OPTIONAL,
//                               dataFinal   [2] EXPLICIT DataHoraJE OPTIONAL,
//                               tamanhoSenha INTEGER, numeroTentativas INTEGER, hashSenha OCTET STRING }
//   CAutenticacao (56 bytes): +0 optional<ptime> inicial (flag +8), +16 optional<ptime> final (flag +24),
//                             +32 numeroTentativas, +36 tamanhoSenha, +40 std::vector<uebyte> hashSenha
//   (DoConverte 9201 writes field 3 from +32 and field 2 from +36, so the layout above is consistent.)
class CConversorAutenticacao
    : public api::asn::IConversorASN<ModuloInformacaoMidia::Autenticacao, CAutenticacao>
{
protected:
    TEntidade DoConverte(const TDado& dado) const override;          // wasm func 9201 (unit u40)
    TDado DoDeconverte(const TEntidade& entidade) const override;    // wasm func 9200
};

// wasm func 9200 (vtable slot 3; was "CConversorAutenticacao::vf3"). Reached from CConversorAplicativo's
// DoDeconverte (func 9196), which inlines IConversorASN<Autenticacao>::Deconverte (srcloc :66 @1126048).
CAutenticacao CConversorAutenticacao::DoDeconverte(const TEntidade& entidade) const
{
    const int tamanhoSenha = entidade.get_tamanhoSenha();              // field 2
    const int numeroTentativas = entidade.get_numeroTentativas();      // field 3
    const std::vector<uebyte> hashSenha(entidade.get_hashSenha().begin(), entidade.get_hashSenha().end());

    // The validity window is only taken when BOTH dates are present. With only one of them the entity is
    // still valid ASN.1, but the date is silently dropped (see "suspicious" in the module doc). DoConverte
    // (9201) is consistent with this: it writes the two dates only when both optionals are engaged.
    if (entidade.hasOptionalField(TEntidade::e_dataInicial) && entidade.hasOptionalField(TEntidade::e_dataFinal)) {
        // func 1877: DataHoraJE text ("YYYYMMDDTHHMMSS", optional trailing 'Z') -> boost ptime (64 bits)
        const boost::posix_time::ptime inicial = ConverteDataHoraJE(entidade.get_dataInicial());
        const boost::posix_time::ptime final = ConverteDataHoraJE(entidade.get_dataFinal());
        return CAutenticacao(inicial, final, numeroTentativas, tamanhoSenha, hashSenha);   // func 9040
    }
    return CAutenticacao(numeroTentativas, tamanhoSenha, hashSenha);                     // func 9037
}

}  // namespace ecourna::app::dados::asn
