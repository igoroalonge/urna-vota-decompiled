// FRAGMENT reconstructed by unit u07 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cformbuilder.cpp (attested by srcloc records of AddStatusHeader, GetLabeledKeyPos)
// and cformbuilder.h (template helpers). Only the functions assigned to u07 are here; see also
// cformbuilder.u02.cpp. Merge into cformbuilder.cpp / .h.
#include "api/gui/cformbuilder.h"

#include <algorithm>
#include <string>

namespace api {

// wasm func 426 (tools: "ecourna_f426"; observed executing)                          // name inferred
// Appends a field to the builder and gives it a name unique inside the form: the field's class name
// (IFormFieldBase slot 7, e.g. "CTextField", "CImageField") followed by 1, 2, 3...
// CFormBuilder is { std::vector<std::shared_ptr<IFormField<IScreen>>> m_campos; } (12 bytes);
// IFormFieldBase keeps the name in a std::string at +12.
void CFormBuilder::Add(std::shared_ptr<IFormField<IScreen>> campo)
{
    std::string nome;
    for (unsigned n = 1;; ++n) {
        nome = campo->GetNomeClasse() + std::to_string(n);                  // slot 7; std::to_string = func 327
        const bool usado = std::any_of(m_campos.begin(), m_campos.end(),
                                       [&](const auto& c) { return c->GetNome() == nome; });
        if (!usado)
            break;
    }
    campo->SetNome(nome);
    m_campos.push_back(campo);                                              // func 1400 (vector<shared_ptr>::push_back)
}

// Template helpers of cformbuilder.h. Every call site builds the text/image source with make_shared,
// then the field with make_shared, and calls Add(). wasm-opt kept one out-of-line body per distinct
// (field, source) combination; those that ended up in unit u07 are:
//
//   func 1191  Add<CTextField>(SPoint, make_shared<CDataText<std::string(*)()>>(alinhamento, fonte), SFont, 2, 1)
//   func 2782  Add<CTextFieldMultiLine>(SRect, make_shared<CFixedText>(alinhamento, texto), SFont)
//   func 3068  Add<CTextFieldDoubleLine>(SPoint, SPoint, TPosition xMax,
//                                        make_shared<CDataText<std::string(*)()>>(0, fonte), SFont, TColor)
//   func 4129  Add<CImageField>(SPoint, make_shared<CDataImage<comum::CCandidaturasDSFoto>>(indice), anchor 1)
//   func 6528  Add<CTextField>(SPoint, make_shared<CDataText<comum::CCargoDSNomeSexoCandidato>>(2, {indice, abrevia}),
//                              SFont, 2, 1)
//   func 3060  Add<CMaskedTextField<CGrayedFramedText>>(CGrayedFramedText(digitos, pos),
//                                    make_shared<CDataText<const std::string&(*)()>>(0, VotoDigitado))
// (their uses are written out in src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp)
//
// Generic form:
//   template <typename FIELD, typename... ARGS>
//   std::shared_ptr<FIELD> CFormBuilder::Add(ARGS&&... args)
//   {
//       auto campo = std::make_shared<FIELD>(std::forward<ARGS>(args)...);
//       Add(campo);                       // func 426
//       return campo;
//   }

} // namespace api
