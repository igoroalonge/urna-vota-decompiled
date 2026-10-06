// uenux2/src/app/comum/relatorios/relatoriosdefs.h  (path inferred: the error family of the directory)
// Reconstructed from vota_web_wasm.wasm (unit u25).
//
// Every error thrown by comum/relatorios is an
//   ecourna::api::exception::CBaseError<comum::EUeComumRelatoriosError, SErrorLimits{9050, 9150}>
// (typeinfo @1543944, vtable @1543964). wasm-opt merged its constructor into the shared CBaseError body
// ecourna_f710; the thin thunk that passes this family's vtable is wasm func 580 (tools: comum_f580,
// 14 direct callers). Codes found at the call sites of this unit (enumerator names are not in the binary):
//
//   9050  "Campo ({}) não informado."                    ccabecalhoqrcodebuilder.cpp:284  (func 2791)
//   9051  "Identificação inválida [..]"                  ccalculacv.cpp:58   (ctor, inlined in 12110)
//   9052  "Primeiro código verificador inválido [..]"    ccalculacv.cpp:65   (ctor, inlined in 12110)
//   9053  "Chave privada com tamanho pequeno..."         ccalculacv.cpp:74   (ctor, inlined in 12110)
//   9054  "String vazia"                                 ccalculacv.cpp:109  (funcs 942, 5967)
//   9055..9064  "<parte> nulo" (10 parts of the BU)      cgeradorbubase.h:66..111 (ctor, inlined in 12110)
//   9065  "Abrangência não encontrada: {}"               cgeradorbuqrcode.cpp:80  (func 3696)
//   9066  "header nulo" / 9067 "trailer nulo"            cgeradorrelbase.h:48/51  (ctor, inlined in 12110)
//   9068  "Eleitor não posicionado"                      cgeradorrellistaeleitores.cpp:100 (func 11233)
//   9081  "Candidato não encontrado: {}/{}"              cpartecandidatos.cpp:75  (func 11214)
//   9083  "Resposta não encontrada: {}/{}"               cpartecandidatos.cpp:174 (func 11212)
//   9084  "Partido não posicionado"                      crelutil.cpp:58          (func 11181)
//   9085  "ID de carga inválido: [..]"                   crelutil.cpp:69          (func 2786)
//   9086  "Tipo inválido: {}"                            crelutil.cpp:307         (func 1543)
//   9087  "Fase inválida: {}"                            crelutil.cpp:554         (func 1919)
//   9088  "Instância já criada"                          csubstituidortitulo.cpp:26 (inlined in 3689)
//   9089  "Abrangência inválida: {}"                     cdatasourcesrelatorio.h:212 (lambda of
//                                                        GetLinhasEleitoresAptos, wasm 11968)
#pragma once

#include "ecourna/api/exception/cbaseerror.hpp"

namespace comum {

enum class EUeComumRelatoriosError : int {};

// wasm func 580 is the constructor thunk of this type:
//   CRelatoriosError(EUeComumRelatoriosError codigo, const std::string& mensagem, const std::source_location&)
using CRelatoriosError =
    ecourna::api::exception::CBaseError<EUeComumRelatoriosError, ecourna::api::exception::SErrorLimits{9050, 9150}>;

// Tipo de cabeçalho passed to CRelUtil::IncluiCabecalhoEleicoesMZS (enumerator names inferred from the
// text each value prints). Formatted in the error message through a std::formatter (format handle slot 3047).
enum class ETipoCabecalho : int {
    SECAO = 0,                    // "Local de Votação" + "Seção Eleitoral" + seções agregadas
    MESA_RECEPTORA = 1,           // "Mesa Receptora" {secao/10} + "Urna" {secao%10}  (MRJ)
    CONTINGENCIA = 2,             // "Urna de contingência"
    INVALIDO = 3,                 // ? throws 9086 "Tipo inválido: {}"
};

} // namespace comum
