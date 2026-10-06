// uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.h  (path inferred from the .cpp srcloc)
// Reconstructed from vota_web_wasm.wasm (unit u25).
//
// comum::CRelatorioTesteImpressora : comum::CImprimirExtratoCarga — the "ESTADO DA URNA" report
// ("URNA OPERANDO EM PERFEITAS CONDIÇÕES DE FUNCIONAMENTO"), printed by vota::CVerificaHorarioZeresima
// (before the zerésima, doubling as a printer test) and by the "Mais informações" menu
// (vota::CImpressaoEstadoUrna, wasm 11911). CImprimirExtratoCarga is a template-method base with no
// vtable of its own in VOTA: its Imprime() is vota_f5591 (tools file: cverificahorariozeresima.cpp),
// which calls the 24 hooks below; the slot meanings come from how 5591 uses each result.
//
// RTTI typeinfo @1576764 (si, base comum::CImprimirExtratoCarga @1576380), vtable @1576660 (26 slots).
// Layout (8 bytes): +0 vptr, +4 bool (0), +5 bool m_usaPleito2 (contingency urna on/after the 2nd-round
// date, computed by the constructor wasm 5586).
#pragma once

#include <string>
#include <vector>

#include "api/gui/cpaperformbuilder.h"
#include "comum/dados/chv.h"
#include "comum/dados/md/estadoaplicacao/cdadocorrespondencia.h"
#include "comum/dados/md/processoeleitoral/cpleito.h"
#include "comum/relatorios/cimprimirextratocarga.h"
#include "ecourna/app/dados/parametrizacaourna/cparametrosurna.h"   // CTituloRelatorio (unit u14)

namespace comum {

class CRelatorioTesteImpressora : public CImprimirExtratoCarga {
public:
    CRelatorioTesteImpressora();                                               // wasm 5586 (other unit)
    ~CRelatorioTesteImpressora() override = default;                          // slot 0 = 174, slot 1 = 144

    // hooks of CImprimirExtratoCarga (all names inferred from their use in vota_f5591)
    bool EhTreinamento() const override;                                      // [2]  wasm 11201
    bool PossuiHorarioVerao() const override;                                 // [3]  wasm 11202
    bool ImprimeQRCode() const override { return true; }                      // [4]  icf_ret_1 (434)
    std::string GetConteudoQRCode() const override;                           // [5]  wasm 11200
    std::vector<std::string> GetConteudosQRCodeExtras() const override { return {}; }   // [6] 902
    const CHV::SPeriodo& GetHorarioVerao() const override;                    // [7]  wasm 11199
    bool EstaEmHorarioVerao() const override;                                 // [8]  wasm 11198
    md::estadoaplicacao::CDadoCorrespondencia GetCorrespondencia() const override;   // [9] wasm 11197
    void ConfiguraLabels() const override;                                    // [10] wasm 11196
    const std::vector<ecourna::app::dados::CTituloRelatorio>& GetCabecalho() const override;   // [11] 11195
    bool EhEleicaoComunitaria() const override;                               // [12] wasm 11194
    std::string GetNomeProcessoEleitoral() const override;                    // [13] wasm 11192
    const md::CPleito& GetPleito() const override;                            // [14] wasm 11191
    std::vector<std::string> GetLinhasLocal() const override;                 // [15] wasm 11188
    std::string GetUF() const override;                                       // [16] wasm 11189
    std::string GetTipoUrna() const override;                                 // [17] wasm 11190
    std::string GetMensagemFinal1() const override;                           // [18] wasm 11186
    std::string GetMensagemFinal2() const override;                           // [19] wasm 11184
    std::string GetSeparadorFase() const override;                            // [20] wasm 11183
    std::string GetTitulo() const override;                                   // [21] wasm 11187
    bool EhModoDemonstracao() const override;                                 // [22] wasm 11193
    void IncluiAposCorrespondencia(api::CPaperFormBuilder&) const override {} // [23] icf_nop (425)
    void IncluiAposCabecalho(api::CPaperFormBuilder&) const override {}       // [24] icf_nop (425)
    void IncluiEmissao(api::CPaperFormBuilder& b) const override;             // [25] wasm 11216

private:
    bool m_reservado = false;        // +4 ?
    bool m_usaPleito2 = false;       // +5
};

// Data sources std::string(*)(const std::string& formato) (table slots 1103 / 1104), used through
// api::CDataTextFmt<...> by the status header of the screens and here by IncluiEmissao.
// Observed executing (screen clock). Path uncertain; the tools filed them here by caller. names inferred
std::string FormataDataAtual(const std::string& formato);                   // wasm 5472
std::string FormataHoraAtual(const std::string& formato);                   // wasm 5473

// Functor behind the rotating "estado da urna" QR codes (std::function<std::string()>, vtable @1538332).
// The constructor COPIES the field list (func 1300 = vector<pair<string,string>> copy constructor; in 11200
// the source vector is a temporary destroyed right after), so it takes a const reference.
class CQRCodeDS {
public:
    explicit CQRCodeDS(const std::vector<std::pair<std::string, std::string>>& campos) : m_campos(campos) {}
    std::string operator()() const;                                          // wasm 5782
    // inlined accessor (name inferred): 11200 and api::CImageFieldUpdate (3059) pass the functor's vector (+0)
    // to AdicionaOrigemQRCode (wasm 5783, cqrcodeds.cpp) after constructing the functor.
    std::vector<std::pair<std::string, std::string>>& Campos() { return m_campos; }

private:
    std::vector<std::pair<std::string, std::string>> m_campos;               // +0
};

} // namespace comum
