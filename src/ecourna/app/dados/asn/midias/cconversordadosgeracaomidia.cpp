// ecourna-lib/ecourna/app/dados/asn/midias/cconversordadosgeracaomidia.cpp   (path inferred)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40. No srcloc (errors come from the callees: CSerialMidia
// func 9259, the IdentificadorGeradorMidia converter, the date parser).
#include "ecourna/app/dados/asn/midias/cconversordadosgeracaomidia.h"

#include "ecourna/api/util/datahora.hpp"
#include "ecourna/app/dados/asn/cconversoridentificadorgeradormidia.h"

namespace ecourna::app::dados::asn {

using api::util::ConverteDataHoraJE;
using api::util::FormataDataHoraJE;

// wasm func 9203 (vtable slot 2)
ModuloInformacaoMidia::DadosGeracaoMidia CConversorDadosGeracaoMidia::DoConverte(const CDadosGeracaoMidia& dado) const
{
    const CConversorIdentificadorGeradorMidia conversorIdentificador;                 // vptr @1123140
    ModuloInformacaoMidia::DadosGeracaoMidia entidade;
    entidade.set_serialMidia(dado.GetSerialMidia().ToString());                      // func 9257 (name inferred)
    entidade.set_usuario(dado.GetUsuario());                                         // +12
    entidade.set_identificadorGeradorMidia(
        conversorIdentificador.Converte(dado.GetIdentificadorGeradorMidia()));       // 3734 -> 9225
    entidade.set_data(ModuloTiposEleitorais::DataHoraJE(FormataDataHoraJE(dado.GetData())));   // +64, func 9220
    return entidade;
}

// wasm func 9202 (vtable slot 3; name curated by the tools). Reached when infomidia.dat is read
// (CFileASN::ReadFromFile<InformacaoMidia>, CConversorInformacaoMidia::DoDeconverte 9179).
CDadosGeracaoMidia CConversorDadosGeracaoMidia::DoDeconverte(const ModuloInformacaoMidia::DadosGeracaoMidia& entidade) const
{
    const CConversorIdentificadorGeradorMidia conversorIdentificador;
    const CSerialMidia serialMidia(entidade.get_serialMidia());                      // func 9259 (validates the text)
    const std::string usuario = entidade.get_usuario();
    const CIdentificadorGeradorMidia identificador =
        conversorIdentificador.Deconverte(entidade.get_identificadorGeradorMidia()); // func 3733 -> 9224
    const boost::posix_time::ptime data = ConverteDataHoraJE(entidade.get_data());  // func 1877
    return CDadosGeracaoMidia(serialMidia, usuario, identificador, data);             // func 9034
}

}  // namespace ecourna::app::dados::asn
