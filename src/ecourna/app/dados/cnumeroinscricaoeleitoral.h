// ecourna-lib/ecourna/app/dados/cnumeroinscricaoeleitoral.h, cnumerocpf.h, cnumeroidentificacaolivre.h
// (paths inferred; the three small declarations are kept together here)
// Reconstructed from vota_web_wasm.wasm, unit u14.
#pragma once

#include "ecourna/app/dados/iidentificadoreleitor.h"

namespace ecourna::app::dados {

// "título de eleitor". vtable @1122028: [0]/[1] trivial dtors (ICF 3492/3491), [2] func 9262.
class CNumeroInscricaoEleitoral : public IIdentificadorEleitor {
public:
    static constexpr uebyte TAMANHO = 12;
    explicit CNumeroInscricaoEleitoral(const std::string& numero);   // func 1241 (-> shared body 3939)
    std::string GetNumeroFormatado() const override;                 // func 9262: "NNNN NNNN NNNN"
};

// CPF. vtable @1121848: [2] func 9264.
class CNumeroCPF : public IIdentificadorEleitor {
public:
    static constexpr uebyte TAMANHO = 11;
    explicit CNumeroCPF(const std::string& numero);                  // func 5107 (-> 3939)
    std::string GetNumeroFormatado() const override;                 // func 9264: "NNN.NNN.NNN-NN"
};

// Free identification number. vtable @1121932: [2] func 9263.
class CNumeroIdentificacaoLivre : public IIdentificadorEleitor {
public:
    static constexpr uebyte TAMANHO = 12;
    explicit CNumeroIdentificacaoLivre(const std::string& numero);   // func 5106 (-> 3939)
    std::string GetNumeroFormatado() const override;                 // func 9263: unformatted
};

} // namespace ecourna::app::dados
