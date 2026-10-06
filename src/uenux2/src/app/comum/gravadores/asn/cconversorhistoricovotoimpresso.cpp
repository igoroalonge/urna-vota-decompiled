// uenux2/src/app/comum/gravadores/asn/cconversorhistoricovotoimpresso.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u35).
#include "comum/gravadores/asn/cconversorhistoricovotoimpresso.h"

#include "comum/asn/util.h"

namespace comum::asn {

// wasm func 10276 - vtable slot 2. Not observed executing.
// md::CHistoricoVotoImpresso: +0 idImpressoraVotos, +4 idRepositorioVotos, +8 api::CDateTime dataHoraLigamento.
ModuloBoletimUrna::HistoricoVotoImpresso
CConversorHistoricoVotoImpresso::DoConverte(const md::CHistoricoVotoImpresso& historico) const
{
    ModuloBoletimUrna::HistoricoVotoImpresso entidade;                                     // SEQUENCE(info @1143848)
    entidade.set_idImpressoraVotos(historico.GetIdImpressoraVotos());
    entidade.set_idRepositorioVotos(historico.GetIdRepositorioVotos());
    entidade.set_dataHoraLigamento(Utils::ConverteDataHoraJE(historico.GetDataHoraLigamento()));   // func 1080
    return entidade;
}

} // namespace comum::asn
