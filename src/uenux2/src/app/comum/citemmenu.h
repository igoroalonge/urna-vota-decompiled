// uenux2/src/app/comum/citemmenu.h (path inferred: class comum::CItemMenu, next to comum::CMenuBase)
// Reconstructed from vota_web_wasm.wasm by unit u37.
//
// One line of a numbered menu drawn by comum::CMenuBase ("[<id>] - <texto>"). The only menu of this kind
// in VOTA is "Mais informações" (vota::CMenuMaisInformacoesVota, built by CTelasVota::CriaTelaMaisInformacoes,
// wasm 6599): the mesário can print extra copies of four reports before the zerésima, or browse the
// candidates.
//
// RTTI: comum::CItemMenu (class, no base; typeinfo @1539904, vtable @1539892):
//   [0] ~CItemMenu()            wasm 2937 (merged body shared_f1727: vptr + ~string at +8)
//   [1] deleting dtor           icf 325 (unreachable: the class is abstract)
//   [2] bool Disponivel() const = 0                                               name inferred (u02)
// Subclasses (comum, abstract, one per report; the key function Disponivel is out of line):
//   CItemImprimeEstadoUrna     vtable @1550788   Disponivel wasm 11669   (citemimprimeestadourna.cpp)
//   CItemImprimeListaEleitores vtable @1550824   Disponivel wasm 11668   (citemimprimelistaeleitores.cpp)
//   CItemParametrosUrna        vtable @1550860   Disponivel wasm 11667   (citemparametrosurna.cpp)
//   CItemVersoesPacotes        vtable @1550896   Disponivel wasm 11666   (citemversoespacotes.cpp)
//   each adds slot [3] GetNumViasImpressas() = 0, implemented by the vota::C...Vota subclass
//   (wasm 12267 / 12266 / 12255 / 12259: bytes +73 / +74 / +76 / +75 of EstadoGeralVota, i.e.
//   numViasImpressasRelatorios {estadoUrna, eleitores, PU, versoesDados} of vota.bin).
// vota::CItemVisualizarCandidatosVota derives directly from CItemMenu (Disponivel = wasm 12244, unit u02).
//
// Layout (32 bytes), from the inlined constructors in wasm 6599:
//   +0 vptr   +4 int m_id   +8 std::string m_texto   +20 api::SPoint m_posicao
//   +24 api::SFont m_fonte = {20, 0}: written as one 8-byte store (20L) in 6599 and read as one 8-byte
//       value by CMenuBase::Monta (5913), which also uses its first field (the size) for the "Escolha a sua
//       opção:" line (y + 2 * size). This is the SFont of gui-common.u15.h and the "SFonte" of cmenubase.u02.cpp.
// The text is COPIED into +8 (copy + free of the formatted temporary in 6599), so the constructor takes it by
// const reference.
#pragma once

#include <string>

#include "api/gui/gui-common.h"     // api::SPoint, api::SFont (header name ?; gui-common.u15.h in unit u15)

namespace comum {

class CItemMenu {
public:
    CItemMenu(int id, const std::string& texto, const api::SPoint& posicao)
        : m_id(id), m_texto(texto), m_posicao(posicao)
    {
    }

    // wasm func 2937 (complete-object destructor; observed executing at start-up, called from the control block
    // of a shared_ptr<CItemVisualizarCandidatosVota> (12172 __on_zero_shared) when the temporary menu of the
    // "Mais informações" form releases its items). Shares its body with every class that only
    // has to destroy a std::string at +8 (merged body shared_f1727(this, vtable CItemMenu)).
    virtual ~CItemMenu() = default;

    /// An unavailable item is drawn in grey (colour 5 instead of 2) and its id is not accepted as an
    /// answer (CMenuBase::AdicionaItem / Monta).
    virtual bool Disponivel() const = 0;                              // slot 2

    int GetId() const { return m_id; }
    const std::string& GetTexto() const { return m_texto; }
    const api::SPoint& GetPosicao() const { return m_posicao; }
    const api::SFont& GetFonte() const { return m_fonte; }                    // used by CMenuBase::Monta
    int GetTamanhoFonte() const { return m_fonte.size; }

private:
    int m_id;                       // +4
    std::string m_texto;            // +8
    api::SPoint m_posicao;          // +20
    api::SFont m_fonte{20, 0};      // +24 size, +28 style
};

} // namespace comum
