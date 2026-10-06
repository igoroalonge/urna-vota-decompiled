// uenux2/src/app/comum/cpacotearquivos.h  (path inferred from cpacotearquivos.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u22).
//
// CPacoteArquivos = one "signature package" (a .vsu file) and the list of files it covers, checked
// through the urna's signing service (comum::IInterfaceSavd, "SAVD"). Non-polymorphic, 36 bytes:
//   +0  std::filesystem::path m_pacote          (full path of the .vsu)
//   +12 std::vector<std::string> m_arquivos     (file names covered, relative to the package dir)
//   +24 ESavdChaveValidar m_chave               (which key signed it, '1'..';')
//   +28 int m_tipo                              (0x1001 files listed / 0x2021 whole package)
//   +32 int m_aplicacao                         (1..9)
#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace comum {

class IInterfaceMensagem;   // progress/UI callback (CInterfaceMensagemVazia @1532504: all no-ops)
class IInterfaceSavd;

// Keys a SAVD signature can be verified against. Values are the ASCII digits the SAVD protocol uses;
// names are the strings returned by RetornaNomeChave (func 5900).
enum class ESavdChaveValidar : int {
    TSE = '1', SECAD = '2', SECINP = '3', SEVIN = '4', UE = '5', SCUE = '6',
    CLOGI = '7', CLOGI_CERT = '8', CLOGI_UPDATE = '9', PU = ':', SEINT = ';',
};

class CPacoteArquivos
{
public:
    // Only exists inlined (into func 4625).
    CPacoteArquivos(std::filesystem::path pacote, const std::string& arquivo, ESavdChaveValidar chave,
                    int aplicacao);

    void Validar(IInterfaceMensagem& mensagem) const;                       // srcloc :116 (inlined)
    static std::string RetornaNomeChave(ESavdChaveValidar chave);           // func 5900 (srcloc :162)

    // wasm func 4625 - the tools named it after the first inlined srcloc; the real name is not in the
    // binary. Verifies the signatures of the work-area files of one flash.             // name inferred
    static void ValidaAssinaturasArquivosTrabalho(const std::filesystem::path& diretorio);

private:
    void ValidarChaveEAplicacaoValida();                                    // srcloc :85, :91 (inlined)
    [[noreturn]] void TrataErroPacote(const IInterfaceSavd& savd) const;    // func 5901 (srcloc :169)
    [[noreturn]] void TrataErroPacoteArquivo(const IInterfaceSavd& savd,
                                             const std::string& arquivo) const;   // srcloc :190 (inlined)

    static constexpr int TIPO_ARQUIVOS = 0x1001;   // 4097: a list of files is signed      // names inferred
    static constexpr int TIPO_PACOTE   = 0x2021;   // 8225: the package as a whole
    static constexpr int TIPO_PACOTE_2 = 0x1011;   // 4113: also validated as a whole (never built here)

    std::filesystem::path    m_pacote;             // +0
    std::vector<std::string> m_arquivos;           // +12
    ESavdChaveValidar        m_chave;              // +24
    int                      m_tipo = TIPO_ARQUIVOS; // +28
    int                      m_aplicacao;          // +32
};

} // namespace comum
