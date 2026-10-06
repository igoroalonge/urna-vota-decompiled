// ecourna-lib/ecourna/app/dados/tiposbasicos.h  (path inferred from tiposbasicos.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14 (only the parts this unit uses).
//
// "Tipos básicos" of the ecourna data model. Most of them are CBaseType<TYPE, MIN, MAX, ID>
// instances (ecourna/api/pattern/cbasetype.hpp, unit u12): a value wrapper whose constructor
// throws CBaseError<EPatternError>(1300, "O tipo '{}' deve ter valores no intervalo [{},{}].")
// (cbasetype.hpp:39) when the value is outside [MIN, MAX]. The '{}' is a per-type static
// std::string holding the type name. The names are built by __wasm_call_ctors (func 14478)
// into globals 1911992..1912208, in this order (the 4th template argument is the index):
//   ProcessoEleitoralID, MunicipioID, OrdemSuplencia, QtdCandidato, OrigemConfiguracao,
//   AmbienteExecucao, TipoAbrangencia, TipoEleicao, SituacaoPleito, TipoImpedimento,
//   TipoLocalVotacao, TipoTransferenciaTemporaria, ErroLeituraBiometria, OrdemEleicao,
//   NumeroVotavel, RepositorioVotosID, ScoreHabilitacao, FederacaoID, TipoIdentificadorEleitor
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "ecourna/api/pattern/cbasetype.hpp"

using uebyte = std::uint8_t;

namespace ecourna::app::dados {

// ---- identifiers --------------------------------------------------------------------------
enum ETipoIdentificadorEleitor : int {            // = ModuloTiposEleitorais::TipoIdentificadorEleitor
    TipoIDNumeroInscricao = 1,                     // título de eleitor (número de inscrição)   (name inferred)
    TipoIDCPF             = 2,                     // CPF                                        (name inferred)
    TipoIDNumeroLivre     = 3,                     // free identification number
    TipoIDPrimeiro        = TipoIDNumeroInscricao, // MIN_VAL in the CBaseType signature
};
// CBaseType<ETipoIdentificadorEleitor, TipoIDPrimeiro, TipoIDNumeroLivre, 40>  (func 2666, unit u12)
using TTipoIdentificadorEleitor = CBaseType<ETipoIdentificadorEleitor, TipoIDPrimeiro, TipoIDNumeroLivre, 40>;

using TProcessoEleitoralID = CBaseType<unsigned int, 0, 99999, 0>;     // "ProcessoEleitoralID"
using TCargoID             = CBaseType<unsigned short, 1, 99, 3>;      // cargo code 1..13 or 25..99
using TTurno               = CBaseType<unsigned short, 1, 2, 11>;      // 1st / 2nd round
using TScoreHabilitacao    = CBaseType<unsigned short, 0, 999, 38>;    // "ScoreHabilitacao" (fingerprint score)
// ? "FederacaoID" is one of the named CBaseTypes (static name @1912196); its bounds are not visible
//   in this unit, CFederacao stores it as a plain 16-bit value.
using TFederacaoID         = std::uint16_t;
using TNumeroPartido       = std::uint16_t;                             // ? partido number 0..99
using TVectorNumeroPartido = std::vector<TNumeroPartido>;

// ---- phase ------------------------------------------------------------------------------
// CFaseID is a distinct 4-byte class in the RTTI (IConversorASN<ModuloTiposEleitorais::Fase, CFaseID>);
// its value uses the ASN numbers 1 simulado, 2 oficial, 3 treinamento. ? exact declaration unknown.
class CFaseID {
public:
    enum EFase : int { Simulado = 1, Oficial = 2, Treinamento = 3 };   // names inferred
    CFaseID(int fase) : m_fase(fase) {}
    operator int() const { return m_fase; }
private:
    int m_fase;   // +0
};
using TFaseID = CFaseID;

// ---- biometrics ---------------------------------------------------------------------------
enum EErroLeituraBiometria : int {               // same numbers as ModuloResultadoUrnaCadastro::ErroLeituraBiometria
    ErroBioPrimeiro = 0,                          // semErro
    // 1 tipoArquivoInvalido ... 12 erroCodificacaoMinuncias
    ErroBioUltimo   = 13,                         // sentinel: accepted by CBaseType, rejected by CErroLeituraBiometria
};
using TErroLeituraBiometria = CBaseType<EErroLeituraBiometria, ErroBioPrimeiro, ErroBioUltimo, 34>;

// wasm func 2668. 4 bytes: the CBaseType value.
class CErroLeituraBiometria : public TErroLeituraBiometria {
public:
    CErroLeituraBiometria(EErroLeituraBiometria erro);   // srcloc tiposbasicos.cpp:1149
};

} // namespace ecourna::app::dados
