// uenux2/src/app/comum/gravadores/cgravadorutil.cpp
// Reconstructed from vota_web_wasm.wasm (unit u23).
#include "comum/gravadores/cgravadorutil.h"

#include <format>

#include "comum/carquivosresultado.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/dados/clocal.h"
#include "comum/gravadores/iresultado.h"   // CUeComumGravadoresError
#include "comum/util/cstringutil.h"

namespace comum {

// Inlined into wasm func 3798 (srcloc lines 28, 33, 38).
std::string CGravadorUtil::DeterminaNomeArquivoSemLetra(TMunicipioID municipio, TZonaID zona, TSecaoID secao,
                                                        char fase)
{
    if (municipio >= 100000)
        throw CUeComumGravadoresError(8645, std::format("Município inválido: {}", municipio));   // :28
    if (zona >= 10000)
        throw CUeComumGravadoresError(8646, std::format("Zona inválida: {}", zona));             // :33
    if (secao >= 10000)
        throw CUeComumGravadoresError(8647, std::format("Seção inválida: {}", secao));           // :38

    const auto pleito = CConfiguracaoEleicao::GetInst().GetPleito();          // CConfiguracaoEleicao +28
    const std::string uf = util::ToLower(CLocal::GetInst().GetUF());          // func 3753 ("GetUF", lower-case)
    // e.g. 't' 02410 "ac" 00001 0001 0001 '-'  ->  "t02410ac0000100010001-"
    return std::format("{:c}{:05}{}{:05}{:04}{:04}-", fase, pleito, uf, municipio, zona, secao);
}

// wasm func 3798 (name inferred; the tools used the name of the inlined function).
// Called by the IGravador constructor (1396), vota_f1276 and CCopiaResultadoParaMR (12134).
std::string CGravadorUtil::DeterminaNomeArquivo(TMunicipioID municipio, TZonaID zona, TSecaoID secao, char fase,
                                                EExtensaoArquivoResultado extensao)
{
    return DeterminaNomeArquivoSemLetra(municipio, zona, secao, fase)
         + CArquivosResultado::GetInst()[extensao];                          // comum_f348 + operator[] ("bu.dat", ...)
}

// wasm func 2274 (srcloc line 97). 'o' -> '1', 's' -> '2', 't' -> '3'.
EUrnaFase CGravadorUtil::ConverteFase(const char fase)
{
    switch (fase) {
    case 'o': return EUrnaFase::OFICIAL;
    case 's': return EUrnaFase::SIMULADO;
    case 't': return EUrnaFase::TREINAMENTO;
    }
    throw CUeComumGravadoresError(8649, std::format("Fase inválida: {:#x}", fase));   // :97
}

}  // namespace comum
