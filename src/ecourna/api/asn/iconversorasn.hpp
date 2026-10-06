// ecourna-lib/ecourna/api/asn/iconversorasn.hpp
// Reconstructed from vota_web_wasm.wasm (unit u11). See docs/modules/u11-ecourna-lib-ecourna-api-asn.md.
// Line numbers 49 and 66 match the std::source_location records (iconversorasn.hpp:49 col 19 / :66 col 19);
// column 19 is the first character after "            throw ".
#pragma once

#include <source_location>
#include <sstream>
#include <string>
#include <typeinfo>

#include "asn1.h"                                    // III ASN.1 runtime: AbstractData, trace_invalid
#include "ecourna/api/exception/cbaseerror.hpp"      // CBaseError<E, SErrorLimits{...}>

namespace ecourna::api::asn {

// Error codes of this header. SErrorLimits{1900, 1910} (RTTI of the CBaseError instantiation).
// Only 1900 and 1902 are thrown anywhere in the binary. Names inferred.
enum class EApiAsnError : int {
    EntidadeGeradaInvalida = 1900,     // Converte: DoConverte() produced an invalid ASN.1 entity
    /* 1901: never thrown in this binary */
    EntidadeRecebidaInvalida = 1902,   // Deconverte: the ASN.1 entity handed in is invalid
};
using CApiAsnError = exception::CBaseError<EApiAsnError, exception::SErrorLimits{1900, 1910}>;  // alias inferred
// wasm func 1074: CApiAsnError::CApiAsnError(EApiAsnError, std::string, std::source_location)
//   = thunk into the shared CBaseError constructor body (func 710) with vtable @1563164.

// Converter between an ASN.1 entity (III ASN.1 generated class, "TEntidade") and a TSE data object
// ("TDado"). Non-virtual interface: Converte/Deconverte validate, DoConverte/DoDeconverte convert.
// RTTI: 35 instantiations, typeinfo only (__class_type_info, abstract). Subclass vtables:
//   [0] ~T() (ICF 174 "return this")  [1] ~T() deleting (144 free)  [2] DoConverte  [3] DoDeconverte
// Subclasses hold no data: sizeof == 4 (vptr only); callers build them on the stack.
template <typename ENTIDADE, typename DADO>
class IConversorASN
{
public:
    using TEntidade = ENTIDADE;
    using TDado = DADO;
    virtual ~IConversorASN() = default;

    // Converts and then checks the RESULT. wasm: see "Out-of-line code" below.
    TEntidade Converte(const TDado& dado) const
    {
        TEntidade entidade = DoConverte(dado);
        if (!entidade.isValid() || !entidade.isStrictlyValid()) {
            const std::string nome = std::string(typeid(TEntidade).name()) + ": ";   // mangled name, e.g.
            std::stringstream erro;                                               // "N21ModuloTiposEleitorais4FaseE: "
            ASN1::trace_invalid(erro, nome.c_str(), entidade);                    // Portuguese InvalidTracer text
            throw CApiAsnError(EApiAsnError::EntidadeGeradaInvalida, erro.str());       // srcloc line 49
        }
        return entidade;
    }

    // Checks the ENTITY FIRST (a decoded file may hold anything), then converts it.
    TDado Deconverte(const TEntidade& entidade) const
    {
        if (!entidade.isValid() || !entidade.isStrictlyValid()) {
            const std::string nome = std::string(typeid(TEntidade).name()) + ": ";
            std::stringstream erro;
            ASN1::trace_invalid(erro, nome.c_str(), entidade);
            // Message: "<mangled type name>: <field path> <reason>", e.g. "...: .configuracoes[0].codigoMunicipio
            // Valor 0 para campo INTEGER é menor que seu limite inferior 1" (tracer format seen at run time).
            // Files read through api::CFileASN were already checked the same way (EUeIoError 5954), so for
            // them this check is a second one. The raw typeinfo name is not demangled.
            // No catch here; the exception object is 40 bytes (__cxa_allocate_exception(40)).
            throw CApiAsnError(EApiAsnError::EntidadeRecebidaInvalida, erro.str());     // srcloc line 66
        }
        return DoDeconverte(entidade);
    }

protected:
    virtual TEntidade DoConverte(const TDado& dado) const = 0;          // vtable slot 2
    virtual TDado DoDeconverte(const TEntidade& entidade) const = 0;    // vtable slot 3
};

// ---------------------------------------------------------------------------------------------------------
// Out-of-line code in the wasm (the full per-instantiation list is in iconversorasn.instances.cpp)
// ---------------------------------------------------------------------------------------------------------
// Every out-of-line Converte/Deconverte instantiation became a 20/21-byte thunk that calls one SHARED
// body (wasm-opt merge-similar-functions), passing (srcloc record, typeid(TEntidade).name()):
//
//   func  682  Converte   body, SEQUENCE entities, exception-safe (invoke_* + landing pads: on a throw it
//              destroys the string, the stringstream and the entity via ASN1::SEQUENCE::~SEQUENCE)
//   func 1167  Deconverte body, exception-safe (invoke_*)
//   func 6031  Converte   body WITHOUT landing pads (direct calls, nothing destroyed on unwind)
//   func 6032  Deconverte body WITHOUT landing pads
//   func 6149  Deconverte body for ENUMERATED entities (Fase, Turno): isValid()/isStrictlyValid() inlined to
//              "value > info->maxEnumValue (signed) => invalid"; returns the small TDado in a register.
//              This is exactly the III runtime rule (ValidChecker::do_visit(const ENUMERATED&), func 8984:
//              "extensible || value <= max"; the strict check drops "extensible"), so there is no lower bound
//              anywhere: 0 or negative values pass (the converters CConversorFase/CConversorTurno reject them).
//   func 6150  Converte   body for ENUMERATED entities (same inlined check on the result)
//
// CHOICE entities do not use a shared body: for IConversorASN<ModuloTiposEleitorais::IdentificadorEleitor,
// CIdentificadorEleitor> the name lookup finds CHOICE::isValid (func 1068) / CHOICE::isStrictlyValid (func
// 1067, labelled "comum_f1067") instead of AbstractData's, so funcs 9111 (Deconverte) and 9113 (Converte)
// are full copies of the template.
//
// 6031/6032 versus 682/1167: same source, different code generation. Besides the missing landing pads
// (no invoke_*), the no-EH copies call std::string::append(const char*) (run-time strlen of ": ") where
// 682/1167 call append(": ", 2) with the length folded. The four instantiations whose bodies are 6031/6032
// (IdentificadorGeradorMidia x2, DadosComparecimento Converte, Foto Deconverte) are exactly the out-of-line
// ones that uenux2 code also calls (CConversorCarga, CConversorDadoCorrespondencia, CGravadorRCSecao,
// CConversorBiometriaEleitor, CConversorFotoCandidato), whose translation units are mostly compiled without
// exception catching; no 682/1167 thunk has a uenux2 caller. Inference: the linker kept the uenux2 COMDAT
// copy, and ecourna callers (e.g. CConversorDadosGeracaoMidia 9202/9203, CConversorResultadoUrnaCadastro
// 9056) use it too.
//
// Many callers inline Converte/Deconverte completely; the srcloc record then names the CALLER after this
// template (see the "naming trap" section of the module doc: 9057, 9092, 9094, 9119, 9120, 9127, 9142,
// 9171, 9176, 9206 carry an IConversorASN<...>::Deconverte name but are other functions).

}  // namespace ecourna::api::asn
