// uenux2/src/app/comum/dados/asn/processoeleitoral/cconversordetalheconsulta.h   (path inferred: RTTI only; included
// under this name by cconversorcargo.cpp, unit u03; its md class is the attested
// dados/md/processoeleitoral/cdetalheconsulta.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u35).
//
// A "consulta" (referendum / plebiscite question) is a cargo whose DetalheCargo is a pergunta:
//   DetalhePergunta ::= SEQUENCE { nome GeneralString (SIZE(1..36)), pergunta GeneralString (SIZE(1..200)),
//                                  textoFonetico [1] GeneralString OPTIONAL, respostas SEQUENCE OF RespostaPergunta }
//   RespostaPergunta ::= SEQUENCE { numero INTEGER (0..99999), resposta GeneralString (SIZE(1..30)),
//                                   textoFonetico [1] GeneralString OPTIONAL }
// md::CDetalheConsulta (48 bytes): +0 nome, +12 pergunta, +24 std::vector<md::CRespostaConsulta>, +36 textoFonetico.
// md::CRespostaConsulta (28 bytes): +0 TCandidatoID numero, +4 resposta, +16 textoFonetico.
//
// RTTI: comum::asn::CConversorDetalheConsulta : IConversorASN<ModuloEleicao::DetalhePergunta, md::CDetalheConsulta>
//       vtable @1569772: [2] 11376 default DoConverte (throws 7655) [3] 11377 DoDesconverte. sizeof 4.
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/processoeleitoral/cdetalheconsulta.h"
#include "ModuloEleicao.h"

namespace comum::asn {

class CConversorDetalheConsulta : public IConversorASN<ModuloEleicao::DetalhePergunta, md::CDetalheConsulta>
{
protected:
    TDado DoDesconverte(const TEntidade& pergunta) const override;   // wasm func 11377
};

} // namespace comum::asn
