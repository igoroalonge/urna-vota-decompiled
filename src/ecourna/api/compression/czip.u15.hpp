// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original: ecourna-lib/ecourna/api/compression/czip.hpp / icompressor.hpp (the rest of CZip -
// Close, DoAdd, ConvertToCompressLevel, destructors - is reconstructed by unit u12).
#pragma once

#include <filesystem>
#include <map>

#include "ecourna/api/exception/cbaseerror.hpp"
#include "ecourna/api/pattern/iobservableprogresswithdescription.hpp"
#include "ecourna/api/pattern/noncopyable.hpp"

typedef void* zipFile;   // minizip

namespace ecourna::api::compression {

// Error codes seen in CZip (vtable CBaseError<ECompressionError> @1110656, typeinfo @1108484,
// built by the merged thunk ecourna_f9584 -> shared body ecourna_f1011).
enum class ECompressionError : int {
    ModoAberturaIncorreto = 1040,   // "Modo de abertura do zip incorreto."          name inferred
    ArquivoNaoCriado      = 1041,   // "O arquivo {} não pode ser criado."             name inferred
};
using CCompressionError = exception::CBaseError<ECompressionError>;

// Compression level enum mapped by CZip::ConvertToCompressLevel (table @1110872 = {0, 1, -1, 9}).
enum class ECompressLevel : int { Armazenar = 0, Rapido = 1, Padrao = 2, Maximo = 3 };   // names inferred

// ICompressor (typeinfo @1110916, vmi: NonCopyable + IObservableProgressWithDescription;
// vtable @1110896) - 16 bytes:
//   +0  vptr
//   +4  boost::signals2::signal<void(unsigned long, unsigned long, const std::string&)> m_progresso
//       (vptr +4, pimpl shared_ptr +8/+12)  - inherited from IObservableProgressWithDescription
// Virtuals: 0/1 dtor, 2 Close(), 3 bool (ICF "return 1"), 4 DoAdd(const path&, const path&).
class ICompressor : public pattern::NonCopyable, public pattern::IObservableProgressWithDescription {
public:
    ICompressor();                                                           // func 5192 (u12)
    virtual ~ICompressor() = default;
    virtual void Close() = 0;                                                // slot 2
    virtual bool IsOpen() const = 0;                                         // slot 3 ?  (CZip: icf "return 1")
    ICompressor& Add(const std::map<std::filesystem::path, std::filesystem::path>& arquivos);   // func 9529
protected:
    virtual void DoAdd(const std::filesystem::path& origem,
                       const std::filesystem::path& destino) = 0;           // slot 4
};

// CZip (typeinfo @1110824, vtable @1110676) - 40 bytes:
//   +16 std::filesystem::path m_arquivo     (the .jez file)
//   +28 zipFile m_zip                       (minizip handle, nullptr when closed)
//   +32 ECompressLevel m_nivel
//   +36 int m_modoAbertura                  (0 = create, 1 = add to existing zip)
class CZip final : public ICompressor {
public:
    CZip(const std::filesystem::path& arquivo, ECompressLevel nivel);       // func 5193
    ~CZip() override;                                                        // funcs 2690/9540 (u12)
    void Close() override;                                                   // func 2691 (u12), czip.cpp:139
    bool IsOpen() const override { return true; }                            // func 434 (ICF)
protected:
    void DoAdd(const std::filesystem::path& origem,
               const std::filesystem::path& destino) override;               // func 9538 (u12)
private:
    int  ConvertToOpen();                                                    // inlined in 9543, czip.cpp:155
    void CreateZipFile();                                                    // func 9543, czip.cpp:170
    void AssertZipIsOpened();                                                // inlined in 9538, czip.cpp:188
    int  ConvertToCompressLevel();                                           // func 9539 (u12), czip.cpp:207

    std::filesystem::path m_arquivo;   // +16
    zipFile m_zip = nullptr;           // +28
    ECompressLevel m_nivel;            // +32
    int m_modoAbertura = 0;            // +36
};

} // namespace ecourna::api::compression
