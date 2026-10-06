// uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.cpp
// Reconstructed from vota_web_wasm.wasm (unit u25).
//
// comum::CCabecalhoQRCodeBuilder — fluent builder of the header of the BU QR payload
// (comum::CCabecalhoQRCode: 33 "TAG:value " strings, 396 bytes; field list in
// cgeradorbuqrcode.u04-fragment.cpp). Its setters are wasm 5618 (zona), 5620 (seção), 5621 (id da urna),
// 5622 (histórico de cargas), 5623 (código de carga) and its constructor 5624 (other units); they are
// called from the inlined CriaQRCode of cgeradorbu.cpp (wasm 12110). preBuild() is inlined into
// CGeradorBUQRCode::GeraQRCodes (wasm 5604); the only function of this file compiled on its own is the
// lambda of preBuild():
//
// srcloc :284  auto CCabecalhoQRCodeBuilder::preBuild()::(lambda)::operator()(
//                  const std::vector<std::pair<std::reference_wrapper<const std::string>, std::string>>&) const
// Not executed in the recorded votes.

#include "comum/relatorios/ccabecalhoqrcodebuilder.h"

#include <format>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "comum/relatorios/relatoriosdefs.h"

namespace comum {

using TCamposObrigatorios = std::vector<std::pair<std::reference_wrapper<const std::string>, std::string>>;

// Inlined into wasm 5604 (the list contents are the ones u04 read there).
void CCabecalhoQRCodeBuilder::preBuild() const
{
    // wasm func 2791 (srcloc :284). Every listed field must already hold its "TAG:value " text.
    const auto verifica = [](const TCamposObrigatorios& campos) {
        for (const auto& [campo, nome] : campos)                     // 16-byte elements
            if (campo.get().empty())
                throw CRelatoriosError(EUeComumRelatoriosError{9050},
                                       std::format("Campo ({}) não informado.", nome));      // :284
    };

    const CCabecalhoQRCode& c = m_cabecalho;
    verifica({{c.orig, "Origem"}, {c.orlc, "OrigemProcessoEleitoral"}, {c.proc, "ProcessoEleitoral"},
              {c.dtpl, "DataPleito"}, {c.plei, "Pleito"}, {c.turn, "Turno"}, {c.fase, "Fase"}, {c.unfe, "UF"},
              {c.muni, "Municipio"}, {c.zona, "Zona"}, {c.seca, "Secao"}, {c.idue, "IdUrna"},
              {c.idca, "CodigoCarga"}, {c.hica, "HistoricoCarga"}, {c.vers, "VersaoSoftware"}});
    if (EhOrigemSA()) {                                                                  // wasm 5617
        verifica({{c.junt, "Junta"}, {c.turm, "Turma"}, {c.dtem, "DataEmissao"}, {c.hrem, "HoraEmissao"}});
    } else {
        verifica({{c.loca, "Local"}, {c.apto, "QtdeAptos"}, {c.apts, "QtdeAptosDaSecao"},
                  {c.aptt, "QtdeAptosTTE"}, {c.comp, "QtdeCompareceram"}, {c.falt, "QtdeFaltosos"}});
        if (EhOrigemRED())                                                               // wasm 5616
            verifica({{c.dtem, "DataEmissao"}, {c.hrem, "HoraEmissao"}});
    }
}

} // namespace comum
