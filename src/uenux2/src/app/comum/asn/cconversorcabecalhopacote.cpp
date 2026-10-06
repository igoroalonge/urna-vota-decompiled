// uenux2/src/app/comum/asn/cconversorcabecalhopacote.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u21).
#include "comum/asn/cconversorcabecalhopacote.h"

#include "comum/asn/util.h"

namespace comum::asn {

// wasm func 11438 (vtable slot 2). Utils::ConverteTipoPacote (util.cpp:231), Utils::ConverteIdPacote (:344, with
// CIDPacote::UF/Municipio/Zona of cidpacote.cpp:67/75/83) and Utils::ConverteIdSistema (:419) are inlined.
// Not observed executing.
ModuloTiposEleitorais::CabecalhoPacote CConversorCabecalhoPacote::DoConverte(const md::CCabecalhoPacote& cabecalho) const
{
    ModuloTiposEleitorais::CabecalhoPacote entidade;
    entidade.set_tipoPacote(Utils::ConverteTipoPacote(cabecalho.GetTipo()));
    entidade.set_idPacote(Utils::ConverteIdPacote(cabecalho.GetIdPacote()));
    entidade.set_nomepacote(cabecalho.GetNome());
    entidade.set_versao(cabecalho.GetVersao());
    // abrangencia ([1] OPTIONAL) is left absent.
    entidade.set_origem(Utils::ConverteIdSistema(cabecalho.GetOrigem()));
    return entidade;
}

} // namespace comum::asn
