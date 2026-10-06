// uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorprocessoeleitoral.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
// The processo eleitoral (election process), file <...>-cp.dat:
//   EntidadeProcessoEleitoral ::= SEQUENCE { cabecalho, nome, dataEleitorado OPTIONAL, pleito1 Pleito,
//     pleito2 Pleito OPTIONAL, tipoEleicaoPrincipal, abrangenciaEleicaoPrincipal, permiteMRJ BOOLEAN,
//     utilizaBiometria BOOLEAN, tiposTTEPermitidos SEQUENCE OF ..., tipoIdentificadorPrincipalEleitor,
//     tiposIdentificadoresPermitidos SEQUENCE OF TipoIdentificadorEleitor, origemConfiguracao, ufs SEQUENCE OF SiglaUF }
// Only id, nome, pleito1/2, utilizaBiometria, the voter-identifier types and origemConfiguracao are kept;
// dataEleitorado, tipoEleicaoPrincipal, abrangência, permiteMRJ, tiposTTEPermitidos and ufs are ignored here.
#include "comum/dados/asn/processoeleitoral/cconversorprocessoeleitoral.h"

#include <algorithm>
#include <format>
#include <optional>
#include <vector>

#include "comum/dados/asn/processoeleitoral/cconversororigemconfiguracao.h"
#include "comum/dados/asn/processoeleitoral/cconversorpleito.h"
#include "comum/dados/asn/processoeleitoral/cconversortipoidentificadoreleitor.h"

namespace comum::asn {

// wasm func 11363 (vtable slot 3; srcloc line 50)
md::CProcessoEleitoralDTO CConversorProcessoEleitoral::DoDesconverte(const TEntidade& processo) const
{
    const CConversorOrigemConfiguracao conversorOrigem;
    const CConversorTipoIdentificadorEleitor conversorTipoIdentificador;

    // The id is read from the cabecalho's IDEleitoral CHOICE without checking which alternative is selected
    // (idProcessoEleitoral is expected).
    const TPEID id = processo.get_cabecalho().get_idEleitoral().get_idProcessoEleitoral();
    const std::string nome = processo.get_nome();
    const bool utilizaBiometria = processo.get_utilizaBiometria();
    const md::EOrigemConfiguracao origem = conversorOrigem.Desconverte(processo.get_origemConfiguracao());
    const md::CPleitoDTO pleito1 = CConversorPleito().Desconverte(processo.get_pleito1());
    const auto tipoPrincipal = conversorTipoIdentificador.Desconverte(processo.get_tipoIdentificadorPrincipalEleitor());

    std::vector<ecourna::app::dados::ETipoIdentificadorEleitor> tiposPermitidos;
    for (const auto* tipo : processo.get_tiposIdentificadoresPermitidos()) {
        tiposPermitidos.push_back(conversorTipoIdentificador.Desconverte(*tipo));
    }
    if (std::ranges::find(tiposPermitidos, tipoPrincipal) == tiposPermitidos.end()) {
        // The format argument is a plain int (packed arg type 3), not the enum (no std::formatter handle), so
        // the enum is converted explicitly in the source.
        throw CDadosError(8185, std::format("Identidade principal ({}) ausente na lista de identidades",
                                            static_cast<int>(tipoPrincipal)));   // line 50
    }

    std::optional<md::CPleitoDTO> pleito2;
    if (processo.pleito2_isPresent()) {
        pleito2 = CConversorPleito().Desconverte(processo.get_pleito2());
    }
    return md::CProcessoEleitoralDTO(id, nome, utilizaBiometria, origem, pleito1, pleito2, tipoPrincipal,
                                     tiposPermitidos);   // vector copied into the DTO, then moved
}

} // namespace comum::asn
