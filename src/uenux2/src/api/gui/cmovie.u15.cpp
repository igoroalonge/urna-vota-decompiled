// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cmovie.cpp (srcloc cmovie.cpp:29). Merge into cmovie.cpp.
#include <vector>

#include "api/gui/gui-common.u15.h"

namespace api {

class CMovieFrame;   // one frame (image + duration); built by api::getResourceMovie (func 1593)

// CMovie - the animations of the voter screens (e.g. "voto nulo"/"fim" animations); 20 bytes:
//   +0  std::vector<CMovieFrame> m_frames
//   +12 size_t m_frameAtual            (reset by CMovieField::Start)
//   +16 SPoint m_tamanho               (frame width/height, used by CMovieField::Rect)
class CMovie {
public:
    CMovie(std::vector<CMovieFrame> frames, const SPoint& tamanho);
private:
    friend class CMovieField;
    std::vector<CMovieFrame> m_frames;   // +0
    size_t m_frameAtual = 0;             // +12
    SPoint m_tamanho;                    // +16
};

// wasm func 5526 (not observed) - srcloc cmovie.cpp:29
// Callers: api::getResourceMovie (func 1593) and simulador::CWasmResource::vf4 (func 8605, the web
// resource loader).
CMovie::CMovie(std::vector<CMovieFrame> frames, const SPoint& tamanho)
    : m_frames(std::move(frames)), m_frameAtual(0), m_tamanho(tamanho)
{
    if (m_frames.empty())
        throw CUeGuiError(static_cast<EUeGuiError>(4943), "Não havia frames");
}

} // namespace api
