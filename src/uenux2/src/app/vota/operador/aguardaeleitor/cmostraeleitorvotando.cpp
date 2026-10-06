// Reconstructed from vota_web_wasm.wasm (unit u10).
// Original: uenux2/src/app/vota/operador/aguardaeleitor/cmostraeleitorvotando.cpp
// (attested by std::source_location records: lines 64, 127, 159).
//
// Not executed by the web simulator (the operator thread never runs there). On a real urna this is
// where the habilitação of a voter becomes persistent: when the voter thread reports that the voter
// started voting (message 6), SalvaHabilitacaoEleitor records in the roll (CEleitores ->
// eleitor_dinamico row of uenux.db) how the voter was released, and stores the fingerprint images
// (WSQ, encrypted) that were captured during the identification.
#include "vota/operador/aguardaeleitor/cmostraeleitorvotando.h"

#include <format>
#include <string>
#include <vector>

#include "api/gui/batteryicondatasource.h"                          // api::BatteryIconDataSource<Vertical>
#include "api/hwil/iinput.h"                                         // api::IInputMT
#include "api/util/cstringutils.h"                                   // api::CStringUtils::PadLeft (func 753)
#include "comum/appinfo/cappinfo.h"                                  // comum::EhTreinamentoEleitor, EhFaseTreinamento
#include "comum/cpath.h"
#include "comum/dados/celeitores.h"
#include "comum/dados/md/eleitor/celeitordadoshabilitacao.h"         // comum::CEleitorDadosHabilitacao (u05)
#include "comum/reconhecimentobiometrico/ccontrolaarmazenamentodeimagens.h"
#include "vota/eleitor/comum/cinformacaoeleitor.h"
#include "vota/operador/cthreadoperador.h"
#include "vota/operador/aguardaeleitor/celeitordemorando.h"
#include "vota/operador/aguardaeleitor/celeitorvotounaovotou.h"
#include "vota/operador/comum/csincronismooperador.h"
#include "vota/operador/comum/iinformacaothreadoperador.h"
#include "vota/operador/confirmaidentidade/ccontrolareconhecimento.h"
#include "vota/comum/votadefs.h"

namespace vota {

using comum::md::CEleitorIdentidade;
using comum::md::ETipoHabilitacao;

namespace {

// ModuloResultadoUrnaCadastro "tipoAtivacaoAudio" of the habilitação, from the voter-side audio mode
// (CInformacaoEleitor::m_modoAudio: 0 conforme cadastro, 1 habilitado, 2 desabilitado).
int TipoAtivacaoAudio()                                                      // inlined 3x, name inferred
{
    const CInformacaoEleitor& info = CInformacaoEleitor::GetInst();         // func 509
    if (info.GetModoAudio() == 0)                                           // icf BN_is_zero (215): +4 == 0
        return 0;
    return info.AudioHabilitado() ? 1 : 2;                                  // func 3078: +4 == 1
}

// Principal identity of the voter the roll is positioned on, left-padded to 12 digits.
std::string IdentidadePrincipalPreenchida()                                 // inlined 3x, name inferred
{
    const auto& eleitor = comum::CEleitores::GetInst().GetCurrent().GetEleitor();
    return api::CStringUtils::PadLeft(
        eleitor.GetIdentidadePorTipo(eleitor.GetTipoIdentificadorPrincipal()).GetIdentidade(), '0', 12);
}

// <trab>/wsq/<subdir> in the internal (0) and external (1) flash: vota_f2298 / comum_f3898 over
// comum_f2864 = CPath::GetPathTrab(flash, trab atual) / "wsq/". Subdirs: "habilitado/" (funcs 5819/5817),
// "nao-habilitado/" (5818/5816), "operador/" (3795/3794).
std::string DiretorioWsq(int flash, const char* subdiretorio);              // other units

// Stores one fingerprint image. CControlaArmazenamentoDeImagens (wasm func 2725, unit u24; the tools
// call it "LeChavePublica" after an inlined callee) checks that the flash has >= 5 MiB free, picks a unique
// "<prefixo>NNNNNN.wsq" name (random % 999999), encrypts the WSQ with the public key (CifrarWsq) and
// writes it to both directories. The prefix passed here is always EMPTY.
void ArmazenaWsq(const std::vector<uebyte>& wsq, const char* subdiretorio)
{
    comum::CControlaArmazenamentoDeImagens::Armazena(wsq, DiretorioWsq(0, subdiretorio),
                                                     DiretorioWsq(1, subdiretorio), std::string{});
}

}  // namespace

// ---------------------------------------------------------------------------------------------
// wasm func 10427 - vtable slot 2                                           name from slot order
void CMostraEleitorVotando::StartState()
{
    *m_textoCargo = CThreadOperador::GetInst().m_textoCargo;                 // CThreadOperador +108
    m_form->Show();                                                          // IForm slot 2
    api::BatteryIconDataSource<api::CPowerInformation::IconOrientation::Vertical>::GetInst()   // func 2284
        .Registra();                                                         // func 5903 (attach once) ?
    m_proximoEstado = this;
}

// ---------------------------------------------------------------------------------------------
// wasm func 10426 - vtable slot 7 (srcloc line 64)
// The mesário's keys are read and discarded while the voter votes.
void CMostraEleitorVotando::ProcessInput()
{
    api::IInputMT& teclado = api::CPolySingletonList::instance<api::IInputMT>();   // line 64
    if (teclado.TemTecla())                                                  // slot 3
        teclado.GetKey();   // iinput.h:86 inlined: throws CUeHwilError 5170 "IInput - Nao havia um caractere
                            // disponivel" if no key; slot 2 reads it; ++counter at +4
}

// ---------------------------------------------------------------------------------------------
// wasm func 10425 - vtable slot 6. The tools named it SalvaHabilitacaoEleitor because that private
// member (srcloc 127/159) is inlined in the case EleitorIniciouVotacao.            name from slot order
void CMostraEleitorVotando::ProcessMessage(uebyte mensagem)
{
    switch (static_cast<EMensagemOperadorRecebida>(mensagem)) {
    case EMensagemOperadorRecebida::EleitorNaoVotou: {                       // 2
        auto& tela = CEleitorVotouNaoVotou::GetInst();                       // func 1901
        tela.m_votou = false;                                                // +11
        m_proximoEstado = &tela;
        break;
    }
    case EMensagemOperadorRecebida::EleitorDemorandoSemVoto: {               // 3
        auto& tela = CEleitorDemorando::GetInst();                           // func 2734
        tela.m_votouParcialmente = false;                                    // +11
        m_proximoEstado = &tela;
        break;
    }
    case EMensagemOperadorRecebida::EleitorDemorandoComVoto: {               // 4
        auto& tela = CEleitorDemorando::GetInst();
        tela.m_votouParcialmente = true;
        m_proximoEstado = &tela;
        break;
    }
    case EMensagemOperadorRecebida::EleitorIniciouVotacao:                   // 6
        SalvaHabilitacaoEleitor();
        break;
    case EMensagemOperadorRecebida::AtualizaCargoAtual:                      // 9
        *m_textoCargo = CThreadOperador::GetInst().m_textoCargo;
        m_form->Show();
        break;
    case EMensagemOperadorRecebida::SincronizaVoto: {                        // 13
        auto& sincronismo = CSincronismoOperador::GetInst();                 // func 1897
        sincronismo.m_suspensaoAutomatica = false;                           // +11
        m_proximoEstado = &sincronismo;
        *m_textoCargo = " ";
        break;
    }
    default:                                                                 // 5, 7, 8, 10-12, others: ignored
        break;
    }
}

// ---------------------------------------------------------------------------------------------
// Inlined into ProcessMessage (srcloc lines 127 and 159).
void CMostraEleitorVotando::SalvaHabilitacaoEleitor()
{
    if (comum::EhTreinamentoEleitor())                                       // func 697 inlined
        return;

    auto& info = impl::IInformacaoThreadOperador::GetInst();
    switch (info.GetTipoHabilitacao()) {                                     // slot 15

    case ETipoHabilitacao::SEM_BIOMETRIA: {                                  // 0: birth year / no biometrics
        const CEleitorIdentidade identidade = info.GetIdentidadeEleitor();  // slot 25 (api_f1535)
        const int audio = TipoAtivacaoAudio();
        CControlaReconhecimento::GetInst();                                  // func 1256 (result unused)
        const comum::md::CEleitorDadosHabilitacaoBiometrica bio(0, 0, 0, 0);          // func 3718
        const comum::CEleitorDadosHabilitacao dados(identidade, ETipoHabilitacao::SEM_BIOMETRIA, audio, bio,
                                                    info.GetApresentacaoFoto());      // funcs 2745, 2733
        comum::CEleitores::GetInst().MarcaEleitorFoiHabilitado(dados);       // func 2825
        break;
    }

    case ETipoHabilitacao::BIOMETRIA: {                                      // 1: fingerprint recognised
        const uebyte dedo = CControlaReconhecimento::DEDOS_1X4[CControlaReconhecimento::s_indiceDedo];
        if (!comum::EhFaseTreinamento()) {                                   // func 1485
            if (CControlaReconhecimento::s_digitalCapturada.empty())
                throw CUeVotaError(EUeVotaError{9392}, "Não há dados salvos de biometria do eleitor.");  // line 127
            const std::vector<uebyte> wsq = CControlaReconhecimento::s_digitalCapturada;
            // Computed and then never used (see the u10 doc, "weird code"): the file gets a random name.
            const std::string nomeArquivo = IdentidadePrincipalPreenchida() +
                std::format("-{:02}-{:02}", CControlaReconhecimento::GetTentativaDigitalSalva(), dedo);
            ArmazenaWsq(wsq, "habilitado/");
        }
        const CEleitorIdentidade identidade = info.GetIdentidadeEleitor();
        const int audio = TipoAtivacaoAudio();
        CControlaReconhecimento::GetInst();
        const comum::md::CEleitorDadosHabilitacaoBiometrica bio(
            dedo, CControlaReconhecimento::s_score, CControlaReconhecimento::GetTentativaDigitalSalva(), 0);
        const comum::CEleitorDadosHabilitacao dados(identidade, ETipoHabilitacao::BIOMETRIA, audio, bio,
                                                    info.GetApresentacaoFoto());
        comum::CEleitores::GetInst().MarcaEleitorFoiHabilitado(dados);
        break;
    }

    case ETipoHabilitacao::CODIGO_MESARIO: {                                 // 2: released with the mesário's fingerprint
        if (!comum::EhFaseTreinamento()) {
            if (!CControlaReconhecimento::s_digitalCapturada.empty()) {
                const std::vector<uebyte> wsq = CControlaReconhecimento::s_digitalCapturada;
                const std::string nomeArquivo = IdentidadePrincipalPreenchida() +      // unused
                    std::format("-{:02}Err", CControlaReconhecimento::GetTentativaDigitalSalva());
                ArmazenaWsq(wsq, "nao-habilitado/");
            }
            if (CControlaReconhecimento::s_digitalMesario.empty())
                throw CUeVotaError(EUeVotaError{9393}, "Não há dados salvos de biometria do mesário.");  // line 159
            const std::vector<uebyte> wsq = CControlaReconhecimento::s_digitalMesario;
            const std::string nomeArquivo = IdentidadePrincipalPreenchida() + "_Operador";   // unused
            ArmazenaWsq(wsq, "operador/");
        }
        const CEleitorIdentidade identidade = info.GetIdentidadeEleitor();
        const int audio = TipoAtivacaoAudio();
        CControlaReconhecimento::GetInst();
        const uebyte tentativas = CControlaReconhecimento::GetTentativa();              // @1590924, truncated
        const int erroDecifrar =
            comum::CEleitores::GetInst().GetCurrent().GetBiometria().GetEstadoDecifracao();   // func 1937, +36
        const comum::md::CEleitorDadosHabilitacaoBiometrica bio =
            CControlaReconhecimento::s_tituloMesario.empty()
                ? comum::md::CEleitorDadosHabilitacaoBiometrica(0, 0, tentativas, erroDecifrar)
                : comum::md::CEleitorDadosHabilitacaoBiometrica(
                      0, 0, tentativas, erroDecifrar,
                      CEleitorIdentidade(CControlaReconhecimento::s_tituloMesario,
                                         ecourna::app::dados::ETipoIdentificadorEleitor::TITULO));   // func 566
        const comum::CEleitorDadosHabilitacao dados(identidade, ETipoHabilitacao::CODIGO_MESARIO, audio, bio,
                                                    info.GetApresentacaoFoto());
        comum::CEleitores::GetInst().MarcaEleitorFoiHabilitado(dados);
        break;
    }

    default:
        break;
    }
}

}  // namespace vota
