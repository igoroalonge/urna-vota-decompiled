// FRAGMENT of uenux2/src/api/gui/iscreen.h reconstructed by unit u28 from vota_web_wasm.wasm.
// api::IScreen itself is reconstructed in iscreen.h (unit u17), which already defines the destructor inline
// with exactly this body; this fragment only documents the compiled function and is not meant to be included
// together with iscreen.h.
#pragma once

#include "api/gui/iform.h"

namespace api {

// wasm func 5071 (tools: api::IScreen::vf0) - IScreen::~IScreen(), complete-object destructor.
//   vtable slot 0 of api::IScreen (table 602, vtable @1529100) and of simulador::CWasmScreen (table 545,
//   vtable @1528824): CWasmScreen adds no member with a destructor, so clang emits ~CWasmScreen as an alias
//   of the base destructor. CWasmScreen's deleting destructor (func 8963) repeats the body and frees.
//   Body: store IScreen's vptr, then IForm<IScreen>::RemoveAll() (func 5069): pop every voter-screen form
//   from the static form stack (@1832632..1832636): clear the form's byte flag at +4, unlock residue of the
//   form's mutex at +28 (func 150), call virtual slot 4 of each object in its 8-byte-entry list (+8..+12), pop;
//   then the mutex residue @1832604 and func 2651 (unit u17 documents RemoveAll).
//
//   class IScreen {
//   public:
//       virtual ~IScreen() { IForm<IScreen>::RemoveAll(); }     // wasm func 5071
//       ...
//   };
//
// Only reached if the screen singleton is destroyed (CPolySingletonList teardown); not seen in the recorded votes.

}  // namespace api
