// uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversorestadogeralvota.h
// Reconstructed from vota_web_wasm.wasm (unit u03). Conventions: see asn/rdv/cconversorvoto.h.
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/estadoaplicacao/cestadogeralvota.h"
#include "ModuloEstadoGeralVota.h"

namespace comum::asn {

// RTTI: CConversorEstadoGeralVota
//         : IConversorASN<ModuloEstadoGeralVota::EstadoGeralVota, md::estadoaplicacao::CEstadoGeralVota>
// vtable @1568968: [0] 174 [1] 144 [2] 11390 DoConverte [3] 11391 DoDesconverte
// Used through comum::IServicoEstado<CEstadoGeralVota, CConversorEstadoGeralVota> for trab1|trab2/vota.bin.
//
// md::estadoaplicacao::CEstadoGeralVota (100 bytes), from its constructor (func 5628) and DoConverte:
//   +0  EEstadoVota estado (char '1'..'@')            +4  EEstadoEncerramento encerramento (char '1'..'4')
//   +8  uebyte qtdBU                                   +12 std::optional<uedword> urnaIdGerouZeresima (flag +16)
//   +20 std::optional<api::CDateTime> dhIniAquisicao (12 bytes, flag +32)
//   +36 std::optional<api::CDateTime> dhFimAquisicao (flag +48)
//   +52 ueword comparecimento                          +54 ueword qtdJustificativa
//   +56 std::optional<api::CDateTime> dhUltimoVoto (flag +68)
//   +72 bool treinamentoEleitor                        +73 CNumViasImpressasRelatorios numVias (4 x uebyte, unaligned)
//   +80 std::optional<api::CDateTime> dhEmissao (flag +92)
//   +96 bool tecladoTestadoPosConversao
class CConversorEstadoGeralVota
    : public IConversorASN<ModuloEstadoGeralVota::EstadoGeralVota, md::estadoaplicacao::CEstadoGeralVota>
{
public:
    ModuloEstadoGeralVota::EstadoVota ConverteEstadoVota(const EEstadoVota estado) const;                          // inlined
    EEstadoVota DesconverteEstadoVota(const ModuloEstadoGeralVota::EstadoVota& estado) const;                      // inlined
    ModuloEstadoGeralVota::EstadoEncerramento ConverteEstadoEncerramento(const EEstadoEncerramento estado) const;  // inlined
    EEstadoEncerramento DesconverteEstadoEncerramento(const ModuloEstadoGeralVota::EstadoEncerramento& e) const;   // inlined

protected:
    TEntidade DoConverte(const TDado& estado) const override;       // wasm func 11390
    TDado DoDesconverte(const TEntidade& estado) const override;    // wasm func 11391
};

} // namespace comum::asn
