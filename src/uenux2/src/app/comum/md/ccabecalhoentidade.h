// uenux2/src/app/comum/md/ccabecalhoentidade.h
// Reconstructed from vota_web_wasm.wasm (unit u24).
//
// md::CCabecalhoEntidade = header of every TSE data/result entity (BU, RDV, hashes, envelopes, voter and
// candidate files...): ModuloTiposEleitorais::CabecalhoEntidade { dataGeracao DataHoraJE,
// idEleitoral IDEleitoral CHOICE { [1] idProcessoEleitoral, [2] idPleito, [3] idEleicao } }.
// Converter: comum::asn::CConversorCabecalhoEntidade (unit u21). Not polymorphic. sizeof 20.
#pragma once

#include "api/util/cdatetime.h"
#include "comum/comumtypes.h"          // uedword   (header name ?)

namespace comum::md {

// Index of the IDEleitoral alternative (names inferred).
enum class ETipoCabecalho : int { ProcessoEleitoral = 0, Pleito = 1, Eleicao = 2 };

class CCabecalhoEntidade {
public:
    CCabecalhoEntidade(const api::CDateTime& dataGeracao, uedword id, ETipoCabecalho tipo);   // wasm 1945

    const api::CDateTime& GetDataGeracao() const { return m_dataGeracao; }
    uedword GetId() const { return m_id; }
    ETipoCabecalho GetTipo() const { return m_tipo; }

private:
    api::CDateTime m_dataGeracao;   // +0  (12 bytes: CDate 8 + CTime 4)
    uedword        m_id;            // +12 0..99999
    ETipoCabecalho m_tipo;          // +16
};

} // namespace comum::md
