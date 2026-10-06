// FRAGMENTS reconstructed by unit u26 from vota_web_wasm.wasm: functions of uenux2/src/api/** files owned by
// other units that the tools filed in unit u26 (merge each block into the file named in its header).
#include <compare>
#include <ctime>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace api {

// =====================================================================================================
// uenux2/src/api/util/cdatetime.cpp / cdate.cpp / ctime.cpp (owners: units u13/u20)
// =====================================================================================================

// wasm func 479 (tools: api_f479; observed executing). The current local date/time:
//   CDateTime(ISystemDateTime::GetInst().GetDataHora())  (api_f1000: CDate(), CTime(), ConvertFromLocalTime).
// Other units write it as "CDateTime::Now()" or "const CDateTime agora;".
CDateTime::CDateTime()
    : CDateTime(CPolySingletonList::instance<ISystemDateTime>().GetDataHora())             // func 1155, slot 0
{
}

// wasm func 759 (tools: vota_f759; observed executing). Three-way comparison: the date first
// (unknown_f1261: year +4, month +2, day +0, as int16), then the time of day (func 5448).
int CDateTime::Compare(const CDateTime& outro) const
{
    const int data = m_data.Compare(outro.m_data);                                         // unknown_f1261
    return data != 0 ? data : m_hora.Compare(outro.m_hora);                                // func 5448
}

// wasm func 5448 (tools: vota_f5448; also table slot 8896). Seconds since midnight (+0, int).
int CTime::Compare(const CTime& outro) const
{
    return (m_segundos > outro.m_segundos) - (m_segundos < outro.m_segundos);
}

// wasm func 5471 (tools: vota_f5471). a - b in seconds, both taken as UTC through timegm.     name inferred
// Callers: the countdown text sources func 11826 (CEsperaRetestar) and func 13012 (power-off warning).
std::int64_t CDateTime::DiferencaSegundos(const CDateTime& a, const CDateTime& b)
{
    auto paraTm = [](const CDateTime& d) {
        std::tm tm{};
        tm.tm_sec = d.m_hora.m_segundos % 60;
        tm.tm_min = (d.m_hora.m_segundos % 3600) / 60;
        tm.tm_hour = (d.m_hora.m_segundos / 3600) & 0xFF;
        tm.tm_mday = d.m_data.m_dia;
        tm.tm_mon = d.m_data.m_mes - 1;
        tm.tm_year = d.m_data.m_ano - 1900;
        tm.tm_isdst = 0;
        return tm;
    };
    std::tm ta = paraTm(a);
    std::tm tb = paraTm(b);
    return static_cast<std::int64_t>(timegm(&ta)) - timegm(&tb);
}

// =====================================================================================================
// uenux2/src/api/gui/ifield (owner: units u15/u16/u17). "api::IField::SetEstado" per unit u09.
// =====================================================================================================

// wasm func 940 (tools: api_f940, "curated: GUI field setter"). Callers: CMostraQRCodeBU::AjustaTela,
// CTesteTeclado (key highlighting: 1 = highlighted, 2 = normal for a CTextBox).
void IField::SetEstado(int estado)
{
    if (estado == m_estado)                                   // +44
        return;
    m_estado = estado;
    if (m_form == nullptr)                                    // +8
        return;
    bool visivel;
    {
        std::lock_guard trava(m_form->m_mutex);               // form +28 (unlock residue only)
        visivel = m_form->m_exibido;                          // form +4
    }
    if (visivel) {
        m_alterado = true;                                    // +4
        m_form->Redesenha();                                  // form vtable slot 4        name inferred
    }
}

// =====================================================================================================
// uenux2/src/api/gui/cpaperformbuilder.h/.cpp and api/gui/reports (owners: units u09/u15/u16)
// =====================================================================================================

// wasm func 1264 (tools: comum_f1264). Paper cut mark.                               name as in unit u09
void CPaperFormBuilder::AddCut()
{
    m_campos.push_back(std::make_shared<CCutFieldPaper>());   // vtable api::CCutFieldPaper @1580796 (24 bytes)
}

// wasm func 3660 (tools: vota_f3660). Report "sub-report" header: two strings (moved in) and a flag.
//                                                                                    name inferred
CSubReport::CSubReport(std::string titulo, std::string subtitulo, bool flag /* ? */)
    : m_titulo(std::move(titulo))                             // +4
    , m_subtitulo(std::move(subtitulo))                       // +16
    , m_extra()                                               // +28 (8 bytes = 0: shared_ptr?)
    , m_flag(flag)                                            // +36
{
}

// wasm func 3671 (tools: comum_f3671). Prints the paper form: the lambda type
// "CPaperFormBuilder::Show(const std::string&, const std::string&, bool)::$_0" (RTTI @1581400) gives the
// signature; every caller passes the bool as true (constant-propagated). Callers: the "estado da urna" /
// extrato de carga report (5591), CGeradorRelVersaoPacoteDados (5592), vota::CImpressaoPU (11902).
void CPaperFormBuilder::Show(const std::string& titulo, const std::string& subtitulo, bool flag)
{
    auto& impressora = CLp::GetInst();                                             // func 3876
    CSubReport relatorio(titulo, subtitulo, flag);                                 // func 3660
    impressora.Imprime(relatorio, [this] { Renderiza(); });                        // func 3875; $_0 renders m_campos
    m_campos.clear();
}

// wasm func 3875 (tools: vota_f3875). Print job front end ("lp").                    name inferred
// Callers: CRelVotaUtil::CortaPapel (2882), CPaperFormBuilder::Show (3671), CImpressaoListaEleitores (11908).
void CLp::Imprime(CSubReport& relatorio, const std::function<void()>& conteudo)
{
    if (m_impressoraRelatorios)
        m_impressoraRelatorios->vf9();                        // IImpressoraRelatorios slot 9 (prepare)   ?
    if (relatorio.vf5())                                      // CSubReport slot 5                        ?
        relatorio.vf6();                                      // CSubReport slot 6                        ?
    conteudo();                                               // std::bad_function_call if empty
}

// =====================================================================================================
// uenux2/src/api/gui/capplicationcontextstack.cpp and capplication.cpp (owner: unit u15)
// =====================================================================================================

// wasm func 1695 (tools: vota_f1695). Copy of the top context (copy ctor func 1841), or a generic one when
// the stack is empty. NOTE the argument order in the binary: detalhe = "Erro inesperado",
// título = "Pilha de contexto vazia" (unit u15's comment has them the other way round).
CApplicationContext CApplicationContextStack::Top() const
{
    if (m_pilha.empty())                                      // vector @1839212 / @1839216
        return CApplicationContext("Erro inesperado", "Pilha de contexto vazia");  // func 5558 (generico = true)
    return m_pilha.back();
}

// wasm func 5568 (tools: vota_f5568). Fatal-error screen of the application. Callers: vota::CThreadVota
// TrataExcecao/TrataExcecaoDesconhecida (7710/7709). In this build only the start of the body survives:
// the std::async task that runs EnterLoopBeeping() (the SOS beep, __async_assoc_state vtable @1577576,
// __execute = func 11151) cannot create a thread, so the function allocates the shared state and a
// std::__thread_struct (func 2541) and throws std::system_error("thread constructor failed")
// (shared_f1223). The code that drew the screen from the five arguments is not in the binary.   ?
void CApplication::ShowExceptionMsg(int tipo, const std::string& detalhe, const std::string& titulo,
                                    const std::string& mensagem, const std::string& excecao)
{
    auto beep = std::async(std::launch::async, [] { EnterLoopBeeping(); });
    // ... ShowExceptionMsgScreen(tipo, detalhe, titulo, mensagem, excecao) / LogExceptionMsg (other srclocs
    //     of capplication.cpp, :377 and :432) presumably follow - removed as unreachable.            ?
}

// =====================================================================================================
// uenux2/src/api/util/ctimer.cpp (path inferred; native timer, not used by the web build)
// =====================================================================================================

// wasm func 10850 (vtable api::CTimer slot 2). api::CTimer objects are created only by the native
// api::CTimerScheduler (func 10846); the web build registers CWasmTimerScheduler instead.     name inferred
void CTimer::Start()
{
    Stop();                                                   // slot 3
    m_ativo = true;                                           // +40
    m_thread = std::thread(&CTimer::Executa, this);           // -> std::system_error("thread constructor failed")
}

// Library functions the tools put in unit u26:
//   func 2541 = std::__thread_struct::__thread_struct() (new __thread_struct_imp, 24 zero bytes)
//   func 4677 = std::filesystem::__space(const path&, error_code*) (statvfs -> {capacity, free, available});
//              the error_code* was removed by wasm-opt (always nullptr), so failures throw filesystem_error

} // namespace api
