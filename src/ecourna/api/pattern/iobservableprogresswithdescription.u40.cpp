// FRAGMENT for ecourna-lib/ecourna/api/pattern/iobservableprogresswithdescription.hpp (class reconstructed by
// unit u12). Reconstructed by unit u40 from vota_web_wasm.wasm.
#include "ecourna/api/pattern/iobservableprogresswithdescription.hpp"

namespace ecourna::api::pattern {

// wasm func 9528 (vtable @1111068 slot 1): the DELETING destructor of
//   virtual ~IObservableProgressWithDescription() = default;
// i.e. ~signal() inlined (store the signal2 vptr @1111084 and the class vptr, release the
// shared_ptr<signal_impl> at +12: use count through control-block slot 2, weak count through slot 3; a
// throwing release goes to __clang_call_terminate) followed by operator delete(this).
// The complete-object destructor is func 1526 (unit u12).

// wasm func 9551 (table slot 6082) is library code reached from here: boost::signals2's grouped_list copy
// constructor (func 9557) re-inserts the group map with
//   std::map<group_key_type, list_iterator, group_key_less<int, std::less<int>>>::insert(first, last)
// where group_key_type = std::pair<slot_meta_group, boost::optional<int>>: the comparator compares the
// meta group (+16) and, only when both are `grouped_slots` (1), the optional int (+24). 32-byte nodes.
// It runs when a signal's slot list is copied on write (connect/disconnect while in use).

} // namespace ecourna::api::pattern
