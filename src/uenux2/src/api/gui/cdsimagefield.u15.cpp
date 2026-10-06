// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cdsimagefield.cpp (srcloc cdsimagefield.cpp:29 = the constructor,
// which only exists inlined in CFormBuilder::AddStatusHeader, func 502). Merge into cdsimagefield.cpp.
#include <algorithm>
#include <memory>
#include <mutex>
#include <vector>

#include "api/gui/gui-common.u15.h"

namespace api {

class IImage;
using SharedIImage = std::shared_ptr<IImage>;

// IObservable<T> layout (seen through the inlined Attach/Detach):
//   +4  T m_valor              (current value, shared_ptr: +4/+8)
//   +12 std::vector<IObserver<T>*> m_observadores
//   +24 std::mutex m_mutex
template <class T> class IObserver {
public:
    virtual ~IObserver();
    virtual void Update(const T& valor) = 0;       // slot 2
};

// CDSImageField ("DS" = data source; typeinfo @1578948, vmi: CImageField + IObserver<SharedIImage>)
// primary vtable @1578868, secondary (IObserver) vtable @1578920 at +44 - 56 bytes:
//   +44 IObserver<SharedIImage> vptr
//   +48 std::shared_ptr<IObservable<SharedIImage>> m_fonte   (+48 ptr, +52 control block)
class CDSImageField : public CImageField, public IObserver<SharedIImage> {
public:
    CDSImageField(const SPoint& pos, const std::shared_ptr<IObservable<SharedIImage>>& fonte,
                  EAnchorPoint ancora);
    ~CDSImageField() override;                                          // 2778 (+ thunks 11094/11095/11096)
    std::string GetClassName() const override { return "CDSImageField"; }   // 11091
    void Update(const SharedIImage& imagem) override;                   // 11093 (slot 10), thunk 11092
private:
    std::shared_ptr<IObservable<SharedIImage>> m_fonte;                 // +48
};

// Constructor - srcloc cdsimagefield.cpp:29 (inlined in func 502 only).
CDSImageField::CDSImageField(const SPoint& pos, const std::shared_ptr<IObservable<SharedIImage>>& fonte,
                             EAnchorPoint ancora)
    : CImageField(pos, SharedIImage{}, ancora), m_fonte(fonte)
{
    if (!m_fonte)
        throw CUeGuiError(static_cast<EUeGuiError>(4923), "A fonte de dados não pode ser nula");
    m_fonte->Attach(this);   // inlined: lock; if not yet registered push_back and call Update(m_valor)
}

// wasm func 2778 (complete-object destructor; 11096 = thunk from the IObserver base,
// 11095 = deleting, 11094 = deleting thunk)
CDSImageField::~CDSImageField()
{
    m_fonte->Detach(this);   // inlined: lock; erase(remove(observers, this)); unlock
}

// wasm func 11093 (slot 10 of the primary vtable) and 11092 (observed executing: the non-virtual thunk
// in the IObserver vtable, this - 44). Both copy the shared_ptr and call SetImage (func 5533).
void CDSImageField::Update(const SharedIImage& imagem)
{
    SetImage(imagem);
}

} // namespace api
