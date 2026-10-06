// ecourna-lib/ecourna/app/dados/iidentificadoreleitor.h  (path inferred from the .cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
// How a voter (or a mesário) is identified: the "título de eleitor" number (número de inscrição
// eleitoral, 12 digits), the CPF (11 digits) or a free identification number (12 digits).
// RTTI: IIdentificadorEleitor (polymorphic, no base) <- CNumeroInscricaoEleitoral, CNumeroCPF,
// CNumeroIdentificacaoLivre (all "si"). Objects are handled through std::shared_ptr
// (make_shared control blocks __shared_ptr_emplace<CNumero...> exist for the three classes).
#pragma once

#include <memory>
#include <string>

#include "ecourna/app/dados/tiposbasicos.h"

namespace ecourna::app::dados {

// vtable @1122204: [0] ~IIdentificadorEleitor (func 778), [1] deleting dtor (ICF 325),
//                  [2] pure virtual (__cxa_pure_virtual) = GetNumeroFormatado.
class IIdentificadorEleitor {                                          // 24 bytes
public:
    virtual ~IIdentificadorEleitor();                                  // wasm func 778

    // Slot 2. Number formatted for display/printing. (name inferred)
    virtual std::string GetNumeroFormatado() const = 0;

    TTipoIdentificadorEleitor GetTipo() const { return m_tipo; }      // inlined (+4)
    // Returns a copy of m_numero. Out-of-line body is ICF-merged with api::CFixedText::vf2
    // (func 1139, "icf_shared_vf2@1139"), which is why several classes seem to call a GUI method.
    std::string GetNumero() const { return m_numero; }
    uebyte GetTamanho() const { return m_tamanho; }                   // inlined (+20)

protected:
    IIdentificadorEleitor(TTipoIdentificadorEleitor tipo, const std::string& numero, uebyte tamanho);   // func 9256

    // +0 vptr
    TTipoIdentificadorEleitor m_tipo;                                  // +4
    std::string m_numero;                                              // +8   left-padded with '0' to m_tamanho
    uebyte m_tamanho;                                                  // +20  maximum number of digits
};

using TSharedIdentificadorEleitor = std::shared_ptr<IIdentificadorEleitor>;

// A distinct class in the RTTI (template argument of IConversorASN<IdentificadorEleitor,
// CIdentificadorEleitor>), 8 bytes, whose first word is the IIdentificadorEleitor pointer:
// a thin wrapper around TSharedIdentificadorEleitor. ? exact declaration unknown.
class CIdentificadorEleitor : public TSharedIdentificadorEleitor {
public:
    using TSharedIdentificadorEleitor::TSharedIdentificadorEleitor;
    CIdentificadorEleitor(TSharedIdentificadorEleitor p) : TSharedIdentificadorEleitor(std::move(p)) {}
};

} // namespace ecourna::app::dados
