// ecourna-lib/ecourna/api/security/cepesc/cplaintext.cpp  --  FRAGMENT written by unit u36 (file owned by u01).
// Reconstructed from vota_web_wasm.wasm.
//
// Why a constructor and not a helper: wasm func 5168 returns its first argument (the wasm C++ ABI makes
// constructors return `this`), stores nothing itself, and forwards to the full constructor (wasm 9465,
// cplaintext.cpp:89..104) through invoke_iiiiiiiiiii (table slot 6328) so that the four moved-from vector
// temporaries are freed if it throws. So it is a delegating constructor.
#include "ecourna/api/security/cepesc/cplaintext.hpp"

#include <memory>
#include <utility>
#include <vector>

namespace ecourna::api::cepesc {

// wasm func 5168                                                         // name inferred (delegating ctor)
// "Plain text" to be encrypted by CCepescCipher (the TSE's CEPESC scheme) for a seção urna:
//   tipoArquivo 0, idCriptografia 1, no CInfoSalt (empty shared_ptr).
// Callers:
//   * comum::CGravadorBU::GravaResultado (11629) - the ENCRYPTED BU (parameter "criptografarBU"):
//     tabela = the urna's 1024-byte crypto table (IUrna slot 4), aleatório = 32 bytes from IRng,
//     chave = the BU public key file, conteúdo = the DER bytes of EntidadeBoletimUrna;
//   * comum::CControlaArmazenamentoDeImagens::CifrarWsq (inlined in 2725) - fingerprint images (WSQ).
// Each vector is taken by value and moved into the full constructor, which throws CSecurityError
// 1519..1522 if one of them is empty.
CPlainText::CPlainText(const ueword zona, const ueword secao, std::vector<uebyte> tabelaCriptografia,
                       std::vector<uebyte> numeroAleatorio, std::vector<uebyte> chave,
                       std::vector<uebyte> conteudo)
    : CPlainText(0 /* tipoArquivo */, 1 /* idCriptografia */, zona, secao, std::move(tabelaCriptografia),
                 std::move(numeroAleatorio), std::move(chave), std::move(conteudo),
                 std::shared_ptr<CInfoSalt>{})
{
}

}  // namespace ecourna::api::cepesc
