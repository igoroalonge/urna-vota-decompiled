// FRAGMENT reconstructed by unit u29 from vota_web_wasm.wasm.
// Original file: UNKNOWN - path inferred: uenux2/src/app/comum/appinfo/servicos/iservicoestado.h (the class
// template comum::IServicoEstado<ESTADO, CONVERSOR> named in docs/modules/u20 §2; its four subclasses live in
// cservicoestadogeral{,vota,gap,sa}.cpp of the same directory, attested by their srclocs).
//
// A "serviço de estado" reads / writes one of the urna's persistent state files on one flash memory:
//   CServicoEstadoGeral      eg.bin    <flash>/dinamico/eg.bin                 vtable @1558024 (8 bytes)
//   CServicoEstadoGeralVota  vota.bin  <flash>/dinamico/trab<turno>/vota.bin   vtable @1558064 (12 bytes)
//   CServicoEstadoGeralGap   gap.bin   <flash>/dinamico/trab<turno>/gap.bin    vtable @1558120 (12 bytes)
//   CServicoEstadoGeralSA    sa.bin    <flash>/dinamico/trab<turno>/sa.bin     vtable @1558176 (12 bytes)
// Layout: +0 vptr, +4 EFlashOrigem m_midia, +8 EUrnaTurno m_turno (per-turno services only).
// vtable: [0]/[1] destructors, [2] GetPathArquivo() (funcs 11565/11567/11568, unit u20).
// Hierarchy (RTTI, all "si"): each of the four services derives DIRECTLY from its IServicoEstado<ESTADO,
// CONVERSOR> instantiation; there is no intermediate "per-turno" base class. GetPathArquivo is not const: the
// srclocs cservicoestadogeral{vota,gap,sa}.cpp:32 record "virtual std::filesystem::path
// comum::CServicoEstadoGeralVota::GetPathArquivo()" (no const qualifier).
//
// The functions below are the ones of unit u29: constructors and Salva. wasm-opt merged the bodies that differ
// only in constants (vtable pointer, table slots) into shared functions that take those constants as extra
// parameters: 3897 (the three per-turno constructors) and 2894 (Salva).
#pragma once

#include <filesystem>
#include <string>

#include "api/io/asn/cfileasn.h"
#include "comum/appinfo/cappinfo.h"          // EUrnaTurno
#include "comum/cpath.h"                     // EFlashOrigem

namespace comum {

template <typename ESTADO, typename CONVERSOR>
class IServicoEstado
{
public:
    explicit IServicoEstado(EFlashOrigem midia) : m_midia(midia) {}
    virtual ~IServicoEstado() = default;

    // Reads and decodes the file (api::CFileASN::ReadFromFile<Entidade> + CONVERSOR::Desconverte, inlined:
    // func 3791 for eg.bin).
    ESTADO Carrega();

    // wasm func 2894 (merged body; the per-class entry points pass the WriteToFile thunk, the Converte slot and
    // the converter's vtable). Encodes the state to BER with the converter and writes the whole file.
    void Salva(const ESTADO& estado)
    {
        const std::string arquivo = GetPathArquivo().string();          // vtable slot 2
        const CONVERSOR conversor;                                      // stateless, vptr only, on the stack
        const auto entidade = conversor.Converte(estado);               // IConversorASN<...>::Converte
        api::CFileASN::WriteToFile(arquivo, entidade);                  // 10168 / 9985 / ... -> 2892 (mode "wb")
    }

protected:
    virtual std::filesystem::path GetPathArquivo() = 0;             // not const (srcloc signature)

    EFlashOrigem m_midia;                                               // +4
};

// ------------------------------------------------------------------------------------------------
// CServicoEstadoGeral (eg.bin). wasm func 1941: the constructor {vptr @1558024, midia}.
// Its Salva entry point is wasm func 3592 = 2894(this, estado, slot 429 -> 10168 WriteToFile<EstadoGeralUrna>,
// slot 428 -> 10170 Converte, vtable CConversorEstadoGeral @1568180).
// ------------------------------------------------------------------------------------------------
class CServicoEstadoGeral final
    : public IServicoEstado<md::estadoaplicacao::CEstadoGeral, asn::CConversorEstadoGeral>
{
public:
    explicit CServicoEstadoGeral(EFlashOrigem midia) : IServicoEstado(midia) {}          // wasm func 1941
protected:
    std::filesystem::path GetPathArquivo() override;                                    // <dinamico>/eg.bin (11569)
};

// ------------------------------------------------------------------------------------------------
// Per-turno services: three SEPARATE classes, each deriving directly from IServicoEstado<> (RTTI), each with its
// own constructor in its own file (cservicoestadogeral{vota,gap,sa}.cpp, whose GetPathArquivo is at line 32).
// The three constructors are identical except for the vtable pointer, so wasm-opt merged them into wasm func
// 3897, which takes the vtable as a 4th parameter:
//   func 3787  CServicoEstadoGeralVota(midia, turno)   -> 3897(this, midia, turno, vtable @1558064)
//   func 5812  CServicoEstadoGeralGap(midia, turno)    -> 3897(this, midia, turno, vtable @1558120)
//   func 11566 CServicoEstadoGeralSA(midia, turno)     -> 3897(this, midia, turno, vtable @1558176)  (unit u30)
// Body of 3897: store {vptr, midia, turno}; if turno is EUrnaTurno::Atual ('3', 51) replace it with the turno
// recorded in eg.bin OF THE SAME FLASH (CServicoEstadoGeral(midia).Carrega(), CEstadoGeral +32). Whether the
// source repeats these lines in each constructor or calls a shared inline helper cannot be told apart.
// ------------------------------------------------------------------------------------------------
class CServicoEstadoGeralVota final                                                      // vota.bin
    : public IServicoEstado<md::estadoaplicacao::CEstadoGeralVota, asn::CConversorEstadoGeralVota>
{
public:
    CServicoEstadoGeralVota(EFlashOrigem midia, EUrnaTurno turno)                       // 3787 -> 3897
        : IServicoEstado(midia), m_turno(turno)
    {
        if (m_turno == EUrnaTurno::Atual)                                                // '3' (51)
            m_turno = CServicoEstadoGeral(midia).Carrega().GetTurno();                   // 1941, 3791
    }
    // Salva entry point: wasm func 5329 = 2894(this, estado, slot 460 -> 9985 WriteToFile<EstadoGeralVota>,
    //                                             slot 459 -> 9997 Converte, vtable CConversorEstadoGeralVota
    //                                             @1568968). Used by CAppInfo::SalvaVota (6039) after every
    //                                             state change and by CAppInfoBuilder::SalvaApp.
protected:
    std::filesystem::path GetPathArquivo() override;                                    // func 11568 (u20)
private:
    EUrnaTurno m_turno;                                                                  // +8
};

class CServicoEstadoGeralGap final                                                       // gap.bin
    : public IServicoEstado<md::estadoaplicacao::CEstadoGeralGap, asn::CConversorEstadoGeralGap>
{
public:
    CServicoEstadoGeralGap(EFlashOrigem midia, EUrnaTurno turno)                        // 5812 -> 3897
        : IServicoEstado(midia), m_turno(turno)
    {
        if (m_turno == EUrnaTurno::Atual)
            m_turno = CServicoEstadoGeral(midia).Carrega().GetTurno();
    }
protected:
    std::filesystem::path GetPathArquivo() override;                                    // func 11567 (u20)
private:
    EUrnaTurno m_turno;                                                                  // +8
};

// CServicoEstadoGeralSA (sa.bin, vtable @1558176, constructor 11566 -> 3897, GetPathArquivo 11565) has the same
// shape; it belongs to unit u30.

}  // namespace comum
