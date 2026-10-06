// uenux2/src/app/comum/dados/asn/rdv/cconversorvotoscargo.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
// The votes of one cargo inside the RDV:
//   ModuloRegistroDigitalVoto::VotosCargo { idCargo CodigoCargoConsulta, quantidadeEscolhas INTEGER (1..50),
//                                           votos SEQUENCE OF Voto }
//   <-> std::pair<md::CVotosCargos::SCargoInfo, md::CVotos>
// The converter is built for ONE cargo (SCargoInfo taken from the election configuration) and refuses data
// that belongs to a different cargo or has a different number of choices.
#include "comum/dados/asn/rdv/cconversorvotoscargo.h"

#include <format>
#include <vector>

#include "ecourna/app/dados/asn/cconversorcargoid.h"

namespace comum::asn {

// wasm func 11352 (srcloc line 28)
ModuloRegistroDigitalVoto::VotosCargo CConversorVotosCargo::DoConverte(const TDado& votosCargo) const
{
    const auto& [cargoInfo, votos] = votosCargo;
    if (cargoInfo.codigo != m_cargoInfo.codigo) {
        throw CDadosError(7976, "Cargo info incompatível");   // line 28
    }

    ModuloRegistroDigitalVoto::VotosCargo entidade;
    // ecourna's IConversorASN<CodigoCargoConsulta, CBaseType<unsigned short,1,99,3>>::Converte: DoConverte +
    // CHOICE validity check (CBaseError<EApiAsnError>(1900, ...), iconversorasn.hpp:49).
    entidade.set_idCargo(ecourna::app::dados::asn::CConversorCargoID().Converte(
        CBaseType<unsigned short, 1, 99, 3>(m_cargoInfo.codigo)));
    entidade.set_quantidadeEscolhas(m_cargoInfo.qtdeEscolhas);

    for (const md::CVoto& voto : votos.GetVotos()) {   // md::CVotos -> std::vector<md::CVoto>
        // Converte = DoConverte + "Entidade deixada em estado inválido" check (7653); the result is cloned
        // (AbstractData vtable slot 3) and appended to the SEQUENCE OF.
        entidade.ref_votos().push_back(m_conversorVoto.Converte(voto));
    }
    return entidade;
}

// wasm func 11351 (srcloc lines 50, 55)
std::pair<md::CVotosCargos::SCargoInfo, md::CVotos>
CConversorVotosCargo::DoDesconverte(const TEntidade& votosCargo) const
{
    // ecourna Deconverte: CHOICE validity check first (CBaseError<EApiAsnError>(1902, ...), iconversorasn.hpp:66).
    const auto cargo = static_cast<unsigned short>(
        ecourna::app::dados::asn::CConversorCargoID().Deconverte(votosCargo.get_idCargo()));
    if (cargo != m_cargoInfo.codigo) {
        throw CDadosError(7977, std::format("Cargo {} difere do esperado: {}", cargo, m_cargoInfo.codigo));   // line 50
    }

    const auto qtdeEscolhas = static_cast<uebyte>(votosCargo.get_quantidadeEscolhas());
    if (qtdeEscolhas != m_cargoInfo.qtdeEscolhas) {
        throw CDadosError(7978, std::format("Quantidade de escolhas difere do esperado: {}/{}",
                                            qtdeEscolhas, m_cargoInfo.qtdeEscolhas));                          // line 55
    }

    std::vector<md::CVoto> votos;
    for (const auto* voto : votosCargo.get_votos()) {
        votos.push_back(m_conversorVoto.Desconverte(*voto));   // "Entidade está inválida" check (7654) per vote
    }
    return {m_cargoInfo, md::CVotos(votos)};   // md::CVotos built from a copy of the vector (func 2794)
}

} // namespace comum::asn
