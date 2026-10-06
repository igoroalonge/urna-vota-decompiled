// uenux2/src/app/comum/nomearquivo/cnomearquivo.cpp
// Reconstructed from vota_web_wasm.wasm (unit u24). MontaNome (seção level, wasm 3772) and
// NomesPorAbrangencia (wasm 2811) are in cnomearquivo.u02.cpp (unit u02).
//
// srclocs: :38 FormataFase (2828), :48 FormataNumero (1161), :73 FormataUF (3773),
// :208 ajustaAbrangenciaUFMunicipio (only inlined, into 3744).
// Observed executing: 1161, 1705, 3744, 3773 (votaInit resolves the names of the static files).
#include "comum/nomearquivo/cnomearquivo.h"

#include <format>
#include <utility>

#include "comum/dados/md/processoeleitoral/cpleito.h"     // md::CPleito::GetEleicao (cpleito.cpp:139)
#include "ecourna/api/util/cstringutils.hpp"               // CStringUtils::ToLower (func 1879)

namespace comum {

// wasm func 2827 (tools: comum_f2827) - "CUeComumNomeArquivoError(code, msg, where)" constructor thunk:
// the shared CBaseError body (ecourna func 710) with vtable @1558772.

namespace {

// wasm func 2828 (srcloc line 38). '1' -> "o", '2' -> "s", '3' -> "t" (compiled as a byte table
// 0x74736F >> 8*(fase - '1')).
std::string FormataFase(EUrnaFase fase)
{
    switch (fase) {
    case EUrnaFase{'1'}: return "o";          // oficial
    case EUrnaFase{'2'}: return "s";          // simulado
    case EUrnaFase{'3'}: return "t";          // treinamento
    default:
        // EUrnaFase is passed as a format "handle" (table slot 2366 -> func 536, the shared TSE enum
        // formatter): it prints the underlying integer, e.g. "Fase inválida: 52" for '4'.
        throw CUeComumNomeArquivoError(EUeComumNomeArquivoError{8950},
                                       std::format("Fase inválida: {}", fase));         // line 38
    }
}

// wasm func 1161 (srcloc line 48). Zero-padded decimal of `casas` digits.
std::string FormataNumero(uedword numero, uedword casas)
{
    std::string texto = std::format("{:0{}}", numero, casas);
    if (texto.size() > casas)
        throw CUeComumNomeArquivoError(EUeComumNomeArquivoError{8951},
            std::format("Número ultrapassou o tamanho máximo de {} casas em [{}]", casas, texto));   // line 48
    return texto;
}

// wasm func 3773 (srcloc line 73). Two letters, lower case.
std::string FormataUF(const std::string& uf)
{
    if (uf.size() != 2)
        throw CUeComumNomeArquivoError(EUeComumNomeArquivoError{8952},
                                       std::format("UF inválida: [{}]", uf));            // line 73
    return ecourna::api::util::CStringUtils::ToLower(uf);                                 // func 1879
}

// Line 208 - only inlined (into wasm 3744). The file of an eleição is named after its abrangência:
// municipal -> UF + município, estadual -> UF + 00000, federal -> "BR" + 00000.
void ajustaAbrangenciaUFMunicipio(TMunicipioID& municipio, std::string& uf, const md::ETipoAbrangencia& abrangencia)
{
    switch (abrangencia) {
    case md::ETipoAbrangencia::Municipal:
        break;
    case md::ETipoAbrangencia::Estadual:
        municipio = 0;
        break;
    case md::ETipoAbrangencia::Federal:
        municipio = 0;
        uf = "BR";
        break;
    default:
        throw CUeComumNomeArquivoError(EUeComumNomeArquivoError{8953},
            std::format("Abrangência inválida: {}", std::to_underlying(abrangencia)));    // line 208
    }
}

} // namespace

// wasm func 1705 (tools: api_f1705). Callers: wasm 7787 (start-up: "pu" partidos files for the pleito,
// for UF-wide id 0 and for "br"), api::CFileASN::ReadFromFile@5778.                     name inferred
std::string CNomeArquivo::MontaNome(EUrnaFase fase, uedword id, const std::string& uf,
                                    const std::string& sufixo, const std::string& extensao)
{
    return FormataFase(fase) + FormataNumero(id, 5) + FormataUF(uf) + ("-" + sufixo) + "." + extensao;
}

// wasm func 3744 (the tools name it after the inlined ajustaAbrangenciaUFMunicipio). Callers: wasm 2811
// (CNomeArquivo::NomesPorAbrangencia, unit u02) and 7787 (start-up, sufixo "ca" / extensão "dat").
// Returns {abrangência of the eleição -> file name}, or an empty map when the eleição has no elective
// office (only referendum questions).                                                   name inferred
std::map<md::ETipoAbrangencia, std::string>
CNomeArquivo::MontaNomesEleicao(const SIdentificacaoCarga& id, const md::CPleito& pleito, TMunicipioID municipio,
                                TEleicaoID idEleicao, const std::string& sufixo, const std::string& extensao)
{
    std::map<md::ETipoAbrangencia, std::string> nomes;

    // Copied by value in the binary (12-byte head, nome, abrangência +24, vector<CCargo> of 140-byte
    // records, vector of municípios) although only two fields are read.
    const auto eleicao = pleito.GetEleicao(idEleicao);                  // func 5651 (throws "Eleição não encontrada")

    // func 5653: any cargo whose optional<CDetalheCandidato> (+20, flag +84) is engaged.
    if (eleicao.PossuiCargoEletivo()) {                                                  // name inferred
        md::ETipoAbrangencia abrangencia = eleicao.GetAbrangencia();                    // +24
        std::string uf = id.uf;
        ajustaAbrangenciaUFMunicipio(municipio, uf, abrangencia);                        // line 208
        nomes.emplace(abrangencia, FormataFase(id.fase) + FormataNumero(idEleicao, 5) + FormataUF(uf) +
                                       FormataNumero(municipio, 5) + ("-" + sufixo) + "." + extensao);
    }
    return nomes;
}

} // namespace comum
