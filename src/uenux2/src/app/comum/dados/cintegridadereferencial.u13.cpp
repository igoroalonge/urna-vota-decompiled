// uenux2/src/app/comum/dados/cintegridadereferencial.cpp  --  FRAGMENT written by unit u13 (owner u04)
//
// Reconstructed from vota_web_wasm.wasm. The unit builder put wasm func 11514 in u13 because it calls
// ecourna::api::util::CStringUtils::ToWord/ToDWord; its real home is the referential-integrity checker.
//
// Caller: comum_f2543 (u02: CIntegridadeReferencial::VerificaRdv), which runs it after CEleitores::CompleteLoad
// (func 6734), in vota::impl::CSincronismoVotoEleitor::vf2 (func 7174) and in vota::CGravaResultado::vf2
// (func 12098, when the results - BU and RDV files - are written at the close of the vote), and throws
// through CIntegridadeReferencial::Lanca (func 2263, code 7871 "Integridade referencial: {}") when the
// result is not ok.
//
// It checks every vote stored in the RDV against the static election data:
//   cargo exists; a party (legenda) vote is only for proportional cargos, has >= 2 digits, names an
//   existing party and is not a full candidate number; a nominal vote names an existing option
//   (consulta) or an existing, apt candidacy; a null vote typed with the full number of digits does not
//   name an existing option / apt candidacy.
//
// Layouts used (from the code): RDV vote md::CVoto = { ETipo tipo +0; std::string conteudo +4 } (16 bytes);
// md::CCargo: codigo +0, tipo +4 (0 majoritário, 1 proporcional, 2 consulta), numeroDigitos +12,
// optional<CDetalheCandidato> engaged flag +84, optional<CDetalheConsulta> engaged flag +136;
// md::CCandidatura situação +52 (0 = apta). Map keys: candidaturas and respostas cargo*1000000+numero
// (func 1939), partidos ueword.
#include "comum/dados/cintegridadereferencial.h"

#include <format>
#include <map>
#include <string>

#include "comum/dados/ccandidaturas.h"
#include "comum/dados/cpartidos.h"
#include "comum/dados/crdvvota.h"
#include "comum/dados/crespostas.h"
#include "comum/dados/md/rdv/cvoto.h"
#include "ecourna/api/util/cstringutils.hpp"

namespace comum {

using ecourna::api::util::CStringUtils;

// wasm func 11514 - name inferred (6,868 bytes; every std::format is expanded inline, which is why the
// decompiled body is long).
CIntegridadeReferencial::TResultado
CIntegridadeReferencial::VerificaVotosRdv(const CRdv& rdv, const std::map<TCargoID, md::CCargo>& cargos,
                                          const CCandidaturas& candidaturas, const CPartidos& partidos,
                                          const CRespostas& respostas)
{
    // CRdv vtable slot 0 (CRdvVota::GetVotos, func 5733): a copy of all votes grouped by cargo.
    const std::map<TCargoID, md::CVotos> votosPorCargo = rdv.GetVotos();

    for (const auto& [codigoCargo, votos] : votosPorCargo) {
        const auto itCargo = cargos.find(codigoCargo);
        if (itCargo == cargos.end())
            return {false, std::format("o cargo ({}) no RDV não foi encontrado na lista de cargos", codigoCargo)};
        const md::CCargo& cargo = itCargo->second;
        const unsigned codigo = cargo.GetCodigo();

        for (const md::CVoto& voto : votos) {
            const std::string& conteudo = voto.GetConteudo();

            switch (voto.GetTipo()) {
            case md::CVoto::ETipo::LEGENDA: {                                                   // 1
                if (!(cargo.TemDetalheCandidato() && cargo.GetTipo() == md::CCargo::ETipo::PROPORCIONAL))
                    return {false, std::format("um voto do cargo ({}) no RDV está com voto de legenda ({}) "
                                               "com cargo que não é proporcional", codigo, conteudo)};
                if (conteudo.size() <= 1)
                    return {false, std::format("um voto do cargo ({}) no RDV está com voto de legenda ({}) "
                                               "com conteúdo insuficiente", codigo, conteudo)};
                const ueword partido = CStringUtils::ToWord(conteudo.substr(0, 2));
                if (!partidos.Existe(partido))                                                 // map<ueword,...>::find  ?name
                    return {false, std::format("um voto do cargo ({}) no RDV está com voto de legenda ({}) "
                                               "para partido não encontrado", codigo, conteudo)};
                if (cargo.GetNumeroDigitos() == conteudo.size()
                    && candidaturas.Busca(CCandidaturas::Chave(codigo, CStringUtils::ToDWord(conteudo))) != nullptr)   // ?name
                    return {false, std::format("um voto do cargo ({}) no RDV está com voto de legenda ({}), "
                                               "mas o candidato existe", codigo, conteudo)};
                break;
            }

            case md::CVoto::ETipo::NOMINAL: {                                                   // 2
                // No length check here: a non-numeric content makes ToDWord throw (EUtilError 1875..1877).
                const auto chave = CCandidaturas::Chave(codigo, CStringUtils::ToDWord(conteudo));
                if (cargo.TemDetalheConsulta()) {
                    if (!respostas.Existe(chave))                                               // ?name
                        return {false, std::format("um voto do cargo ({}) no RDV está com voto nominal ({}) "
                                                   "para resposta não encontrada", codigo, conteudo)};
                } else {
                    const md::CCandidatura* candidatura = candidaturas.Busca(chave);            // ?name
                    if (candidatura == nullptr)
                        return {false, std::format("um voto do cargo ({}) no RDV está com voto nominal ({}) "
                                                   "para candidatura não encontrada", codigo, conteudo)};
                    if (candidatura->GetSituacao() != 0)                                       // +52: not apt
                        return {false, std::format("um voto do cargo ({}) no RDV está com voto nominal ({}) "
                                                   "para candidatura inapta", codigo, conteudo)};
                }
                break;
            }

            default: {
                // NULO, NULO_APOS_SUSPENSAO (not NULO_POR_REPETICAO) typed with the cargo's full number of digits.
                if (!voto.EhNulo() || voto.GetTipo() == md::CVoto::ETipo::NULO_POR_REPETICAO   // func 5645 {4,6,7}; 7 excluded
                    || cargo.GetNumeroDigitos() != conteudo.size())
                    break;
                const auto chave = CCandidaturas::Chave(codigo, CStringUtils::ToDWord(conteudo));
                if (cargo.TemDetalheConsulta()) {
                    if (respostas.Existe(chave))
                        return {false, std::format("um voto do cargo ({}) no RDV está com voto nulo ({}), "
                                                   "mas a resposta existe", codigo, conteudo)};
                } else {
                    const md::CCandidatura* candidatura = candidaturas.Busca(chave);
                    if (candidatura != nullptr && candidatura->GetSituacao() == 0)
                        return {false, std::format("um voto do cargo ({}) no RDV está com voto nulo ({}) "
                                                   "para candidatura apta", codigo, conteudo)};
                }
                break;
            }
            }
        }
    }
    return {true, {}};
}

} // namespace comum
