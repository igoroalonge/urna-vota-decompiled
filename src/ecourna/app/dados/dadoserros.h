// ecourna-lib/ecourna/app/dados/dadoserros.h  (path inferred; the real enums live in per-module
// headers that are not in the binary)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
// Every error thrown by this unit is an ecourna::api::exception::CBaseError<E, SErrorLimits{lo, hi}>
// (a CError with an integer code, a message and a std::source_location). The RTTI gives the enum
// type and the code range of each family; the code values used below are read from the call sites.
// wasm-opt merged the nine constructors into thin thunks over one body (func 1011, which calls the
// CError constructor func 1143 and stores the family's vtable):
//
//   thunk  family (RTTI)                                         codes          vtable
//   9266   CBaseError<EDadosError, {1935, 2135}>                 1951..2030     1121828
//   9046   CBaseError<EDadosFederacoesError, {2940, 2965}>       2940           1138064
//   9041   CBaseError<EDadosMidiasError, {3040, 3065}>           3040..3044     1138224
//   9026   CBaseError<EDadosProcessoEleitoralError, {3140,3190}> 3145..3146     1138476
//   9025   CBaseError<EDadosResultadoUrnaCadastroError,{3265,3315}> 3267..3283  1138636
//   9229   CBaseError<asn::EAsnError, {2235, 2335}>              2235..2268     1122772
//   (9181) CBaseError<asn::EAsnMidiasError, {2485, 2510}>        2485..2489     1127416  (thunk in unit u40)
//   9158   CBaseError<asn::EAsnParametrizacaoUrnaError,{2560,2585}> 2560..2577  1129096
//   9107   CBaseError<asn::EAsnResultadoUrnaCadastroError,{2635,2665}> 2635..2660 1132848
// In the reconstructed .cpp files the codes are written as plain integers (e.g.
// CDadosError(2014, ...)), meaning the enumerator with that value; the enumerator names are unknown.
#pragma once

#include "ecourna/api/exception/cbaseerror.hpp"

namespace ecourna::app::dados {

enum class EDadosError : int {};
enum class EDadosFederacoesError : int {};
enum class EDadosMidiasError : int {};
enum class EDadosProcessoEleitoralError : int {};
enum class EDadosResultadoUrnaCadastroError : int {};

using CDadosError = api::exception::CBaseError<EDadosError, api::exception::SErrorLimits{1935, 2135}>;
using CDadosFederacoesError =
    api::exception::CBaseError<EDadosFederacoesError, api::exception::SErrorLimits{2940, 2965}>;
using CDadosMidiasError = api::exception::CBaseError<EDadosMidiasError, api::exception::SErrorLimits{3040, 3065}>;
using CDadosProcessoEleitoralError =
    api::exception::CBaseError<EDadosProcessoEleitoralError, api::exception::SErrorLimits{3140, 3190}>;
using CDadosResultadoUrnaCadastroError =
    api::exception::CBaseError<EDadosResultadoUrnaCadastroError, api::exception::SErrorLimits{3265, 3315}>;

namespace asn {
enum class EAsnError : int {};
enum class EAsnMidiasError : int {};
enum class EAsnParametrizacaoUrnaError : int {};
enum class EAsnResultadoUrnaCadastroError : int {};

using CAsnError = api::exception::CBaseError<EAsnError, api::exception::SErrorLimits{2235, 2335}>;
using CAsnMidiasError = api::exception::CBaseError<EAsnMidiasError, api::exception::SErrorLimits{2485, 2510}>;
using CAsnParametrizacaoUrnaError =
    api::exception::CBaseError<EAsnParametrizacaoUrnaError, api::exception::SErrorLimits{2560, 2585}>;
using CAsnResultadoUrnaCadastroError =
    api::exception::CBaseError<EAsnResultadoUrnaCadastroError, api::exception::SErrorLimits{2635, 2665}>;
} // namespace asn

} // namespace ecourna::app::dados
