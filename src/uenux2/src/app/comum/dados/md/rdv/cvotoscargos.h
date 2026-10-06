// uenux2/src/app/comum/dados/md/rdv/cvotoscargos.h  (path inferred from cvotoscargos.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u22).
//
// The in-memory RDV ("registro digital do voto") is a tree of plain classes (no RTTI):
//   comum::CRdvVota (singleton func 555)                         +4 CRdvPosicionador* (where to insert)
//     +20 md::CVotosEleicoesVota                                  per eleição
//           +0  TMapa = std::map<TEleicaoID, CVotosCargos>
//           +12 TMapaCargoEleicao = std::map<TCargoID, TEleicaoID>  (built by MontaMapa)
//         md::CVotosCargos                                        per cargo of one eleição
//           +0  TMapa = std::map<SCargoInfo, CVotos>   (node: key +16, value +28)
//         md::CVotos                                              one cargo's votes (ordered)
//           +0  std::vector<CVoto> (16-byte elements; reserve(1000) in the constructor, func 2794)
//   SCargoInfo (12 bytes): +0 TCargoID codigo  ...  +8 uebyte numeroDigitos  +9 TQtdEscolha qtdEscolhas
//
// A "cédula" (ballot) is what one voter confirmed for one eleição: a vector of (cargo, CVoto).
// Every cargo holds exactly qtdEscolhas votes per ballot, so after N voters each CVotos holds
// N * qtdEscolhas votes, kept SORTED by (tipo, digitado) by the positioner (func 11496): the RDV does
// not preserve the voting order.
#pragma once

#include <cstdint>
#include <map>
#include <utility>
#include <vector>

#include "comum/dados/md/rdv/cvoto.h"

namespace comum::md {

using TCargoID = std::uint8_t;
using TQtdEscolha = std::uint8_t;
using TQtdVoto = std::uint16_t;
using TEleicaoID = std::uint32_t;

class CVotos {
public:
    explicit CVotos(std::vector<CVoto> votos);                      // func 2794 (reserve(1000))
    std::size_t Tamanho() const { return m_votos.size(); }
    void Insere(const CVoto& voto, std::size_t posicao);            // srcloc cvotos.cpp:40 (inlined, see cvotos.u22.cpp)
    const std::vector<CVoto>& GetVotos() const { return m_votos; }
private:
    std::vector<CVoto> m_votos;
};

class CRdvPosicionador {                                            // polymorphic; VOTA: CRdvPosicionadorVota
public:
    virtual ~CRdvPosicionador() = default;
    virtual std::size_t Posiciona(const CVotos& votos, const CVoto& voto) const = 0;   // slot 2 (func 11496)
};

using CCedula = std::vector<std::pair<TCargoID, CVoto>>;          // 20-byte elements  (name attested by srclocs)

class CVotosCargos
{
public:
    struct SCargoInfo {
        TCargoID codigo;                 // +0
        std::uint8_t numeroDigitos;      // +8
        TQtdEscolha qtdEscolhas;         // +9
        auto operator<=>(const SCargoInfo& o) const { return codigo <=> o.codigo; }
    };
    using TMapa = std::map<SCargoInfo, CVotos>;

    explicit CVotosCargos(TMapa mapa);                                   // func 5643 (srcloc :50, :58, :66)

    const CVotos& GetVotos(TCargoID cargo) const;                        // srcloc :91 (in func 11350)
    TQtdVoto Comparecimento() const;                                     // inlined into func 5639
    void ConfereCedula(const CCedula& cedula) const;                     // srcloc :248..:295 (inlined into 4454)
    void InsereCedula(const CRdvPosicionador& posicionador,
                      const CCedula& cedula);                            // srcloc :199 (inlined into 4454)

private:
    TQtdEscolha QtdVotosPorCedula() const;                               // func 5642   name inferred
    TMapa m_mapa;                                                        // +0
};

} // namespace comum::md
