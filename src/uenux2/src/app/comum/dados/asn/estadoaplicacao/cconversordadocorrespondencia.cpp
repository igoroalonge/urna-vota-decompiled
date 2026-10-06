// uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadocorrespondencia.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u35).
#include "comum/dados/asn/estadoaplicacao/cconversordadocorrespondencia.h"

#include <vector>

#include "comum/asn/util.h"
#include "comum/dados/asn/estadoaplicacao/cconversorlocalidadeeleitoral.h"
#include "ecourna/app/dados/asn/midias/cconversoridentificadorgeradormidia.hpp"   // vtable @1123140

namespace comum::asn {

std::vector<char> ConverteSerialFlash(const std::string& serial);        // gravadores/asn/util.cpp, func 3605
std::string DesconverteSerialFlash(const std::vector<char>& bytes);     // gravadores/asn/util.cpp, func 3604

// wasm func 11399 - vtable slot 2. Observed executing: eg.bin is written at votaInit (CConversorEstadoGeral ->
// IConversorASN<DadoCorrespondencia>::Converte, func 5691).
ModuloEstadoGeralUrna::DadoCorrespondencia CConversorDadoCorrespondencia::DoConverte(
    const md::estadoaplicacao::CDadoCorrespondencia& correspondencia) const
{
    const ecourna::app::dados::asn::CConversorIdentificadorGeradorMidia conversorGerador;

    ModuloEstadoGeralUrna::DadoCorrespondencia entidade;                                   // SEQUENCE(info @1145784)
    auto& carga = entidade.ref_carga();
    carga.set_numeroInternoUrna(correspondencia.GetNumeroInternoUrna());                    // +0
    carga.set_codigoCarga(correspondencia.GetCodigoCarga());                                // +28
    const std::vector<char> serial = ConverteSerialFlash(correspondencia.GetNumeroSerieFC());   // +4
    carga.set_numeroSerieFC(ASN1::OCTET_STRING(serial.begin(), serial.end()));
    carga.set_dataHoraCarga(Utils::ConverteDataHoraJE(correspondencia.GetDataHoraCarga()));     // +16, func 1080
    carga.set_identificadorGeradorMidia(conversorGerador.Converte(correspondencia.GetGeradorMidia()));  // +60, 3734

    entidade.set_secaoCarga(CConversorLocalidadeEleitoral().Converte(correspondencia.GetSecaoCarga()));  // +40, 5696
    const std::vector<uebyte>& assinatura = correspondencia.GetAssinatura();               // +48
    entidade.set_assinatura(ASN1::OCTET_STRING(assinatura.begin(), assinatura.end()));     // shared_f1927
    return entidade;
}

// wasm func 11400 - vtable slot 3. Observed executing: eg.bin is read back at votaInit.
// No check at all on the content (the "assinatura" blob is copied, never verified here).
md::estadoaplicacao::CDadoCorrespondencia CConversorDadoCorrespondencia::DoDesconverte(
    const ModuloEstadoGeralUrna::DadoCorrespondencia& correspondencia) const
{
    const ecourna::app::dados::asn::CConversorIdentificadorGeradorMidia conversorGerador;
    const auto& carga = correspondencia.get_carga();

    const auto numeroInternoUrna = carga.get_numeroInternoUrna();
    const std::string serial = DesconverteSerialFlash(carga.get_numeroSerieFC());
    const api::CDateTime dataHora = Utils::DesconverteDataHoraJE(carga.get_dataHoraCarga());     // func 1713
    const std::string codigoCarga = carga.get_codigoCarga();
    const md::estadoaplicacao::CLocalidadeEleitoral secao =
        CConversorLocalidadeEleitoral().Desconverte(correspondencia.get_secaoCarga());          // func 5697
    const std::vector<uebyte> assinatura(correspondencia.get_assinatura().begin(),
                                         correspondencia.get_assinatura().end());
    auto gerador = conversorGerador.Deconverte(carga.get_identificadorGeradorMidia());         // func 3733

    return md::estadoaplicacao::CDadoCorrespondencia(numeroInternoUrna, serial, dataHora, codigoCarga, secao,
                                                     assinatura, std::move(gerador));           // func 5630
}

} // namespace comum::asn
