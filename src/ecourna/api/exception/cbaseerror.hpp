// ecourna-lib/ecourna/api/exception/cbaseerror.hpp   (path inferred; 34 reconstructed files already include it)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
//
// CBaseError<E, SErrorLimits{min, max}> : CError. One instantiation per error family (45 in the RTTI:
// EIoError{1175,1275}, ESecurityError{1325,1725}, EDadosError{1935,2135}, vota::EUeVotaError, ...; three more
// RTTI entries are classes derived from one of them: api::CUePrinterError, api::CUeDesligandoError,
// ecourna::api::io::CIoError, and those three CBaseError<> bases have no vtable of their own in the binary).
// It adds no data member and no virtual function: its vtable repeats CError's slots (4636 / 1940 or 454 /
// 11562; 42 vtables). The limits appear only in the type (RTTI name); no function of the binary checks the code against
// them, so they are documentation / compile-time information.                                        // ?
#pragma once

#include <source_location>
#include <string>

#include "ecourna/api/exception/cerror.hpp"

namespace ecourna::api::exception {

// Structural class type usable as a non-type template parameter (C++20). The RTTI prints it as
// "SErrorLimits{1175, 1275}".
struct SErrorLimits {
    int min;   // first code of the family
    int max;   // last code of the family
};

template <typename E, SErrorLimits LIMITS>
class CBaseError : public CError {
public:
    // One constructor per family in the source; in the binary wasm-opt (merge-similar-functions) folded all
    // of them into three shared bodies that take the family's vtable as an extra argument, reached through
    // 18/21-byte thunks (one per family):
    //
    //   wasm func 1011  body used by the ecourna families (thunks 9025, 9026, 9041, 9046, 9107, 9158, 9181,
    //                   9229, 9266, 9410, 9461, 9505, 9584 ...). The CError constructor is called through
    //                   invoke (slot 169) with a landing pad that destroys the moved message.
    //   wasm func 710   same body without the landing pad (direct call to func 1143): the uenux2 families
    //                   (vota_f253 EUeVotaError, comum_f283 EUeComumGravadoresError, api_f346 EUeUtilError,
    //                   api_f406 EUeGuiError, 463 EUeAssertError, 480 EUeComumError, 580 EUeComumRelatoriosError,
    //                   591 EUeComumMdError, 655 EUeRdvError, 1074 EApiAsnError, 1229 EUeHwilError, 1710
    //                   EUeComumAppInfoError, 1715 EUePersistenciaError, 1889 EUeIpcError, 2260
    //                   EUeComumJustificativaError, ...).
    //   wasm func 2294  third variant (rt:shared) for EPatternError (shared_f331), EUePatternError (api_f235),
    //                   EUeComumDadosError (comum_f170), EUeComumAsnError (comum_f255), EUeIoError (api_f210):
    //                   it calls the "CError part" through a table slot (144/159/469/477/483 -> func 2379,
    //                   five identical per-family functions folded by ICF) and then stores the vptr.
    //   wasm func 2379  the folded "CError part": move the message into a by-value temporary, invoke the CError
    //                   constructor (func 1143, slot 169), destroy the temporary, return this.
    //                   The same five families are the only ones whose deleting destructor is 1940 (out-of-line
    //                   ~CError) rather than 454, so both differences come from the translation unit(s) that
    //                   define them, not from the template.
    //
    // All variants: the message parameter is taken BY VALUE and moved (the i64+i32 copy of the 12-byte
    // string followed by zeroing the source), the code is passed as int, the location is a pointer to the
    // static std::source_location record of the throw site.
    CBaseError(E codigo, std::string mensagem, std::source_location local = std::source_location::current())
        : CError(static_cast<int>(codigo), std::move(mensagem), local)   // wasm func 1143
    {
    }
};

} // namespace ecourna::api::exception
