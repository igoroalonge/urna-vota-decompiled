// FRAGMENT of uenux2/mock/app/simulador/wasm/cwasmresource.cpp (path inferred: class simulador::CWasmResource,
// vtable @1530184, lives with the other CWasm* mocks) reconstructed by unit u29 from vota_web_wasm.wasm.
// The class itself (its five virtuals, funcs 8591/8600/8605/8611/8633) belongs to unit u31; this file holds the
// three file-local helpers those virtuals call. All names here are inferred.
//
// api::IResource is how the urna application loads its images and animations. On the urna the resources are
// compiled into the Qt application (names such as ":/resource/images/x.png"); in the browser they are files
// in MEMFS, shipped in wasm/vota_web_wasm.data:
//     /uenux/app/img/**                 54 images (battery icons, biometrics, ...)
//     /resource/gifs/*Mulher4.gif       12 GIF animations (only the "Mulher4" variant is shipped)
// CWasmResource maps the application's name to one of those files ("resolve"), reads the whole file, and
// reports every lookup to JavaScript through js_resource_log(acao, nome, caminho, tamanho) (printed only when
// Module.uenuxDebug is set).
//
//   vf2 (8633) image  -> CFixedImage(LeArquivo(ResolveCaminho(nome)))     js_resource_log("image", ...)
//   vf3 (8611) data   -> shared_ptr<vector<uchar>>                         js_resource_log(..., size)
//   vf4 (8605) movie  -> GIF frames                                        js_resource_log("movie", ...)
//   vf5 (8600) exists -> !ResolveCaminho(nome).empty()
//   vf6 (8591) "is it a resource name?" -> nome starts with ':'

#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

extern "C" void js_resource_log(const char* acao, const char* nome, const char* caminho, int tamanho);   // import 29

namespace simulador {
namespace {

// ------------------------------------------------------------------------------------------------
// wasm func 4996. Inserts "Mulher4" before the LAST ".gif" of the name ("x/vereador.gif" ->
// "x/vereadorMulher4.gif"); returns the name unchanged when there is no ".gif". The animations of the
// instruction screens exist in several avatars on the urna; the simulator ships only avatar "Mulher4"
// (woman #4), so every animation is redirected to it.                                     name inferred
// ------------------------------------------------------------------------------------------------
std::string NomeGifMulher4(const std::string& nome)
{
    const auto pos = nome.rfind(".gif");          // inlined find_end
    std::string resultado = nome;
    if (pos != std::string::npos)
        resultado.insert(pos, "Mulher4");
    return resultado;
}

}  // namespace

// ------------------------------------------------------------------------------------------------
// wasm func 2626 (3,824 bytes). Callers: CWasmResource vf2, vf3, vf4, vf5.                name inferred
// Returns the first candidate file that opens for reading, or "" (and logs "missing"). At most 4 candidates
// (images: 3, GIFs: 4), each probed by opening an std::ifstream.
// ------------------------------------------------------------------------------------------------
std::string ResolveCaminho(const std::string& nome)
{
    // 1. Normalise: drop the Qt resource prefix ':' (all of them) and make the path absolute.
    std::string caminho;
    if (!nome.empty()) {
        std::string s = nome;
        while (!s.empty()) {
            if (s.front() != ':') {
                if (s.front() != '/')
                    s.insert(s.begin(), '/');                 // wasm func 3326 (libc++, see below; u41)
                caminho = std::move(s);
                break;
            }
            s.erase(0, 1);
        }
    }

    // 2. Candidate files, in order of preference.
    std::vector<std::string> candidatos;
    if (!caminho.empty()) {
        candidatos = {caminho};

        if (caminho.rfind("/resource/images/", 0) == 0) {                 // "starts with" (rfind at 0)
            const std::string resto = caminho.substr(17);
            candidatos.push_back("/uenux/app/img/" + resto);
            candidatos.push_back("/pkg/img/" + resto);
        }
        if (caminho.rfind("/resource/gifs/", 0) == 0) {
            const std::string resto = caminho.substr(15);
            candidatos.push_back("/pkg/gifs/" + resto);
            candidatos.push_back(NomeGifMulher4(caminho));                     // func 4996
            candidatos.push_back("/pkg/gifs/" + NomeGifMulher4(resto));
        }
        candidatos.erase(std::unique(candidatos.begin(), candidatos.end()), candidatos.end());
    }

    // 3. The first one that can be opened wins.
    for (const std::string& candidato : candidatos) {
        if (std::ifstream(candidato, std::ios::in | std::ios::binary).good()) {
            js_resource_log("resolve", nome.c_str(), candidato.c_str(), -1);
            return candidato;
        }
    }
    js_resource_log("missing", nome.c_str(), "", -1);
    return "";
}

// ------------------------------------------------------------------------------------------------
// wasm func 3326 (unit u41; the tools called it simulador_f3326 because ResolveCaminho is one of its callers).
// It is NOT simulador code. It is the libc++ instantiation std::basic_string<char>::insert(const_iterator, char),
// shared by the whole binary: its other callers are the ASN.1 runtime (func 5055, AVN decoder of OCTET/BIT
// STRING) and Boost.Regex (func 5139, basic_regex_creator<char>::append_set, through table slot 6483 with
// invoke_iiii). Body as in libc++ <string>, for reference:
//
//   iterator basic_string::insert(const_iterator pos, value_type c)
//   {
//       size_type ip  = pos - begin();                       // pos - data()   (SSO: size byte +11 < 0 = long)
//       size_type sz  = size();
//       size_type cap = capacity();                          // long: (cap_word & 0x7fffffff) - 1, short: 10
//       value_type* p;
//       if (cap == sz) {
//           __grow_by_without_replace(cap, 1, sz, ip, 0, 1); // func 1716: always leaves the string "long"
//           p = __get_long_pointer();
//       } else {
//           p = __get_pointer();
//           if (sz - ip != 0)
//               traits_type::move(p + ip + 1, p + ip, sz - ip);   // memory.copy (memmove semantics)
//       }
//       p[ip]   = c;
//       p[++sz] = '\0';
//       __set_size(sz);                                      // long: word +4; short: byte +11 = sz & 0x7f
//       return begin() + ip;
//   }
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
// wasm func 3452. Whole file as bytes; an empty vector if it cannot be opened (no error).  name inferred
// Reads through istreambuf_iterator, i.e. one byte and one possible vector growth at a time (the animation GIFs,
// 0.83 MB (vereador) to 1.58 MB (votoNulo) each, are read this way on every screen that shows them).
// ------------------------------------------------------------------------------------------------
std::vector<unsigned char> LeArquivo(const std::string& caminho)
{
    std::ifstream arquivo(caminho, std::ios::in | std::ios::binary);
    if (arquivo.fail())                                                  // rdstate & (failbit | badbit)
        return {};
    return std::vector<unsigned char>(std::istreambuf_iterator<char>(arquivo), std::istreambuf_iterator<char>());
}

}  // namespace simulador
