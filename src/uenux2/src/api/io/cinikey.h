// uenux2/src/api/io/cinikey.h   (path inferred from cinikey.cpp)
//
// Reconstructed from vota_web_wasm.wasm. Unit u18.
#pragma once

#include <string>

namespace api {

class CIniKey {
public:
    CIniKey(std::string nome, std::string valor) : m_nome(std::move(nome)), m_valor(std::move(valor)) {}   // inlined

    CIniKey& operator=(const CIniKey& outra);            // wasm 5483 (cinikey.cpp:24)

    const std::string& GetNome() const  { return m_nome; }    // names inferred
    const std::string& GetValor() const { return m_valor; }

private:
    std::string m_nome;    // +0
    std::string m_valor;   // +12
};

} // namespace api
