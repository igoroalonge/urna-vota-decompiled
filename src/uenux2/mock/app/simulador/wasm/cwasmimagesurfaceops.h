// Reconstructed from vota_web_wasm.wasm (unit u31).
// Original file UNKNOWN - path inferred: uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.h
//
// simulador::CWasmImageSurfaceOps : api::IImageSurfaceOps (RTTI: typeinfo @1529060 / @1529072,
// vtable @1528988, 18 slots, table entries 584..601). It is not registered on its own: CWasmScreen embeds
// one at +44 and returns it from IScreen::GetImageSurfaceOps() (slot 34).
//
// In this build an image "surface" is nothing more than the ENCODED file bytes (PNG/JPEG/GIF/BMP) in a
// std::shared_ptr<std::vector<uebyte>>: decoding and scaling are left to the browser (js_image).
// Only slots 12..17 (image size and placement) are used by the application in this binary:
//   slot 13  <- comum::CInfoMTLCD::Update (centre an image on the MT LCD)
//   slot 14  <- api::CMovieField::Rect, vota::adicionaFotoEmoldurada (func 6561)
//   slot 16  <- api::CImageField::Rect, api::CImageFieldUpdate::Rect
// Slots 2..11 are never called; their names are unknown (behaviour documented below, "?").
#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "api/gui/iscreen.h"   // api::SPoint, api::SRect, api::TPosition, api::EAnchorPoint

namespace api {
class IImage;              // slot 2: std::vector<uebyte> GetImage() const
using uebyte = std::uint8_t;

// Unknown polymorphic argument of IImageSurfaceOps slots 3, 6 and 10: only its virtual slot 3 (no argument,
// returns an int) is used. Nothing in this build creates such an object.                  // name inferred
class ISurfaceSource {
public:
    virtual ~ISurfaceSource() = default;   // slots 0/1
    virtual void Slot2() = 0;              // ?
    virtual int Slot3() const = 0;         // ?
};
using SharedSurface = std::shared_ptr<std::vector<uebyte>>;   // name inferred

// api::IImageSurfaceOps (original header path unknown, probably api/gui/iimagesurfaceops.h). It has no code
// of its own in the binary; the declaration below is the slot order observed in CWasmImageSurfaceOps.
class IImageSurfaceOps {
public:
    virtual ~IImageSurfaceOps() = default;                                                      // slots 0/1
    virtual SharedSurface CreateSurface(const IImage& imagem, TPosition largura, TPosition altura) = 0;   // 2 ?
    virtual int  Slot3(const ISurfaceSource& fonte, int, int) = 0;                            // 3 ? (calls fonte.slot3)
    virtual int  Slot4(const int& valor, int, int) = 0;                                         // 4 ?
    virtual SharedSurface CreateSurface(const IImage& imagem, int) = 0;                         // 5 ?
    virtual int  Slot6(const ISurfaceSource& fonte, int) = 0;                                 // 6 ?
    virtual int  Slot7(const int& valor, int) = 0;                                              // 7 ?
    virtual uebyte* CopyToBuffer(const std::vector<uebyte>& dados) = 0;                         // 8 ?
    virtual SharedSurface CreateEmptySurface(const int& parametro) = 0;                         // 9 ?
    virtual SharedSurface CreateEmptySurface(const ISurfaceSource& fonte) = 0;                // 10 ?
    virtual void FreeBuffer(uebyte* buffer) = 0;                                                // 11 ?
    virtual SPoint GetImageSize(const std::vector<uebyte>& dados) = 0;                          // 12
    virtual SPoint GetImageSize(const IImage& imagem) = 0;                                      // 13
    virtual SRect CalcRect(TPosition largura, TPosition altura, const SPoint& pos, EAnchorPoint ancora) = 0;   // 14
    virtual SRect GetImageRect(const std::vector<uebyte>& dados, const SPoint& pos, EAnchorPoint ancora) = 0;  // 15
    virtual SRect GetImageRect(const IImage& imagem, const SPoint& pos, EAnchorPoint ancora) = 0;              // 16
    virtual SRect GetImageRect(std::shared_ptr<IImage> imagem, const SPoint& pos, EAnchorPoint ancora) = 0;   // 17
};
}  // namespace api

namespace simulador {

using api::uebyte;

class CWasmImageSurfaceOps : public api::IImageSurfaceOps {
public:
    // slots 0/1: trivial destructor (ICF 174 / 144). No data members (4 bytes: vptr only).
    api::SharedSurface CreateSurface(const api::IImage& imagem, api::TPosition, api::TPosition) override;     // 9365
    int  Slot3(const api::ISurfaceSource& fonte, int, int) override;                                       // 9362
    int  Slot4(const int& valor, int, int) override;                                                         // 9356
    api::SharedSurface CreateSurface(const api::IImage& imagem, int) override;                               // 9348
    int  Slot6(const api::ISurfaceSource& fonte, int) override;                                            // 9339
    int  Slot7(const int& valor, int) override;                                                              // 9333
    uebyte* CopyToBuffer(const std::vector<uebyte>& dados) override;                                         // 9328
    api::SharedSurface CreateEmptySurface(const int& parametro) override;                                    // 9322
    api::SharedSurface CreateEmptySurface(const api::ISurfaceSource& fonte) override;                      // 9317
    void FreeBuffer(uebyte* buffer) override;                                                                 // 9307
    api::SPoint GetImageSize(const std::vector<uebyte>& dados) override;                                     // 3493
    api::SPoint GetImageSize(const api::IImage& imagem) override;                                            // 8998
    api::SRect CalcRect(api::TPosition largura, api::TPosition altura, const api::SPoint& pos,
                        api::EAnchorPoint ancora) override;                                                   // 9289
    api::SRect GetImageRect(const std::vector<uebyte>& dados, const api::SPoint& pos,
                            api::EAnchorPoint ancora) override;                                               // 8994
    api::SRect GetImageRect(const api::IImage& imagem, const api::SPoint& pos, api::EAnchorPoint ancora) override;   // 8991
    api::SRect GetImageRect(std::shared_ptr<api::IImage> imagem, const api::SPoint& pos,
                            api::EAnchorPoint ancora) override;                                               // 8985
};

// The image-header parser of GetImageSize, also inlined verbatim into CWasmResource::GetMovie (func 8605).
api::SPoint TamanhoImagem(const std::vector<uebyte>& dados);   // name inferred

}  // namespace simulador
