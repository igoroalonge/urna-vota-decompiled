// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cdataimage.h (path inferred; ctelasvota.cpp of unit u07 already includes it).
//
// api::CDataImage<SRC> : api::IImage - the image counterpart of CDataText: an image whose bytes are produced
// by a data source at draw time (candidate photos on the voter screen, the vice/suplente photos, the QR code
// images). IImage (typeinfo 1530296, "class") has 3 slots: [0] ~IImage [1] deleting [2] GetImage() const.
#pragma once

#include <utility>
#include <vector>

#include "api/gui/primitives.h"

namespace api {

using uebyte = unsigned char;

class IImage {
public:
    virtual ~IImage() = default;
    virtual std::vector<uebyte> GetImage() const = 0;    // slot 2 (name inferred; u15 calls it GetDados)
};

template <class SRC>
class CDataImage : public IImage {
public:
    explicit CDataImage(SRC fonte) : m_fonte(std::move(fonte)) {}
    ~CDataImage() override = default;

    std::vector<uebyte> GetImage() const override { return m_fonte(); }

private:
    SRC m_fonte;   // +4 (or +8 when SRC needs 8-byte alignment)
};

// -----------------------------------------------------------------------------------------------------------
// Instantiations (vtable: slot0 D1 / slot1 D0 / slot2 GetImage):
//
// CDataImage<std::__bind<vota::CTelasVota::CriaTelaVisualizacaoCandidato(...)::$_0&,
//                        const comum::md::CDadosCandidato&>>          @1538556   12524 / 12522 / 12520
//     The candidate photo of the "visualizar candidatos" screen (built by adicionaFotoEmoldurada, func 6561).
//     Layout: +4 the (empty) lambda $_0, +8 the bound copy of comum::md::CDadosCandidato =
//       { std::string (+8), std::string (+20), std::optional<std::string> (+32, engaged flag +44) }.
//     wasm func 12524 (D1) / 12522 (D0): destroy the optional string only if engaged, then the two strings.
//     wasm func 12520 (GetImage): the lambda body is inlined -
//         comum::CFotos::GetInst();                          // func 2819, result unused (re-fetched inside)
//         return comum::CFotos::GetImagem(dados.<1st string>); // func 3755 (LeEntidadeEm<CConversorFotoCandidato>)
//
// CDataImage<comum::CCandidaturasDSFoto>                    @1537532   174 / 144 / 12641 (other unit)
//     photo of the current candidate / running mate n (uebyte n at +4).
// CDataImage<std::vector<uebyte> (*)()>                     @1538628   174 / 144 / 6330 (ICF `return f()`)
// -----------------------------------------------------------------------------------------------------------

} // namespace api
