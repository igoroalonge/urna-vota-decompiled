// uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversorestadogeralgap.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
// State of the GAP (the urna's application launcher: which application runs next), file gap.bin:
//   EstadoGeralGap ::= SEQUENCE {
//     correspondencias SEQUENCE (SIZE(0..10)) OF DadoCorrespondencia, appId UrnaAplicativo,
//     appAnteriorId UrnaAplicativo, executadoRED BOOLEAN, identificadoATUE BOOLEAN, audio BOOLEAN,
//     data2T BOOLEAN, numViasImpressasRelatorios1T/2T NumViasImpressasRelatorios,
//     dataSegundoTurno DataJE OPTIONAL }
//   UrnaAplicativo ::= ENUMERATED { apvotatreinaeleitor1(0) ... apsemaplicativo(14), apultimoid(15) }
// md EUrnaAplicativo uses the same numbers 0..14; 15 (the "last id" sentinel) is refused both ways.
// Observed at run time: gap.bin after votaInit decodes to appId = appAnteriorId = apsemaplicativo,
// dataSegundoTurno = "20801231", everything else zero/false.
#include "comum/dados/asn/estadoaplicacao/cconversorestadogeralgap.h"

#include <format>
#include <optional>
#include <vector>

#include "api/util/cdate.h"
#include "comum/asn/util.h"
#include "comum/dados/asn/estadoaplicacao/cconversordadocorrespondencia.h"
#include "comum/dados/asn/estadoaplicacao/cconversornumviasimpressasrelatorios.h"

namespace comum::asn {

// wasm func 5690 (srcloc lines 150, 154)
EUrnaAplicativo CConversorEstadoGeralGap::DesconverteUrnaAplicativo(const ModuloEstadoGeralGap::UrnaAplicativo& app) const
{
    const int valor = app.asInt();
    switch (valor) {
    case -1:                                             // ? generated "invalid" enumerator
    case ModuloEstadoGeralGap::UrnaAplicativo::apultimoid:
        throw CDadosError(7921, std::format("Valor inválido para UrnaAplicativo: {}", app));   // line 150
    default:
        if (valor >= 0 && valor <= ModuloEstadoGeralGap::UrnaAplicativo::apsemaplicativo) {
            return static_cast<EUrnaAplicativo>(valor);   // identity 0..14
        }
    }
    throw CDadosError(7922, std::format("Valor inválido para UrnaAplicativo: {}", app));       // line 154
}

// wasm func 5689 (srcloc lines 194, 198)
ModuloEstadoGeralGap::UrnaAplicativo CConversorEstadoGeralGap::ConverteUrnaAplicativo(const EUrnaAplicativo app) const
{
    const auto valor = static_cast<unsigned>(app);
    if (valor == 15) {   // EUrnaAplicativo "ultimo id" sentinel
        throw CDadosError(7923, std::format("Valor inválido para EUrnaAplicativo: {}", app));   // line 194
    }
    if (valor > 15) {
        throw CDadosError(7924, std::format("Valor inválido para EUrnaAplicativo: {}", app));   // line 198
    }
    return ModuloEstadoGeralGap::UrnaAplicativo(static_cast<int>(valor));
}

// wasm func 11394 (vtable slot 2)
ModuloEstadoGeralGap::EstadoGeralGap CConversorEstadoGeralGap::DoConverte(const TDado& estado) const
{
    ModuloEstadoGeralGap::EstadoGeralGap entidade;

    ModuloEstadoGeralGap::EstadoGeralGap::correspondencias_type correspondencias;   // SEQUENCE OF DadoCorrespondencia
    const CConversorDadoCorrespondencia conversorCorrespondencia;
    for (const auto& correspondencia : estado.GetCorrespondencias()) {
        correspondencias.push_back(conversorCorrespondencia.Converte(correspondencia));   // cloned
    }
    entidade.set_correspondencias(correspondencias);   // element-wise clone + swap

    entidade.set_appId(ConverteUrnaAplicativo(estado.GetAppId()));
    entidade.set_appAnteriorId(ConverteUrnaAplicativo(estado.GetAppAnteriorId()));
    entidade.set_executadoRED(estado.GetExecutadoRED());
    entidade.set_identificadoATUE(estado.GetIdentificadoATUE());
    entidade.set_audio(estado.GetAudio());
    entidade.set_data2T(estado.GetData2T());

    const CConversorNumViasImpressasRelatorios conversorVias;
    entidade.set_numViasImpressasRelatorios1T(conversorVias.Converte(estado.GetNumViasImpressasRelatorios1T()));
    entidade.set_numViasImpressasRelatorios2T(conversorVias.Converte(estado.GetNumViasImpressasRelatorios2T()));

    if (const auto& data = estado.GetDataSegundoTurno(); data.has_value()) {
        entidade.set_dataSegundoTurno(Utils::ConverteDataJE(*data));   // func 3802; includeOptionalField(0, 9)
    } else {
        entidade.omit_dataSegundoTurno();
    }
    return entidade;
}

// wasm func 11395 (vtable slot 3; curated name)
md::estadoaplicacao::CEstadoGeralGap CConversorEstadoGeralGap::DoDesconverte(const TEntidade& estado) const
{
    std::vector<md::estadoaplicacao::CDadoCorrespondencia> correspondencias;
    {
        const CConversorDadoCorrespondencia conversor;
        const auto& lista = estado.get_correspondencias();
        for (std::size_t i = 0; i < lista.size(); ++i) {
            correspondencias.push_back(conversor.Desconverte(lista[i]));
        }
    }
    const auto appId = DesconverteUrnaAplicativo(estado.get_appId());
    const auto appAnteriorId = DesconverteUrnaAplicativo(estado.get_appAnteriorId());
    const bool data2T = estado.get_data2T();
    const bool audio = estado.get_audio();
    const bool identificadoATUE = estado.get_identificadoATUE();
    const bool executadoRED = estado.get_executadoRED();

    const CConversorNumViasImpressasRelatorios conversorVias;
    const auto vias1T = conversorVias.Desconverte(estado.get_numViasImpressasRelatorios1T());
    const auto vias2T = conversorVias.Desconverte(estado.get_numViasImpressasRelatorios2T());

    std::optional<api::CDate> dataSegundoTurno;
    if (estado.dataSegundoTurno_isPresent()) {
        dataSegundoTurno = Utils::DesconverteDataJE(estado.get_dataSegundoTurno());   // func 2276
    }
    // The vector is passed by value (a copy is made, then both are destroyed).
    return md::estadoaplicacao::CEstadoGeralGap(correspondencias, appId, appAnteriorId, executadoRED,
                                                identificadoATUE, audio, data2T, vias1T, vias2T,
                                                dataSegundoTurno);
}

} // namespace comum::asn

// ----------------------------------------------------------------------------------------------------------
// Emitted in this TU (the tool attributes them here; real homes in other files):
//
// wasm func 3802 — comum::asn::Utils::ConverteDataJE(const api::CDate&)   // name inferred; home comum/asn/util.cpp
//   (path inferred). Also called by CConversorHorarioVerao::DoConverte (func 11385).
//     ModuloTiposEleitorais::DataJE Utils::ConverteDataJE(const api::CDate& data)
//     {
//         return ModuloTiposEleitorais::DataJE(data.Format("YYYYMMDD"));   // api::CDate::Format, then DataJE(string)
//     }
//
// wasm func 5626 — md::estadoaplicacao::CEstadoGeralGap::CEstadoGeralGap(...)   // name inferred
//   (home md/estadoaplicacao/cestadogeralgap.cpp, path inferred). Also used by the simulator fixture mock_f10123.
//   NOTE: the data2T argument is stored and then immediately OVERWRITTEN: the constructor recomputes it as
//   "the 2nd-round date is present and is today or in the past", using api::CDate::Hoje() (func 1382, gmtime of
//   the clock singleton) and CDate's three-way comparison (func 1261: year, month, day).
//     CEstadoGeralGap::CEstadoGeralGap(std::vector<CDadoCorrespondencia> correspondencias, EUrnaAplicativo appId,
//                                      EUrnaAplicativo appAnteriorId, bool executadoRED, bool identificadoATUE,
//                                      bool audio, bool data2T, const CNumViasImpressasRelatorios& vias1T,
//                                      const CNumViasImpressasRelatorios& vias2T,
//                                      const std::optional<api::CDate>& dataSegundoTurno)
//         : m_correspondencias(std::move(correspondencias)), m_appId(appId), m_appAnteriorId(appAnteriorId),
//           m_executadoRED(executadoRED), m_identificadoATUE(identificadoATUE), m_audio(audio), m_data2T(data2T),
//           m_numVias1T(vias1T), m_numVias2T(vias2T), m_dataSegundoTurno(dataSegundoTurno)
//     {
//         const api::CDate hoje = api::CDate::Hoje();
//         m_data2T = m_dataSegundoTurno.has_value() && *m_dataSegundoTurno <= hoje;
//     }
