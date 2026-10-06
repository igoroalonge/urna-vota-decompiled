// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/reports/csubreport.h (path inferred: next to creport.cpp, attested).
//
// api::CSubReport : api::ISubReport   (typeinfo 1543212, vtable @1543180, 40 bytes)
// The description of one print job handed to the report printer front end api::CLp (cwasmclp.cpp, unit
// u19): CLp::Imprime(const ISubReport&, std::function<void()> job) (func 3875, name inferred) does
//     if (m_impressoraRelatorios) m_impressoraRelatorios->vf9();       // IImpressoraRelatorios slot 9
//     if (sub.TemCabecalho()) sub.ImprimeCabecalho();                   // slots 5 / 6
//     job();                                                            // prints the report itself
// The object is always a stack temporary built by the out-of-line constructor func 3660 (called, not
// inlined; the CSubReport vtable stores in the callers are the inlined destructor after CLp::Imprime) in
//   vota::CRelVotaUtil::CortaPapel (2882)            ("", "", false)
//   api::CPaperFormBuilder::Show(nome, via) (3671)   (nome, via, true)
//   vota::CImpressaoListaEleitores::vf2 (11908)      (nome, "{}ª via", true)
// Layout:
//   +4  std::string m_nome
//   +16 std::string m_via            e.g. std::format("{}ª via", n)
//   +28 std::shared_ptr<IReportPart> m_cabecalho   (never set in this build: always null)          ?
//   +36 bool m_relatorio
// All slot names are inferred.
#pragma once

#include <memory>
#include <string>

#include "api/gui/cformpart.h"   // api::IReportPart

namespace api {

class ISubReport {
public:
    virtual ~ISubReport() = default;                                  // slots 0/1
    virtual std::string GetNome() const = 0;                          // 2
    virtual std::string GetVia() const = 0;                           // 3
    virtual bool TemNome() const = 0;                                 // 4
    virtual bool TemCabecalho() const = 0;                            // 5
    virtual void ImprimeCabecalho() const = 0;                        // 6
    virtual bool EhRelatorio() const = 0;                             // 7
};

class CSubReport : public ISubReport {
public:
    // func 3660 (other unit; the tools attribute it to cverificahorariozeresima.cpp): moves both strings in.
    CSubReport(std::string nome, std::string via, bool relatorio)
        : m_nome(std::move(nome)), m_via(std::move(via)), m_relatorio(relatorio)
    {
    }
    ~CSubReport() override;                                           // wasm 12014 / 12013

    std::string GetNome() const override { return m_nome; }           // ICF 3874
    std::string GetVia() const override;                              // wasm 12012
    bool TemNome() const override;                                    // wasm 12011
    bool TemCabecalho() const override;                               // wasm 12010
    void ImprimeCabecalho() const override;                           // wasm 12009
    bool EhRelatorio() const override;                                // wasm 12008

private:
    std::string                  m_nome;             // +4
    std::string                  m_via;              // +16
    std::shared_ptr<IReportPart> m_cabecalho;        // +28/+32
    bool                         m_relatorio;        // +36
};

} // namespace api
