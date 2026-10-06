// ecourna-lib/ecourna/app/dados/ccabecalhoentidade.h   (path inferred: the data classes of ModuloTiposEleitorais
//   live directly in ecourna/app/dados/, cf. cregistroidentificacaoeleitor.cpp and tiposbasicos.cpp; five
//   reconstructed files include this name)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
//
// = ModuloTiposEleitorais::CabecalhoEntidade { dataGeracao DataHoraJE, idEleitoral CHOICE { idProcessoEleitoral,
//   idPleito, idEleicao } }: the header ("cabeçalho") of every ecourna data file (-pu parametrização, -cfm
//   configuração de municípios, -fe federações, the RC attendance result ...). Converter:
//   asn::CConversorCabecalhoEntidade (unit u14). Trivially copyable, no validation, 16 bytes.
#pragma once

#include <boost/date_time/posix_time/ptime.hpp>

namespace ecourna::app::dados {

class CCabecalhoEntidade {
public:
    // Which identifier the header carries = the CHOICE alternative index of idEleitoral.
    enum ETipoId : int {
        IdProcessoEleitoral = 0,   // idProcessoEleitoral [1]  ("processo eleitoral" = the whole election cycle)
        IdPleito = 1,              // idPleito [2]             ("pleito" = one election day)
        IdEleicao = 2,             // idEleicao [3]            ("eleição" = one election of the pleito)
        IdInvalido = 3,            // ? sentinel tested by CConversorCabecalhoEntidade::DoConverte (line 48)
    };

    // wasm func 5112 (callers: CConversorCabecalhoEntidade::DoDeconverte 9218, comum::CGravadorRCSecao 11616)
    CCabecalhoEntidade(const boost::posix_time::ptime& dataGeracao, int id, ETipoId tipo);

    const boost::posix_time::ptime& GetDataGeracao() const { return m_dataGeracao; }
    int GetId() const { return m_id; }
    ETipoId GetTipoId() const { return m_tipoId; }

private:
    boost::posix_time::ptime m_dataGeracao;   // +0  (int64 microseconds)
    int m_id;                                 // +8
    ETipoId m_tipoId;                         // +12
};                                            // sizeof 16

} // namespace ecourna::app::dados
