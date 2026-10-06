// uenux2/src/api/hwil/iurna.h   (path inferred: the other HWIL interfaces, ipower.h / iinput.h / iimpressora.h,
//                                 live in uenux2/src/api/hwil/)
//
// Reconstructed from vota_web_wasm.wasm (unit u33).
//
// api::IUrna: identification and "hardware secrets" of the voting machine (urna eletrônica, UE).
// Obtained with CPolySingletonList::instance<api::IUrna>() (wasm func 923, srclocs cpolysingleton.h:78 and
// cpolysingletonlist.h:99/105). RTTI: class without base, typeinfo @1530936. The binary has no vtable of its
// own (every slot is pure); the only implementation linked in the web build is api::teste::CUrnaMock
// (uenux2/mock/api/hwil/curnamock.h), registered by the simulator bootstrap (wasm func 8302).
//
// Slot order comes from CUrnaMock's vtable (@1530888). Only slots 0, 4 and 6 are called in this binary
// (14 call sites in 13 functions, all through func 923; CEscolheOpcao::StartState has two):
//   slot 0  CPedeIdentidade, CEscolheOpcao, CPedeDigital, CPedeDigitalMesario, CRegistraDigitalOperador,
//           CTesteTeclado, CThreadMonitor, CGravaResultado, IInterfaceInit::MontarMRSemHabilitar,
//           CInstrucaoVotacaoAcessibilidade::GetKeyboardPosition  (tests "<= 2019", ">= 2020", == 2009/2010/2020/2022)
//   slot 4  CGravadorBU::GravaResultado (encrypted BU) and CControlaArmazenamentoDeImagens (encrypted WSQ):
//           the 1024-byte table given to ecourna::api::cepesc::CPlainText
//   slot 6  comum::CRdv (RDV key derivation, inlined in wasm func 7787): the 128-byte table from which
//           32 key bytes are picked (see src/uenux2/src/app/comum/dados/crdv.cpp, unit u02)
// Slots 1, 2, 3 and 5 have no caller in the binary. All method names are inferred.        // name inferred
#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace api {

using uebyte = std::uint8_t;

class IUrna {
public:
    // slot 0 - model year of the urna: 2009, 2010, 2011, 2013, 2015, 2020, 2022 ...
    virtual int GetModelo() const = 0;

    // slot 1 - the model as two digits ("20" for UE2020)                                   ?
    virtual std::string GetModeloAbreviado() const = 0;

    // slot 2 - an identification number (the mock returns 87654321, the same value as
    //          numeroInternoUrna in the simulator's eg.bin fixture)                        ?
    virtual std::uint32_t GetNumeroInterno() const = 0;

    // slot 3 - a small integer (mock: 255); meaning unknown                               ?
    virtual int GetRevisaoHardware() const = 0;

    // slot 4 - 1024-byte table used by the CEPESC encryption (BU and fingerprint images).
    //          Other units call it GetTabelaCripto (u23, u17).
    virtual void GetTabelaCepesc(std::array<uebyte, 1024>& tabela) const = 0;

    // slot 5 - 32 bytes (mock: all 0x02); not used                                        ?
    virtual void GetDados32(std::array<uebyte, 32>& dados) const = 0;

    // slot 6 - 128-byte table from which the RDV key is derived. Unit u02 calls it GetCryptoTable.
    virtual void GetTabelaRdv(std::array<uebyte, 128>& tabela) const = 0;

    // slots 7 / 8 (declared after the seven methods, as the vtable shows)
    virtual ~IUrna() = default;
};

} // namespace api
