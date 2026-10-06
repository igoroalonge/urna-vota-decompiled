// ecourna-lib/ecourna/app/dados/federacoes/cfederacoes.cpp   (path inferred: next to cfederacao.cpp; the class
//   name comes from the RTTI of IConversorASN<ModuloFederacoes::EntidadeFederacoes, ecourna::app::dados::CFederacoes>)
// Reconstructed from vota_web_wasm.wasm (unit u11).
//
// CFederacoes:
//   +0  CCabecalhoEntidade      m_cabecalho    (16 bytes, trivially copyable)
//   +16 std::vector<CFederacao> m_federacoes   (40-byte items: +0 ushort identificador, +4 std::string sigla,
//                                               +16 std::string nome, +28 std::vector partidos)
#include "ecourna/app/dados/federacoes/cfederacoes.h"

namespace ecourna::app::dados {

// wasm func 9045 (table slot 6720) - name inferred. Only caller: CConversorFederacoes::DoDeconverte (9207).
// The vector is copied element by element with the CFederacao copy constructor (func 9044) under an
// exception guard (func 1272 on failure). No validation.
CFederacoes::CFederacoes(const CCabecalhoEntidade& cabecalho, const std::vector<CFederacao>& federacoes)
    : m_cabecalho(cabecalho)
    , m_federacoes(federacoes)
{
}

}  // namespace ecourna::app::dados
