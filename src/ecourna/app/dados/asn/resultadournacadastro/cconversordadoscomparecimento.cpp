// ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimento.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u11).
//
// Attendance ("comparecimento") of one polling section: who came, how each voter was enabled, which voters
// justified their absence, and which mesários (poll workers) were present at opening and closing. The urna
// writes it at the end of the day (comum::CGravadorRCSecao, func 11616) inside an
// EntidadeResultadoUrnaCadastro, in plain or encrypted form (InfoDadosComparecimento CHOICE).
#include "ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimento.h"

#include <algorithm>
#include <iterator>
#include <optional>
#include <vector>

#include "ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentomesario.h"
#include "ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentosecao.h"
#include "ecourna/app/dados/asn/resultadournacadastro/cconversoridentificacaojustificativa.h"
#include "ecourna/app/dados/asn/convertelista.h"   // ConverteLista (func 1970 + thunks 9098/9100), header and name inferred

namespace ecourna::app::dados::asn {

using ModuloResultadoUrnaCadastro::ComparecimentoMesario;
using ModuloResultadoUrnaCadastro::IdentificacaoJustificativa;

// wasm func 9095 (vtable slot 3; name already curated). Not observed at run time (the urna only WRITES
// this structure; reading it back is for tools / recovery paths).
CDadosComparecimento CConversorDadosComparecimento::DoDeconverte(const TEntidade& entidade) const
{
    const CConversorComparecimentoMesario conversorMesario;               // vptr @1132424
    const CConversorComparecimentoSecao conversorSecao;                   // vptr @1131064
    const CConversorIdentificacaoJustificativa conversorJustificativa;    // vptr @1136564

    // `auto` makes a deep COPY of the SEQUENCE OF (SEQUENCE_OF_Base copy ctor, func 2656); the copy lives
    // until the end of the function.
    auto listaJustificativas = entidade.get_justificativas();
    std::vector<CIdentificacaoJustificativa> justificativas;
    // wasm func 9094 = this std::transform instantiation, with IConversorASN<IdentificacaoJustificativa>::
    // Deconverte (srcloc :66 @1134080) and the vector push_back inlined into it.
    std::transform(listaJustificativas.begin(), listaJustificativas.end(), std::back_inserter(justificativas),
                   [&conversorJustificativa](const IdentificacaoJustificativa& justificativa) {
                       return conversorJustificativa.Deconverte(justificativa);
                   });

    // func 9093 -> 1167 -> CConversorComparecimentoSecao::DoDeconverte (func 9120)
    const CComparecimentoSecao secao = conversorSecao.Deconverte(entidade.get_identificacaoComparecimento());

    // wasm func 9092 = the ONE std::transform instantiation used by both blocks below (same iterator type,
    // same closure type; ComparecimentoMesario Deconverte inlined, srcloc :66 @1134112). In both blocks the
    // transformed vector is moved straight into the optional's storage (flag set afterwards).
    const auto deconverteMesario = [&conversorMesario](const ComparecimentoMesario& mesario) {
        return conversorMesario.Deconverte(mesario);
    };

    std::optional<std::vector<CComparecimentoMesario>> mesariosAbertura;
    std::optional<std::vector<CComparecimentoMesario>> mesariosEncerramento;
    if (entidade.hasOptionalField(TEntidade::e_mesariosAbertura)) {          // optional field 0
        auto lista = entidade.get_mesariosAbertura();                        // copy again (func 2656)
        std::vector<CComparecimentoMesario> mesarios;
        std::transform(lista.begin(), lista.end(), std::back_inserter(mesarios), deconverteMesario);
        mesariosAbertura = std::move(mesarios);
    }
    if (entidade.hasOptionalField(TEntidade::e_mesariosEncerramento)) {      // optional field 1
        auto lista = entidade.get_mesariosEncerramento();
        std::vector<CComparecimentoMesario> mesarios;
        std::transform(lista.begin(), lista.end(), std::back_inserter(mesarios), deconverteMesario);
        mesariosEncerramento = std::move(mesarios);
    }

    // All four arguments are copied (vector copies inlined; CComparecimentoSecao's CEstadoComparecimento
    // vector through func 2278) and then moved into the object by the constructor (func 3485).
    return CDadosComparecimento(justificativas, secao, mesariosAbertura, mesariosEncerramento);
}

// wasm func 9102 (vtable slot 2; was "CConversorDadosComparecimento::vf2"). Called through
// IConversorASN<DadosComparecimento>::Converte (func 5365 -> 6031) by CConversorResultadoUrnaCadastro::
// DoConverte and by comum::CGravadorRCSecao when the attendance file is written at the end of the day.
ModuloResultadoUrnaCadastro::DadosComparecimento CConversorDadosComparecimento::DoConverte(const TDado& dado) const
{
    const CConversorComparecimentoMesario conversorMesario;               // vptr @1132424
    const CConversorComparecimentoSecao conversorSecao;                   // vptr @1131064
    const CConversorIdentificacaoJustificativa conversorJustificativa;    // vptr @1136564

    ModuloResultadoUrnaCadastro::DadosComparecimento entidade;           // SEQUENCE info @1147692

    // func 9101 -> 682 -> CConversorComparecimentoSecao::DoConverte (9126); assigned with the SEQUENCE
    // assignment helper (func 339). The argument is the CComparecimentoSecao member at +12 of the dado
    // (getter inlined, name inferred).
    entidade.set_identificacaoComparecimento(conversorSecao.Converte(dado.GetComparecimentoSecao()));

    // The vector is copied into a local (inlined copy loop), then converted through func 9100
    // (= shared body 1970 with Converte slot 6928 = func 9097). set_justificativas is the III copy-and-swap
    // assignment: func 9099 = SEQUENCE_OF<IdentificacaoJustificativa>(first, last) builds the copy that is
    // swapped into field 0.
    const std::vector<CIdentificacaoJustificativa> justificativas = dado.GetJustificativas();
    entidade.set_justificativas(ConverteLista<ModuloResultadoUrnaCadastro::DadosComparecimento::justificativas>(
        justificativas, conversorJustificativa));

    if (dado.PossuiMesariosAbertura()) {       // inlined test of the optional's flag (byte +52), name inferred
        // func 9024 (cdadoscomparecimento.cpp:38) returns const TVectorComparecimentoMesario& and throws
        // "Comparecimento de mesários na abertura não definido para este objeto." when absent; copied here.
        const TVectorComparecimentoMesario mesarios = dado.GetMesariosAbertura();
        // func 9098 (= 1970 with Converte slot 6929 = func 9096). includeOptionalField(0, 2) (func 515) then
        // every element is cloned (new 52 + SEQUENCE copy ctor 853) and inserted into field 2 (func 1076).
        entidade.set_mesariosAbertura(ConverteLista<ModuloResultadoUrnaCadastro::DadosComparecimento::mesariosAbertura>(
            mesarios, conversorMesario));
    } else {
        entidade.omit_mesariosAbertura();                                  // removeOptionalField(0), func 432
    }

    if (dado.PossuiMesariosEncerramento()) {   // byte +68, name inferred
        const TVectorComparecimentoMesario mesarios = dado.GetMesariosEncerramento();   // func 9023, copied
        entidade.set_mesariosEncerramento(
            ConverteLista<ModuloResultadoUrnaCadastro::DadosComparecimento::mesariosEncerramento>(
                mesarios, conversorMesario));                               // includeOptionalField(1, 3)
    } else {
        entidade.omit_mesariosEncerramento();                              // removeOptionalField(1)
    }
    return entidade;
}

// -------------------------------------------------------------------------------------------------------
// Template instantiations emitted for this file (library code, summarised):
//   wasm func 9092  std::transform<SEQUENCE_OF<ComparecimentoMesario>::const_iterator,
//                   std::back_insert_iterator<std::vector<CComparecimentoMesario>>, $lambda>
//                   (push_back fast path inlined, slow path func 3813 "rhvoice_f3813"; element dtor 1876 on unwind)
//   wasm func 9094  std::transform<..., std::back_insert_iterator<std::vector<CIdentificacaoJustificativa>>, $lambda>
//                   (slow path func 5842)
//   wasm func 9099  ASN1::SEQUENCE_OF<IdentificacaoJustificativa>::SEQUENCE_OF(first, last)  -> merged body 2926
//   wasm func 6145  std::vector<T>::__destroy_vector::operator()() merged body (T with a CRegistroIdentificacaoEleitor
//                   at +0: releases shared_ptr +4 and optional shared_ptr +12 (flag +16) of each element)
//     func 2660       T = CComparecimentoMesario      (6145(v, 40, 36, 28, 24))
//     func 5099       T = CIdentificacaoJustificativa (6145(v, 24, 20, 12, 8))
//   wasm func 6146  std::vector<T>::~vector() merged body, same element layout
//     func 2661       T = CComparecimentoMesario
//     func 3488       T = CIdentificacaoJustificativa
//   wasm func 5097  CComparecimentoSecao::~CComparecimentoSecao() (implicit: destroys the
//                   std::vector<CEstadoComparecimento> at +16; the first 16 bytes are trivial)
// -------------------------------------------------------------------------------------------------------

}  // namespace ecourna::app::dados::asn
