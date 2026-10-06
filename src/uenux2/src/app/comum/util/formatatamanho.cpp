// uenux2/src/app/comum/util/formatatamanho.cpp   (path and name inferred: no std::source_location record)
// Reconstructed from vota_web_wasm.wasm (unit u26).
//
// wasm func 1947 (tools: vota_f1947, "curated: strings '{}.{} MB'/'{}.{} KB'"). Human-readable byte count.
// Callers in three translation units: comum::IInterfaceInit::MontarMRSemHabilitar (5896, "Tamanho da MR"),
// vota::CLogVota::LogaQuantidade (3268) and vota::CThreadMonitor::Run (10226, disk space) - so it is a comum
// helper (external linkage or an inline function of a header, deduplicated by the linker).
//
// Header (formatatamanho.h): namespace comum::util { std::string FormataTamanho(std::size_t bytes); }
//
// The parameter is size_t, i.e. 32 bits on wasm32: every caller that holds a 64-bit count (statvfs sizes,
// memory * 1024) is silently truncated modulo 4 GiB before formatting. The "decimal" digit is
// (resto / 1024) * 10 / 1024 truncated, so 1.99 GB prints as "1.9 GB".
#include <cstddef>
#include <format>
#include <string>

namespace comum::util {

std::string FormataTamanho(std::size_t bytes)
{
    const std::size_t gb = bytes >> 30;
    const std::size_t mb = bytes >> 20;
    const std::size_t kb = bytes >> 10;
    if (gb)
        return std::format("{}.{} GB", gb, ((mb & 1023) * 10) >> 10);
    if (mb)
        return std::format("{}.{} MB", mb, ((kb & 1023) * 10) >> 10);
    if (kb)
        return std::format("{}.{} KB", kb, ((bytes & 1023) * 10) >> 10);
    return std::format("{} B", bytes);
}

} // namespace comum::util
