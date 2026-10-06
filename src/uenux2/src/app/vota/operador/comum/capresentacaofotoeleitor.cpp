// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original file UNKNOWN - path inferred: uenux2/src/app/vota/operador/comum/capresentacaofotoeleitor.cpp
// (the analysis tools attributed func 3616 to md/eleitor/cbiometriaeleitor.cpp because
// CBiometriaEleitor::GetFoto, cbiometriaeleitor.cpp:115, is inlined into it).
//
// Shows the identified voter's photo on the poll worker's terminal LCD (micro-terminal, "MT") and
// records the outcome in IInformacaoThreadOperador, which later persists it as
// eleitor_dinamico.estado_apresentacao_foto / resultado_decifracao_foto and in the RC file
// (ModuloResultadoUrnaCadastro::ApresentacaoFotoEleitor).
// Callers: vota::CNomeEleitor::StartState (10501), vota::IConfirmaJustificativa::StartState (10589),
//          vota::IEleitorImpedidoVotar::StartState (10625).
#include "vota/operador/comum/iinformacaothreadoperador.h"
#include "vota/log/clogvota.h"
#include "comum/cinfomtlcd.h"
#include "comum/dados/celeitores.h"
#include "ecourna/app/dados/cfoto.h"          // ? CFoto(formato, bytes) = shared_f5111

namespace vota {

namespace {
// ModuloResultadoUrnaCadastro::ResultadoApresentacaoFotoEleitor
enum EResultadoApresentacaoFoto : int {
    SEM_ERRO = 0, ERRO_FORMATO = 1, ERRO_FORMACAO_IMAGEM = 2, ERRO_RESOLUCAO = 3, ERRO_COMPRESSAO = 4,
    ERRO_PROFUNDIDADE_CORES = 5, ERRO_APRESENTACAO = 6, ERRO_DECIFRANDO_BUFFER = 7,
    ERRO_DESCONHECIDO_CRIPTOGRAFIA = 8,
};

// wasm func 1904  name inferred: IInformacaoThreadOperador vtable slot 29 -> {estado = 2, resultado}
void RegistraErroFoto(int resultado)
{
    impl::IInformacaoThreadOperador::GetInst().SetFotoNaoApresentadaPorErro(resultado);
}

// wasm func 5413  name inferred: slot 27 -> {estado = 0, resultado = 0} (voter without photo)
void RegistraEleitorSemFoto()
{
    impl::IInformacaoThreadOperador::GetInst().SetEleitorSemFoto();
}

// wasm func 3617  name inferred
void MostraFotoIndisponivel()
{
    comum::CInfoMTLCD::GetInst().MostrarImagemNoLCD(
        api::CFixedImage(":/resource/images/biometria/fotoIndisponivel.jpg"), {});
}

// Inlined JPEG header scan (probably an api/ecourna image helper): walks the markers of the file and
// records the last SOFn frame size. Truncated segments throw std::out_of_range (vector::at).
struct SCabecalhoJpeg {
    int largura = -1;          // +12
    int altura = -1;           // +16
    int bitsPorPixel = -1;     // +20 = 8 * number of components
    int tamanhoComentario = 0; // +24 (COM segment)
    bool temExif = false;      // +28 (APP1)
};

SCabecalhoJpeg LeCabecalhoJpeg(const std::vector<uebyte>& d)
{
    SCabecalhoJpeg h;
    std::size_t i = 0;
    while (i < d.size()) {
        if (d[i] != 0xFF) { ++i; continue; }
        if (i + 1 >= d.size() || i + 2 >= d.size()) break;
        const uebyte marcador = d[i + 1];
        const auto tamanho = [&] { return (d.at(i + 2) << 8) | d.at(i + 3); };
        switch (marcador) {
        case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC5: case 0xC6: case 0xC7:
        case 0xC9: case 0xCA: case 0xCB: case 0xCD: case 0xCE: case 0xCF: case 0xDE:   // SOFn / DHP
            h.altura = (d.at(i + 5) << 8) | d.at(i + 6);
            h.largura = (d.at(i + 7) << 8) | d.at(i + 8);
            h.bitsPorPixel = d.at(i + 9) * 8;
            i = i + 2 + tamanho();
            break;
        case 0xDA: return h;                                            // SOS: stop
        case 0xE1: h.temExif = true; i = i + 2 + tamanho(); break;      // APP1
        case 0xFE: h.tamanhoComentario = tamanho() - 2; i = i + 2 + tamanho(); break;  // COM
        case 0x01: case 0xC4: case 0xC8: case 0xCC: case 0xD0: case 0xDB: case 0xDC: case 0xDD:
        case 0xDF: case 0xE0: case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE6: case 0xE7:
        case 0xE8: case 0xE9: case 0xEA: case 0xEB: case 0xEC: case 0xED: case 0xEE: case 0xEF:
        case 0xF0:
            i = i + 2 + tamanho();                                      // skip segment
            break;
        default:
            i = i + 2;                                                  // RSTn, SOI, fill bytes, ...
        }
    }
    return h;
}
}  // namespace

// wasm func 3616  name inferred (tools: comum::md::CBiometriaEleitor::GetFoto)
void ApresentaFotoEleitor()
{
    const comum::md::CEleitorIdentidade identidade =
        impl::IInformacaoThreadOperador::GetInst().GetIdentidadeEleitor();          // slot 25 (api_f1535)
    const comum::CEleitorDetalhe* eleitor = comum::CEleitores::GetInst().Procura(   // comum_f2264
        comum::md::CEleitorIdentidade::Formata(identidade.GetIdentidade(), identidade.GetTipo()),
        identidade.GetTipo());
    if (eleitor == nullptr)
        return;

    if (!eleitor->PossuiBiometria()) {                                              // byte +100
        RegistraEleitorSemFoto();
        comum::CInfoMTLCD::GetInst().MostrarImagemNoLCD(
            api::CFixedImage(":/resource/images/biometria/semBiometria.jpg"), {});
        return;
    }

    const comum::md::CBiometriaEleitor biometria = eleitor->GetBiometria();          // 1937
    if (biometria.GetEstadoDecifracao() > 0) {
        MostraFotoIndisponivel();
        RegistraErroFoto(biometria.GetEstadoDecifracao() == 9 ? ERRO_DECIFRANDO_BUFFER
                                                              : ERRO_DESCONHECIDO_CRIPTOGRAFIA);
        return;
    }
    if (!biometria.PossuiFoto()) {
        RegistraEleitorSemFoto();
        MostraFotoIndisponivel();
        return;
    }

    const std::vector<uebyte> jpeg = biometria.GetFoto().GetImagem();   // GetFoto inlined (:115)
    // The scan does not run on `jpeg` directly: the wasm first builds a second 16-byte object
    // {int 1; copy of jpeg} with shared_f5111 (the same constructor CConversorFoto::DeconverteFormatoImagem
    // uses, i.e. probably ecourna::app::dados::CFoto(formato = 1, bytes)) and the inlined header scan reads
    // that object's vector. `jpeg` itself is what is later shown on the LCD.
    const ecourna::app::dados::CFoto fotoJpeg(1, jpeg);                 // ? shared_f5111
    const SCabecalhoJpeg h = LeCabecalhoJpeg(fotoJpeg.GetImagem());
    bool valida = false;
    if (h.largura == -1 || h.altura == -1 || h.bitsPorPixel == -1) {
        RegistraErroFoto(ERRO_FORMATO);
        CLogVota::GetInst().Registra("Erro ao carregar a foto do eleitor");
    } else if (h.largura < 120 || h.largura > 240) {
        RegistraErroFoto(ERRO_RESOLUCAO);
        CLogVota::GetInst().Registra(std::format("Largura da foto do eleitor inválida: {} px", h.largura));
    } else if (h.altura < 160 || h.altura > 320) {
        RegistraErroFoto(ERRO_RESOLUCAO);
        CLogVota::GetInst().Registra(std::format("Altura da foto do eleitor inválida: {} px", h.altura));
    } else if (h.bitsPorPixel != 8) {                                     // grayscale only
        RegistraErroFoto(ERRO_PROFUNDIDADE_CORES);
        CLogVota::GetInst().Registra(
            std::format("Quantidade de cores não conforme: {} bits por pixel", h.bitsPorPixel));
    } else {
        valida = true;
    }

    if (valida) {
        comum::CInfoMTLCD::GetInst().MostrarImagemNoLCD(api::CFixedImage(jpeg), {});
        impl::IInformacaoThreadOperador::GetInst().SetFotoApresentada();           // slot 28 -> {1, 0}
        CLogVota::GetInst().Registra("Foto do eleitor apresentada");
    } else {
        MostraFotoIndisponivel();
    }
}

}  // namespace vota
