// Reconstructed from vota_web_wasm.wasm (unit u17).
// Original: uenux2/src/api/gui/iresource.h
//   srclocs: iresource.h:82 getResourceFile, :87 getResourceMovie, :97 isResource.
//
// The embedded resources of the voting application (images, GIF animations, sounds) are reached
// through the poly-singleton api::IResource. The web build registers simulador::CWasmResource
// (vtable @1530184), which reads them from the Emscripten file system. Resource names start with
// ":/resource/..." (Qt-style), e.g. ":/resource/gifs/votoNulo.gif".
//
// The three free helpers below are one-line inline functions; they have no body of their own in
// the wasm. getResourceMovie is inlined into the two vota screen helpers 1593 and 3076 (the tools
// named those "api::getResourceMovie"); see vota/eleitor/u17-foreign-fragments.cpp.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "api/gui/gui-common.u15.h"
#include "api/pattern/cpolysingletonlist.h"

namespace api {

class CMovie;                                           // cmovie.u15.cpp: {frames, frameAtual, tamanho}
using SharedVector = std::shared_ptr<std::vector<uebyte>>;

// vtable of simulador::CWasmResource @1530184. Slot names from the helpers below and from u15.
class IResource {
public:
    virtual ~IResource() = default;                                           // slots 0/1
    virtual void GetImage(/*...*/) = 0;                                       // slot 2 (?)  (8633 uses "image")
    virtual SharedVector GetFile(const std::string& nome) = 0;                // slot 3  (getResourceFile)
    virtual CMovie GetMovie(const std::string& nome) = 0;                     // slot 4  (getResourceMovie)
    virtual bool slot5(const std::string& nome) = 0;                          // slot 5  ?
    virtual bool Exists(const std::string& nome) = 0;                         // slot 6  (isResource)
};

// iresource.h:82 (inlined in api::CFixedImage::CFixedImage, func 2246)
inline SharedVector getResourceFile(const std::string& nome)
{
    return CPolySingletonList::instance<IResource>().GetFile(nome);
}

// iresource.h:87 (inlined in funcs 1593 and 3076)
inline CMovie getResourceMovie(const std::string& nome)
{
    return CPolySingletonList::instance<IResource>().GetMovie(nome);
}

// iresource.h:97 (inlined in func 2246)
inline bool isResource(const std::string& nome)
{
    return CPolySingletonList::instance<IResource>().Exists(nome);
}

}  // namespace api
