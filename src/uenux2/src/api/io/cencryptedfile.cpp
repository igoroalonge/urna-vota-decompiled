// uenux2/src/api/io/cencryptedfile.cpp
//
// Reconstructed from vota_web_wasm.wasm. Unit u18 (Save, wasm 2767, is in cencryptedfile.u12-fragment.cpp).
//
// Error codes: api::EUeIoError (CBaseError limits {5950, 6150}):
//   5987 :36 nome vazio          5988 :39 não existe         5989 :44 tamanho inválido (NOT THROWN)
//   5990 :51 falha ao ler (NOT THROWN)                        5991 :59 padding inválido (> 16)
//   5992 :65 padding excessivo   5993 :73 padding secundário (NOT THROWN)
//   5994 :88 (Save) nome vazio   5996/5997 :142/:145 MemWrite   5998/5999/6000 :161/:164/:167 MemRead
//
// IMPORTANT (found in the binary): at lines 44, 51 and 73 the code builds the exception object and
// destroys it without throwing (the wasm constructs a CBaseError<EUeIoError> in a stack temporary with
// ecourna_f1143 and immediately runs its destructor; there is no __cxa_allocate_exception/__cxa_throw).
// In C++ that is a statement of the form `CUeIoError(...);` - a missing `throw`. So Load() does NOT reject
// a file whose size is not a multiple of 16, a short read, or padding bytes that differ from the pad
// length. See the "suspicious" list of the u18 doc. (A 0-byte file is still rejected, but by the next
// layer: the empty `cifrado` vector has no buffer, so CFile::RawRead(nullptr, 0) throws
// ecourna::api::io::CIoError 1192 "buffer invalido" - not EUeIoError 5989.)
#include "api/io/cencryptedfile.h"

#include <cstring>
#include <format>

#include "api/io/euioerror.h"            // api::EUeIoError, api::CUeIoError
#include "api/util/csystem.h"            // api::CSystem::IsRegularFile (412), GetFileSize (2759)
#include "ecourna/api/io/cfile.hpp"

namespace api {

namespace {
constexpr uebyte TAMANHO_BLOCO = 16;     // AES block; the class pads to 16 bytes itself   name inferred
}

// wasm func 3653 (srcloc lines 36, 39, 44, 51, 59, 65, 73)
void CEncryptedFile::Load(const std::string& arquivo)
{
    if (arquivo.empty())
        throw CUeIoError(EUeIoError(5987), "Nome de arquivo vazio");                                    // line 36

    if (!CSystem::IsRegularFile(arquivo))
        throw CUeIoError(EUeIoError(5988), std::format("Arquivo [{}] não existe", arquivo));             // line 39

    const uedword tamanho = CSystem::GetFileSize(arquivo);
    if (tamanho == 0 || tamanho % TAMANHO_BLOCO != 0)
        CUeIoError(EUeIoError(5989), std::format("Arquivo [{}] com tamanho inválido", arquivo));        // line 44 - no throw!

    ecourna::api::io::CFile file(arquivo, "rb");                                                        // wasm 517
    std::vector<uebyte> cifrado(tamanho);                                                               // zero-filled
    // wasm 598. With tamanho == 0 the data pointer is null and RawRead throws CIoError 1192 ("buffer invalido").
    if (file.RawRead(cifrado.data(), tamanho) != tamanho)
        CUeIoError(EUeIoError(5990), std::format("Falha ao ler arquivo [{}]", arquivo));                // line 51 - no throw!

    std::vector<uebyte> claro;
    m_cifrador->Decrypt(cifrado, claro);                            // ISymmetricCipher vtable slot 3 (u01)

    const uebyte padding = claro.back();                            // no emptiness check (wasm: null data -> reads
                                                                    // address 0xFFFFFFFF -> trap; else the heap byte before)
    if (padding > TAMANHO_BLOCO)
        throw CUeIoError(EUeIoError(5991), std::format("Tamanho de padding inválido em [{}]", arquivo));    // line 59
    if (padding > tamanho)                                          // compares with the *file* size
        throw CUeIoError(EUeIoError(5992), std::format("Tamanho de padding excessivo em [{}]", arquivo));   // line 65

    for (uebyte i = 1; i < padding; ++i) {
        if (claro[claro.size() - 1 - i] != padding)
            CUeIoError(EUeIoError(5993),
                       std::format("Tamanho de padding secundário inválido em [{}]", arquivo));        // line 73 - no throw!
    }

    // A padding byte of 0 is accepted and removes nothing.
    m_dados.assign(claro.begin(), claro.end() - padding);
    m_posicao = 0;
}   // ~CFile closes the file

// Inlined in wasm 2766 (srcloc lines 142, 145)
uedword CEncryptedFile::MemWrite(const void* buffer, const uedword tamanho)
{
    if (tamanho == 0)
        throw CUeIoError(EUeIoError(5996), "Tamanho inválido");                                         // line 142
    if (buffer == nullptr)
        throw CUeIoError(EUeIoError(5997), "Buffer inválido");                                          // line 145

    if (m_posicao + tamanho > m_dados.size())
        m_dados.resize(m_posicao + tamanho);                                                            // grows with zeros
    std::memcpy(m_dados.data() + m_posicao, buffer, tamanho);
    m_posicao += tamanho;
    return tamanho;
}

// wasm func 2766 (overload name inferred): an empty vector is ignored. Callers: the RDV writers
// (unknown_f5736 / unknown_f5737, CSincronismoVotoEleitor::SincronizaVoto 7174).
void CEncryptedFile::MemWrite(const std::vector<uebyte>& dados)
{
    if (dados.empty())
        return;
    MemWrite(dados.data(), static_cast<uedword>(dados.size()));
}

// Inlined in wasm 3652 (srcloc lines 161, 164, 167). Returns 0 (and reads nothing) when fewer than
// `tamanho` bytes remain after the cursor.
uedword CEncryptedFile::MemRead(void* buffer, const uedword tamanho)
{
    if (tamanho == 0)
        throw CUeIoError(EUeIoError(5998), "Tamanho inválido");                                         // line 161
    if (buffer == nullptr)
        throw CUeIoError(EUeIoError(5999), "Buffer inválido");                                          // line 164
    if (m_dados.empty() || m_dados.size() < tamanho)
        throw CUeIoError(EUeIoError(6000), "Não há nada para ler");                                     // line 167

    if (m_dados.size() < m_posicao + tamanho)
        return 0;
    std::memcpy(buffer, m_dados.data() + m_posicao, tamanho);
    m_posicao += tamanho;
    return tamanho;
}

// wasm func 3652 (overload name inferred): appends everything after the cursor to `destino`.
// The new bytes are first filled with 0xFF, then overwritten by MemRead.
// Callers: CEleitores::CompleteLoad (6734), CSincronismoVotoEleitor::SincronizaVoto (7174).
void CEncryptedFile::MemRead(std::vector<uebyte>& destino)
{
    const uedword restante = static_cast<uedword>(m_dados.size()) - m_posicao;
    if (restante == 0)
        return;
    const std::size_t inicio = destino.size();
    destino.resize(inicio + restante, 0xFF);
    MemRead(destino.data() + inicio, restante);
}

} // namespace api
