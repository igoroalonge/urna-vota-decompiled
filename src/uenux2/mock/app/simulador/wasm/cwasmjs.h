// Reconstructed from vota_web_wasm.wasm (unit u31).
// Original file UNKNOWN - name and path inferred: the declarations of the TSE-written JavaScript functions
// (the js_* members of the Emscripten --js-library merged into upstream/site/wasm/vota_web_wasm.js) used by
// the simulator mocks of this directory. Import indices and signatures: analysis/imports.tsv.
// All arguments are i32 unless noted; strings are UTF-8 pointers read with UTF8ToString.
#pragma once

#include <cstddef>   // std::size_t (js_image)
#include <cstdint>

extern "C" {

// --- voter screen (simulador::CWasmScreen) -------------------------------------------------------------
void js_init(int largura, int altura);                                              // import 88 (Ia)
void js_clear(const char* cor);                                                     // import 56 (ca)
void js_fill(int x, int y, int w, int h, const char* cor);                          // import 38 (M)
void js_line(int x1, int y1, int x2, int y2, const char* cor, int espessura);       // import 87 (Ha)
void js_circle(int x, int y, int raio, const char* cor, int espessura);             // import 86 (Ga)
void js_round_rect(int x, int y, int w, int h, int raio, const char* cor, int espessura);   // import 85 (Fa)
void js_gray_gradient(int x, int y, int w, int h);                                  // import 84 (Ea)
void js_path(int n, const int* tipos, const double* valores, const char* cor, int preenche, int espessura);   // 55 (ba)
void js_text(int x, int y, int larguraMax, const char* texto, int tamanho, int negrito, int italico,
             const char* cor, int alinhamento);                                     // import 83 (Da)
int  js_measure_text_width(const char* texto, int tamanho, int negrito, int italico);   // import 82 (Ca)
void js_image(const void* dados, std::size_t tamanho, int x, int y, int w, int h);  // import 34 (I)
void js_log(const char* mensagem);                                                  // import 35 (J), debug only

// --- microterminal (simulador::CWasmScreenMT) ----------------------------------------------------------
void js_mt_set(const char* texto, int ocupado);                                     // import 81 (Ba)

// --- resources, threads, clock -------------------------------------------------------------------------
void js_resource_log(const char* acao, const char* recurso, const char* caminho, int tamanho);   // import 29 (D)
void js_thread_log(const char* mensagem);                                           // import 26 (A)
double js_obter_data_hora_local_navegador();                                        // import 54 (aa), f64

// --- speech (simulador::CWasmWebSound) -----------------------------------------------------------------
void js_wasm_web_sound_play_wav(const void* cabecalho, int tamanhoCabecalho, const void* dados, int tamanho,
                                int volume, int modo);                              // import 57 (da)
int  js_wasm_web_sound_get_status();                                                // import 39 (N)
void js_wasm_web_sound_wait_async(int id);                                          // import 90 (Ka)
void js_wasm_web_sound_cancel_wait(int id);                                         // import 89 (Ja)
void js_wasm_web_sound_mute(int mudo);                                              // import 91 (La)
void js_wasm_web_sound_stop();                                                      // import 92 (Ma)
void js_wasm_web_sound_pause();                                                     // import 93 (Na)

}  // extern "C"
