// uenux2/mock/api/hwil/curnamock.h   (path inferred: namespace api::teste; the simulator's other mocks live in
//                                      uenux2/mock/app/..., mirroring uenux2/src/app/...; IUrna is api/hwil)
//
// Reconstructed from vota_web_wasm.wasm (unit u33).
//
// api::teste::CUrnaMock : api::IUrna      typeinfo @1530924, vtable @1530888, 28 bytes.
// Registered once by the simulator bootstrap (wasm func 8302, see
// src/uenux2/mock/app/simulador/wasm/csimuladorwasm.u19.cpp):
//     CPolySingletonList::push<api::IUrna>(std::make_unique<api::teste::CUrnaMock>(), info);
// The constructor is inlined there and stores the constants below (one i64 store = {2020, 87654321}, one
// i64 store = {255, 0}, one i64 store = {0, 0}).
//
// It replaces the urna's hardware identification and its secret tables with FIXED, PUBLIC values:
//   * model 2020 (so the web build always behaves as a UE2020: keypad "abaixo" in the audio instructions,
//     2020-specific fingerprint capture, MR port...);
//   * CEPESC table = 1024 x 0x01, 32-byte block = 32 x 0x02, RDV table = 128 x 0x03.
// Anything derived from these tables (the RDV key, an encrypted BU or fingerprint file) can therefore be
// recomputed by anyone. Expected in a simulator; documented in docs/modules/u33-*.md §2.1 and §9.
#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <format>
#include <string>
#include <vector>

#include "api/hwil/iurna.h"

namespace api::teste {

class CUrnaMock : public IUrna {
public:
    CUrnaMock() = default;                         // inlined in wasm func 8302

    // slot 0 - wasm func 1661 (ICF "return this->[+4]", shared with unrelated classes)
    int GetModelo() const override { return m_modelo; }

    // slot 1 - wasm func 8168                                                           // name inferred
    std::string GetModeloAbreviado() const override
    {
        return std::format("{:02}", m_modelo - 2000);          // "20"; the format string @8970 is shared with
                                                               // CGeradorBUBase::ImprimeConsulta/ImprimeMajoritario
    }

    // slot 2 - wasm func 2587 (ICF "return this->[+8]")                                // name inferred
    std::uint32_t GetNumeroInterno() const override { return m_numeroInterno; }

    // slot 3 - wasm func 3383 (ICF "return this->[+12]")                               // name inferred
    int GetRevisaoHardware() const override { return m_revisao; }

    // slot 4 - wasm func 8144                                                           // name inferred
    // Copies the configured table and pads the rest of the 1024 bytes with 0x01; with no table configured
    // (always, in this build: nothing ever fills m_tabelaCepesc) the result is 1024 bytes of 0x01.
    // NOTE: no bound check - a configured table longer than 1024 bytes would overflow the caller's buffer
    // (memcpy) and then trap in memory.fill with a negative length. Unreachable here.
    void GetTabelaCepesc(std::array<uebyte, 1024>& tabela) const override
    {
        if (m_tabelaCepesc.empty()) {
            std::fill(tabela.begin(), tabela.end(), uebyte{1});
            return;
        }
        std::copy(m_tabelaCepesc.begin(), m_tabelaCepesc.end(), tabela.data());
        std::fill(tabela.data() + m_tabelaCepesc.size(), tabela.data() + tabela.size(), uebyte{1});
    }

    // slot 5 - wasm func 8134: four i64 stores of 0x0202020202020202                  // name inferred
    void GetDados32(std::array<uebyte, 32>& dados) const override { dados.fill(uebyte{2}); }

    // slot 6 - wasm func 8131: memset(tabela, 3, 128)                                   // name inferred
    // This is the "hardware" table of the RDV key derivation (units u01/u02): key = HKDF(SHA-512(cargos),
    // 32 bytes picked from this table, "RDV"), i.e. the RDV key is a function of public data in this build.
    void GetTabelaRdv(std::array<uebyte, 128>& tabela) const override { tabela.fill(uebyte{3}); }

    // slot 7 - wasm func 8130 (complete destructor: frees m_tabelaCepesc, returns this)
    // slot 8 - wasm func 8129 (deleting destructor: the same + operator delete)
    ~CUrnaMock() override = default;

private:
    int                 m_modelo = 2020;            // +4
    std::uint32_t       m_numeroInterno = 87654321;  // +8
    int                 m_revisao = 255;            // +12  ?
    std::vector<uebyte> m_tabelaCepesc;             // +16  (begin +16, end +20, capacity +24) - always empty
};

} // namespace api::teste
