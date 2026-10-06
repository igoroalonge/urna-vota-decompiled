// uenux2/src/app/comum/gravadores/asn/cconversorcarga.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u35).
#include "comum/gravadores/asn/cconversorcarga.h"

#include <vector>

#include "comum/asn/util.h"
#include "ecourna/app/dados/asn/midias/cconversoridentificadorgeradormidia.hpp"   // ecourna converter, vtable @1123140

namespace comum::asn {

// Helpers of gravadores/asn/util.cpp (unit u23): 8 hex digits <-> 4 bytes.
std::vector<char> ConverteSerialFlash(const std::string& serial);        // func 3605
std::string DesconverteSerialFlash(const std::vector<char>& bytes);     // func 3604

// wasm func 10291 - vtable slot 2. Not observed executing (only result files contain a Carga; eg.bin's copy of it
// goes through CConversorDadoCorrespondencia instead).
ModuloTiposEcoUrna::Carga CConversorCarga::DoConverte(const md::CCarga& carga) const
{
    const ecourna::app::dados::asn::CConversorIdentificadorGeradorMidia conversorGerador;

    ModuloTiposEcoUrna::Carga entidade;                                                   // SEQUENCE(info @1146376)
    entidade.set_numeroInternoUrna(carga.GetNumeroInternoUrna());                         // +0
    const std::vector<char> serial = ConverteSerialFlash(carga.GetNumeroSerieFC());       // +4
    entidade.set_numeroSerieFC(ASN1::OCTET_STRING(serial.begin(), serial.end()));
    entidade.set_dataHoraCarga(Utils::ConverteDataHoraJE(carga.GetDataHoraCarga()));      // +16, func 1080
    entidade.set_codigoCarga(carga.GetCodigoCarga());                                     // +28
    entidade.set_identificadorGeradorMidia(conversorGerador.Converte(carga.GetGerador()));// +40, func 3734
    return entidade;
}

// wasm func 10290 - vtable slot 3. Not observed executing.
// No validation of its own: md::CCarga's constructor (func 2802) receives the converted fields.
md::CCarga CConversorCarga::DoDesconverte(const ModuloTiposEcoUrna::Carga& carga) const
{
    const ecourna::app::dados::asn::CConversorIdentificadorGeradorMidia conversorGerador;

    const auto numeroInternoUrna = carga.get_numeroInternoUrna();
    const std::string serial = DesconverteSerialFlash(carga.get_numeroSerieFC());
    const api::CDateTime dataHora = Utils::DesconverteDataHoraJE(carga.get_dataHoraCarga());   // func 1713
    const std::string codigo = carga.get_codigoCarga();
    const auto gerador = conversorGerador.Deconverte(carga.get_identificadorGeradorMidia());  // func 3733
    return md::CCarga(numeroInternoUrna, serial, dataHora, codigo, gerador);                   // func 2802
}

} // namespace comum::asn
