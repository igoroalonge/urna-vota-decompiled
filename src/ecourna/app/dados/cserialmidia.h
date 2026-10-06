// ecourna-lib/ecourna/app/dados/cserialmidia.h  (path inferred)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
// Serial number of a flash medium (DadosGeracaoMidia.serialMidia): 8 hexadecimal characters,
// stored as 4 bytes. In all eight simulator files infomidia-fv-{1,2}-t.dat it is "A1B2C3DB".
#pragma once

#include <string>
#include <vector>

#include "ecourna/app/dados/tiposbasicos.h"

namespace ecourna::app::dados {

class CSerialMidia {                                // 12 bytes
public:
    explicit CSerialMidia(const std::string& serial);                      // func 9259
    const std::vector<uebyte>& GetSerial() const { return m_serial; }      // (inlined)

private:
    std::vector<uebyte> m_serial;                   // +0
};

} // namespace ecourna::app::dados
