// uenux2/src/app/comum/dados/asn/rdv/cconversoreleicoesvota.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
// The body of the RDV (Registro Digital do Voto) for the VOTA application:
//   ModuloRegistroDigitalVoto::Eleicoes ::= CHOICE { eleicoesVota [1] SEQUENCE OF EleicaoVota,
//                                                   eleicoesSA   [2] SEQUENCE OF EleicaoSA }
//   EleicaoVota ::= SEQUENCE { idEleicao INTEGER (0..99999), votosCargos SEQUENCE OF VotosCargo }
// <-> md::CVotosEleicoesVota, whose first member is
//     std::map<TEleicaoID, md::CVotosCargos> (CVotosCargos::TMapa = std::map<SCargoInfo, CVotos>).
//
// Both directions are driven by m_cargos (the configured cargos of every eleição): an RDV must contain
// exactly the configured eleições, and every VotosCargo must name a configured cargo.
// In the web build this runs while votaInit creates the empty RDV (CRdvVota vtable slot 11, func 11488,
// through CGeraDadosDinamicos); it was observed executing only at initialisation.
#include "comum/dados/asn/rdv/cconversoreleicoesvota.h"

#include <algorithm>
#include <format>
#include <string>

#include "comum/dados/asn/rdv/cconversorvotoscargo.h"
#include "ecourna/app/dados/asn/cconversorcargoid.h"

namespace comum::asn {

namespace {

// The SCargoInfo the RDV uses for a configured cargo (4 fields copied from md::CCargo: +0, +4, +12, +13).
md::CVotosCargos::SCargoInfo MontaCargoInfo(const md::CCargo& cargo)   // name inferred (inlined twice)
{
    return {cargo.GetCodigo(), cargo.GetTipo(), cargo.GetNumeroDigitos(), cargo.GetQtdeEscolhas()};
}

} // namespace

// wasm func 11350 (srcloc lines 34, 46; inlined md::CVotosCargos::GetVotos, cvotoscargos.cpp:91)
ModuloRegistroDigitalVoto::Eleicoes CConversorEleicoesVota::DoConverte(const TDado& votos) const
{
    const auto& votosEleicoes = votos.GetMapa();   // std::map<TEleicaoID, md::CVotosCargos> at +0
    if (votosEleicoes.size() != m_cargos.size()) {
        throw CDadosError(7965, "Eleições incompatíveis");   // line 34
    }

    ModuloRegistroDigitalVoto::Eleicoes entidade;
    auto& eleicoesVota = entidade.select_eleicoesVota();   // CHOICE alternative 0

    for (const auto& [idEleicao, votosCargos] : votosEleicoes) {
        ModuloRegistroDigitalVoto::EleicaoVota eleicaoVota;
        eleicaoVota.set_idEleicao(idEleicao);

        const auto itCargos = m_cargos.find(idEleicao);
        if (itCargos == m_cargos.end()) {
            throw CDadosError(7966, "Eleição " + std::to_string(idEleicao) + " não encontrada");   // line 46
        }

        // Iterates the CONFIGURED cargos (configuration order), not the map of votes: a cargo without votes
        // still produces a VotosCargo with an empty SEQUENCE OF, and a cargo missing from the votes throws.
        for (const md::CCargo& cargo : itCargos->second) {
            const auto cargoInfo = MontaCargoInfo(cargo);
            const CConversorVotosCargo conversor(cargoInfo);   // func 5683
            // md::CVotosCargos::GetVotos(TCargoID) is inlined: throws
            // CBaseError<api::EUeRdvError>(4671, "Cargo " + to_string(id) + " nao encontrado") (cvotoscargos.cpp:91).
            const md::CVotos votosCargo = votosCargos.GetVotos(cargoInfo.codigo);
            eleicaoVota.ref_votosCargos().push_back(conversor.Converte({cargoInfo, votosCargo}));
        }
        eleicoesVota.push_back(eleicaoVota);   // cloned into the SEQUENCE OF
    }
    return entidade;
}

// wasm func 11349 (srcloc lines 69, 74, 94, 104)
md::CVotosEleicoesVota CConversorEleicoesVota::DoDesconverte(const TEntidade& eleicoes) const
{
    if (eleicoes.currentSelection() != ModuloRegistroDigitalVoto::Eleicoes::eleicoesVota_) {   // choice id 0
        throw CDadosError(7967, "Dado não é do Vota");   // line 69 (an RDV of the Sistema de Apuração)
    }
    const auto& eleicoesVota = eleicoes.get_eleicoesVota();
    if (m_cargos.size() != eleicoesVota.size()) {
        throw CDadosError(7968, "Eleições incompatíveis");   // line 74
    }

    md::CVotosEleicoesVota::TMapa votosEleicoes;   // std::map<TEleicaoID, md::CVotosCargos>
    for (const auto* eleicaoVota : eleicoesVota) {
        md::CVotosCargos::TMapa votosCargos;       // std::map<SCargoInfo, md::CVotos>, ordered by codigo
        const TEleicaoID idEleicao = eleicaoVota->get_idEleicao();

        const auto itCargos = m_cargos.find(idEleicao);
        if (itCargos == m_cargos.end()) {
            throw CDadosError(7969, std::format("Eleição {} não encontrada", idEleicao));   // line 94
        }

        for (const auto* votosCargo : eleicaoVota->get_votosCargos()) {
            // ecourna Deconverte with CHOICE validity check (EApiAsnError 1902, iconversorasn.hpp:66).
            const auto idCargo = static_cast<uebyte>(
                ecourna::app::dados::asn::CConversorCargoID().Deconverte(votosCargo->get_idCargo()));
            const auto itCargo = std::ranges::find_if(itCargos->second, [idCargo](const md::CCargo& cargo) {
                return cargo.GetCodigo() == idCargo;
            });
            if (itCargo == itCargos->second.end()) {
                throw CDadosError(7970, std::format("Cargo {} não encontrado para eleição {}", idCargo, idEleicao));   // line 104
            }
            const CConversorVotosCargo conversor(MontaCargoInfo(*itCargo));   // func 5683
            votosCargos.insert(conversor.Desconverte(*votosCargo));           // duplicates silently ignored
        }
        // Both constructors take their map BY VALUE (copies visible in the wasm):
        // md::CVotosCargos::CVotosCargos(TMapa) (func 5643, srclocs cvotoscargos.cpp:50/58/66).
        // A repeated idEleicao is silently dropped too (unique-key insert, the new CVotosCargos is discarded).
        // Together with the size-only check at line 74 this means an RDV {X, X} against a configuration {X, Y}
        // decodes without error into a map that lacks Y.
        votosEleicoes.emplace(idEleicao, md::CVotosCargos(votosCargos));
    }
    // md::CVotosEleicoesVota::CVotosEleicoesVota(TMapa) with the anonymous MontaMapa() inlined
    // (cvotoseleicoesvota.cpp:33).
    return md::CVotosEleicoesVota(votosEleicoes);
}

} // namespace comum::asn

// ----------------------------------------------------------------------------------------------------------
// Library code emitted in this TU (listed in the unit, no source of its own):
//  * wasm func 3869: std::map<TEleicaoID, md::CVotosCargos>::__find_equal(hint, parent, dummy, key) — the
//    hinted insert used when the map is copied into the by-value CVotosEleicoesVota(TMapa) argument (also called
//    by api_f5808 and comum_f3870).
//  * wasm func 5683: CConversorVotosCargo::CConversorVotosCargo(const SCargoInfo&) — see the header.
