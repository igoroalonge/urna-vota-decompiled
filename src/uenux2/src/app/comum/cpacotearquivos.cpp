// uenux2/src/app/comum/cpacotearquivos.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by std::source_location records
// :85, :91 (ValidarChaveEAplicacaoValida), :116 (Validar), :162 (RetornaNomeChave),
// :169 (TrataErroPacote), :190 (TrataErroPacoteArquivo).
//
// Web build: IInterfaceSavd is (anonymous)::CWasmSavd (registered by main). Its RecebeMensagem
// (func 10949) ignores the request and always writes the same canned 12-byte answer, and every .vsu
// written by votaInit contains the text "assinatura simulada para vota_web_wasm". The start-up check
// below (observed executing twice per votaInit) therefore always succeeds in the simulator.
#include "comum/cpacotearquivos.h"

#include <format>
#include <map>

#include "api/pattern/cpolysingletonlist.h"
#include "comum/comumdefs.h"            // CUeComumError (factory func 480)
#include "comum/iinterfacemensagem.h"   // CInterfaceMensagemVazia
#include "comum/iinterfacesavd.h"

namespace comum {

namespace fs = std::filesystem;

// Constructor - inlined into func 4625.
CPacoteArquivos::CPacoteArquivos(fs::path pacote, const std::string& arquivo, ESavdChaveValidar chave,
                                 int aplicacao)
    : m_pacote(std::move(pacote)), m_chave(chave), m_aplicacao(aplicacao)
{
    ValidarChaveEAplicacaoValida();
    if (!arquivo.empty())
        m_arquivos.push_back(arquivo);                                   // func 1108 on reallocation
    m_tipo = m_arquivos.empty() ? TIPO_PACOTE : TIPO_ARQUIVOS;
}

// srcloc :85 / :91 (inlined). Both checks are unsigned range tests in the binary
// (`chave - 60 <= -12u`, `aplicacao - 10 <= -10u`).
void CPacoteArquivos::ValidarChaveEAplicacaoValida()
{
    const int chave = static_cast<int>(m_chave);
    if (chave < '1' || chave > ';')
        throw CUeComumError(EUeComumError{7219}, std::format("A chave informada não é válida: {}", m_chave));      // :85
    if (m_aplicacao < 1 || m_aplicacao > 9)
        throw CUeComumError(EUeComumError{7220}, std::format("A aplicação informada não é válida: {}", m_aplicacao)); // :91
}

// srcloc :116 (inlined into func 4625).
void CPacoteArquivos::Validar(IInterfaceMensagem& mensagem) const
{
    IInterfaceSavd& savd = api::CPolySingletonList::instance<IInterfaceSavd>();          // :116
    if (m_tipo == TIPO_PACOTE || m_tipo == TIPO_PACOTE_2) {
        mensagem.ValidandoPacote(*this);                                                   // slot 2 (no-op here)
        // inlined IInterfaceSavd request: arguments are checked, a TLV request is built (func 3831)
        // and the answer decoded (func 3830, IInterfaceSavd::DesconverteMensagem).
        if (!savd.ValidaAssinaturaPacote(m_pacote, m_chave, m_tipo, m_aplicacao))         // name inferred
            TrataErroPacote(savd);
        // (an argument error - chave or aplicacao > 255, or an empty path - sets savd's error text
        //  "Argumento inválido validar assinar o pacote." and is reported as a failure)
        return;
    }
    for (const std::string& arquivo : m_arquivos) {
        mensagem.ValidandoArquivo(*this, arquivo);                                         // slot 3 (no-op)
        if (!savd.ValidaAssinaturaArquivo(m_aplicacao, m_chave, m_pacote, arquivo))       // func 5892  name inferred
            TrataErroPacoteArquivo(savd, arquivo);
    }
}

// wasm func 5900 (srcloc :162)
std::string CPacoteArquivos::RetornaNomeChave(ESavdChaveValidar chave)
{
    switch (chave) {
    case ESavdChaveValidar::TSE:          return "TSE";
    case ESavdChaveValidar::SECAD:        return "SECAD";
    case ESavdChaveValidar::SECINP:       return "SECINP";
    case ESavdChaveValidar::SEVIN:        return "SEVIN";
    case ESavdChaveValidar::UE:           return "UE";
    case ESavdChaveValidar::SCUE:         return "SCUE";
    case ESavdChaveValidar::CLOGI:        return "CLOGI";
    case ESavdChaveValidar::CLOGI_CERT:   return "CLOGI_CERT";
    case ESavdChaveValidar::CLOGI_UPDATE: return "CLOGI_UPDATE";
    case ESavdChaveValidar::PU:           return "PU";
    case ESavdChaveValidar::SEINT:        return "SEINT";
    }
    throw CUeComumError(EUeComumError{7221}, std::format("Chave desconhecida: {}", chave));   // :162
}

// wasm func 5901 (srcloc :169)
void CPacoteArquivos::TrataErroPacote(const IInterfaceSavd& savd) const
{
    throw CUeComumError(EUeComumError{7222},
        std::format("Falha ao validar assinatura {}\npacote: ({})\nErro SAVD: ({} - {})",
                    RetornaNomeChave(m_chave), m_pacote.string(), savd.GetCodigoErro(), savd.GetMensagemErro()));  // :169
}

// srcloc :190 (inlined into func 4625). SAVD error codes 1..3 concern the package itself.
void CPacoteArquivos::TrataErroPacoteArquivo(const IInterfaceSavd& savd, const std::string& arquivo) const
{
    const int codigo = savd.GetCodigoErro();                                   // IInterfaceSavd +16
    if (codigo < 1 || codigo > 3) {
        throw CUeComumError(EUeComumError{7223},
            std::format("Falha ao validar assinatura {}\npacote: ({})\narquivo: ({})\nErro SAVD: ({} - {})",
                        RetornaNomeChave(m_chave), m_pacote.string(), arquivo, codigo, savd.GetMensagemErro()));  // :190
    }
    TrataErroPacote(savd);
}

// wasm func 4625 - name inferred. Observed executing: called twice by the voter start-up routine
// (func 7787, "CInformacaoEleitor::Inicializar" in u02) with CPath::GetPathTrab(INTERNA/EXTERNA, turno).
// For every result/state file present in the work directory, verify it against its .vsu with the
// urna's own key (UE), application 1.
void CPacoteArquivos::ValidaAssinaturasArquivosTrabalho(const fs::path& diretorio)
{
    const std::map<std::string, std::string> arquivos{      // func 1348 builds each pair; func 1221 inserts
        {"vota.bin", "vota.vsu"},
        {"rdv.dat",  "rdv.vsu"},
        {"ze.dat",   "ze.vsu"},
        {"rze.dat",  "rze.vsu"},
        {"bu.dat",   "bu.vsu"},
        {"buj.dat",  "buj.vsu"},
        {"bim.dat",  "bim.vsu"},
    };
    for (const auto& [arquivo, assinatura] : arquivos) {     // std::map order: bim, bu, buj, rdv, rze, vota, ze
        if (!fs::exists(diretorio / arquivo))                // func 1055 = fs::status; type none/not_found -> skip
            continue;
        const CPacoteArquivos pacote(diretorio / assinatura, arquivo, ESavdChaveValidar::UE, 1);
        CInterfaceMensagemVazia mensagem;
        pacote.Validar(mensagem);
    }
}

} // namespace comum

// ---------------------------------------------------------------------------------------------------
// wasm func 1348 (library, emitted here): std::pair<std::string, std::string>::pair(const char*, const char*)
//   used for the map initializer above and by func 5782.
