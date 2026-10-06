// Reconstructed from vota_web_wasm.wasm (unit u20).
// Original: uenux2/src/app/comum/appinfo/cappinfo.h (path inferred from cappinfo.cpp srclocs).
//
// comum::CAppInfo = in-memory cache of the urna's persistent "estado" files (BER/ASN.1, see
// src/asn1/ModuloEstadoGeral*.asn), one per application and per turno:
//     eg.bin    CEstadoGeral      (ModuloEstadoGeralUrna)  - urna/carga/section/fase/turno
//     gap.bin   CEstadoGeralGap   (ModuloEstadoGeralGap)   - launcher: history of cargas, ...
//     vota.bin  CEstadoGeralVota  (ModuloEstadoGeralVota)  - VOTA state machine, qtdBU, dates ...
//     sa.bin    CEstadoGeralSA    (ModuloEstadoGeralSA)
// Each lives in dinamico/trab<turno>/ of both flash memories (MI = internal, MV = external/voting media)
// and has a CServicoEstado* that reads/writes it (appinfo/servicos/).
//
// Singleton: std::unique_ptr<CAppInfo> @1838664 guarded by the mutex residue @1838640; getter =
// wasm func 185 (tools: comum_f185, 118 callers); constructor func 5813, destructor func 3792.
#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

#include "comum/dados/md/estadoaplicacao/cestadogeral.h"
#include "comum/dados/md/estadoaplicacao/cestadogeralgap.h"
#include "comum/dados/md/estadoaplicacao/cestadogeralvota.h"

namespace comum {

// EUrnaTurno is a char-valued enum: '0' sem turno, '1' 1º turno, '2' 2º turno, '3' "turno atual"
// (resolved through CEstadoGeral::GetTurno(), +32).                          (values from the binary)
enum class EUrnaTurno : int { SemTurno = '0', Primeiro = '1', Segundo = '2', Atual = '3' };

// ecourna::api::exception::CBaseError<comum::EUeComumAppInfoError, ...> (typeinfo @1557408,
// vtable @1558004; ctor thunk = wasm func 1710, tools: comum_f1710).
enum class EUeComumAppInfoError : int;

class CAppInfo {                                                   // 532 bytes
public:
    static CAppInfo& GetInst();                                    // func 185

    md::estadoaplicacao::CEstadoGeral& GetGeral();                 // func 291 -> 6040
    const md::estadoaplicacao::CEstadoGeral& GetGeral() const;     // func 457 -> 6040
    md::estadoaplicacao::CEstadoGeralVota& GetVota(EUrnaTurno turno = EUrnaTurno::Atual);   // func 261
    const md::estadoaplicacao::CEstadoGeralVota& GetVota() const;  // func 903
    bool TemVota(EUrnaTurno turno) const;                          // func 2841
    std::vector<std::string> GetHistoricoCargas() const;           // func 3788 (from gap.bin)

    void SalvaVotaInterno();                                       // func 3790 -> 6039
    void SalvaVotaExterno();                                       // func 3789 -> 6039

    static std::size_t IndiceTurno(const std::string& funcao, EUrnaTurno turno);   // func 1553

private:
    void SalvaVota(const std::string& funcao, int midia);          // func 6039 (merged body)

    std::optional<md::estadoaplicacao::CEstadoGeral> m_geral;                          // +0   (flag +180)
    std::array<std::optional<md::estadoaplicacao::CEstadoGeralGap>, 2> m_gap;          // +184 (48 B each, flag +44)
    std::array<std::optional<md::estadoaplicacao::CEstadoGeralVota>, 2> m_vota;        // +280 (104 B each, flag +100)
    // +488 .. +532: not touched by this unit (CEstadoGeralSA cache?)                                 // ?
};

// Free helpers over CAppInfo (out-of-line functions; EhTreinamentoEleitor is also inlined in some callers,
// e.g. funcs 10210 and 12062). Names follow the other units.
bool EhTreinamentoEleitor();                        // func 697  (other unit)
bool EhTreinamentoSemTreinamentoEleitor();          // func 1823
bool EhModoDemonstracaoSemTreinamentoEleitor();     // func 2520 (unit u09 calls it DeveRegistrarMesarios)
void SalvaEstado();                                 // func 491
void CopiaAssinaturaEstadoVotaParaMV();             // func 4687

} // namespace comum
