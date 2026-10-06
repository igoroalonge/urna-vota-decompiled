// FRAGMENTS reconstructed by unit u34 from vota_web_wasm.wasm.
//
// Small out-of-line pieces of the uenux2 "api" library (and of libc++) that the tools left in unit u34
// because no std::source_location or RTTI names them: destructor bodies merged by wasm-opt, one-line thunks,
// atexit handlers of static objects, and library template instantiations. Each block names the original
// file (path inferred unless stated) - merge it there. The bodies that another unit already wrote are only
// referenced.
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "api/gui/cformpart.h"
#include "api/gui/cmaskedtextfield.h"
#include "api/gui/ctextfielddoubleline.h"
#include "api/gui/ctextrectfield.h"
#include "api/ipc/cmessagequeue.h"
#include "api/pattern/cpolysingletonlist.h"
#include "api/pattern/igenericfactory.h"

namespace api {

// =========================================================================================================
// uenux2/src/api/gui/iform.h  (attested; reconstructed by unit u17 - bodies are there, not repeated here)
//   wasm func 5541  IForm<IScreen>::DoShow(IForm*)       (observed) - form replaces the whole voter-screen stack
//   wasm func 5540  IForm<IScreen>::DoShowOnTop(IForm*)  - form goes (or moves) to the top of the stack
//   wasm func 5987  IForm<IScreen>::Remove(IForm*)       - called by ~IForm<IScreen> (2781) and by
//                   vota::CEmitirMaisBU::ProcessInput (12081, the "emitir mais vias do BU" screen)
// Static stack @1832632 (vector<IForm*>), mutex @1832604; NotifyStackChanged = func 2651.
// =========================================================================================================

// =========================================================================================================
// Destructor bodies merged by wasm-opt (merge-similar-functions): the vtable of the class being destroyed
// is the extra parameter `vptr` (stored first, as the C++ ABI does for a complete-object destructor).
// =========================================================================================================

// uenux2/src/api/gui/ctextrectfield.{h,cpp} and ctextfielddoubleline.{h,cpp} (attested; unit u16)
// wasm func 6022 - complete destructor body, returns this. Thunks: CTextRectField slot 0 (10904, vtable
//                  @1583152), CTextFieldDoubleLine slot 0 (10928, vtable @1582696).
// wasm func 6021 - the same + operator delete (deleting destructor). Thunks: 10903, 10927 (slot 1).
// Both classes end with a SharedIText at +36/+40 (pointer/control block; released first) and derive from
// IFormFieldBase<IScreen> (vtable @1537360; std::string m_nome at +12).
CTextRectField::~CTextRectField() = default;          // members: ..., SharedIText m_texto (+36/+40)
CTextFieldDoubleLine::~CTextFieldDoubleLine() = default;

// uenux2/src/api/gui/cmaskedtextfield.h (unit u32)
// wasm func 6063 - complete destructor body; thunks CMaskedTextField<CFramedText> slot 0 (12604),
//                  CMaskedTextField<CGrayedFramedText> slot 0 (12719). SharedIText at +52/+56, then the base.
// wasm func 6062 - deleting destructor body; thunks 12595, 12713 (slot 1).
template <class MASK>
CMaskedTextField<MASK>::~CMaskedTextField() = default;

// uenux2/src/api/gui/cformpart.cpp (unit u32) and comum::CParteEleitores (unit u24)
// wasm func 6026 - deleting destructor body: release the shared_ptr at +4/+8 (the paper form; the body only
//                  touches the control block at +8), operator delete.
//                  Thunks: api::CFormPart slot 1 (10887, vtable @1583696), comum::CParteEleitores slot 1
//                  (11204, vtable @1576596).

// =========================================================================================================
// uenux2/src/api/pattern/igenericfactory.h  (attested; unit u20; instances listed in igenericfactory.u32.cpp)
// wasm func 6023 (observed executing) - merged body of CDefaultGenericFactory<I, C>::Create() for the
// classes whose constructor may throw: allocate `tamanho` bytes, run the constructor (table slot) inside an
// invoke_ii, free the memory and rethrow if it throws. Thunks: <IRWSyncCtl, CPosixRWMutex> (10864: slot
// 244, 36 bytes), <ISyncCtl, CPosixMutex> (10869: slot 243, 32 bytes).
// =========================================================================================================
static void* CriaObjeto(void* (*construtor)(void*), std::size_t tamanho)      // name inferred
{
    void* memoria = ::operator new(tamanho);
    try {
        return construtor(memoria);                                            // constructors return `this`
    } catch (...) {
        ::operator delete(memoria);
        throw;
    }
}
// i.e. the source line `return std::make_unique<C>();` (unique_ptr<I> returned in the first word).

// =========================================================================================================
// uenux2/src/api/ipc/cmessagequeue.h  (attested; unit u18; Envia(id) by unit u33 in cmessagequeue.u33.h)
// wasm func 7708 (observed executing) - Envia(msg) = Add(msg, 1). Priority 1 is used by every Add call of
// the binary except one: CVerificaEleicaoPassou::StartState (11914) posts message 0 to the operator queue
// with priority 100.
// Caller: Envia(int16 id) (func 3903, used by votaInit/votaTick). The lock guard's destructor (func 5528)
// is in clockguard.u34.h.
// =========================================================================================================
template <typename TMessage>
void CPriorityMessageQueue<TMessage>::Envia(const TMessage& mensagem)
{
    Add(mensagem, 1);                                                          // rhvoice_f501 (misnamed Add)
}

// =========================================================================================================
// uenux2/src/api/pattern/cpolysingletonlist.h  (attested; units u19 and u33)
// =========================================================================================================
// wasm func 11265 (observed executing) - DefaultPolySingletonsInfo(): the registry object @1832448 (mutexes
// + vector<SPolySingleton>); the two flag stores are its one-time initialisation. Every
// "GetPolySingletonsInfo()" of the binary calls it through the function pointer @1526320 (see u19).
//
// Eight registration wrappers called by main() (func 10307, observed executing): each is
//     push<I>(std::unique_ptr<I>(instancia), info)       -> merged body 1562 with the push<I> table slot
// (body in cpolysingletonlist.u33.cpp):
//   wasm func 9372  slot 136  push<vota::IExecucaoVota>                      (8728)
//   wasm func 9574  slot 135  push<IGenericFactory<ISemaphore>>              (8758)
//   wasm func 9650  slot 134  push<IGenericFactory<IRWSyncCtl>>              (8834)
//   wasm func 9685  slot 133  push<IGenericFactory<ISyncCtl>>                (8911)
//   wasm func 9731  slot 132  push<IGenericFactory<IThreadImpl>>             (8986)
//   wasm func 9831  slot 131  push<IFingerPrepare>                           (9039)
//   wasm func 9897  slot 130  push<ecourna::api::security::ISymmetricCipherFactory> (9135)
//   wasm func 9986  slot 129  push<ecourna::api::security::IRng>             (9233)
//
// vector<SPolySingleton>::emplace_back(nome, &typeid(I), instancia) slow path (func 1009, element ctor
// func 1005 passed by table slot) - one thunk per interface whose push is not inlined:
//   wasm func 10424  (table slot 277, used by push<vota::IExecucaoVota> 5395)                  element slot 278
//   wasm func 10464  (table slot 274, push<vota::impl::ISincronismoVotoEleitor> 5398)          element slot 275
//   wasm func 10507  (table slot 271, push<vota::impl::IPoliticaExecucaoEleitor> 5407)         element slot 272
// wasm func 11548 - std::__split_buffer<SPolySingleton, allocator&>::__split_buffer(capacidade, inicio, alloc)
//                   (24-byte elements: std::string nome, const std::type_info*, std::shared_ptr<void>;
//                   max 178956970 elements) - the growth buffer of that slow path.
// wasm func 12979 (observed executing) - out-of-line std::string::operator=(const char*) (= assign(s,
//                   strlen(s)), func 276), called through table slot 168 by CPolySingletonList::log (func 212:
//                   `demangled = nomeReal.get();`).

// =========================================================================================================
// atexit handlers (Emscripten registers one small function per static object with a destructor). Their
// bodies are the destructors of these statics; listed here with their owner.
// =========================================================================================================
//   wasm func 8641   ~std::mutex IForm<IScreen>::ms_mutex    @1832604  (only the unlock residue, func 150)
//   wasm func 8177   ~std::vector IForm<IScreenMT>::ms_pilha @1832892  (free the buffer)
//   wasm func 8187   ~std::mutex IForm<IScreenMT>::ms_mutex  @1832864
//   wasm func 8727   ~IObservable IForm<IScreenMT>::ms_observavel @1529936 (vf0 = func 3458)
//   wasm func 12108  ~std::vector IForm<IPaper>::ms_pilha    @1833528
//   wasm func 12109  ~std::mutex IForm<IPaper>::ms_mutex     @1833500
//   wasm func 11003  ~IObservable IForm<IPaper>::ms_observavel @1581132 (vf0 = func 3669)
//   wasm func 10866  ~std::mutex @1839300 (CSynchronizer's instance mutex, csynchronizer.cpp)          ?
//   wasm func 10867  ~std::unique_ptr<CSynchronizer> CSynchronizer::s_instancia @1839324 (shared_f349)
//   wasm func 10879  ~std::vector<std::pair<std::string, char>> ESCAPES @1839288 (cinistrings.cpp: 16-byte
//                    elements, merged vector-of-strings destructor unknown_f6071(.., 8, 5, 16))
//   wasm func 10901  ~std::regex marcadores("%[ST]") @1839244 of CInputMenuField (cinputmenufield.cpp,
//                    func 5492): shared_ptr at @1839276 + std::locale (unknown_f6096)
//   wasm func 11162  ~std::string CApplication::ms_descricao @1839180 ("Software de Votação", capplication.cpp)
//   wasm func 11662  ~std::string CInputMenuField::ms_textoInstrucaoPadrao @1832432 ("Digite a sua opção: ")
//   (comum) wasm func 11655 ~std::mutex @1838548 and wasm func 11656 ~std::unique_ptr<comum::CInfoMTLCD>
//                    @1838572 (virtual deleting destructor, slot 1) - cinfomtlcd.cpp (unit u22)

}  // namespace api

// =========================================================================================================
// libc++ (Emscripten's LLVM >= 21 libc++, not TSE code) - instances filed in unit u34 by the tools.
// Not reconstructed (library internals):
//   wasm func 4656   std::(anonymous)::do_strerror_r(int)  (src/system_error.cpp): strerror_r into a 1024-byte
//                    buffer, EINVAL (28) -> "", other failures abort(), "" -> "Unknown error %d"; callers
//                    __generic_error_category::message, __iostream_category::message
//   wasm func 4680   std::unique_ptr<char[]>::reset() (delete[] = free), the `buff` of __current_path inlined
//                    into std::filesystem::__do_absolute (7749): on musl (no __GLIBC__/__APPLE__) libc++ sizes
//                    the getcwd buffer with pathconf and allocates it with new char[size + 1]; the second
//                    unique_ptr `hold` has a no-op deleter there
//   wasm func 4683   std::filesystem::status(const path&, error_code&) noexcept -> __status (func 1055) in an
//                    invoke, std::terminate on exception (observed executing: create_directories at votaInit)
//   wasm func 4684   std::filesystem::detail::ErrorHandler<bool>::report(const std::errc&) =
//                    report(error_code(err, generic_category()))   (table slot 8853)
//   wasm func 7747   std::filesystem::_PathCVT<char>::__append_source(string&, const char* const&):
//                    append(s, s + strlen(s)) (table slot 8850; path(const char*) in CThreadMonitor, 4686)
//   wasm func 8095   std::filesystem::detail::vformat_string(const char*, va_list) (src/filesystem/
//                    format_string.h): vsnprintf into a 256-byte buffer, resize and retry when longer
//                    (table slot 8297; filesystem_error messages)
//   wasm func 12293  `return strerror(e);` thunk kept in the function table (slot 6211, next to
//                    ecourna::api::io::CFile::Open's slots) so that it can be called through invoke_ii
// =========================================================================================================
