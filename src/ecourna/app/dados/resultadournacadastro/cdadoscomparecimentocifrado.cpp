// ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscomparecimentocifrado.cpp   (path inferred: class
// CDadosComparecimentoCifrado, declared by unit u14 in cdadoscifracao.h). Reconstructed by unit u40.
#include "ecourna/app/dados/resultadournacadastro/cdadoscifracao.h"

namespace ecourna::app::dados {

// wasm func 3484 (table slot 7018). Both arguments by value, moved in (no validation). This constructor must be
// added to the declaration in cdadoscifracao.h. Callers: IConversorASN<DadosCifracao>::Deconverte host 9057
// (= CConversorDadosComparecimentoCifrado::DoDeconverte) and comum::CGravadorRCSecao (11616).
CDadosComparecimentoCifrado::CDadosComparecimentoCifrado(CDadosCifracao dadosCifracao, std::vector<uebyte> conteudo)
    : m_dadosCifracao(std::move(dadosCifracao))   // +0 (three vectors)
    , m_conteudo(std::move(conteudo))             // +36
{
}

} // namespace ecourna::app::dados
