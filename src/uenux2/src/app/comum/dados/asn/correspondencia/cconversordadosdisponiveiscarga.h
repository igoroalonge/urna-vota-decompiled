// uenux2/src/app/comum/dados/asn/correspondencia/cconversordadosdisponiveiscarga.h   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u21). The md class lives in
// uenux2/src/app/comum/dados/md/correspondencia/cdadosdisponiveiscarga.cpp (srcloc lines 41..69).
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/correspondencia/cdadosdisponiveiscarga.h"
#include "ModuloDadosDisponiveisCarga.h"

namespace comum::asn {

// "Dados disponíveis na carga": what the load medium (flash de carga) prepared for this urna offers:
//   DadosDisponiveisCarga ::= SEQUENCE { fase, idPE, idPleito, serialFlashCarga GeneralString, pais,
//        uf UF {sigla, nome}, zona, municipios SEQUENCE OF MunicipioDisponivel {municipio, complementoMunicipio,
//        secoes SEQUENCE OF SecaoEleitoral}, pacotesImportados SEQUENCE OF CabecalhoPacote }
// md::CDadosDisponiveisCarga (88 bytes): +0 EUrnaFase, +4 idPE, +8 idPleito, +12 serial, +24 país, +36 sigla UF,
//   +48 nome UF, +60 TZonaID, +64 std::vector<CMunicipioDisponivel> (56-byte elements: CInfoMunicipio (44) +
//   std::vector<CSecaoEleitoral> at +44), +76 std::vector<CCabecalhoPacote> (72-byte elements).
// RTTI: CConversorDadosDisponiveisCarga : IConversorASN<ModuloDadosDisponiveisCarga::DadosDisponiveisCarga,
//                                                       md::CDadosDisponiveisCarga>
// vtable @1565796: [0] 174 [1] 144 [2] 11424 DoConverte [3] 11423 DoDesconverte. sizeof 4.
// Only user: vota::CImpressaoVersaoPacotes::vf2 (wasm 11905, the "versão dos pacotes" report), through
// api::CFileASN + IConversorASN::Desconverte (srcloc iconversorasn.h:71 record of 11905).
class CConversorDadosDisponiveisCarga
    : public IConversorASN<ModuloDadosDisponiveisCarga::DadosDisponiveisCarga, md::CDadosDisponiveisCarga>
{
protected:
    TEntidade DoConverte(const TDado& dados) const override;         // wasm func 11424
    TDado DoDesconverte(const TEntidade& dados) const override;      // wasm func 11423
};

} // namespace comum::asn
