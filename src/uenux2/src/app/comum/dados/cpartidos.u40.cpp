// FRAGMENT of uenux2/src/app/comum/dados/cpartidos.cpp (path inferred by units u23/u36; class comum::CPartidos =
// api::CDataMap<TPartidoID, md::CPartido>("CPartidos"), 28 bytes). Reconstructed by unit u40.
#include <memory>
#include <mutex>
// (no cpartidos.h has been reconstructed yet; CPartidos : api::CDataMap<TPartidoID, md::CPartido>, see u36)

namespace comum {

// wasm func 2815. CPartidos::~CPartidos() (implicit): ~m_nome (std::string at +16) then the map's
// __tree::destroy(root) (api_f1932: frees each node and its two strings, sigla +24 and nome +36).
// Callers: CPartidos::GetInst (819, replacing an old instance), the text sources 11499/11501 and the atexit
// handler 11503 below.
CPartidos::~CPartidos() = default;

// Statics of CPartidos::GetInst() (func 819, reconstructed in u23-foreign-fragments.cpp):
//   static std::mutex mutex;              // @1838904 -> atexit destructor wasm func 11502 (pthread stub residue 150)
//   static std::unique_ptr<CPartidos> s;  // @1838928 -> atexit destructor wasm func 11503:
//                                         //    reset(): ~CPartidos (2815) + operator delete

} // namespace comum
