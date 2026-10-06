// uenux2/src/api/io/cencryptedfile.h   (path inferred from cencryptedfile.cpp, attested by srclocs)
//
// Reconstructed from vota_web_wasm.wasm. Unit u18.
//
// api::CEncryptedFile: an in-memory byte buffer with a read/write cursor that is stored on disk encrypted
// with a symmetric cipher (the RDV key: CBlockCipher<CAesCipher, CTrng>, unit u01) and with a PKCS#7-like
// padding added by the class itself. Its only user in VOTA is the Registro Digital do Voto:
//   write: CSincronismoVotoEleitor::SincronizaVoto (7174): MemWrite(rdv.Converte()), Save(<trab>/rdv.dat.tmp)
//   check: Load(<trab>/rdv.dat.tmp), MemRead(buf), CRdvVota::ConfereConteudo(buf)
//   boot : CEleitores::CompleteLoad (6734): Load(<trab MI>/rdv.dat), MemRead, CRdvVota::Desconverte
//
// Not polymorphic, 24 bytes:
//   +0  std::shared_ptr<ecourna::api::security::ISymmetricCipher> m_cifrador   (+0 pointer, +4 control block)
//   +8  std::vector<uebyte> m_dados                                            (clear text)
//   +20 uedword m_posicao                                                      (cursor for MemRead/MemWrite)
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ecourna/api/security/isymmetriccipher.hpp"

namespace api {

using uebyte  = unsigned char;
using uedword = unsigned int;

class CEncryptedFile {
public:
    explicit CEncryptedFile(std::shared_ptr<ecourna::api::security::ISymmetricCipher> cifrador)   // func 1692 (not u18)
        : m_cifrador(std::move(cifrador)) {}

    void Load(const std::string& arquivo);                        // wasm 3653 (srcloc 36..73)
    void Save(const std::string& arquivo) const;                  // wasm 2767 (srcloc 88, unit u12 fragment)

    uedword MemWrite(const void* buffer, uedword tamanho);        // lines 142/145, inlined in 2766
    uedword MemRead(void* buffer, uedword tamanho);               // lines 161/164/167, inlined in 3652
    void MemWrite(const std::vector<uebyte>& dados);              // wasm 2766 (overload name inferred)
    void MemRead(std::vector<uebyte>& destino);                   // wasm 3652 (overload name inferred)

private:
    std::shared_ptr<ecourna::api::security::ISymmetricCipher> m_cifrador;   // +0
    std::vector<uebyte> m_dados;                                             // +8
    uedword m_posicao = 0;                                                   // +20
};

} // namespace api
