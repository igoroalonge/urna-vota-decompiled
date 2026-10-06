// uenux2/src/app/comum/iinterfacesavd.cpp
// Reconstructed from vota_web_wasm.wasm (unit u23). See iinterfacesavd.h for the wire format.
// Error class: CUeComumError (CBaseError<EUeComumError, {7200, 7600}>), codes 7305..7340.
#include "comum/iinterfacesavd.h"

#include <format>

#include "api/generictags/cgenerictags.h"

namespace comum {

// wasm func 5503 (slot 0; also slot 0 of CWasmSavd, whose deleting destructor is func 10965)
IInterfaceSavd::~IInterfaceSavd() = default;     // destroys m_mensagem

// Inlined into 3830 (srcloc :553, :559)
const ueRespostaSavd* IInterfaceSavd::DesconverteHeader(const std::vector<uebyte>& header)
{
    if (header.size() != sizeof(ueRespostaSavd))
        throw CUeComumError(7305, std::format("Header com tamanho incorreto: {}/{}", header.size(),
                                              sizeof(ueRespostaSavd)));                       // :553
    const auto* resposta = reinterpret_cast<const ueRespostaSavd*>(header.data());
    if (resposta->marca != 0xFE)
        throw CUeComumError(7306, "Header com marca incorreta");                             // :559
    return resposta;
}

// Inlined into 3830 (srcloc :568, :574, :578)
std::string IInterfaceSavd::DesconverteMensagem(const ueRespostaSavd* header, const std::vector<uebyte>& msg)
{
    if (msg.size() <= 1)
        throw CUeComumError(7307, std::format("Mensagem com tamanho incorreto: {}/{}", msg.size(), 2));   // :568 ?
    if (msg[0] != 0xFE)
        throw CUeComumError(7308, "Mensagem com marca incorreta");                           // :574
    if (msg.size() < header->tamanho)
        throw CUeComumError(7309, std::format("Buffer com tamanho insuficiente para mensagem: {}/{}",
                                              msg.size(), header->tamanho));                   // :578
    return std::string(msg.begin() + 1, msg.begin() + header->tamanho);
}

// wasm func 3830 (name inferred; the tools used the inlined DesconverteMensagem). Observed executing once, called
// by func 5892 (tools: "api::CGenericTags::WalkTreeTLV") during start-up; other callers: ValidarUE (5890) and
// CPacoteArquivos::ValidarChaveEAplicacaoValida (4625).
// Sends header + payload, reads the 12-byte answer; on error stores the code (through `codigo`) and the service's
// message (m_mensagem).
bool IInterfaceSavd::EnviaRequisicao(ueint32& codigo, ueRequisicaoSavd& requisicao, const std::string& dados)
{
    m_mensagem.clear();
    requisicao.marca = 0xFE;
    requisicao.tamanho = static_cast<ueint32>(dados.size());

    const auto* bytes = reinterpret_cast<const uebyte*>(&requisicao);
    EnviaMensagem(std::vector<uebyte>(bytes, bytes + sizeof(requisicao)));                // slot 2
    if (!dados.empty())
        EnviaMensagem(std::vector<uebyte>(dados.begin(), dados.end()));

    ueint32 estado = 0;
    std::vector<uebyte> header;
    RecebeMensagem(estado, header, sizeof(ueRespostaSavd));                               // slot 3
    const ueRespostaSavd* resposta = DesconverteHeader(header);
    const bool erro = resposta->erro != 0;
    if (erro) {
        codigo = resposta->codigo;                                                     // written before reading the text
        if (resposta->tamanho != 0) {
            std::vector<uebyte> msg;
            RecebeMensagem(estado, msg, resposta->tamanho);
            m_mensagem = DesconverteMensagem(resposta, msg);
        } else {
            m_mensagem = "Motivo não informado pelo serviço.";
        }
    }
    return !erro;
}

// wasm func 3829 (name inferred; observed executing). Same exchange without exceptions: protocol errors
// become the returned text. "" = success.
std::string IInterfaceSavd::EnviaComando(IInterfaceSavd& savd, ueint32& codigo, ueRequisicaoSavd& requisicao,
                                         const std::vector<uebyte>& dados)
{
    requisicao.marca = 0xFE;
    requisicao.tamanho = static_cast<ueint32>(dados.size());
    const auto* bytes = reinterpret_cast<const uebyte*>(&requisicao);
    savd.EnviaMensagem(std::vector<uebyte>(bytes, bytes + sizeof(requisicao)));
    if (!dados.empty())
        savd.EnviaMensagem(dados);

    ueint32 estado = 0;
    std::vector<uebyte> header;
    savd.RecebeMensagem(estado, header, sizeof(ueRespostaSavd));
    if (header.size() != sizeof(ueRespostaSavd))
        return "Resposta com tamanho incorreto.";
    const auto* resposta = reinterpret_cast<const ueRespostaSavd*>(header.data());
    if (resposta->marca != 0xFE)
        return "Resposta com marca incorreta.";
    if (resposta->erro == 0)
        return "";
    codigo = resposta->codigo;
    if (resposta->tamanho == 0)
        return "Motivo não informado pelo serviço.";
    std::vector<uebyte> msg;
    savd.RecebeMensagem(estado, msg, resposta->tamanho);
    if (msg[0] != 0xFE)                        // NO size check: msg[0] and msg[1..tamanho) are read even when the
                                               // service sent fewer bytes (doc "suspicious" #4)
        return "Mensagem com marca incorreta.";
    return std::string(msg.begin() + 1, msg.begin() + resposta->tamanho);
}

// wasm func 5891 (name inferred: u07 calls it AssinaArquivo). Observed executing (votaInit signs its files).
bool IInterfaceSavd::AssinaArquivo(ESavdAplic aplicacao, ueint32 comando, ueint32 arquivo)
{
    if (comando > 0xFFFF || static_cast<ueint32>(aplicacao) > 0xFF || arquivo >= 256) {
        m_mensagem = "Argumento inválido validar assinar.";
        return false;
    }
    ueRequisicaoSavd requisicao{0, static_cast<uebyte>(aplicacao), static_cast<ueint16>(comando), 0};
    const std::vector<uebyte> dados{0xFE, '=', static_cast<uebyte>(arquivo)};
    m_mensagem = EnviaComando(*this, m_codigoErro, requisicao, dados);
    return m_mensagem.empty();
}

// iinterfacesavd.cpp:1004, inlined into CAssinador::Assina (wasm func 1277, cassinador.cpp).
void AssinarUE(IInterfaceSavd& savd, ESavdAplic aplicacao, ueint32 comando, ueint32 arquivo)
{
    if (!savd.AssinaArquivo(aplicacao, comando, arquivo))
        throw CUeComumError(7336, std::format("Falha ao assinar ({}-UE[{}])", savd.GetUltimoErro(), arquivo));   // :1004
}

// wasm func 5890 (srcloc :886). The aplicacao argument was constant-propagated (1) by LTO.
// Callers: vota::VerificaAssinaturaMV / VerificaAssinaturaMI.
void ValidarUE(IInterfaceSavd& savd, ESavdAplic aplicacao, const std::string& pacote)
{
    bool ok = false;
    if (pacote.empty()) {
        savd.m_mensagem = "Argumento inválido validar assinar o pacote.";
    } else {
        const std::string tlv = api::MontaTLVPacote(pacote, 53);                  // api_f3831 (CGenericTags, "sup" ...)
        ueRequisicaoSavd requisicao{0, static_cast<uebyte>(aplicacao), 0x2021, 0};
        ok = savd.EnviaRequisicao(savd.m_codigoErro, requisicao, tlv);
    }
    if (!ok)
        throw CUeComumError(7323, std::format("Falha ao validar assinatura UE\npacote: ({})\nErro SAVD: ({})\n{}",
                                              pacote, savd.GetCodigoErro(), savd.GetUltimoErro()));   // :886
}

// wasm func 5889 (srcloc :1040). Called by CGravaResultado (12098) through CAssinador::AssinaArquivosResultado.
void EnviarAcaoHSM(IInterfaceSavd& savd, ESavdAplic aplicacao, ESavdCmdHSM acao)
{
    bool ok = false;
    if ((static_cast<ueint32>(aplicacao) | static_cast<ueint32>(acao)) >= 256) {
        savd.m_mensagem = "Argumento inválido para ação do hsm.";
    } else if (static_cast<ueint32>(acao) >= 2) {
        savd.m_mensagem = "IInterfaceSavd::EnviarAcaoHSM - Tentativa de envio de ação inválido ao serviço.";
    } else {
        ueRequisicaoSavd requisicao{0, static_cast<uebyte>(aplicacao), 0x1604, 0};
        const std::vector<uebyte> dados{0xFE, static_cast<uebyte>(acao)};
        savd.m_mensagem = IInterfaceSavd::EnviaComando(savd, savd.m_codigoErro, requisicao, dados);
        ok = savd.m_mensagem.empty();
    }
    if (!ok)
        throw CUeComumError(7340, std::format("Falha ao enviar ação ao HSM. {}", savd.GetUltimoErro()));   // :1040
}

}  // namespace comum
