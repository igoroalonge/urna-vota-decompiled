// uenux2/src/api/audio/crhvoicetexttospeech.cpp  -- index written by unit u34 (the file belongs to unit u32;
// see also crhvoicetexttospeech.u33.cpp).
//
// RHVoice 1.14.0 functions (upstream src/core/..., src/include/core/...) that the tools filed in unit u34
// under "app:api" because their main caller is api::CRHVoiceTextToSpeech::Sintetiza (vtable slot 11, func
// 10841: the whole RHVoice request pipeline inlined, 75 KB). They were identified against the upstream sources
// of tag 1.14.0 (downloaded from github.com/RHVoice/RHVoice) and are library code: not rewritten here.
// All of them run only when the voter's audio is on (accessible vote); none was sampled in the recorded
// sessions (audio off).
//
//   wasm func 5213  RHVoice::sentence_event::sentence_event(const utterance&)       events.hpp
//                   start = first "Token".position, length = last.position + last.length - start;
//                   throws relation_not_found / item_not_found / feature_not_found (vtable set to
//                   sentence_event; the caller then stores sentence_starts/ends_event's vptr)
//   wasm func 5212  RHVoice::word_event::word_event(const item& token)                events.hpp
//                   start = token.get("position"), length = token.get("length") (unsigned long features)
//   wasm func 5255  RHVoice::speech_processor::finish()                              speech_processor.cpp
//                   flush input, on_end_of_input, on_output, pass insertions/output to `next`, next->finish()
//                   (recursive), on_finished; stops as soon as the stream state says "stopped"
//   wasm func 5254  RHVoice::equalizer::read_coefs(coefs_t&, std::istream&)          equalizer.cpp
//                   skip_comments() inlined ('#' lines); reads 6 doubles and divides them by a0 (the 4th).
//                   Only reached for eq.txt, which Letícia-F123 does not have (see docs/libraries/rhvoice.md)
//   wasm func 5277  RHVoice::pitch::editor::reset()                                  pitch.cpp
//                   clears the value vectors and the two std::queue (deque, 170 x 24-byte blocks),
//                   prev_point = point_t() with def.time='m', def.value='x' (the 16-bit 0x786D store),
//                   mod_flag = has_edits = false. Called from the inlined editor::init (skipped when the
//                   key is lzero = -1e10) and at the end of each utterance
//   wasm func 5280  std::vector<RHVoice::pitch::target_t>::push_back(const target_t&) (4-byte target_t:
//                   first, last, time, value) - pitch::targets_spec_parser::parse, inlined into 10841
//   wasm func 5298  RHVoice::fst::append_input_symbol(const item& i, input_symbols& dest) const   fst.hpp
//                   dest.push_back({name, symbols.id(name, 1)}) with name = i.get("name"). Callers:
//                   fst::translate over items (3572: brazilian_portuguese/russian post_lex, homographs) and 10841
//   wasm func 5302  RHVoice::language::decode_as_character(item&, const std::string&) const   language.cpp
//                   if !decode_as_known_character (vtable slot 6) and verbosity & verbosity_spell (4):
//                   decode_as_unknown_character inlined - for each word of msg_char_code: "%d" -> the code
//                   point number spoken by numbers_fst (language +124), else a child item named after the
//                   word; then token.set("unknown", true)
//   wasm func 5303  RHVoice::language::decode_as_digit_string(item&, const std::string&) const
//                   one spell_fst translation (language +332, alphabet +344) per code point; throws
//                   utf8::invalid_code_point for surrogates / > 0x10FFFF
//   wasm func 5263  RHVoice::userdict::dict::simple_search(const std::string& w) const   userdict.cpp
//                   std::map<std::string, std::string> simple (+12): value or ""
//   wasm func 5340  RHVoice::voice_profile::voice_for_text<std::vector<utf8::uint32_t>::const_iterator>
//                   (voice_profile.hpp): the voice whose language has most letters of the text
//                   (language_info letters set at +776; has_common_letters and count_letters_in_text inlined;
//                   returns at once when the first voice wins)
//   wasm func 5432  std::distance(utf::text_iterator first, utf::text_iterator last)  utf.hpp / document.hpp
//                   forward-iterator distance: counts code points (utf8::next inlined with its
//                   invalid_utf8 / not_enough_room exceptions); used for token positions and lengths
// ---------------------------------------------------------------------------------------------------------
