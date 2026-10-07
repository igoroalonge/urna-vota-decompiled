// uenux2/src/app/comum/nomearquivo/cnomearquivo.h
// Reconstructed from vota_web_wasm.wasm (unit u24; two more functions in cnomearquivo.u02.cpp, unit u02).
//
// Names of the election data files ("dados estáticos") the urna loads from <flash>/estatico/:
//   <fase><id:05><uf>[<município:05>[<zona:04><seção:04>]]-<sufixo>.<extensão>
//   fase 'o' oficial / 's' simulado / 't' treinamento, uf in lower case ("br" for federal data).
// Examples from the scenarios: t02400ac-pu.dat (processo eleitoral 2400, UF AC, "pu" = parametrização da urna),
// t00000br-pu.dat, t02411ac00001-ca.dat (municipal eleição 2411, município 1, "ca" = candidatos),
// t02511br00000-ca.dat (federal eleição), t02512ac00000-ca.dat (estadual eleição),
// t02400ac0000100010001-tte.dat (zona 1, seção 1).
// "pu" is the urna parametrization (EntidadeParametrizacaoUrna), not partidos, which are "pa" (2026 urna
// data: investigation/README.md, finding E12).
// Error family: CBaseError<EUeComumNomeArquivoError, SErrorLimits{8950, 9000}> (typeinfo @1558752,
// vtable @1558772, constructor thunk wasm 2827): 8950 fase, 8951 número, 8952 UF, 8953 abrangência.
#pragma once

#include <map>
#include <string>
#include <vector>

#include "comum/comumtypes.h"                  // EUrnaFase, uedword, TMunicipioID ...   (header name ?)
#include "comum/md/cabrangencia.h"             // md::ETipoAbrangencia
#include "ecourna/api/exception/cbaseerror.hpp"

namespace comum {

namespace md { class CPleito; class CPleitoDTO; }

enum class EUeComumNomeArquivoError : int {};
using CUeComumNomeArquivoError =
    ecourna::api::exception::CBaseError<EUeComumNomeArquivoError, ecourna::api::exception::SErrorLimits{8950, 9000}>;

// What identifies the data package of this urna in a file name. The same 20-byte prefix exists at
// CConfiguracaoEleicao +620 and is built from CEstadoGeral by wasm 5728 (unit u02 calls it
// SIdentificacaoCarga). Only the members read here are listed.                         name inferred
struct SIdentificacaoCarga {
    EUrnaFase   fase;       // +0  CEstadoGeral +48, '1' oficial / '2' simulado / '3' treinamento
    uedword     pleito;     // +4  CEstadoGeral +4. The processo eleitoral id (idPE), not the pleito:
                            //     real 2026 -el/-tte/-imp names carry 01219, result files the pleito 03220
                            //     (2026 urna data: investigation/README.md, finding E11).
    std::string uf;         // +8  CEstadoGeral +8 (already lower case)
};

class CNomeArquivo {
public:
    // wasm 1705 - pleito/UF level: <fase><id:05><uf>-<sufixo>.<extensão>               name inferred
    static std::string MontaNome(EUrnaFase fase, uedword id, const std::string& uf,
                                 const std::string& sufixo, const std::string& extensao);

    // wasm 3772 (unit u02) - seção level: ...<município:05><zona:04><seção:04>-...        name inferred
    static std::string MontaNome(EUrnaFase fase, uedword pleito, const std::string& uf, uedword municipio,
                                 uedword zona, uedword secao, const std::string& sufixo,
                                 const std::string& extensao);

    // wasm 3744 - eleição level: name of the file of eleição `idEleicao` for its own abrangência,
    // keyed by that abrangência (0 or 1 entry).                                           name inferred
    static std::map<md::ETipoAbrangencia, std::string>
    MontaNomesEleicao(const SIdentificacaoCarga& id, const md::CPleito& pleito, TMunicipioID municipio,
                      TEleicaoID idEleicao, const std::string& sufixo, const std::string& extensao);

    // wasm 2811 (unit u02) - all eleições of the município.                               name inferred
    static std::vector<std::string> NomesPorAbrangencia(const SIdentificacaoCarga& id, const md::CPleito& pleito,
                                                        TMunicipioID municipio, const std::string& sufixo,
                                                        const std::string& extensao);
};

} // namespace comum
