// ecourna-lib/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipio.h   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u11).
#pragma once

#include "ecourna/api/asn/iconversorasn.hpp"
#include "ecourna/api/pattern/cbasetype.hpp"
#include "ecourna/app/dados/asn/processoeleitoral/cconversorhorariosurna.h"   // 9132/9133 (unit u40)
#include "ecourna/app/dados/processoeleitoral/cconfiguracaomunicipios.h"
#include "ModuloConfiguracaoMunicipios.h"

namespace ecourna::app::dados::asn {

// Range-checked wrapper of the municipality code (converts to TMunicipioID, u14): func 2851 (cbasetype.hpp:39)
using TCodigoMunicipio = CBaseType<unsigned int, 0, 99999, 6>;   // alias name inferred

// RTTI: CConversorConfiguracaoMunicipio
//         : IConversorASN<ModuloConfiguracaoMunicipios::ConfiguracaoMunicipio, CConfiguracaoMunicipio>
// vtable @1129456: [0] 174  [1] 144  [2] 9144 DoConverte (unit u40)  [3] 9142 DoDeconverte
//
//   ConfiguracaoMunicipio ::= SEQUENCE { codigoMunicipio INTEGER (1..99999), horariosUrna HorariosUrna }
//   HorariosUrna ::= SEQUENCE { emissaoZeresima, inicioVotacao, encerramentoVotacao, terminoVotacao DataHoraJE }
//   CConfiguracaoMunicipio (40 bytes, trivially copyable): +0 TCodigoMunicipio, +8 CHorariosUrna (4 x 64-bit
//   date-time: zerésima, início, encerramento, término)
class CConversorConfiguracaoMunicipio
    : public api::asn::IConversorASN<ModuloConfiguracaoMunicipios::ConfiguracaoMunicipio, CConfiguracaoMunicipio>
{
protected:
    TEntidade DoConverte(const TDado& dado) const override;          // wasm func 9144 (unit u40)
    TDado DoDeconverte(const TEntidade& entidade) const override;    // wasm func 9142
};

}  // namespace ecourna::app::dados::asn
