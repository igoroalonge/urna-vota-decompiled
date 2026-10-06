// ecourna-lib/ecourna/app/dados/midias/cautenticacao.cpp   (srcloc-attested file of unit u14)
// FRAGMENT written by unit u11: the two constructors below were attached to u11 because their only caller is
// CConversorAutenticacao::DoDeconverte (func 9200). Merge into the u14 reconstruction of cautenticacao.cpp.
// Reconstructed from vota_web_wasm.wasm.
//
// CAutenticacao (56 bytes; declared by u14 in cinformacaomidia.h):
//   +0  std::optional<boost::posix_time::ptime> m_dataHoraInicial   (value +0, engaged flag +8)
//   +16 std::optional<boost::posix_time::ptime> m_dataHoraFinal     (value +16, engaged flag +24)
//   +32 int m_numeroTentativas      <- NOTE: u14's header currently puts m_tamanhoSenha at +32. Both converter
//   +36 int m_tamanhoSenha             directions say otherwise: DoConverte (9201) writes ASN.1 field 3
//                                      (numeroTentativas, see the names table @1627872) from +32 and field 2
//                                      (tamanhoSenha) from +36; DoDeconverte (9200) passes field 3 as the
//                                      first int argument, which 9037/9040 store at +32.
//   +40 std::vector<uebyte> m_hashSenha
// GetDataHoraInicial (9036) / GetDataHoraFinal (9035) are srcloc-named members of the same file (u14).
#include "ecourna/app/dados/midias/cinformacaomidia.h"

namespace ecourna::app::dados {

// wasm func 9037 (table slot 6745) - no validity window. Neither constructor validates anything.
CAutenticacao::CAutenticacao(int numeroTentativas, int tamanhoSenha, const std::vector<uebyte>& hashSenha)
    : m_dataHoraInicial(std::nullopt)
    , m_dataHoraFinal(std::nullopt)
    , m_numeroTentativas(numeroTentativas)
    , m_tamanhoSenha(tamanhoSenha)
    , m_hashSenha(hashSenha)
{
}

// wasm func 9040 (table slot 6746) - with validity window [inicial, final]; the order of the two dates is
// not checked.
CAutenticacao::CAutenticacao(const boost::posix_time::ptime& inicial, const boost::posix_time::ptime& final,
                             int numeroTentativas, int tamanhoSenha, const std::vector<uebyte>& hashSenha)
    : m_dataHoraInicial(inicial)
    , m_dataHoraFinal(final)
    , m_numeroTentativas(numeroTentativas)
    , m_tamanhoSenha(tamanhoSenha)
    , m_hashSenha(hashSenha)
{
}

}  // namespace ecourna::app::dados
