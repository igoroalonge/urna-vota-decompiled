// uenux2/src/api/pattern/iobservable.h  -- FRAGMENT written by unit u33 (the header is reconstructed in
// iobservable.h; this file only documents one merged destructor body).
#include <memory>
#include <mutex>
#include <vector>

#include "api/gui/iform.h"            // api::FormHandle<MEDIA>
#include "api/pattern/iobservable.h"

namespace api {

// wasm func 3940 - merged body (merge-similar-functions: the vtable is the 2nd parameter) of
//     IObservable<std::vector<FormHandle<MEDIA>>>::~IObservable()
// for MEDIA = IScreen (slot 0 thunk 3470), IScreenMT (3458) and IPaper (3669).
// In order: store the vptr; ~std::mutex at +28 (only the noexcept pthread stub, func 150, survives); free the
// observer vector (+16); release every FormHandle of the value vector (+4..+12, 12-byte elements whose last
// word is a shared_ptr control block) and free it. Returns this.
//
// Source form (the template's implicit destructor):
//   template <class T> IObservable<T>::~IObservable() = default;
//   // members: T m_valor (+4); std::vector<IObserver<T>*> m_observadores (+16); std::mutex m_mutex (+28)
template class IObservable<std::vector<FormHandle<IScreen>>>;
template class IObservable<std::vector<FormHandle<IScreenMT>>>;
template class IObservable<std::vector<FormHandle<IPaper>>>;

} // namespace api
