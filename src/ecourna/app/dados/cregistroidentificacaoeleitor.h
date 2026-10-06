// ecourna-lib/ecourna/app/dados/cregistroidentificacaoeleitor.h  (path inferred)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
// = ModuloTiposEcoUrna::RegistroIdentificacaoEleitor { identificacaoUtilizada IdentificadorEleitor,
//                                                      identificacaoPrincipal IdentificadorEleitor OPTIONAL }
// "Habilitação" identifier = the document actually used to enable the voter at the urna;
// "principal" identifier = the voter's main identifier in the roll (e.g. the título when the voter
// was found by CPF).
#pragma once

#include <optional>

#include "ecourna/app/dados/iidentificadoreleitor.h"

namespace ecourna::app::dados {

class CRegistroIdentificacaoEleitor {                                   // 20 bytes
public:
    explicit CRegistroIdentificacaoEleitor(TSharedIdentificadorEleitor habilitacao);            // func 1675
    CRegistroIdentificacaoEleitor(TSharedIdentificadorEleitor habilitacao,
                                  TSharedIdentificadorEleitor principal);                       // func 1878

    TSharedIdentificadorEleitor GetIdentificadorHabilitacao() const;   // func 2669
    TSharedIdentificadorEleitor GetIdentificadorPrincipal() const;     // func 9260
    bool PossuiIdentificadorPrincipal() const { return m_identificadorPrincipal.has_value(); }   // (inlined)

private:
    TSharedIdentificadorEleitor m_identificadorHabilitacao;                  // +0  (ptr, ctrl)
    std::optional<TSharedIdentificadorEleitor> m_identificadorPrincipal;     // +8  (engaged flag at +16)
};

} // namespace ecourna::app::dados
