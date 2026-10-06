// uenux2/src/app/comum/dados/md/rdv/cvotoscargos.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by std::source_location records
// :50, :58, :66 (constructor), :91 (GetVotos, in func 11350 - other unit), :199 (InsereCedula),
// :248, :259, :265, :278, :285, :295 (ConfereCedula), and :30 / :37 (criaMapa, inlined into the
// start-up routine func 7787 - other unit).
// ConfereCedula and InsereCedula exist only inlined into vota::CEleitorVotando::GravaVotos (func 4454,
// see vota/eleitor/celeitorvotando.u22.cpp), which the tools named "CVotosCargos::ConfereCedula".
//
// Errors: CRdvError = CBaseError<api::EUeRdvError, SErrorLimits{4650, 4850}> (thunk func 655).
// Messages are built with std::to_string + operator+, not std::format.
#include "comum/dados/md/rdv/cvotoscargos.h"

#include <algorithm>
#include <string>

#include "comum/dados/rdvdefs.h"   // CRdvError

namespace comum::md {

// wasm func 5643 (srcloc :50, :58, :66) - observed? no (built from the RDV file by
// CConversorVotosCargo / at start-up by criaMapa). Every cargo must hold the same number of ballots.
CVotosCargos::CVotosCargos(TMapa mapa)
    : m_mapa(std::move(mapa))
{
    if (m_mapa.empty())
        throw CRdvError(api::EUeRdvError{4666}, "Mapa de votos vazio");                            // :50
    const auto& [primeiroInfo, primeirosVotos] = *m_mapa.begin();
    const TQtdVoto cedulas = static_cast<TQtdVoto>(primeirosVotos.Tamanho()) / primeiroInfo.qtdEscolhas;
    for (const auto& [info, votos] : m_mapa) {
        const TQtdVoto qtd = static_cast<TQtdVoto>(votos.Tamanho());
        if (qtd % info.qtdEscolhas != 0)
            throw CRdvError(api::EUeRdvError{4667},
                "Numero de votos (" + std::to_string(qtd) + ") do cargo " + std::to_string(info.codigo) +
                " nao eh multiplo da quantidade de escolhas (" + std::to_string(info.qtdEscolhas) + ")");   // :58
        const TQtdVoto esperado = static_cast<TQtdVoto>(cedulas * info.qtdEscolhas);
        if (qtd != esperado)
            throw CRdvError(api::EUeRdvError{4668},
                "Numero de votos (" + std::to_string(qtd) + ") do cargo " + std::to_string(info.codigo) +
                " diferente do esperado (" + std::to_string(esperado) + ")");                           // :66
    }
}

// wasm func 5642 - sum of qtdEscolhas over the cargos = size of one ballot. The loop iterates the
// map BY VALUE (each CVotos vector is copied and freed), which the optimiser could not remove.
TQtdEscolha CVotosCargos::QtdVotosPorCedula() const                                   // name inferred
{
    TQtdEscolha total = 0;
    for (const std::pair<SCargoInfo, CVotos> par : m_mapa)   // (sic) copy
        total += par.first.qtdEscolhas;
    return total;
}

// Inlined into func 5639 (CVotosEleicoesVota::Comparecimento): ballots = votes of the first cargo /
// its qtdEscolhas.
TQtdVoto CVotosCargos::Comparecimento() const
{
    const auto& [info, votos] = *m_mapa.begin();
    return static_cast<TQtdVoto>(votos.Tamanho()) / info.qtdEscolhas;
}

// srcloc :248..:295, inlined into func 4454. Checks one ballot before it is inserted.
void CVotosCargos::ConfereCedula(const CCedula& cedula) const
{
    if (cedula.size() != QtdVotosPorCedula())
        throw CRdvError(api::EUeRdvError{4672},
            "Tamanho da cedula (" + std::to_string(cedula.size()) + ") difere do esperado (" +
            std::to_string(QtdVotosPorCedula()) + ")");                                             // :248

    std::map<TCargoID, int> votosPorCargo;                                   // destroyed by func 3709
    for (const auto& [cargo, voto] : cedula)
        ++votosPorCargo[cargo];
    for (const auto& [cargo, quantidade] : votosPorCargo) {
        const auto it = m_mapa.find(SCargoInfo{cargo});
        if (it == m_mapa.end())
            throw CRdvError(api::EUeRdvError{4673},
                "Cedula com cargo nao encontrado (" + std::to_string(cargo) + ")");                   // :259
        if (quantidade != it->first.qtdEscolhas)
            throw CRdvError(api::EUeRdvError{4674},
                "Cedula do cargo " + std::to_string(cargo) + " com numero de votos (" +
                std::to_string(quantidade) + ") diferente da quantidade de escolhas (" +
                std::to_string(it->first.qtdEscolhas) + ")");                                        // :265
    }

    for (const auto& [cargo, voto] : cedula) {
        const SCargoInfo& info = m_mapa.find(SCargoInfo{cargo})->first;       // present (checked above)
        switch (voto.GetTipo()) {
        case CVoto::ETipo::NOMINAL:
            if (voto.GetDigitado().size() != info.numeroDigitos)
                throw CRdvError(api::EUeRdvError{4675},
                    "Voto nominal para o cargo " + std::to_string(cargo) + " com tamanho (" +
                    std::to_string(voto.GetDigitado().size()) + ") diferente do numero de digitos (" +
                    std::to_string(info.numeroDigitos) + ")");                                       // :278
            break;
        case CVoto::ETipo::NULO_POR_REPETICAO:
            // a repeated candidate (e.g. the same Senator for both seats) is only possible when the
            // cargo has several choices, and the ballot must also hold the nominal vote it repeats
            if (info.qtdEscolhas == 1)
                throw CRdvError(api::EUeRdvError{4676},
                    "Voto nulo por repeticao para o cargo " + std::to_string(cargo) + " que so tem uma escolha");   // :285
            if (std::ranges::find(cedula, std::pair{cargo, CVoto(CVoto::ETipo::NOMINAL, voto.GetDigitado())})
                == cedula.end())                                              // func 2256 builds the probe
                throw CRdvError(api::EUeRdvError{4677},
                    "Voto nulo por repeticao para o cargo " + std::to_string(cargo) +
                    " sem voto nominal correspondente");                                              // :295
            break;
        default:
            break;
        }
    }
}

// srcloc :199, inlined into func 4454. Transactional: works on a copy of the map and swaps it in
// only when every vote of the ballot was placed.
void CVotosCargos::InsereCedula(const CRdvPosicionador& posicionador, const CCedula& cedula)
{
    TMapa novo(m_mapa);                                                        // func 5681 per node
    for (const auto& [cargo, voto] : cedula) {
        const auto it = novo.find(SCargoInfo{cargo});
        if (it == novo.end())
            throw CRdvError(api::EUeRdvError{4669}, "Cargo nao encontrado " + std::to_string(cargo));   // :199
        CVotos& votos = it->second;
        votos.Insere(voto, posicionador.Posiciona(votos, voto));              // positioner slot 2 (func 11496)
    }
    std::swap(m_mapa, novo);                                                   // old map freed by func 1268
}

} // namespace comum::md

// ---------------------------------------------------------------------------------------------------
// Library instantiations emitted with this file (listed in the unit):
//   func 3709  std::__tree<std::__value_type<TCargoID, int>>::destroy(node)   (votosPorCargo above)
//   func 3750  std::__tree<std::__value_type<TEleicaoID, CCedula>>::destroy(node)  (GravaVotos' map)
