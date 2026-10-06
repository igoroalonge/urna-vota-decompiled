// uenux2/src/app/comum/relatorios/cgeradorbu.h  (path inferred from cgeradorbu.cpp srclocs)
// Reconstructed from vota_web_wasm.wasm (unit u25).
//
// comum::CGeradorBU : CGeradorBUBase<CRdvVota, CEleitores> — the Boletim de Urna generator of VOTA.
// RTTI typeinfo @1575204 (si), vtable @1575164:
//   [0] ~CGeradorBU                 wasm 3698     [1] deleting dtor  wasm 11258
//   [2] ImprimePreTexto() const     wasm 11256 (srcloc cgeradorbu.cpp:257)
//   [3..5] inherited from CGeradorBUBase (11255, 11254, 11253)
// Layout: CGeradorBUBase (116 bytes) + std::vector<std::string> m_historicoCargas at +116 (128 bytes).
// The constructor exists only inlined into vota::CGeraBU::StartState (wasm 12110), which creates the
// object on the stack, calls GeraRelatorio(trab1/bu.dat) and destroys it.
#pragma once

#include <string>
#include <vector>

#include "api/util/cdatetime.h"
#include "comum/dados/celeitores.h"
#include "comum/dados/crdvvota.h"
#include "comum/dados/md/estadoaplicacao/cdadocorrespondencia.h"
#include "comum/relatorios/cgeradorbubase.h"

namespace comum {

class CGeradorBU : public CGeradorBUBase<CRdvVota, CEleitores> {
public:
    CGeradorBU(const std::string& titulo, TMunicipioID municipio, TZonaID zona, TSecaoID secao,
               const std::string& nomeMunicipio, const md::estadoaplicacao::CDadoCorrespondencia& correspondencia,
               const std::vector<std::string>& historicoCargas, const api::CDateTime& dataHoraEmissao,
               const std::string& separadorFase);                    // inlined in 12110
    ~CGeradorBU() override;                                          // wasm 3698 (+ deleting 11258)

protected:
    void ImprimePreTexto() const override;                           // wasm 11256

private:
    std::vector<std::string> m_historicoCargas;                      // +116 códigos de carga (gap.bin)
};

// Free functions of cgeradorbu.cpp (inlined into 12110; signatures from RTTI / srclocs):
SharedPaperForm CriaHeader(const std::string& titulo, unsigned municipio, unsigned short zona,
                           unsigned short secao, const std::string& nomeMunicipio,
                           const md::estadoaplicacao::CDadoCorrespondencia& correspondencia,
                           bool temVota, api::CDateTime dataHoraEmissao);            // RTTI of its lambda
SharedPaperForm CriaQRCode(const TZonaID zona, const TSecaoID secao,
                           const md::estadoaplicacao::CDadoCorrespondencia& correspondencia,
                           const std::vector<std::string>& historicoCargas, const bool imprimeQRCode,
                           const api::CDateTime& dataHoraEmissao, const bool origemRED,
                           const bool incluiEmissao);                                // srcloc :124
char DefineTipo(const bool modoDemonstracao);                                        // srcloc :161

} // namespace comum
