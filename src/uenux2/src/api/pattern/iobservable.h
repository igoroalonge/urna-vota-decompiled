// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/pattern/iobservable.h (path inferred: api::IObservable / api::IObserver are
// generic patterns like the other headers of uenux2/src/api/pattern).
//
// A value with a list of observers, notified when the value changes (Observer pattern).
//   api::IObserver<T>    ("class"): [0] dtor [1] deleting [2] Update(const T&)
//   api::IObservable<T>  ("class"): [0] dtor [1] deleting - no other virtual function
// Layout of IObservable<T>:
//   +0 vptr   +4 T m_valor   then std::vector<IObserver<T>*> m_observadores   then std::mutex m_mutex
//   (T = shared_ptr<IImage>: +4 value, +12 observers, +24 mutex;
//    T = std::vector<FormHandle<MEDIA>>: +4 value, +16 observers, +28 mutex)
//
// Instantiations in the binary:
//   IObservable<std::shared_ptr<IImage>>  vtable @1551952 - base of the battery-icon data sources
//       (BatteryIconDataSource<Horizontal/Vertical>, cpowerinformation.cpp) observed by CDSImageField and
//       comum::CInfoMTLCD.
//   IObservable<std::vector<FormHandle<MEDIA>>> for MEDIA = IScreen / IScreenMT / IPaper - base of
//       api::CFormStack<MEDIA>::CObservable (the published copy of the form stack of each device, iform.h;
//       unit u17 calls the element type SFormInfo: RTTI says api::FormHandle<MEDIA>, 12 bytes
//       {IForm<MEDIA>*, shared_ptr<FormControlBlock>}). Nothing registers an observer in this build.
#pragma once

#include <algorithm>
#include <memory>
#include <mutex>
#include <vector>

namespace api {

template <class T>
class IObserver {
public:
    virtual ~IObserver() = default;
    virtual void Update(const T& valor) = 0;                          // slot 2
};

template <class T>
class IObservable {
public:
    // Destructors:
    //   T = std::shared_ptr<IImage>
    //     wasm func 11651 (slot 0): destroy the mutex (+24; the pthread stub, func 150), free the observer
    //                               vector (+12), release the value (+8 control block).
    //     wasm func 11650 (slot 1): the same + operator delete.
    //   T = std::vector<FormHandle<MEDIA>> - slot 0 is a thunk into the merged body api_f3940 (mutex +28,
    //   observers +16, then every FormHandle of the value vector releases its control block), slot 1 = slot 0
    //   + operator delete:
    //     MEDIA = IScreen    wasm func 3470 / 5052   (also slots of CFormStack<IScreen>::CObservable)
    //     MEDIA = IScreenMT  wasm func 3458 / 5019
    //     MEDIA = IPaper     wasm func 3669 / 5511
    virtual ~IObservable() = default;

    void Attach(IObserver<T>* observador)                              // inlined (CDSImageField ctor)
    {
        std::lock_guard trava(m_mutex);
        m_observadores.push_back(observador);
    }

    void Detach(IObserver<T>* observador)                              // inlined (~CInfoMTLCD, ...)
    {
        std::lock_guard trava(m_mutex);
        m_observadores.erase(std::remove(m_observadores.begin(), m_observadores.end(), observador),
                             m_observadores.end());
    }

    const T& GetValor() const { return m_valor; }                     // name inferred

protected:
    void SetValor(const T& valor)                                      // inlined; name inferred
    {
        std::lock_guard trava(m_mutex);
        m_valor = valor;
        for (auto* observador : m_observadores)
            observador->Update(m_valor);                               // IObserver slot 2
    }

    T                           m_valor;          // +4
    std::vector<IObserver<T>*>  m_observadores;   // +12 / +16
    std::mutex                  m_mutex;          // +24 / +28
};

} // namespace api
