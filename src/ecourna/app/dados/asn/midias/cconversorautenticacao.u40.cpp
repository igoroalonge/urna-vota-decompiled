// FRAGMENT of ecourna-lib/ecourna/app/dados/asn/midias/cconversorautenticacao.cpp (path inferred; class +
// DoDeconverte by unit u11). Reconstructed by unit u40 from vota_web_wasm.wasm.
#include "ecourna/api/util/datahora.hpp"
#include "ecourna/app/dados/midias/cinformacaomidia.h"
#include "ModuloInformacaoMidia.h"

namespace ecourna::app::dados::asn {

using api::util::FormataDataHoraJE;

// wasm func 9201 (vtable @1125420 slot 2)
//   Autenticacao ::= SEQUENCE { dataInicial [1] EXPLICIT DataHoraJE OPTIONAL, dataFinal [2] EXPLICIT DataHoraJE
//                               OPTIONAL, tamanhoSenha INTEGER, numeroTentativas INTEGER, hashSenha OCTET STRING }
// The validity window is written only when BOTH dates are present (flags +8 and +24); otherwise both optional
// fields are omitted - the same "both or nothing" rule as DoDeconverte (9200).
// Field 3 (numeroTentativas) is read from +32 and field 2 (tamanhoSenha) from +36 of CAutenticacao, which
// agrees with u11's layout comment; u14's cinformacaomidia.h lists +32/+36 the other way round.
ModuloInformacaoMidia::Autenticacao CConversorAutenticacao::DoConverte(const CAutenticacao& dado) const
{
    ModuloInformacaoMidia::Autenticacao entidade;
    if (dado.PossuiDataHoraInicial() && dado.PossuiDataHoraFinal()) {                 // inline optional tests
        // GetDataHoraInicial/Final (funcs 9036/9035) would throw if absent; includeOptionalField(0|1) (func 515)
        entidade.set_dataInicial(ModuloTiposEleitorais::DataHoraJE(FormataDataHoraJE(dado.GetDataHoraInicial())));
        entidade.set_dataFinal(ModuloTiposEleitorais::DataHoraJE(FormataDataHoraJE(dado.GetDataHoraFinal())));
    } else {
        entidade.omit_dataInicial();                                                  // removeOptionalField(0)
        entidade.omit_dataFinal();                                                    // removeOptionalField(1)
    }
    entidade.set_numeroTentativas(dado.GetNumeroTentativas());                        // +32
    entidade.set_tamanhoSenha(dado.GetTamanhoSenha());                                // +36
    // vector<char>::insert(begin, first, last) into the (empty) field (shared_f1927)
    entidade.ref_hashSenha().insert(entidade.ref_hashSenha().begin(),
                                    dado.GetHashSenha().begin(), dado.GetHashSenha().end());
    return entidade;
}

}  // namespace ecourna::app::dados::asn
