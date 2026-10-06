// FRAGMENT of uenux2/src/app/comum/asn/cconversorhasharquivo.cpp (path inferred). The class declaration and
// DoDesconverte (func 10269) are in cconversorhasharquivo.u17.cpp (unit u17).
// Reconstructed from vota_web_wasm.wasm (unit u35).
//
// ModuloTiposEleitorais: ArquivoAssinatura ::= SEQUENCE { nomeArquivo GeneralString, assinatura GeneralString }
// Used for every entry of hash.dat (ModuloHashes::EntidadeHashes, CConversorEntidadeHashes): the name of a file of
// the installation and the Base64 of its SHA-512.
#include "api/hash/chasharquivo.h"
#include "comum/asn/iconversorasn.h"
#include "ModuloTiposEleitorais.h"

namespace comum::asn {

// wasm func 10267 - vtable slot 2 (DoConverte). Not observed executing (hash.dat is written at the encerramento).
ModuloTiposEleitorais::ArquivoAssinatura CConversorHashArquivo::DoConverte(const api::hash::CHashArquivo& hash) const
{
    ModuloTiposEleitorais::ArquivoAssinatura entidade;      // SEQUENCE(info @1140956)
    entidade.set_nomeArquivo(hash.GetNome());               // field 0 <- CHashArquivo +0
    entidade.set_assinatura(hash.GetHash());                // field 1 <- CHashArquivo +12
    return entidade;
}

} // namespace comum::asn
