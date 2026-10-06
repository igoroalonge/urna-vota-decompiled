// uenux2/src/app/comum/relatorios/cgeradorrelbase.h
// Reconstructed from vota_web_wasm.wasm (unit u25). Only instantiation: <comum::CRdvVota, comum::CEleitores>.
//
// comum::CGeradorRelBase<RDV, DSAptos> — skeleton of a vote-count report (BU; the zerésima uses the
// sibling parts classes of cpartecandidatos.cpp): header, per-cargo blocks, QR codes and trailer, rendered
// into the report "paper" (api::IPaperRelatorios, which on the real urna writes the printer byte stream
// into a file such as trab1/bu.dat, later exported as -imgbu.dat).
//
// RTTI: comum::CGeradorRelBase<CRdvVota, CEleitores> (typeinfo @1575228), vtable @1575404:
//   [0] ~CGeradorRelBase           wasm 5609
//   [1] deleting destructor        icf_tiny_vf1@325 (shared body)
//   [2] ImprimePreTexto() const    pure
//   [3] ImprimeProporcionalPartido() const   pure
//   [4] ImprimeMajoritario() const pure
//   [5] ImprimeConsulta() const    pure
// Layout (36 bytes):
//   +0  vptr
//   +4  std::shared_ptr<api::IForm<api::IPaper>> m_header
//   +12 std::shared_ptr<api::IForm<api::IPaper>> m_trailer
//   +20 std::shared_ptr<api::IForm<api::IPaper>> m_qrcode        (may be null: no QR form)
//   +28 std::shared_ptr<api::IForm<api::IPaper>> m_linhaVazia    (a form holding AddText("", 1, 0))
// srclocs: :48 / :51 (constructor checks, inlined in vota::CGeraBU::StartState = wasm 12110),
//          :61 GeraRelatorio(const std::string&) (IPaperRelatorios lookup, inlined in wasm 12110).
#pragma once

#include <memory>
#include <string>

#include "api/gui/cpaperformbuilder.h"
#include "api/gui/iform.h"
#include "api/hwil/ipaperrelatorios.h"
#include "api/pattern/cpolysingleton.h"
#include "api/util/csynchronizer.h"
#include "comum/dados/ccargos.h"
#include "comum/relatorios/relatoriosdefs.h"

namespace comum {

using SharedPaperForm = std::shared_ptr<api::IForm<api::IPaper>>;

template <typename RDV, typename DSAptos>
class CGeradorRelBase {
public:
    // Inlined into wasm 12110 (srclocs :48, :51). The blank-line form is built before the checks.
    CGeradorRelBase(SharedPaperForm header, SharedPaperForm trailer, SharedPaperForm qrcode)
        : m_header(std::move(header))
        , m_trailer(std::move(trailer))
        , m_qrcode(std::move(qrcode))
        , m_linhaVazia(CriaLinhaVazia())
    {
        if (!m_header)
            throw CRelatoriosError(EUeComumRelatoriosError{9066}, "header nulo");      // :48
        if (!m_trailer)
            throw CRelatoriosError(EUeComumRelatoriosError{9067}, "trailer nulo");     // :51
    }

    // wasm func 5609 (vtable slot 0): releases the four shared_ptrs (+28, +20, +12, +4).
    virtual ~CGeradorRelBase() = default;

    // Inlined into wasm 12110 (srcloc :61).
    void GeraRelatorio(const std::string& caminho)
    {
        auto& papel = api::CPolySingleton<api::IPaperRelatorios>::instance();    // func 905, :61
        papel.Abre(caminho);                                                    // IPaperRelatorios slot 5

        m_header->Imprime();                                                    // IForm slot 2
        ImprimePreTexto();                                                      // slot 2

        CCargos& cargos = CCargos::GetInst();
        cargos.OrdenaPorOrdemImpressao();                                       // func 3782
        // Start with the eleição of the LAST cargo, so a single-election report gets no banner.
        TEleicaoID eleicaoAnterior = cargos.GetUltimaEleicao();                 // 0 if the list is empty
        for (cargos.First(); !cargos.IsEnd(); cargos.Next()) {                  // 2269 / shared_f602 / 1708
            const TEleicaoID eleicao = cargos.GetCurrentEleicao().GetId();
            if (eleicao != eleicaoAnterior) {
                api::CPaperFormBuilder b;
                b.AddText("======================================", 1, 0);
                b.AddText(cargos.GetCurrentEleicao().GetNome(), 1, 2);
                b.AddText("======================================", 1, 0);
                b.AddNewLine(1);
                b.Build()->Imprime();
                eleicaoAnterior = eleicao;
            }
            const md::CCargo& cargo = cargos.GetCurrent();
            if (cargo.TemDetalheCandidato() && cargo.GetTipo() == md::CCargo::ETipo::PROPORCIONAL)
                ImprimeProporcionalPartido();                                   // slot 3
            else if (cargo.TemDetalheCandidato() && cargo.GetTipo() == md::CCargo::ETipo::MAJORITARIO)
                ImprimeMajoritario();                                           // slot 4
            else
                ImprimeConsulta();                                              // slot 5
        }

        if (m_qrcode)
            m_qrcode->Imprime();
        m_trailer->Imprime();
        papel.Fecha();                                                          // slot 6
        api::CSynchronizer::CreateInst()->Sincroniza();                         // shared_f620
    }

protected:
    virtual void ImprimePreTexto() const = 0;
    virtual void ImprimeProporcionalPartido() const = 0;
    virtual void ImprimeMajoritario() const = 0;
    virtual void ImprimeConsulta() const = 0;

    SharedPaperForm m_header;        // +4
    SharedPaperForm m_trailer;       // +12
    SharedPaperForm m_qrcode;        // +20
    SharedPaperForm m_linhaVazia;    // +28

private:
    static SharedPaperForm CriaLinhaVazia()                                     // inlined, name inferred
    {
        api::CPaperFormBuilder b;
        b.AddText("", 1, 0);
        return b.Build();
    }
};

} // namespace comum
