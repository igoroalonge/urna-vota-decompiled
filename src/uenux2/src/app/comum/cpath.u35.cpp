// FRAGMENT of uenux2/src/app/comum/cpath.cpp (attested; owner u22, declarations in cpath.h).
// Reconstructed from vota_web_wasm.wasm (unit u35).
#include "comum/cpath.h"

namespace comum {

// wasm func 1948 - name inferred (other units call it GetPathChaves / GetDiretorioChaves). Not observed executing.
// Directory of the public/secret keys of the static data: "/dsk/fi/estatico/chave/", always on the internal flash,
// re-rooted by GetPathRoot (func 6048: the "/" prefix @1838600 lets tests relocate the tree). Used for bu.pk1
// (CGravadorBU, CGeraBU), wsq.pk1 (CControlaArmazenamentoDeImagens), bio.sk1 (CConversorBiometriaEleitorCifrada),
// the jufa key (CGravadorRCSecao), cv.ber.pri (código verificador) and the hash.dat writer.
std::filesystem::path CPath::GetPathChaves()
{
    return GetPathRoot("/dsk/fi/estatico/chave/");
}

} // namespace comum
