// FRAGMENT of uenux2/src/app/comum/comparecimentomesario/cregistradormesario.cpp (attested by srcloc :85; the class is
// reconstructed by unit u20 in src/uenux2/src/app/comum/u20-foreign-fragments.cpp).
// Reconstructed from vota_web_wasm.wasm (unit u35).
namespace comum {

// wasm func 2221 - name inferred (u20). Not observed executing.
// Whether the poll-worker attendance cache ("registro de comparecimento de mesários", table comparecimento_mesario
// of uenux.db) was created, WITHOUT creating it (GetInst creates it lazily). The lock of the static mutex @1909908
// is a no-op in this single-threaded build; only the unlock residue remains.
// Callers: CGravadorRCSecao::LeChavePublica (11616) and vota::CGeraRelatorios::StartState (12105).
bool CRegistradorMesario::Existe()
{
    std::lock_guard<std::mutex> lock(s_mutex);          // @1909908
    return s_instancia != nullptr;                      // @1909932
}

} // namespace comum
