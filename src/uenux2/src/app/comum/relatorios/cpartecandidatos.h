// uenux2/src/app/comum/relatorios/cpartecandidatos.h  (path inferred from cpartecandidatos.cpp srclocs)
// Reconstructed from vota_web_wasm.wasm (unit u25).
//
// Report parts (api::IReportPart) of the "LISTA DE CANDIDATOS" section of the zerésima (built by
// vota::CGeraZeresimaBase::StartState, wasm 11946, the only function that stores their vtables).
// api::IReportPart slots: [0] destructor, [1] deleting destructor, [2] Imprime() const.
//
//   class                              typeinfo  vtable    [0]     [1]     [2] Imprime
//   comum::CParteCandidatos            1576500   1576396   2876*   5590*   11215
//   comum::CParteCandidatosMajoritarios 1576520  1576416   5589*   5588*   11214 (srcloc :75)
//   comum::CParteCandidatosProporcionais 1576532 1576452   3867    11210   11213 (unit u04 fragment)
//   comum::CParteCandidatosConsultas   1576544   1576472   5589*   5588*   11212 (srcloc :174)
//   (* = ICF bodies shared with other classes of the same layout)
#pragma once

#include <memory>

#include "api/gui/iform.h"
#include "api/gui/reports/ireportpart.h"

namespace comum {

using SharedPaperForm = std::shared_ptr<api::IForm<api::IPaper>>;
using SharedReportPart = std::shared_ptr<api::IReportPart>;

// Dispatcher for the current cargo.                                      44 bytes
class CParteCandidatos : public api::IReportPart {
public:
    CParteCandidatos(SharedReportPart headerCargo, SharedReportPart semCandidatos, SharedReportPart majoritarios,
                     SharedReportPart proporcionais, SharedReportPart consultas);
    void Imprime() const override;                                      // wasm 11215

private:
    SharedReportPart m_headerCargo;          // +4
    SharedReportPart m_semCandidatos;        // +12  "Não há candidatos concorrendo"
    SharedReportPart m_majoritarios;         // +20
    SharedReportPart m_proporcionais;        // +28
    SharedReportPart m_consultas;            // +36
};

// Majoritarian cargo: header, one detail line per apt candidate, trailer.   28 bytes
class CParteCandidatosMajoritarios : public api::IReportPart {
public:
    CParteCandidatosMajoritarios(SharedPaperForm header, SharedPaperForm detalhe, SharedPaperForm trailer);
    void Imprime() const override;                                      // wasm 11214 (srcloc :75)

private:
    SharedPaperForm m_header;                // +4
    SharedPaperForm m_detalhe;               // +12  DetalheCandidatoZE (slot 1635)
    SharedPaperForm m_trailer;               // +20
};

// Referendum ("consulta"): header, one line per answer, trailer.            28 bytes
class CParteCandidatosConsultas : public api::IReportPart {
public:
    CParteCandidatosConsultas(SharedPaperForm header, SharedPaperForm detalhe, SharedPaperForm trailer);
    void Imprime() const override;                                      // wasm 11212 (srcloc :174)

private:
    SharedPaperForm m_header;                // +4
    SharedPaperForm m_detalhe;               // +12  answer of the current CRespostas entry (slot 3051)
    SharedPaperForm m_trailer;               // +20
};

// CParteCandidatosProporcionais: see src/uenux2/src/app/comum/dados/u04-foreign-fragments.cpp (wasm 11213).

} // namespace comum
