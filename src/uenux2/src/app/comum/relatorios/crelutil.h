// uenux2/src/app/comum/relatorios/crelutil.h  (path inferred from crelutil.cpp srclocs)
// Reconstructed from vota_web_wasm.wasm (unit u25).
//
// comum::CRelUtil — static helpers shared by every printed report of the urna ("relatórios": BU,
// zerésima, BUJ, BIM, lista de eleitores, parâmetros de urna, versões de pacotes, estado da urna...).
// All of them write into an api::CPaperFormBuilder, the paper-report form builder of uenux2:
//     AddText(texto, fonte, alinhamento)   wasm shared_f193   (fonte 1 normal / 2 expandida;
//                                                             alinhamento 0 esquerda, 1 direita, 2 centro)
//     AddNewLine(n)                        wasm func 198
//     AddData(&fonteDeDados, flags)        wasm func 604      (CTextFieldPaper over a CDataText<std::string(*)()>:
//                                                             the text is produced when the form is PRINTED)
//     Build()                              wasm shared_f357   -> std::shared_ptr<api::IForm<api::IPaper>>
// The report strings are Latin-1 in the binary (the thermal printer's encoding).
#pragma once

#include <functional>
#include <string>

#include "api/gui/cpaperformbuilder.h"
#include "api/util/cdatetime.h"
#include "comum/dados/md/estadoaplicacao/cdadocorrespondencia.h"
#include "comum/relatorios/relatoriosdefs.h"
#include "comum/tipos.h"              // TMunicipioID, TZonaID, TSecaoID, EUrnaFase

namespace comum {

struct SQtdeAptos;                    // {u16 originais; u16 temporarios}  (CEleitores::GetQtdAptos)

class CRelUtil {
public:
    // ---- this unit (u25) ------------------------------------------------------------------------
    // wasm 1543 (srcloc :307). Standard report header: demo banner, titles of the PU, report title,
    // processo eleitoral / pleito / date, eleições (when > 1), município/zona/seção block.
    static void IncluiCabecalhoEleicoesMZS(api::CPaperFormBuilder& b, const std::string& titulo,
                                           const TMunicipioID municipio, const TZonaID zona,
                                           const TSecaoID secao, const std::string& nomeMunicipio,
                                           const ETipoCabecalho tipo, const bool usaPleitoContingencia);

    static std::string GetIDCargaFormatado(const std::string& idCarga);   // wasm 2786 (srcloc :69)
    static std::string GetSeparadorFase(const EUrnaFase fase);             // wasm 1919 (srcloc :554)
    static std::string DSCodigoVerificador();                              // wasm 11182 (srcloc :412), slot 3048
    static std::string DSCodigoIdentificacaoUE();                          // wasm 1942, slot 1567   name inferred
    static void IncluiAvisoModoDemonstracao(api::CPaperFormBuilder& b);    // wasm 5583              name inferred
    static std::string FormataSecoesAgregadas(const std::string& formato); // wasm 5738              name inferred

    // ---- other units (declared here because they are CRelUtil-style helpers, names from those units)
    static std::string CompletaDireita(const std::string& texto, std::size_t largura);  // wasm 1881
    static std::string FormataQtdAptos(std::function<SQtdeAptos()> aptos);              // wasm 1921
    static void IncluiDataHora(api::CPaperFormBuilder& b, const api::CDateTime& dataHora,
                               const std::string& rotuloData, const std::string& rotuloHora);   // wasm 1542
    static void IncluiResumoCorrespondencia(api::CPaperFormBuilder& b,
                                            const md::estadoaplicacao::CDadoCorrespondencia& c); // wasm 1541
    static std::string FormataCorrespondencia(
        const md::estadoaplicacao::CDadoCorrespondencia& c);     // wasm 2792 (crelutil.u33.cpp): "DDD.DDD"
};

// crelutil.cpp also defines, in an anonymous namespace, TituloPartido() (wasm 11181, srcloc :58),
// the data source of the "Partido: NN - SIGLA" line (table slot 3050).

} // namespace comum
