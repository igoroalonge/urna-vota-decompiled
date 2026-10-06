// Reconstructed from vota_web_wasm.wasm (unit u07). Original: uenux2/src/app/vota/eleitor/csincronismovotoeleitor.h
// (path inferred from csincronismovotoeleitor.cpp, attested by the srcloc record @1534132, line 67).
//
// "Sincronismo do voto do eleitor": the step that makes one voter's votes durable. After the voter
// confirms the last cargo, CEleitorVotando hands the votes to the RDV (Registro Digital do Voto) in
// memory and switches to the state vota::CSincronismoEleitor ("GRAVANDO" progress screen). When that
// state receives message 5 it calls ISincronismoVotoEleitor::GetInst().SincronizaVoto(); on success it
// moves to CFimVotoEleitor ("FIM").
//
// RTTI:  vota::impl::ISincronismoVotoEleitor
//          <- vota::impl::CSincronismoVotoEleitor               typeinfo @1534176, vtable @1534164
//          <- (anonymous namespace)::CSincronismoVotoEleitorWeb typeinfo @1527264, vtable @1527252
//                                                                (uenux2/wasm/vota_web/vota_web_wasm.cpp)
//        vtable: [0] ~dtor (icf 174)  [1] deleting dtor (free, 144)  [2] SincronizaVoto
//
// Web build: main() (func 10307) pushes CSincronismoVotoEleitorWeb into the CPolySingletonList first,
// and its SincronizaVoto is the ICF body "return true" (func 434). The real implementation below is
// therefore compiled in but never executed: no RDV, vota.bin, uenux.db or signature file is rewritten
// after a vote in the simulator (only the log changes, see analysis/runtime/README.md).
#pragma once

namespace vota::impl {

class ISincronismoVotoEleitor {
public:
    virtual ~ISincronismoVotoEleitor() = default;

    /// Persists the current voter's votes (RDV + application state + voter roll) on both flash
    /// memories and re-signs them. Returns true on success (throws on failure).        // name inferred
    virtual bool SincronizaVoto() = 0;                                                  // slot 2

    /// csincronismovotoeleitor.cpp:67. Registers CSincronismoVotoEleitor as the default implementation
    /// if nobody registered one before (inlined into CSincronismoEleitor::ProcessMessage, func 7181).
    static ISincronismoVotoEleitor& GetInst();
};

/// 4 bytes (vptr only): operator new(4) in func 7181.
class CSincronismoVotoEleitor final : public ISincronismoVotoEleitor {
public:
    bool SincronizaVoto() override;                                                     // wasm func 7174
};

} // namespace vota::impl
