// FRAGMENTS written by unit u37 for two classes of uenux2/src/app/comum/dados/md/estadoaplicacao/:
//   cestadogeralvota.cpp (attested by srclocs :108/:113) - md side of vota.bin (EstadoGeralVota)
//   cestadogeral.cpp     (attested)                        - md side of eg.bin  (EstadoGeralUrna)
// Both functions are inline one-liners that were kept out of line.
#include "comum/dados/md/estadoaplicacao/cestadogeral.h"
#include "comum/dados/md/estadoaplicacao/cestadogeralvota.h"

namespace comum::md::estadoaplicacao {

// wasm func 3701 (tools: vota_f3701)                                name as in units u09/u27 (name inferred)
// EstadoGeralVota.qtdBU (+8): number of BU copies already printed. Callers: vota::CImprimindoBU slot 7
// (12077, 1st copy accepted by the mesário), CImprimirBUOutrasObrigatorias::StartState (12065, each
// mandatory copy), CEmitirMaisBU::ProcessInput (12081, each extra copy). The caller then saves vota.bin.
// NOTE: the ASN.1 type is INTEGER (0..999) but the C++ member is one byte, incremented without a check.
void CEstadoGeralVota::IncrementaQtdBU()
{
    ++m_qtdBU;                                                        // uebyte at +8
}

// wasm func 3705 (tools: vota_f3705)                                      name as in units u06/u10
// EstadoGeralUrna.ajusteDataHora (+52, 8 bytes: {TipoAjusteDataHora tipo; int valor}): records how far
// the clock was moved (alterarDataEleicao / alterarDataSistema). Callers: CAjusteInicial::StartState (7160),
// CIniciodeCiclo::AjustaDataHora (7306), CEncerramentoHorarioInvalido::StartState (10710) - each followed
// by comum::GravaEstadoGeral() (wasm 3333).
void CEstadoGeral::SetAjusteDataHora(const CAjusteDataHora& ajuste)
{
    m_ajusteDataHora = ajuste;
}

} // namespace comum::md::estadoaplicacao
