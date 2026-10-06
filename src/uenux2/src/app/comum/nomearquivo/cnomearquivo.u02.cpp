// uenux2/src/app/comum/nomearquivo/cnomearquivo.cpp  --  FRAGMENT written by unit u02 (owner u24)
//
// Scenario/result file names. Anonymous helpers already named from srcloc in this file:
// FormataFase (wasm 2828, line 38), FormataNumero (1161, line 48), FormataUF (3773, line 73),
// ajustaAbrangenciaUFMunicipio (3744, line 208). Example of a real name built this way:
// "t02411ac00001-ca.dat" (fase 't', pleito 02411, UF "ac", município 00001, suffix "ca").

#include "comum/nomearquivo/cnomearquivo.h"

namespace comum {

// wasm func 3772                                                       // name inferred
// "<fase><pleito:05><uf><municipio:05><zona:04><secao:04>-<sufixo>.<extensao>"
// (the pleito-level variant without zona/seção is wasm 1705, not in this unit)
std::string CNomeArquivo::MontaNome(EUrnaFase fase, uedword pleito, const std::string& uf,
                                    uedword municipio, uedword zona, uedword secao,
                                    const std::string& sufixo, const std::string& extensao)
{
    return FormataFase(fase) + FormataNumero(pleito, 5) + FormataUF(uf) + FormataNumero(municipio, 5)
         + FormataNumero(zona, 4) + FormataNumero(secao, 4) + "-" + sufixo + "." + extensao;
}

// wasm func 2811                                                       // name inferred
// Names of the per-abrangência files of one kind (e.g. suffix "pa" = partidos, "fe" = federações,
// extension "dat") for every election of the município. Both callers (the inlined CPartidos /
// CFederacoes loads in wasm 7787) pass CConfiguracaoEleicao +620 (the SIdentificacaoCarga of the
// section), +640 (its const CPleito*) and +644 (the município):
//   2811(out, config + 620, config[+640], config[+644], "pa"/"fe", "dat").
// For each election that covers the município (EleicoesDoMunicipio, wasm 5649) the helper
// ajustaAbrangenciaUFMunicipio (wasm 3744) yields a map abrangência -> file name, and a name is kept
// when one of the election's candidate offices (140-byte CCargo with its optional<CDetalheCandidato>
// engaged, byte +84) has that abrangência (+8).
std::vector<std::string> CNomeArquivo::NomesPorAbrangencia(const SIdentificacaoCarga& identificacao,
                                                           const md::CPleito& pleito,
                                                           TMunicipioID municipio,
                                                           const std::string& sufixo,
                                                           const std::string& extensao)
{
    std::vector<std::string> nomes;
    for (const auto& eleicao : EleicoesDoMunicipio(pleito, municipio)) {          // 52-byte CEleicaoPE
        const std::map<int, std::string> arquivos =
            ajustaAbrangenciaUFMunicipio(identificacao, pleito, municipio, eleicao.id, sufixo, extensao);
        for (const auto& [abrangencia, nome] : arquivos) {
            const bool usado = std::ranges::any_of(eleicao.cargos, [&](const md::CCargo& cargo) {
                return cargo.GetDetalheCandidato().has_value() /*+84*/ && cargo.m_abrangencia /*+8*/ == abrangencia;
            });
            if (usado)
                nomes.push_back(nome);
        }
    }   // wasm 2839 = std::__tree<map<int, std::string>>::destroy
    return nomes;
}

} // namespace comum
