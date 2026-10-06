// FRAGMENT of uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeralvota.cpp (attested by srclocs
// cestadogeralvota.cpp:108/:113 of the sibling MarcaFimAquisicao, see src/uenux2/src/app/vota/u20-foreign-fragments.cpp)
// reconstructed by unit u29 from vota_web_wasm.wasm.
//
// CEstadoGeralVota is the md side of vota.bin (ModuloEstadoGeralVota::EstadoGeralVota). Relevant layout:
//   +0  EEstadoVota m_estadoVota      (C++ values = ASN.1 value + '1': EAVINICIAL '1' .. EAVVOTAR '8' ..)
//   +20 std::optional<api::CDateTime> m_dhIniAquisicao   (engaged flag +32)
//   +36 std::optional<api::CDateTime> m_dhFimAquisicao   (engaged flag +48)
//   +72 bool m_treinamentoEleitor
#include "comum/dados/md/estadoaplicacao/cestadogeralvota.h"

#include "api/util/cdatetime.h"

namespace comum::md::estadoaplicacao {

// wasm func 5627 (table slot 52). Records the start of vote acquisition ("aquisição" = collecting votes), once:
// unlike MarcaFimAquisicao it silently keeps an existing value. Callers: votaInit (after SetEstadoVota(EAVVOTAR))
// and vota_f5972 (vota::CInicioVotacao, the urna's own start of voting, when no RDV exists yet).  name inferred
void CEstadoGeralVota::MarcaInicioAquisicao()
{
    if (!m_dhIniAquisicao)
        m_dhIniAquisicao = api::CDateTime::Agora();                  // api_f479 (the urna's system clock)
}

}  // namespace comum::md::estadoaplicacao
