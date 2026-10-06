// uenux2/src/api/gui/{cmoviefield,cimagefield,cinputfield}.*  -- FRAGMENT written by unit u33.
//
// wasm func 2904 - a body shared by four unrelated functions that each return an 11-character literal:
//     api::CMovieField::GetClassName()                (slot 7, thunk 11059)  "CMovieField"
//     api::CImageField::GetClassName()                (slot 7, thunk 11097)  "CImageField"
//     api::CInputField<CFramedText>::GetClassName()   (slot 7, thunk 12411)  "CInputField"
//     RHVoice::userdict::empty_string::describe()     (thunk 9690)           "EmptyString"
// std::string(const char (&)[12]) is inlined as: allocate 16 bytes, copy 8 bytes from s and 4 bytes from s+7,
// NUL at [11], size 11. merge-similar-functions turned the two load addresses (s, s+7) into parameters,
// so the thunks call 2904(result, this, s, s + 7). `this` is unused.
#include <string>

namespace api {

//   std::string CMovieField::GetClassName() const { return "CMovieField"; }
//   std::string CImageField::GetClassName() const { return "CImageField"; }
//   template <class MASK> std::string CInputField<MASK>::GetClassName() const { return "CInputField"; }

} // namespace api
