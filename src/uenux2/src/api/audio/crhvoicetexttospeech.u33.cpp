// uenux2/src/api/audio/crhvoicetexttospeech.cpp  -- FRAGMENT written by unit u33 (path inferred; the file
// belongs to unit u32, see crhvoicetexttospeech.h/.cpp).
//
// 1. api::Logger, the RHVoice::event_logger that CRHVoiceTextToSpeech's constructor (func 10842) installs
//    in place of RHVoice's default logger (engine::init_params::logger).
//    RTTI: api::Logger : RHVoice::event_logger   typeinfo @1586208, vtable @1586196 = {174, 144, 10815}
//    (trivial destructors, then log). 4 bytes (vptr only).
// 2. The index of the RHVoice / libc++ helper functions that the tools filed in unit u33 because their
//    main caller is the 75 KB CRHVoiceTextToSpeech::Sintetiza (func 10841, the whole RHVoice request
//    pipeline inlined). They are upstream RHVoice 1.14.0 / libc++ code and are not rewritten here.
#include <iostream>
#include <string>

#include "core/event_logger.hpp"     // RHVoice::event_logger, RHVoice_log_level

namespace api {

class Logger : public RHVoice::event_logger {
public:
    void log(const std::string& tag, RHVoice_log_level nivel, const std::string& mensagem) const override;
};

// wasm func 10815 - vtable slot 2 (RHVoice::event_logger::log)
// Only errors are reported; RHVoice's info/warning messages ("creating a new engine", "engine created", ...)
// are dropped. The stream is std::clog (@1923832: initialised on stderr without unitbuf and without tie,
// unlike std::cerr @1923688), so in the browser the line goes to Emscripten's stderr, i.e. the glue's
// err() = console.error (unless the page sets Module.printErr).
void Logger::log(const std::string& tag, RHVoice_log_level nivel, const std::string& mensagem) const
{
    if (nivel == RHVoice_log_level_error)                    // 4
        std::clog << "RHVoice [" << tag << "]: " << mensagem << std::endl;
}

} // namespace api

// ---------------------------------------------------------------------------------------------------------
// RHVoice 1.14.0 functions (src/core/..., src/include/core/...) that ended up in unit u33.
// Identified against the upstream sources (tag 1.14.0); bodies are the upstream ones.
//
//   wasm func 590 RHVoice::unicode::properties(c)      unicode.cpp  - binary search of records[] (23,697 x 20 bytes
//         @626784: {code, category, upper, lower, properties}), returns .properties or 0
//   wasm func 854 RHVoice::unicode::tolower(c)         unicode.cpp  - .lower or c   (used by str::less, config, profiles)
//   wasm func 1144 RHVoice::unicode::category(c)        unicode.cpp  - .category or empty_category (@626775)
//   wasm func 1896 RHVoice::emoji_scanner::process(cp)  emoji.cpp    - find_emoji_char inlined (1,523 x 8 bytes @552688),
//         valid() = p & (emoji | emoji_component) (the i64 mask 0x11_00000000), state->next(c),
//         ++length, result = length if state->can_end_emoji()
//   wasm func 2210 RHVoice::userdict::position::forward_token()        userdict.cpp  (throws item_not_found from
//         item::parent(); clear() stores token = 0, character = token_start 0x110000)
//   wasm func 2703 RHVoice::userdict::position::set_token(item&)       userdict.hpp  (text = &token.get("name")
//         .as<std::string>(); throws feature_not_found / bad_cast)
//   wasm func 3547 RHVoice::userdict::dict::should_ignore_token(pos)   userdict.cpp  ("verbosity" == silent, or "pos"
//         not in {"word", "sym", "char"})
//   wasm func 2214 RHVoice::item::append_child(item& other)            item.cpp      (new 32-byte item sharing
//         other's data, append_item inlined; the returned reference is unused)
//   wasm func 3529 RHVoice::voice_search_criteria::operator()(const voice_info&) const   voice.cpp
//   wasm func 3574 RHVoice::fst::translate<str::utf8_string_iterator, item::back_insert_iterator>(first, last, out)
//         fst.hpp - append_input_symbol (rhvoice_f727) per code point, then do_translate (rhvoice_f2709)
//   wasm func 2756 std::find_if(first, last, std::not1(str::is_space))  } str::tokenizer<str::is_space>::iterator::
//   wasm func 3571 std::find_if(first, last, str::is_space)             } operator++ (utf::text_iterator, utf8::next)
//   wasm func 2754 copy of a utf::text_iterator range into a utf8::uint32_t buffer (std::copy /
//         __uninitialized_allocator_copy: identical code; callers vector<char32_t> insert 2755, 5321, 10841);
//         utf8::next inlined (throws utf8::invalid_utf8 / invalid_code_point / not_enough_room)
//   wasm func 2715 std::__tree<std::string, std::shared_ptr<T>>::destroy(node) - ICF for every map<string, shared_ptr>
//         of RHVoice (utterance relation_map, language feature_functions, resource_list<T>)
//   wasm func 2716 std::__tree<std::string, str::less>::__emplace_hint_unique_key_args  (set<string, str::less>
//         insert with hint; str::less = shared_f447, case-insensitive UTF-8 compare through tolower 854)
//
// Two more libc++ functions of unit u33 that have nothing to do with RHVoice (listed here only so that
// every function of the unit appears in a source file):
//   wasm func 1840 std::filesystem::path::__parent_path() const (libc++ path.cpp: PathParser begin +
//         ConsumeRootDir, else two decrements from the end). Callers: CMontadorHash::CalculaHashGeral (5360),
//         the 7-Zip glob helper (5821), func 3706 and the simulator bootstrap (8302). Observed.
//   wasm func 3266 std::map<uebyte, std::string>::insert(hint, const value_type&) (__emplace_hint_unique_key_args,
//         32-byte node: key byte +16, string +20) used by initializer-list construction of uebyte-keyed maps.
//         Callers: api::getResourceMovie (3076) and vota::CDigitalReconhecida::ProcessInput (10481). Observed.
// ---------------------------------------------------------------------------------------------------------
