// uenux2/src/app/comum/iinterfaceinit.cpp
// Reconstructed from vota_web_wasm.wasm (unit u23).
//
// Error class: CUeComumError = CBaseError<comum::EUeComumError, SErrorLimits{7200, 7600}> (comum_f480 is its
// merged constructor thunk). Error codes 7231..7296.
#include "comum/iinterfaceinit.h"

#include <filesystem>
#include <format>

#include "api/pattern/cpolysingletonlist.h"
#include "api/util/cwait.h"
#include "comum/log/ieventoslog.h"
#include "ecourna/api/io/cfile.h"
#include "ecourna/api/util/cstringutils.h"

namespace comum {

namespace {
// vota_f1947 (other unit): human readable size, "{}.{} GB" / "{}.{} MB" / "{}.{} KB".
std::string FormataTamanho(int bytes);
// vota_f5895 -> vota_f2921 (other unit): syslog(LOG_INFO) when /dev/urna exists, printf when DEBUG_UENUX is set.
void DebugUenux(const char* formato, ...);
}  // namespace

// wasm func 11646 (vtable slot 2; srcloc :47). Overridden in the web build by CWasmInit (icf "return 1").
EBootDeviceId IInterfaceInit::CurrentBootDevice()
{
    const std::vector<int> resposta = EnviarMensagem(CMD_BOOT_DEVICE);          // 37
    if (resposta.empty())
        throw CUeComumError(7231, "Resposta nula");                            // :47
    switch (resposta[0]) {
    case 1: return EBootDeviceId::DISPOSITIVO_1;
    case 2: return EBootDeviceId::DISPOSITIVO_2;
    default: return EBootDeviceId::DESCONHECIDO;
    }
}

// wasm func 6046 (name inferred): shared body of the two "is the MR ...?" queries (merge-similar-functions:
// the srcloc and the error code are parameters). True when the service answers 0.
bool IInterfaceInit::ConsultaFlag(const std::source_location& local, int codigoErro, ueint32 comando)
{
    const std::vector<int> resposta = EnviarMensagem(comando);
    if (resposta.empty())
        throw CUeComumError(codigoErro, "Resposta nula", local);
    return resposta[0] == 0;
}

// wasm func 2862 (srcloc :137)
bool IInterfaceInit::IsMRPresenteSemHabilitar()
{
    return ConsultaFlag(std::source_location::current(), 7237, CMD_MR_PRESENTE);       // command 18
}

// wasm func 5897 (srcloc :148)
bool IInterfaceInit::IsMRMontadoSemHabilitar()
{
    return ConsultaFlag(std::source_location::current(), 7238, CMD_MR_MONTADO);        // command 17
}

// wasm func 5896 (srcloc :159, :163, :172; DispositivoMR :632/:636 and GetSerialNumberMR :855 inlined).
// Mounts the result medium (MR, pen drive) and logs its size and serial the first time a given MR is seen.
void IInterfaceInit::MontarMRSemHabilitar()
{
    const std::vector<int> resposta = EnviarMensagem(CMD_MONTA_MR);                     // 34
    if (resposta.empty())
        throw CUeComumError(7239, "Resposta nula");                                   // :159
    if (resposta[0] != 0)
        throw CUeComumError(7240, std::format("Falha ao montar mídia de resultado ({})", resposta[0]));   // :163

    const int dispositivo = DispositivoMR();
    const std::string serial = GetSerialNumberMR(dispositivo);
    if (serial == m_serialMR)
        return;

    // size = /sys/block/sdX/size (sectors) * /sys/block/sdX/queue/logical_block_size (0 if no device)
    int tamanho = 0;     // 32-bit: overflows for media larger than 2 GiB (only used in the log line)
    if (dispositivo > 0) {
        const std::string disco = std::format("sd{:c}", dispositivo + 96);            // 1 -> "sda"
        const int setores = ecourna::api::util::CStringUtils::ToInt32(
            ecourna::api::io::CFile::ReadFileContent(std::format("/sys/block/{}/size", disco)));
        const int bloco = ecourna::api::util::CStringUtils::ToInt32(
            ecourna::api::io::CFile::ReadFileContent(std::format("/sys/block/{}/queue/logical_block_size", disco)));
        tamanho = bloco * setores;                                                     // i32.mul in the wasm
    }
    api::CPolySingletonList::instance<IEventosLog>().Loga(
        std::format("Tamanho da MR: {}", FormataTamanho(tamanho)));                    // :172
    DebugUenux("Serial da MR: %s", serial.c_str());
    m_serialMR = serial;
}

// Inlined into 5896 (srcloc :632, :636)
int IInterfaceInit::DispositivoMR()
{
    EnviarMensagemThrowVoid(CMD_HABILITA_MR, "habilitando MR");                         // 10
    // 500 ms wait, compiled as `if (byte@1584624 == 1) emscripten_sleep(500)` (helper name inferred). It is NOT
    // std::this_thread::sleep_for, which libc++ routes to nanosleep (a spin loop in this build). The byte @1584624
    // is a static initialised to 1 in the data segment and no code stores to it (24 functions only read it), so
    // the sleep always runs; the build has no ASYNCIFY and the glue's _emscripten_sleep aborts the module.
    api::CWait::Sleep(500);
    const std::vector<int> resposta = EnviarMensagem(CMD_DISPOSITIVO_MR);               // 12
    if (resposta.empty())
        throw CUeComumError(7282, "Resposta nula");                                   // :632
    if (resposta[0] == -1)
        throw CUeComumError(7283, std::format("Falha ao obter dispositivo da MR: {}", resposta[0]));   // :636
    return resposta[0];
}

// Inlined into 5896 (srcloc :855). The USB port of the MR depends on the urna model (api::IUrna slot 0).
std::string IInterfaceInit::GetSerialNumberMR(int dispositivo)
{
    std::string serial;
    if (dispositivo <= 0)
        return serial;
    const std::string disco = std::format("sd{:c}", dispositivo + 96);   // built and discarded (unused in the wasm)

    std::vector<std::string> candidatos;
    const int modelo = api::CPolySingletonList::instance<api::IUrna>().GetModelo();     // func 923, slot 0
    switch (modelo) {
    case 2009: case 2010: case 2011: case 2013:
        candidatos = {"/sys/devices/pci0000:00/0000:00:1d.8/usb1/1-0/serial",
                      "/sys/devices/pci0000:00/0000:00:1d.8/usb1/1-8/serial"};
        break;
    case 2015:
        candidatos = {"/sys/devices/pci0000:00/0000:00:1d.7/usb1/1-0/serial",
                      "/sys/devices/pci0000:00/0000:00:1d.7/usb1/1-7/serial"};
        break;
    case 2020: case 2022:
        candidatos = {"/sys/devices/pci0000:00/0000:00:15.0/usb1/1-3/serial",
                      "/sys/devices/pci0000:00/0000:00:15.0/usb2/2-3/serial"};
        break;
    default:                     // 2012, 2014, 2016..2019, 2021, 2023+: no known port, serial stays empty
        break;
    }
    for (const auto& caminho : candidatos) {
        std::error_code ec;
        const auto tipo = std::filesystem::status(caminho, ec).type();                  // api_f1055
        if (tipo == std::filesystem::file_type::none || tipo == std::filesystem::file_type::not_found)
            continue;
        serial = ecourna::api::util::CStringUtils::Trim(ecourna::api::io::CFile::ReadFileContent(caminho));   // f1374
        break;
    }
    return serial;
}

// wasm func 3832 (srcloc :184, :188)
void IInterfaceInit::DesmontarMRSemDesabilitar()
{
    const std::vector<int> resposta = EnviarMensagem(CMD_DESMONTA_MR);                  // 58
    if (resposta.empty())
        throw CUeComumError(7241, "Resposta nula");                                   // :184
    if (resposta[0] != 0)
        throw CUeComumError(7242, std::format("Falha ao desmontar mídia de resultado ({})", resposta[0]));   // :188
}

// wasm func 5894 (srcloc :522, :527, :531)
void IInterfaceInit::DesligarUrna()
{
    api::CPolySingletonList::instance<IEventosLog>().Loga("Urna desligada a pedido do aplicativo");   // :522, @124164
    const std::vector<int> resposta = EnviarMensagem(CMD_DESLIGA);                      // 48
    if (resposta.empty())
        throw CUeComumError(7268, "Resposta nula");                                   // :527
    if (resposta[0] < 0)
        throw CUeComumError(7269, std::format("Erro ao desligar urna: {}", resposta[0]));   // :531
}

// wasm func 3833 (srcloc :731, :737)
void IInterfaceInit::EnviarMensagemThrowVoid(int comando, const std::string& descricao)
{
    const std::vector<int> resposta = EnviarMensagem(comando);
    if (resposta.empty())
        throw CUeComumError(7293, std::format("{}: Resposta nula", descricao));                       // :731
    if (resposta[0] != 0)
        throw CUeComumError(7294, std::format("Falha ao enviar mensagem {}, {}: {}", comando, descricao,
                                              resposta[0]));                                          // :737
}

// wasm func 729 (srcloc :753, :757). Observed executing (every CInformacaoEleicao getter calls it).
bool IInterfaceInit::GetDemoMode()
{
    if (m_demoMode)
        return *m_demoMode;
    const std::vector<int> resposta = EnviarMensagem(CMD_MODO_DEMONSTRACAO);            // 67
    if (resposta.empty())
        throw CUeComumError(7295, "Resposta nula");                                   // :753
    if (resposta[0] == -1)
        throw CUeComumError(7296, std::format("Falha ao obter estado do modo de demonstração: {}", resposta[0]));   // :757
    m_demoMode = std::make_shared<bool>(resposta[0] != 0);   // shared_ptr<bool>(new bool) (__shared_ptr_pointer)
    return *m_demoMode;
}

}  // namespace comum
