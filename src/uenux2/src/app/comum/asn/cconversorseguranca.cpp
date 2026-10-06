// uenux2/src/app/comum/asn/cconversorseguranca.cpp   (path inferred: class known from RTTI only; its md class is
// the attested comum/md/cseguranca.cpp and the converters of comum/md classes live in comum/asn/)
// Reconstructed from vota_web_wasm.wasm (unit u35: DoConverte). The class declaration and DoDesconverte
// (func 11412) were reconstructed by unit u24 in src/uenux2/src/app/comum/u24-foreign-fragments.cpp.
//
// ModuloTiposEleitorais:
//   Seguranca ::= SEQUENCE { idTipoArquivo INTEGER (0..2), idCriptografia INTEGER (1..3),
//                            idArquivoCD INTEGER (0..255), idArquivoChave OCTET STRING }
// md::CSeguranca (unit u24): +0 uebyte tipoArquivo, +1 uebyte criptografia, +4 std::vector<uebyte> chave.
// It describes how a data file is protected (e.g. the encrypted biometrics inside the voter roll *-el.dat).
//
// RTTI: comum::asn::CConversorSeguranca : IConversorASN<ModuloTiposEleitorais::Seguranca, md::CSeguranca>
//       vtable @1566572 = {174, 144, 11413 DoConverte, 11412 DoDesconverte}. sizeof 4.
#include "comum/asn/cconversorseguranca.h"

namespace comum::asn {

// wasm func 11413 - vtable slot 2. Not observed executing (the urna never writes a file with a Seguranca block;
// the voter roll is only read).
// idArquivoCD (field 2) is not written: it keeps the default 0 of the freshly constructed SEQUENCE, because
// md::CSeguranca has no such member.
ModuloTiposEleitorais::Seguranca CConversorSeguranca::DoConverte(const md::CSeguranca& seguranca) const
{
    ModuloTiposEleitorais::Seguranca entidade;                                      // SEQUENCE(info @1141080)
    entidade.set_idTipoArquivo(seguranca.GetIdTipoArquivo());                       // byte +0
    entidade.set_idCriptografia(seguranca.GetIdCriptografia());                     // byte +1
    const std::vector<uebyte> chave = seguranca.GetChave();                         // copy of the vector at +4
    entidade.set_idArquivoChave(ASN1::OCTET_STRING(chave.begin(), chave.end()));    // shared_f1158 (assign)
    return entidade;
}

} // namespace comum::asn
