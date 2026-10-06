// Reconstructed from vota_web_wasm.wasm (unit u31) - DECLARATIONS ONLY.
// The functions below are defined by unit u29 (the "web platform: free functions" of the tools), in fragments
// of the same original files; they are declared here, with u29's names, so that the u31 classes read
// naturally. All names are inferred.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace simulador {

// cwasmscreen.u29.cpp - func 5098: ISO-8859-1 -> UTF-8 (a byte >= 0x80 becomes 0xC0 | b >> 6, 0x80 | (b & 0x3F)).
std::string Latin1ParaUtf8(const std::string& latin1);
// (func 5113, the path scaler, is the private member CWasmScreen::EscalaCaminho - see cwasmscreen.h.)

// cwasmresource.u29.cpp - func 2626: resource name (":/resource/...") -> readable MEMFS path, "" when missing
// (strips ':', maps /resource/images/ and /resource/gifs/ to the package directories, func 4996 inserts the
// "Mulher4" GIF variant; logs "resolve"/"missing" through js_resource_log).
std::string ResolveCaminho(const std::string& nome);
// cwasmresource.u29.cpp - func 3452: whole file as bytes (empty when it cannot be opened).
std::vector<unsigned char> LeArquivo(const std::string& caminho);

// cwasminit.u29.cpp - func 1524: CWasmLogBus::GetInst().Publica(INIT, std::format("CWasmInit::{} {}", comando, detalhe)).
void LogComando(std::uint16_t comando, const std::string& detalhe);

}  // namespace simulador

// vota_web_wasm.u29.cpp - func 3533: the only caller of js_emit_event (window CustomEvent "vota:*").
void EmitEvent(const char* nome, const std::string& json);
