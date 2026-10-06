// ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscifracao.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u11). Only DoDeconverte is in this unit.
//
// DadosCifracao is the key envelope of the ENCRYPTED attendance data (DadosComparecimentoCifrado):
// the wrapped symmetric key ("chave"), the HKDF salt and the HKDF info ("informacaoAdicional").
// In the web build CEPESC does not encrypt (docs/modules/u01-...md section 4.1), so the key is all zero.
#include "ecourna/app/dados/asn/resultadournacadastro/cconversordadoscifracao.h"

namespace ecourna::app::dados::asn {

// wasm func 9061 (vtable slot 3; was "CConversorDadosCifracao::vf3"). Not observed at run time.
CDadosCifracao CConversorDadosCifracao::DoDeconverte(const TEntidade& entidade) const
{
    const std::vector<uebyte> chave = ConverteOctetString(entidade.get_chave());                  // field 0
    const std::vector<uebyte> salt = ConverteOctetString(entidade.get_salt());                    // field 1
    const std::vector<uebyte> informacao = ConverteOctetString(entidade.get_informacaoAdicional()); // field 2

    // The constructor takes the three vectors BY VALUE (srcloc signature of func 5091), so three more
    // copies are made here before the call; it throws if salt or informacaoAdicional is shorter than 16.
    return CDadosCifracao(chave, salt, informacao);
}

}  // namespace ecourna::app::dados::asn
