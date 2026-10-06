// Reconstructed from vota_web_wasm.wasm (unit u17) - partial: only the functions of this unit.
// Original: uenux2/src/api/gui/reports/creport.cpp (srclocs creport.cpp:31 and :37).
//
// A report (api::CReport) is a list of parts (forms) printed one after another on the "paper"
// device api::IPaperRelatorios. The same report can be sent to the printer (DoPrintJob, anonymous
// namespace, RTTI of its lambda $_0 -> func 10884) or rendered into a file: the report files
// ze.dat (zerésima), behb.dat, ... are produced by opening a "report file" on IPaperRelatorios
// (slot 5) for the duration of the rendering (RAII class CScopedReportFile).
//
// Web build: IPaperRelatorios is simulador::CWasmNullPaper (no paper); the report "file" writing is
// done by that mock (not in this unit).
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "api/pattern/cpolysingletonlist.h"

namespace api {

class IPaperRelatorios;   // : IPaper. slot 5 = Abre(const std::string& arquivo), slot 6 = Fecha()

class IFormPart {         // part of a report; slot 2 = Imprime()                       name inferred
public:
    virtual ~IFormPart() = default;
    virtual void Imprime() const = 0;
};

class CReport {
public:
    // wasm func 5486 - renders every part in order (part slot 2). Also the body of the
    // DoPrintJob lambda (func 10884: `[&relatorio] { relatorio.Imprime(); }`).     name inferred
    void Imprime() const
    {
        for (const auto& parte : m_partes)
            parte->Imprime();
    }

private:
    std::vector<std::shared_ptr<IFormPart>> m_partes;   // +0 (8-byte elements)
};

// RAII: redirects the paper device into a file. Both members are inlined (func 3654).
class CScopedReportFile {
public:
    // creport.cpp:31
    explicit CScopedReportFile(const std::filesystem::path& arquivo)
        : m_arquivo(arquivo)
    {
        CPolySingletonList::instance<IPaperRelatorios>().Abre(m_arquivo.string());   // slot 5
    }

    // creport.cpp:37
    ~CScopedReportFile()
    {
        CPolySingletonList::instance<IPaperRelatorios>().Fecha();          // IPaperRelatorios slot 6
    }

private:
    std::filesystem::path m_arquivo;
};

// wasm func 3654 (tools: "api::CScopedReportFile::CScopedReportFile")            name inferred
// Renders `relatorio` into `arquivo`. The name is taken by value (the callers copy it) and converted
// to a temporary std::filesystem::path for CScopedReportFile.
// Callers: vota::CGeraResumoZeresimaBase slot 2 (11943), vota::CGeraZeresimaBase slot 2 (11946),
// vota::CGeraRelatorios::StartState (12105) - unit u09 writes the call as `relatorio.Gera(path)`.
void GeraRelatorioEmArquivo(std::string arquivo, const CReport& relatorio)
{
    CScopedReportFile escopo(std::filesystem::path(arquivo));
    relatorio.Imprime();                                 // func 5486
}

}  // namespace api
