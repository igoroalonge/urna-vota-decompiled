// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/iregraidentidade.h  (file name from srcloc iregraidentidade.cpp:23)
//
// RTTI: comum::md::IRegraIdentidade (abstract)
//         ├── comum::md::CRegraTitulo            vtable @1574700  (cregratitulo.cpp, path inferred)
//         ├── comum::md::CRegraCPF               vtable @1574740  (cregracpf.cpp, path inferred)
//         └── comum::md::CRegraIdentidadeLivre   vtable @1574652  (unit u35)
// Slots: 0 ~dtor (trivial, ICF 174)  1 deleting dtor  2 GetTipo  3 Formata  4 Verifica
#pragma once

#include <string>

#include "eleitor/celeitoridentidade.h"

namespace comum::md {

class IRegraIdentidade {
public:
    virtual ~IRegraIdentidade() = default;
    virtual ETipoIdentificadorEleitor GetTipo() const = 0;                     // slot 2
    virtual std::string Formata(const std::string& identidade) const = 0;      // slot 3 name inferred
    virtual bool Verifica(const std::string& semZerosEsquerda) const = 0;      // slot 4 name inferred (check digits)

    bool EhValida(const std::string& identidade) const;    // func 3723  name inferred
    void Valida(const std::string& identidade) const;      // iregraidentidade.cpp:23 (inlined into 566)
};

// Concrete rules of this unit (one class per file in the original tree)
class CRegraTitulo final : public IRegraIdentidade {
public:
    ETipoIdentificadorEleitor GetTipo() const override { return ETipoIdentificadorEleitor::TITULO; } // ICF 434 "return 1"
    std::string Formata(const std::string& identidade) const override;   // func 11271
    bool Verifica(const std::string& identidade) const override;         // func 11272
};

class CRegraCPF final : public IRegraIdentidade {
public:
    ETipoIdentificadorEleitor GetTipo() const override { return ETipoIdentificadorEleitor::CPF; }    // ICF 2254 "return 2"
    std::string Formata(const std::string& identidade) const override;   // func 11269
    bool Verifica(const std::string& identidade) const override;         // func 11270
};

}  // namespace comum::md
