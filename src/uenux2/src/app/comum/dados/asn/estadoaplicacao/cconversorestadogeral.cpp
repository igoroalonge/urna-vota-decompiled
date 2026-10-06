// uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversorestadogeral.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
// General state of the urna, file dinamico/eg.bin:
//   EstadoGeralUrna ::= SEQUENCE { idPE INTEGER (0..99999), estadoUrna EstadoUrna, dadoCarga DadoCarga,
//                                  dadoLocal DadoLocal, ajusteDataHora AjusteDataHora,
//                                  correspondencia DadoCorrespondencia, versao GeneralString,
//                                  hashVersoesPacotes OCTET STRING }
//   EstadoUrna ::= ENUMERATED { carregando(1), carregada(2), testada(3) }
// md EEstadoUrna is a char enum '1', '2', '3' plus a 4th value '4' that has no ASN.1 counterpart.
// In the web build eg.bin is a hard-coded fixture (mock_f10205 calls the CEstadoGeral constructor 5637);
// DoConverte was observed running during votaInit (IServicoEstado writing eg.bin).
#include "comum/dados/asn/estadoaplicacao/cconversorestadogeral.h"

#include <format>

#include "comum/dados/asn/estadoaplicacao/cconversorajustedatahora.h"
#include "comum/dados/asn/estadoaplicacao/cconversordadocarga.h"
#include "comum/dados/asn/estadoaplicacao/cconversordadocorrespondencia.h"
#include "comum/dados/asn/estadoaplicacao/cconversordadolocal.h"

namespace comum::asn {

// Inlined into wasm func 11398 (srcloc lines 87, 91)
EEstadoUrna CConversorEstadoGeral::DesconverteEstadoUrna(const ModuloEstadoGeralUrna::EstadoUrna& estado) const
{
    switch (estado.asInt()) {
    case ModuloEstadoGeralUrna::EstadoUrna::carregando: return EEstadoUrna{'1'};
    case ModuloEstadoGeralUrna::EstadoUrna::carregada:  return EEstadoUrna{'2'};
    case ModuloEstadoGeralUrna::EstadoUrna::testada:    return EEstadoUrna{'3'};
    case -1:   // ? generated "invalid" enumerator
        throw CDadosError(7917, std::format("Valor inválido para estado da urna: {}", estado));   // line 87
    }
    throw CDadosError(7918, std::format("Valor inválido para estado da urna: {}", estado));       // line 91
}

// Inlined into wasm func 11397 (srcloc lines 106, 110)
ModuloEstadoGeralUrna::EstadoUrna CConversorEstadoGeral::ConverteEstadoUrna(const EEstadoUrna estado) const
{
    switch (static_cast<char>(estado)) {
    case '1': return ModuloEstadoGeralUrna::EstadoUrna::carregando;
    case '2': return ModuloEstadoGeralUrna::EstadoUrna::carregada;
    case '3': return ModuloEstadoGeralUrna::EstadoUrna::testada;
    case '4':   // md-only state (name unknown) that must never be persisted
        throw CDadosError(7919, std::format("Valor inválido para estado da urna: {}", estado));   // line 106
    }
    throw CDadosError(7920, std::format("Valor inválido para estado da urna: {}", estado));       // line 110
}

// wasm func 11397 (vtable slot 2; named after the inlined ConverteEstadoUrna)
ModuloEstadoGeralUrna::EstadoGeralUrna CConversorEstadoGeral::DoConverte(const TDado& estado) const
{
    ModuloEstadoGeralUrna::EstadoGeralUrna entidade;
    // Each sub-converter goes through IConversorASN::Converte (post-condition check 7653).
    entidade.set_ajusteDataHora(CConversorAjusteDataHora().Converte(estado.GetAjusteDataHora()));
    entidade.set_dadoCarga(CConversorDadoCarga().Converte(estado.GetDadoCarga()));
    entidade.set_dadoLocal(CConversorDadoLocal().Converte(estado.GetDadoLocal()));
    entidade.set_correspondencia(CConversorDadoCorrespondencia().Converte(estado.GetCorrespondencia()));
    entidade.set_idPE(estado.GetIdPE());
    entidade.set_estadoUrna(ConverteEstadoUrna(estado.GetEstado()));
    entidade.set_versao(estado.GetVersao());
    entidade.set_hashVersoesPacotes(estado.GetHashVersoesPacotes());   // vector<uebyte> -> OCTET STRING
    return entidade;
}

// wasm func 11398 (vtable slot 3; named after the inlined DesconverteEstadoUrna)
md::estadoaplicacao::CEstadoGeral CConversorEstadoGeral::DoDesconverte(const TEntidade& estado) const
{
    const TPEID idPE = estado.get_idPE();
    const EEstadoUrna estadoUrna = DesconverteEstadoUrna(estado.get_estadoUrna());
    // Each sub-converter goes through IConversorASN::Desconverte (pre-condition check 7654).
    auto ajuste = CConversorAjusteDataHora().Desconverte(estado.get_ajusteDataHora());
    auto local = CConversorDadoLocal().Desconverte(estado.get_dadoLocal());
    auto carga = CConversorDadoCarga().Desconverte(estado.get_dadoCarga());
    auto correspondencia = CConversorDadoCorrespondencia().Desconverte(estado.get_correspondencia());
    std::vector<uebyte> hash(estado.get_hashVersoesPacotes().begin(), estado.get_hashVersoesPacotes().end());
    std::string versao = estado.get_versao();
    return md::estadoaplicacao::CEstadoGeral(estadoUrna, idPE, std::move(local), carga, ajuste,
                                             correspondencia, std::move(versao), std::move(hash));
}

} // namespace comum::asn

// ----------------------------------------------------------------------------------------------------------
// Emitted in this TU (the tool attributes it here; real home md/estadoaplicacao/cestadogeral.{h,cpp}):
//
// wasm func 5637 — md::estadoaplicacao::CEstadoGeral::CEstadoGeral(...)           // name inferred
// Plain member-wise constructor: moves CDadoLocal's string, copies CDadoCarga (20 bytes) and CAjusteDataHora,
// copy-constructs CDadoCorrespondencia (func 1249), moves versao and hashVersoesPacotes. No validation.
// Also called by the simulator fixture mock_f10205 (the fake eg.bin, see docs/data-model/asn1-schemas.md §4.4).
//
//   CEstadoGeral::CEstadoGeral(EEstadoUrna estado, TPEID idPE, CDadoLocal&& local, const CDadoCarga& carga,
//                              const CAjusteDataHora& ajuste, const CDadoCorrespondencia& correspondencia,
//                              std::string&& versao, std::vector<uebyte>&& hash)
//       : m_estado(estado), m_idPE(idPE), m_local(std::move(local)), m_carga(carga), m_ajuste(ajuste),
//         m_correspondencia(correspondencia), m_versao(std::move(versao)), m_hashVersoesPacotes(std::move(hash))
//   {}
