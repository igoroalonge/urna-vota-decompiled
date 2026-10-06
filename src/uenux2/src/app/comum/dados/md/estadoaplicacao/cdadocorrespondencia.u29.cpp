// FRAGMENT of uenux2/src/app/comum/dados/md/estadoaplicacao/cdadocorrespondencia.cpp (path inferred from the
// class name in the srcloc IConversorASN<ModuloEstadoGeralUrna::DadoCorrespondencia,
// comum::md::estadoaplicacao::CDadoCorrespondencia>) reconstructed by unit u29 from vota_web_wasm.wasm.
//
// "Correspondência" = the identity of the carga (media preparation) that produced this urna's software/data:
// ASN.1 DadoCorrespondencia {carga Carga, secaoCarga DadoSecao, assinatura OCTET STRING}, where
// Carga = {numeroInternoUrna, numeroSerieFC, identificadorGeradorMidia, dataHoraCarga, codigoCarga}.
// The C++ class flattens Carga. 96 bytes (destructor func 857, copy constructor func 1249):
//   +0  uedword m_numeroInternoUrna        +4  std::string m_numeroSerieFC
//   +16 api::CDateTime m_dataHoraCarga     +28 std::string m_codigoCarga
//   +40 CLocalidadeEleitoral m_secaoCarga  +48 std::vector<uebyte> m_assinatura
//   +60 ecourna::app::dados::CIdentificadorGeradorMidia m_geradorMidia (3 strings)
// eg.bin keeps one (the current carga); gap.bin keeps up to 10 (histórico de cargas printed on the BU).
#include <cstdint>
#include <string>
#include <vector>

#include "api/util/cdatetime.h"
#include "comum/dados/md/estadoaplicacao/clocalidadeeleitoral.h"
#include "ecourna/app/dados/midias/cidentificadorgeradormidia.hpp"

namespace comum::md::estadoaplicacao {

// wasm func 5630 (table slot 450). Member-wise constructor: strings, vector and generator moved, date and
// section copied; no validation. Callers: CConversorDadoCorrespondencia::DoDesconverte (func 11400) and the
// builder's copy of the state (func 10155).                                             parameter names inferred
CDadoCorrespondencia::CDadoCorrespondencia(std::uint32_t numeroInternoUrna, std::string numeroSerieFC,
                                           const api::CDateTime& dataHoraCarga, std::string codigoCarga,
                                           const CLocalidadeEleitoral& secaoCarga, std::vector<std::uint8_t> assinatura,
                                           ecourna::app::dados::CIdentificadorGeradorMidia geradorMidia)
    : m_numeroInternoUrna(numeroInternoUrna),
      m_numeroSerieFC(std::move(numeroSerieFC)),
      m_dataHoraCarga(dataHoraCarga),
      m_codigoCarga(std::move(codigoCarga)),
      m_secaoCarga(secaoCarga),
      m_assinatura(std::move(assinatura)),
      m_geradorMidia(std::move(geradorMidia))
{
}

}  // namespace comum::md::estadoaplicacao
