// uenux2/src/app/comum/comparecimentomesario/md/ccomparecimentomesario.h  (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u22).
//
// comum::md::CComparecimentoMesario = one row of the SQLite table comparecimento_mesario (uenux.db):
// the presence of one mesário (poll worker) in one period. The constructors are small out-of-line
// functions listed in this unit (2738, 2730, 2731); the class name is attested by the RTTI of
// shared_ptr<comum::md::CComparecimentoMesario> (@1593504) and by the DAO srclocs.
#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "api/util/cdatetime.h"
#include "comum/dados/md/cvalidadoridentidade.h"   // md::CEleitorIdentidade (string + tipo, 16 bytes)
#include "comum/dados/md/eleitor/cdedo.h"          // md::CDedo::TipoDedo

namespace comum::md {

// Period recorded in the row (column periodo_presente). The states return 1 for the registrations
// made before and during the vote and 2 for the final one (IPedeTituloMesario / IGestorDadoMesario
// slot 9: ICF bodies `return 1` (func 434) and `return 2` (func 2254)).        // names inferred
enum class EPeriodoPresente : int { ABERTURA = 1, ENCERRAMENTO = 2 };

class CComparecimentoMesario
{
public:
    // Accepted values 1..3 (ConverteEstadoBiometria). Set by CPedeDigitalMesario (func 10316):
    // 1 initial value (no capture: biometrics disabled or urna abroad), 2 the mesário has
    // fingerprints in the roll (they were compared), 3 no fingerprints in the roll / not a voter of the
    // section (the captured image is stored).                                   // names inferred
    enum class EstadoReconhecimentoBiometrico : int { NAO_COLETADA = 1, CONFERIDA_CADASTRO = 2, COLETADA = 3 };

    // wasm func 2730 - with id_arquivo                                          // name inferred
    CComparecimentoMesario(CEleitorIdentidade identidade, EPeriodoPresente periodo, bool pertenceSecao,
                           EstadoReconhecimentoBiometrico estado, CDedo::TipoDedo dedo,
                           const api::CDateTime& dataHora, std::uint32_t idArquivo);
    // wasm func 2731 - id_arquivo NULL                                           // name inferred
    CComparecimentoMesario(CEleitorIdentidade identidade, EPeriodoPresente periodo, bool pertenceSecao,
                           EstadoReconhecimentoBiometrico estado, CDedo::TipoDedo dedo,
                           const api::CDateTime& dataHora);

    const CEleitorIdentidade& GetIdentidade() const { return m_identidade; }
    EPeriodoPresente GetPeriodo() const { return m_periodo; }
    bool PertenceSecao() const { return m_pertenceSecao; }
    EstadoReconhecimentoBiometrico GetEstadoBiometria() const { return m_estado; }
    CDedo::TipoDedo GetDedo() const { return m_dedo; }
    const api::CDateTime& GetDataHora() const { return m_dataHora; }
    const std::optional<std::uint32_t>& GetIdArquivo() const { return m_idArquivo; }

private:                                                   // 52 bytes
    CEleitorIdentidade             m_identidade;           // +0  (titulo +0, tipo +12)
    EPeriodoPresente               m_periodo;              // +16
    bool                           m_pertenceSecao;        // +20
    EstadoReconhecimentoBiometrico m_estado;               // +24
    CDedo::TipoDedo                m_dedo;                 // +28 (0..10)
    api::CDateTime                 m_dataHora;             // +32 (12 bytes)
    std::optional<std::uint32_t>   m_idArquivo;            // +44, engaged flag +48
};

// Primary key: (título, período) - UNIQUE (numero_titulo, periodo_presente) in the DDL.
class CComparecimentoMesarioPK
{
public:
    // wasm func 2738 (also used by vota::CRegistraDigitalOperador, func 3614)   // name inferred
    CComparecimentoMesarioPK(const CEleitorIdentidade& identidade, EPeriodoPresente periodo)
        : m_identidade(identidade), m_periodo(periodo) {}

    const std::string& GetTitulo() const { return m_identidade.GetIdentidade(); }
    EPeriodoPresente GetPeriodo() const { return m_periodo; }
    auto operator<=>(const CComparecimentoMesarioPK&) const = default;

private:                                                   // 20 bytes
    CEleitorIdentidade m_identidade;                       // +0
    EPeriodoPresente   m_periodo;                          // +16
};

// wasm funcs 2730 / 2731 (bodies: func 2738 for the identity/period part, then the fields)
inline CComparecimentoMesario::CComparecimentoMesario(CEleitorIdentidade identidade, EPeriodoPresente periodo,
        bool pertenceSecao, EstadoReconhecimentoBiometrico estado, CDedo::TipoDedo dedo,
        const api::CDateTime& dataHora, std::uint32_t idArquivo)
    : m_identidade(std::move(identidade)), m_periodo(periodo), m_pertenceSecao(pertenceSecao),
      m_estado(estado), m_dedo(dedo), m_dataHora(dataHora), m_idArquivo(idArquivo) {}

inline CComparecimentoMesario::CComparecimentoMesario(CEleitorIdentidade identidade, EPeriodoPresente periodo,
        bool pertenceSecao, EstadoReconhecimentoBiometrico estado, CDedo::TipoDedo dedo,
        const api::CDateTime& dataHora)
    : m_identidade(std::move(identidade)), m_periodo(periodo), m_pertenceSecao(pertenceSecao),
      m_estado(estado), m_dedo(dedo), m_dataHora(dataHora), m_idArquivo(std::nullopt) {}

} // namespace comum::md
