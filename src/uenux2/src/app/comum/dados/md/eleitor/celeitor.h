// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/eleitor/celeitor.h
//
// CEleitor = static voter record of the section's electoral roll (ModuloEleitores::EleitorSequencia /
// EleitorUrna in the *-el.dat files). sizeof == 104. Field meaning follows the ASN.1 order; offsets
// marked "?" are only known from copy/move code (funcs 337, 946, 379).
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "celeitoridentidade.h"

namespace comum::md {

class CEleitor {
public:
    CEleitor(const CEleitor&) = default;              // wasm func 946 (member-wise copy, 104 bytes)
    CEleitor& operator=(CEleitor&&) noexcept = default;   // wasm func 337 (member-wise move)
    ~CEleitor() = default;                            // shared_f379 (ICF, not in this unit)

    void ValidaCriacao() const;                       // wasm func 5667, celeitor.cpp:103..113

    const std::vector<CEleitorIdentidade>& GetIdentidades() const { return m_identidades; }

private:
    int m_sequencial;                                  // +0   (0..9999)
    std::uint16_t m_secao;                             // +4   seção where the voter is listed: CEleitorDetalhe::
                                                       //      GetBiometria (1937) passes it (b[2]:ushort) with the
                                                       //      configured município/zona to build the roll file name
    std::vector<CEleitorIdentidade> m_identidades;     // +8   (identificacaoEleitor: título, CPF, ...)
    int m_necessidadeEspecial;                         // +20  (ValidaCriacao rejects >= 2, signed compare)
    std::string m_nome;                                // +24
    std::string m_nomeSocial;                          // +36  (empty = none)
    int m_tipoTransferenciaTemporaria;                 // +48  0 = none; GetBiometria switches on it: 0 and 4
                                                       //      read the -el.dat roll, other values the -tte.dat roll
    std::string m_uf;                                  // +52 ? (domicile UF, u04 doc)
    int m_municipio;                                   // +64 ? (domicile município, u04 doc)
    std::string m_dataNascimento;                      // +68  "YYYYMMDD" (confirmed by GetAnoNascimento, u13)
    // +80..+103: where the voter's encrypted biometrics are in the roll file. GetBiometria copies
    // +80 (byte vector) and +92 (8 bytes) as the CIndexer returned by GetOffsetBiometria
    // (celeitor.cpp:79), which throws 8063 "Não há informação de biometria" when +100 is false.
    // Copy (946) and destructor (379) handle +80 unconditionally, so it is not a std::optional.
    std::vector<std::uint8_t> m_indexerBiometria;      // +80 ?
    std::uint64_t m_offsetTamanhoBiometria;            // +92 ?
    bool m_possuiBiometria;                            // +100 (also tested by the photo code, func 3616)
};

}  // namespace comum::md
