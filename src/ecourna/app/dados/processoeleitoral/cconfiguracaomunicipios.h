// ecourna-lib/ecourna/app/dados/processoeleitoral/cconfiguracaomunicipios.h  (path inferred)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
// Per-municipality urna schedule, file <fase><pleito><uf>-cfm.dat
// (ModuloConfiguracaoMunicipios::EntidadeConfiguracaoMunicipios). In the simulator scenarios every
// município has zerésima 07:00, voting 08:00-17:00 and "término" 04:00 of the next day.
#pragma once

#include <map>

#include <boost/date_time/posix_time/ptime.hpp>

#include "ecourna/app/dados/ccabecalhoentidade.h"   // CCabecalhoEntidade (unit u40; 16 bytes)

namespace ecourna::app::dados {

using TMunicipioID = unsigned int;   // "MunicipioID" (CBaseType, bounds not visible here)

struct CHorariosUrna {                              // 32 bytes (converter in unit u40)
    boost::posix_time::ptime m_emissaoZeresima;     // +0
    boost::posix_time::ptime m_inicioVotacao;       // +8
    boost::posix_time::ptime m_encerramentoVotacao; // +16
    boost::posix_time::ptime m_terminoVotacao;      // +24
};

class CConfiguracaoMunicipio {                      // 40 bytes (converter in unit u40)
public:
    TMunicipioID GetCodigoMunicipio() const { return m_codigoMunicipio; }
    const CHorariosUrna& GetHorariosUrna() const { return m_horarios; }
private:
    TMunicipioID m_codigoMunicipio;                 // +0
    CHorariosUrna m_horarios;                       // +8
};
using TMapConfiguracaoMunicipio = std::map<TMunicipioID, CConfiguracaoMunicipio>;

class CConfiguracaoMunicipios {
public:
    CConfiguracaoMunicipios(const CCabecalhoEntidade& cabecalho,
                            const TMapConfiguracaoMunicipio& configuracoes);   // func 9028

    const CCabecalhoEntidade& GetCabecalho() const { return m_cabecalho; }
    const TMapConfiguracaoMunicipio& GetConfiguracoes() const { return m_configuracoes; }

private:
    CCabecalhoEntidade m_cabecalho;                 // +0  (16 bytes)
    TMapConfiguracaoMunicipio m_configuracoes;      // +16 (libc++ __tree: begin, end-node, size)
};

} // namespace ecourna::app::dados
