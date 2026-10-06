// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/eleitor/cbiometriaeleitor.h
#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

#include "cdedo.h"                          // comum::md::CDedo (20 bytes)
#include "ecourna/app/dados/cfoto.h"         // ecourna::app::dados::CFoto (wraps the JPEG bytes)

namespace comum::md {

class CBiometriaEleitor {
public:
    void CriaComum(std::vector<CDedo> dedos);                     // func 2799
    CDedo GetDedo(CDedo::TipoDedo tipo) const;                    // func 3720
    const ecourna::app::dados::CFoto& GetFoto() const;            // inlined (line 115)
    bool PossuiFoto() const { return m_foto.has_value(); }
    int GetEstadoDecifracao() const { return m_estadoDecifracao; }   // name inferred

private:
    std::optional<ecourna::app::dados::CFoto> m_foto;             // +0 (CFoto {int formato; vector imagem +4},
                                                                  //     engaged +16)
    std::map<CDedo::TipoDedo, CDedo> m_dedos;                     // +20 (a flag at +32 marks "has fingers")
    int m_estadoDecifracao;                                       // +36 ?
};

}  // namespace comum::md
