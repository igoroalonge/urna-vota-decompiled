// ecourna-lib/ecourna/api/security/cepesc/ccepesccipher.cpp   (path inferred)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40 (Encrypt). Decrypt (func 5171) was reconstructed by unit
// u03 inside cconversorbiometriaeleitorcifrada.cpp (its only caller) and is repeated here for completeness.
#include "ecourna/api/security/cepesc/ccepesccipher.hpp"

#include "ecourna/api/security/cepesc/ccipheredin.hpp"

// The accessors used below (CPlainText::GetConteudo/GetInfoSalt, CCipheredIn::GetConteudo) are inline field
// reads in the binary (+44, +56, +28); u01's headers do not declare them yet.            // names inferred

namespace ecourna::api::cepesc {

// wasm func 2681 (vtable slot 2). NOT an encryption in this build:
//  * the "session key" is 32 zero bytes (operator new(32) + four i64 zero stores);
//  * the content is CPlainText::m_conteudo (+44) copied unchanged;
//  * the optional CInfoSalt (+56, shared_ptr) is forwarded when present;
//  * tipoArquivo, idCriptografia, zona/seção, the 1024-byte UE crypto table, the random number and the public
//    key (+32) are ignored.
// CCipheredOut's constructors (9467 / 9466) only check that key and content are not empty (1504..1507).
// Callers: comum::CGravadorBU::GravaResultado (BU file envelope, func 11629), comum::CGravadorRCSecao
// (encrypted attendance, 11616), comum::CControlaArmazenamentoDeImagens (fingerprint images, 2725).
CCipheredOut CCepescCipher::Encrypt(const CPlainText& claro) const
{
    const std::vector<uebyte> chave(32, 0);
    if (claro.GetInfoSalt())                                                     // +56   name inferred
        return CCipheredOut(chave, claro.GetConteudo(), *claro.GetInfoSalt());   // wasm func 9466
    return CCipheredOut(chave, claro.GetConteudo());                             // wasm func 9467
}

// wasm func 5171 (vtable slot 3; unit u03): returns the ciphered content unchanged.
std::vector<uebyte> CCepescCipher::Decrypt(const CCipheredIn& cifrado) const
{
    return cifrado.GetConteudo();                                                // CCipheredIn +28
}

} // namespace ecourna::api::cepesc
