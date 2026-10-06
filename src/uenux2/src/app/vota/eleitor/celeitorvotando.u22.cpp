// uenux2/src/app/vota/eleitor/celeitorvotando.cpp  --  FRAGMENT written by unit u22
// (the rest of the file is celeitorvotando.cpp by unit u06, which deferred func 4454 to u22).
//
// wasm func 4454 (5,617 bytes). The tools named it "comum::md::CVotosCargos::ConfereCedula" after the
// first std::source_location found in it; it is the private CEleitorVotando::GravaVotos (name given by
// u06, inferred), called by NeedChangeState (func 7352) when the last cargo is done and by
// suspenderEleitor (func 4442). Observed executing in both recorded votes.
// It inlines, in order: CRdvVota::RecebeCedula, CVotosEleicoesVota::RecebeCedula (cvotoseleicoesvota.cpp
// :215), CVotosCargos::ConfereCedula (:248..:295), CVotosCargos::InsereCedula (:199), CVotos::Insere
// (cvotos.cpp:40), CEstadoGeralVota::MarcaUltimoVoto (cestadogeralvota.cpp:131/:136) and
// CSincronismoEleitor::GetInst.
#include "vota/eleitor/celeitorvotando.h"

#include <algorithm>
#include <map>

#include "api/util/cdatetime.h"
#include "comum/appinfo/cappinfo.h"
#include "comum/cconfiguracaoeleicao.h"
#include "comum/dados/crdvvota.h"
#include "comum/dados/md/rdv/cvotoscargos.h"
#include "vota/eleitor/csincronismoeleitor.h"
#include "vota/eleitor/cthreadeleitor.h"

namespace vota {

// wasm func 4454                                                                    name inferred (u06)
void CEleitorVotando::GravaVotos()
{
    CThreadEleitor::GetInst().StopTick(m_tickEleitorDemorando);                 // vota_f422 (func 316 = GetInst)

    // 1. Split the voter's votes into one ballot (cédula) per eleição (municipal elections have one,
    //    general elections two: federal and estadual). Pairs (eleição, cargo) come from the
    //    configuration (func 3774).
    comum::CRdvVota& rdv = comum::CRdvVota::GetInst();                           // func 555
    std::map<comum::md::TEleicaoID, comum::md::CCedula> cedulas;                 // destroyed by func 3750
    const auto eleicoesCargos = comum::CConfiguracaoEleicao::GetInst().GetEleicoesCargos();   // vector<{eleicao, cargo}>
    for (const auto& voto : g_votosEleitor) {                                    // @1833300
        const auto it = std::ranges::find_if(eleicoesCargos,
                                             [&](const auto& ec) { return ec.cargo == voto.first; });
        const comum::md::TEleicaoID eleicao = (it == eleicoesCargos.end()) ? 0 : it->eleicao;
        cedulas[eleicao].push_back(voto);
    }

    // 2. Add each ballot to the in-memory RDV (checked, then inserted at the positions chosen by the
    //    positioner, i.e. sorted - see cvotoscargos.cpp).
    for (const auto& [eleicao, cedula] : cedulas)
        rdv.RecebeCedula(eleicao, cedula);   // = rdv.m_votos(+20).RecebeCedula(*rdv.m_posicionador(+4), eleicao, cedula)

    // 3. Time of the last vote + turnout counter of the general state (inlined
    //    CEstadoGeralVota::MarcaUltimoVoto, cestadogeralvota.cpp:131/:136).
    auto& estado = comum::GetEstadoVota(comum::CAppInfo::GetInst());            // func 261
    if (!estado.InicioAquisicaoMarcado())                                        // +32
        throw comum::CDadosError(comum::EUeComumDadosError{8086}, "Ainda não foi marcado o início da aquisição");   // :131
    if (estado.FimAquisicaoMarcado())                                            // +48
        throw comum::CDadosError(comum::EUeComumDadosError{8087}, "Já foi marcado o fim da aquisição");            // :136
    estado.m_dataHoraUltimoVoto = api::CDateTime::Now();                         // optional<CDateTime> +56 (flag +68), func 479
    ++estado.m_qtdVotantes;                                                      // uint16 +52

    // 4. Next top-level state: persist the vote (CSincronismoEleitor, lazy singleton @1833160).
    m_proximoEstado = &CSincronismoEleitor::GetInst();
    if (m_estadoCargo)
        m_estadoCargo->FinishState();                                            // slot 5
    m_estadoCargo = nullptr;
}
// Note: nothing here writes a file. Durability is CSincronismoEleitor's job, which in the web build is
// the no-op CSincronismoVotoEleitorWeb (u07): the ballot stays in memory, the RDV file is not updated.

} // namespace vota
