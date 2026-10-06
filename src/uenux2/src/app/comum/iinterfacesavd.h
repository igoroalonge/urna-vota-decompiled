// uenux2/src/app/comum/iinterfacesavd.h
// Reconstructed from vota_web_wasm.wasm (unit u23).
//
// IInterfaceSavd = client of SAVD, the urna's signing/validation daemon ("Serviço de Assinatura e Validação
// Digital", name inferred). The application never holds keys: to sign a file, open/close an HSM session,
// or validate a package signature it sends a binary request to SAVD and reads the answer.
//
// RTTI: comum::IInterfaceSavd (vtable @1526736)
//         └ (anonymous namespace)::CWasmSavd (vtable @1526688, web build; registered by main): EnviaMensagem is a
//           no-op and RecebeMensagem always answers the 12-byte "OK" header {0xFE, 0, 0...} (func 10949), so every
//           request "succeeds" and nothing is ever signed.
//
// Wire format (little-endian, reconstructed from funcs 3829 / 3830 and CWasmSavd):
//   request header  (8 bytes):  uebyte marca = 0xFE; uebyte aplicacao (ESavdAplic); ueint16 comando;
//                               ueint32 tamanhoDados
//   request payload (tamanhoDados bytes, optional)
//   answer header  (12 bytes):  uebyte marca = 0xFE; uebyte erro (0 = OK); 2 bytes padding; ueint32 codigo;
//                               ueint32 tamanhoMensagem
//   answer message (tamanhoMensagem bytes, only when erro != 0): uebyte marca = 0xFE, then the text
//
// Commands used by the VOTA application (header "comando" field):
//   0x0042 (66)   sign a file for the UE (AssinarUE)        payload {0xFE, '=', arquivo}
//   0x0080 (128)  sign a result file for Ecourna (.vsc)     payload {0xFE, '=', arquivo}    (AssinarEcourna, u07)
//   0x0404        define the "local" (UF+mun+zona+seção)    payload {0xFE, local[15]}       (inlined, u07)
//   0x1604        HSM action (open/close session)           payload {0xFE, acao}
//   0x2021        validate the UE signature of a package    payload = TLV built by api_f3831(pacote, 53)
#pragma once

#include <string>
#include <vector>

#include "comum/comumdefs.h"

namespace comum {

enum class ESavdAplic : uebyte;          // 1 = VOTA (the only value seen)                        // ?
enum class ESavdCmdHSM : uebyte { ABRE_SESSAO = 0, FECHA_SESSAO = 1 };                            // names inferred
enum class ESavdArquivoUE : ueint32;     // SAVD id of a file (vota.bin, rdv.dat, bu.dat ... see CArquivosSavd)

#pragma pack(push, 1)
struct ueRequisicaoSavd { uebyte marca; uebyte aplicacao; ueint16 comando; ueint32 tamanho; };            // 8 bytes
struct ueRespostaSavd   { uebyte marca; uebyte erro; uebyte pad[2]; ueint32 codigo; ueint32 tamanho; };   // 12 bytes
#pragma pack(pop)

class IInterfaceSavd {
public:
    virtual ~IInterfaceSavd();                                                          // slot 0 func 5503
    virtual void EnviaMensagem(const std::vector<uebyte>& dados) = 0;                   // slot 2
    virtual void RecebeMensagem(ueint32& estado, std::vector<uebyte>& dados, std::size_t tamanho) = 0;   // slot 3
    virtual ueint32 Slot4() = 0;          // slot 4 (CWasmSavd: return 0x0CABECA0)                 // ?

    // Throwing variant: protocol errors throw (DesconverteHeader/DesconverteMensagem are inlined). The error
    // code goes to an explicit out-parameter (wasm args: this, &codigo, &requisicao, &dados); all three callers
    // (5890 ValidarUE, 5892, 4625 CPacoteArquivos::ValidarChaveEAplicacaoValida) pass &m_codigoErro (this +16).
    bool EnviaRequisicao(ueint32& codigo, ueRequisicaoSavd& requisicao, const std::string& dados);   // func 3830 (name inferred)
    // Non-throwing variant: returns the error text ("" = success).
    static std::string EnviaComando(IInterfaceSavd& savd, ueint32& codigo, ueRequisicaoSavd& requisicao,
                                    const std::vector<uebyte>& dados);                   // func 3829 (name inferred)
    bool AssinaArquivo(ESavdAplic aplicacao, ueint32 comando, ueint32 arquivo);        // func 5891 (name inferred, u07)

    static const ueRespostaSavd* DesconverteHeader(const std::vector<uebyte>& header);  // inlined (:553, :559)
    static std::string DesconverteMensagem(const ueRespostaSavd* header, const std::vector<uebyte>& msg);   // (:568..:578)

    const std::string& GetUltimoErro() const { return m_mensagem; }
    ueint32 GetCodigoErro() const { return m_codigoErro; }

protected:
    std::string m_mensagem;       // +4   last error text
    ueint32 m_codigoErro = 0;     // +16  last error code
};

void AssinarUE(IInterfaceSavd& savd, ESavdAplic aplicacao, ueint32 comando, ueint32 arquivo);   // :1004 (inlined in 1277)
void ValidarUE(IInterfaceSavd& savd, ESavdAplic aplicacao, const std::string& pacote);        // func 5890 (:886)
void AssinarEcourna(IInterfaceSavd& savd, ESavdAplic aplicacao, ueint32 comando, ueint32 arquivo);  // :1022 (u07)
void EnviarAcaoHSM(IInterfaceSavd& savd, ESavdAplic aplicacao, ESavdCmdHSM acao);             // func 5889 (:1040)

}  // namespace comum
