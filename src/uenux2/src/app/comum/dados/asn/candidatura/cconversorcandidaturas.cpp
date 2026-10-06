// uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidaturas.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u21).
#include "comum/dados/asn/candidatura/cconversorcandidaturas.h"

#include "comum/dados/asn/candidatura/cconversorcandidatura.h"
#include "comum/dados/asn/processoeleitoral/cconversorcargo.h"   // CConversorCodigoCargoConsulta (vtable @1555736)

namespace comum::asn {

namespace {

// wasm func 5710 - name inferred. The tool named it IConversorASN<Candidatura, CCandidatura>::Desconverte after the
// inlined call (iconversorasn.h:71, srcloc record 1562404). Observed executing (candidates are loaded at votaInit).
// Appends every candidacy of one list, validating each element first; std::vector growth relocates the 72-byte
// md::CCandidatura elements with func 5950 (move + destroy).
void DesconverteLista(std::vector<md::CCandidatura>& candidaturas, const CConversorCandidatura& conversor,
                      const ASN1::SEQUENCE_OF<ModuloCandidatos::Candidatura>& lista)
{
    for (std::size_t i = 0; i < lista.size(); ++i) {
        candidaturas.push_back(conversor.Desconverte(lista[i]));
    }
}

} // namespace

// wasm func 11445 (vtable slot 3). Observed executing.
// cabecalho, abrangencia and quantidadeVagas are NOT read here (other loaders use them).
std::vector<md::CCandidatura> CConversorCandidaturas::DoDesconverte(const ModuloCandidatos::EntidadeCandidatos& entidade) const
{
    std::vector<md::CCandidatura> candidaturas;
    const CConversorCodigoCargoConsulta conversorCargo;
    for (const auto* cargo : entidade.get_cargos()) {
        const TCargoID codigo = conversorCargo.Desconverte(cargo->get_cargo());     // wasm 5828 (CHOICE checks)
        if (!cargo->partidos_isPresent()) {
            continue;
        }
        for (const auto* partido : cargo->get_partidos()) {
            const TPartidoID numeroPartido = partido->get_partido();
            DesconverteLista(candidaturas, CConversorCandidatura(codigo, numeroPartido, true),
                             partido->get_candidatosAptos());
            DesconverteLista(candidaturas, CConversorCandidatura(codigo, numeroPartido, false),
                             partido->get_candidatosInaptos());
        }
    }
    return candidaturas;
}

} // namespace comum::asn
