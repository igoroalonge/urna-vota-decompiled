// uenux2/src/app/comum/dados/asn/eleitor/cconversorbiometriaeleitor.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
// The decrypted biometrics of one voter:
//   BiometriaEleitor ::= SEQUENCE { foto [1] Foto OPTIONAL, dedos [2] SEQUENCE OF Dedo OPTIONAL }
// (the voter file stores it encrypted as BiometriaEleitorCifrada, see cconversorbiometriaeleitorcifrada.cpp).
#include "comum/dados/asn/eleitor/cconversorbiometriaeleitor.h"

#include <vector>

#include "comum/dados/asn/eleitor/cconversordedo.h"
#include "ecourna/app/dados/asn/cconversorfoto.h"

namespace comum::asn {

// wasm func 11422 (vtable slot 3; srcloc line 26)
md::CBiometriaEleitor CConversorBiometriaEleitor::DoDesconverte(const TEntidade& biometria) const
{
    if (!biometria.foto_isPresent() && !biometria.dedos_isPresent()) {
        throw CDadosError(7893, "Não tem nem foto nem dedos, mas falou que tem biometria.");   // line 26
    }

    std::vector<md::CDedo> dedos;
    if (biometria.dedos_isPresent()) {
        const CConversorDedo conversorDedo;
        // Dead code in the binary: an empty ModuloEleitores::BiometriaEleitor::dedos (SEQUENCE OF Dedo, with
        // a prototype Dedo) is constructed and destroyed here without being used — probably a local copy or
        // a typedef'd temporary left in the source.
        for (const auto* dedo : biometria.get_dedos()) {
            dedos.push_back(conversorDedo.Desconverte(*dedo));   // "Entidade está inválida" check per finger
        }
    }

    if (biometria.foto_isPresent()) {
        const auto foto = ecourna::app::dados::asn::CConversorFoto().Deconverte(biometria.get_foto());
        if (biometria.dedos_isPresent()) {
            return md::CBiometriaEleitor(foto, dedos);   // inline ctor: foto + CBiometriaEleitor::CriaComum(dedos)
        }
        return md::CBiometriaEleitor(foto);              // photo only
    }
    return md::CBiometriaEleitor(dedos);                 // fingers only: CriaComum(dedos)
}

} // namespace comum::asn
