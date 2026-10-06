// ecourna-lib/ecourna/api/pattern/iobservableprogresswithdescription.hpp   (path inferred from the class name)
//
// Reconstructed from vota_web_wasm.wasm. Unit u12 (only the destructor, func 1526, belongs to the unit).
//
// RTTI: ecourna::api::pattern::IObservableProgressWithDescription (typeinfo @1110992, polymorphic, no base).
// Vtable @1111068 = { [0] ~D1 func 1526, [1] ~D0 func 9528 }.   (A second, one-slot vtable @1110944 with
// func 10620 exists but is never stored by any function.)
//
// It carries a Boost.Signals2 signal. Observers (e.g. a progress bar) connect to it and receive
// (current, total, description). CZip emits (permil, 1024, "Compactando arquivo <name>...").
#pragma once

#include <string>

#include <boost/signals2/signal.hpp>

namespace ecourna::api::pattern {

class IObservableProgressWithDescription {
public:
    using TSignal = boost::signals2::signal<void(unsigned long, unsigned long, const std::string&)>;

    // wasm func 1526 (complete-object destructor; also slot 0 of ICompressor, whose destructor is trivial).
    // Body = boost::signals2::signal::~signal(): re-stores both vptrs, then releases the
    // shared_ptr<signal_impl> at +8/+12 (use count, then weak count, through the control block's vtable).
    virtual ~IObservableProgressWithDescription() = default;

    // Observer registration: not seen in this unit (name and shape guessed).
    boost::signals2::connection Connect(const TSignal::slot_type& slot) { return m_progresso.connect(slot); }   // ? name inferred

protected:
    // Emitted by the implementations (e.g. CZip, func 9537) as m_progresso(current, total, description).
    TSignal m_progresso;   // +4 (signal object: vptr +4, shared_ptr<signal_impl> +8/+12)
};                         // sizeof 16

} // namespace ecourna::api::pattern
