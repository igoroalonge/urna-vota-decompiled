// FRAGMENT reconstructed by unit u39 from vota_web_wasm.wasm.
// Original file (attested: srcloc cmostraqrcodebu.cpp:38 is the IQRCodeBUDS constructor):
// uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.cpp   (rest: cmostraqrcodebu.cpp by u09, .u19.cpp by u19)
//
// BOLETIM DE URNA: vota::IQRCodeBUDS is the poly-singleton that holds the texts of the BU QR codes shown on
// the urna screen at the end of the day (CMostraQRCodeBU; the paper BU prints the same content split in
// parts of at most 1100 characters, the screen version uses parts of at most 2500) and the index of the part
// on display. RTTI: vota::IQRCodeBUDS (typeinfo @1542224, class without base, vtable @1542092):
//   [0] destructor 12050  [1] deleting destructor 12049   (declared in cmostraqrcodebu.h, unit u09)
// It is owned by a std::unique_ptr inside api::CPolySingletonList (shared_ptr_pointer<IQRCodeBUDS*,
// default_delete> vtable @1542272), so it is destroyed only when the list is cleared.
#include "vota/eleitor/fimvotacao/cmostraqrcodebu.h"

namespace vota {

// wasm func 12050 - vtable slot 0 (complete-object destructor, returns `this`): stores its own vptr, destroys
// the strings of m_qrcodes (+4, 12-byte elements) from the back and frees the buffer. m_indice (+16) is trivial.
// wasm func 12049 - vtable slot 1: the same body followed by operator delete (free).
IQRCodeBUDS::~IQRCodeBUDS() = default;

}  // namespace vota
