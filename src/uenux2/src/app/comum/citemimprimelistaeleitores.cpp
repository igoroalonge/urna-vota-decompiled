// uenux2/src/app/comum/citemimprimelistaeleitores.cpp (path inferred from the class name)
// Reconstructed from vota_web_wasm.wasm by unit u37.
#include "comum/citemimprimelistaeleitores.h"

#include "comum/appinfo/cappinfo.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/informacao/cinformacaoeleicao.h"

namespace comum {

// wasm func 11668 (vtable slot 2)
// The voter list exists only on urnas that carry the section's voters. The urna type of the current turno
// (EstadoGeralUrna.dadoCarga.tipoUrnaT1/T2, CEstadoGeral +36 / +40 chosen by the turno at +32, stored as the
// character '0' + TipoUrnaOperacao) must be vota ('1'), contingenciavota ('3') or contingenciavotarecupera
// ('4'); semtipo ('0') and a plain contingência urna ('2') never offer the list.
bool CItemImprimeListaEleitores::Disponivel() const
{
    const auto& geral = CAppInfo::GetInst().GetGeral();                             // wasm 185 + 291
    const char tipo = geral.GetTurno() == EUrnaTurno::Primeiro ? geral.GetTipoUrnaT1()    // +36
                                                               : geral.GetTipoUrnaT2();   // +40
    // compiled as: unsigned(tipo - '1') > 3 || tipo == '2'  -> false
    if (tipo != '1' && tipo != '3' && tipo != '4')
        return false;

    const CInformacaoEleicao informacao(CConfiguracaoEleicao::GetInst());
    const unsigned maximo = informacao.GetNumRelatorioEleitores();                  // wasm 5917
    return GetNumViasImpressas() < maximo;                                           // slot 3 (+74)
}

} // namespace comum
