// uenux2/src/app/comum/asn/iconversorasn.h
// Reconstructed from vota_web_wasm.wasm (unit u21). See docs/modules/u21-uenux2-src-app-comum-asn-uenux2-src-app-comum-carquivosresul.md
//
// comum::asn::IConversorASN<ENTIDADE, DADO> is the uenux2 twin of ecourna::api::asn::IConversorASN (unit u11).
// It is the base class of every "conversor" of the voting application: an object that turns a plain C++ data
// object of comum::md ("dado") into the III ASN.1 object that is BER-encoded into a file ("entidade", the
// Converte direction, used to WRITE a file) and back (Desconverte, used after READING a file).
//
// Differences from the ecourna twin:
//  * the method is spelled "Desconverte" (ecourna: "Deconverte");
//  * DoConverte/DoDesconverte are NOT pure virtual: the defaults throw "Método DoXxx() não implementado para {}".
//    A read-only data file (candidates, parties, ...) only overrides DoDesconverte, a write-only one (BU, hashes)
//    only DoConverte;
//  * errors are CBaseError<comum::EUeComumAsnError, SErrorLimits{7650, 7750}> (typeinfo @1528108, vtable @1528128);
//  * the typeinfo name handed to ASN1::trace_invalid is the raw mangled name without the ": " suffix.
//
// The std::source_location records give the line of each throw (column 19 = the first character after
// "            throw "): Converte :56, Desconverte :71, DoConverte :89, DoDesconverte :98.
//
// ---------------------------------------------------------------------------------------------------------------
// How the template looks in the wasm (wasm-opt merge-similar-functions):
//   func 1563  Converte     merged body, no landing pads   (thunks pass &srcloc(:56) and typeid(ENTIDADE).name())
//   func 2893  Converte     the same source compiled WITH landing pads (invoke_*): ~SEQUENCE on unwind.
//                           Used only by the four EstadoGeral* instantiations (eg.bin, vota.bin, sa.bin, gap.bin).
//   func 1008  Desconverte  merged body (thunks pass &srcloc(:71) and the type name)
//   func 731   DoConverte / DoDesconverte defaults, merged body (thunks pass srcloc(:89|:98), code 7655|7656,
//              two pointers into the format-string literal and the type name)
//   func 9843  std::make_format_args(std::string&) for the throw in 2893 (library)
// Instantiations whose Converte/Desconverte were NOT merged (the call to DoXxx() is devirtualised into the caller's
// vtable slot, or the entity is an ENUMERATED/CHOICE and isValid() is inlined) carry the srcloc name of the INLINED
// method but are really a converter's DoConverte/DoDesconverte. They are listed in the doc (section "naming traps").
// ---------------------------------------------------------------------------------------------------------------
#pragma once

#include <format>
#include <source_location>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>

#include "asn1.h"                                     // III ASN.1 runtime: AbstractData, trace_invalid
#include "ecourna/api/exception/cbaseerror.hpp"       // CBaseError<E, SErrorLimits{...}>
#include "api/io/asn/cfileasn.h"                      // api::CFileASN::DecodeObject<T>

namespace comum {

// Declared in comum's error header (not in this file). Values used by comum/asn (names inferred):
//   7650 CConversorAbrangencia: "Tipo inválido [{}]"                7651 CConversorCabecalhoEntidade: "Tipo inválido [{}]"
//   7652 "Tipo de id de cabeçalho inválido"                         7653 "Entidade deixada em estado inválido: {}"
//   7654 "Entidade está inválida: {}"                               7655 "Método DoConverte() não implementado para {}"
//   7656 "Método DoDesconverte() não implementado para {}"          7658 IConversorBiometriaASN: "Entidade está inválida: {}"
//   7659 IConversorBiometriaASN: "Método DoConverte() não ..."      7665 IConversorParcialASN (other unit)
//   7667/7668 LeEntidadeEm seek/read    7669/7670 LeEntidadeBiometria seek/read    7671 "Formato de data inválido."
//   7673/7679/7682 util.cpp (Desconverte* of CabecalhoPacote)       7674/7675 Fase   7678 TipoPacote  7680 IDEleitoral
//   7681 Sistema   7684 Sexo   7685/7686 Abrangência   7687/7688 TipoLocalVotacao   7689/7690 Turno
enum class EUeComumAsnError : int;
using CUeComumAsnError =
    ecourna::api::exception::CBaseError<EUeComumAsnError, ecourna::api::exception::SErrorLimits{7650, 7750}>;

using uebyte = unsigned char;

namespace asn {

// RTTI: comum::asn::IConversorASN<E, D> is abstract-looking (typeinfo only, no vtable of its own).
// Every subclass vtable: [0] ~T() (ICF 174 "return this")  [1] deleting ~T() (144 free)
//                        [2] DoConverte(const TDado&)        [3] DoDesconverte(const TEntidade&)
template <typename ENTIDADE, typename DADO>
class IConversorASN
{
public:
    using TEntidade = ENTIDADE;
    using TDado = DADO;

    virtual ~IConversorASN() = default;

    // Converts, then requires the result to satisfy every constraint of the ASN.1 schema.
    // wasm: merged body 1563 (or 2893), thunks 2275 3726 3736 3799 5691 5696 5713 5714 9997 10019 10051 10170.
    TEntidade Converte(const TDado& dado) const
    {
        TEntidade entidade = DoConverte(dado);                                   // vtable slot 2
        if (!entidade.isValid() || !entidade.isStrictlyValid()) {
            std::ostringstream erro;
            ASN1::trace_invalid(erro, typeid(TEntidade).name(), entidade);
            throw CUeComumAsnError(EUeComumAsnError(7653),
                                   std::format("Entidade deixada em estado inválido: {}", erro.str()));  // line 56
        }
        return entidade;
    }

    // Checks the entity FIRST (a decoded file may hold anything the BER decoder accepted), then converts it.
    // wasm: merged body 1008, thunks 3727 3730 3737 3738 5686 5688 5692 5697 5705 5711 5715 5716 5721.
    TDado Desconverte(const TEntidade& entidade) const
    {
        if (!entidade.isValid() || !entidade.isStrictlyValid()) {
            std::ostringstream erro;
            ASN1::trace_invalid(erro, typeid(TEntidade).name(), entidade);
            throw CUeComumAsnError(EUeComumAsnError(7654),
                                   std::format("Entidade está inválida: {}", erro.str()));             // line 71
        }
        return DoDesconverte(entidade);                                          // vtable slot 3
    }

    // Decodes a BER buffer and converts it (used for rdv.dat by CRdvVota::Desconverte / ConfereConteudo).
    // wasm func 5680 (only instantiation: EntidadeRegistroDigitalVoto -> CVotosEleicoesVota)   // name inferred (overload)
    // The byte vector is copied into a std::vector<char> because CFileASN works on char buffers.
    TDado Desconverte(const std::vector<uebyte>& conteudo) const
    {
        const std::vector<char> buffer(conteudo.begin(), conteudo.end());
        const TEntidade entidade = api::CFileASN::DecodeObject<TEntidade>(buffer);   // func 5825
        return Desconverte(entidade);                                                // inlined (:71)
    }

protected:
    // Default: the direction is not supported by this converter.
    // wasm: merged body 731 (code 7655), thunks 11358 11360 11362 11364 11368 11370 11372 11374 11376 11380 11409
    //       11415 11421 11439 11444 11446 11449
    virtual TEntidade DoConverte(const TDado& /*dado*/) const
    {
        throw CUeComumAsnError(EUeComumAsnError(7655),
            std::format("Método DoConverte() não implementado para {}", typeid(TEntidade).name()));    // line 89
    }

    // wasm: merged body 731 (code 7656), thunks 10265 (EntidadeHashes), 10272 (EntidadeBoletimUrna),
    //       10275 (HistoricoVotoImpresso): these three files are write-only for the voting application.
    virtual TDado DoDesconverte(const TEntidade& /*entidade*/) const
    {
        throw CUeComumAsnError(EUeComumAsnError(7656),
            std::format("Método DoDesconverte() não implementado para {}", typeid(TEntidade).name())); // line 98
    }
};

// ---------------------------------------------------------------------------------------------------------------
// Out-of-line instantiations present in the binary (thunk -> merged body). typeid names are the mangled strings
// passed to trace_invalid / the "não implementado" message.
//
//  Converte (-> 1563):
//    2275  CabecalhoEntidade -> CCabecalhoEntidade          3726  NumViasImpressasRelatorios -> CNumViasImpressasRelatorios
//    3736  ComplementoMunicipio -> CComplementoMunicipio    3799  ModuloTiposResultadosEcoUrna::Urna -> CUrna
//    5691  DadoCorrespondencia -> CDadoCorrespondencia      5696  DadoSecao -> CLocalidadeEleitoral
//    5713  ModuloLocal::SecaoEleitoral -> CSecaoEleitoral   5714  ModuloTiposCadastro::Municipio -> CMunicipio
//  Converte (-> 2893, exception-safe copy):
//    9997  EstadoGeralVota (vota.bin)   10019 EstadoGeralSA (sa.bin)   10051 EstadoGeralGap (gap.bin)
//    10170 EstadoGeralUrna (eg.bin)
//  Desconverte (-> 1008):
//    3727  NumViasImpressasRelatorios   3730 Seguranca   3737 ComplementoMunicipio   3738 CabecalhoEntidade
//    5686  ModuloProcessoEleitoral::Pleito -> CPleitoDTO     5688 NomesCargo   5692 DadoCorrespondencia
//    5697  DadoSecao   5705 CabecalhoPacote   5711 DadosCandidato   5715 SecaoEleitoral   5716 Municipio
//    5721  Abrangencia
//  Desconverte, full copies (isValid inlined):
//    5684  TipoIdentificadorEleitor (ENUMERATED: value > info->maxEnum) -> ETipoIdentificadorEleitor
//    5828  CodigoCargoConsulta (CHOICE: CHOICE::isValid 1068 + CHOICE::isStrictlyValid 1067) -> unsigned char
//  Default DoConverte (-> 731, code 7655): see list above.   Default DoDesconverte (-> 731, code 7656): see above.
// ---------------------------------------------------------------------------------------------------------------

} // namespace asn
} // namespace comum
