// uenux2/src/app/comum/carquivossavd.h
// Reconstructed from vota_web_wasm.wasm (unit u21).
//
// SAVD = the urna's file signing / verification service ("Serviço de Assinatura e Verificação Digital"?, expansion
// inferred; see iinterfacesavd.cpp). Each signed file of the urna has an ESavdArquivoUE id, and each signature
// package (".vsu" on the urna, ".vsc" on the result medium) an ESavdPacote id. CArquivosSavd maps both ids to the
// full path of the file. The tables are filled by the singleton constructor, fully inlined into
// CArquivosSavd::GetInst (wasm func 1164, 77 KB, reconstructed by unit u22 in carquivossavd.u22.cpp with the full
// table): it combines CPath::GetPathTrab / GetPathResult / GetPathDinamico with CArquivosResultado suffixes and
// names such as "uenux.vsu", "rdv.vsu", "bu.vsu", "eg.vsu"...
// Examples of ids (from unit u07): 31 "vota.bin", 83 "rdv.dat", 110/111 "uenux.db" (MI/MV);
// pacotes 122..125 vota.vsu, 134..137 rdv.vsu, 199..202 uenux.vsu (MI/MV x turno 1/2).
#pragma once

#include <map>
#include <string>

namespace comum {

enum class ESavdPacote : int;       // signature package id (values > 100 seen)
enum class ESavdArquivoUE : int;    // signed file id
enum class ESavdAplicacao : int;    // SAVD application / key id (u22)

class CArquivosSavd
{
public:
    static const CArquivosSavd& GetInst();                          // wasm func 1164 (unit u22)   // name inferred

    std::string operator[](ESavdPacote pacote) const;               // wasm func 275 (srcloc line 49)
    std::string operator[](ESavdArquivoUE arquivo) const;           // wasm func 680 (srcloc line 73)

private:
    std::map<ESavdPacote, std::string> m_pacotes;                   // +0   (node: key +16, value +20)
    std::map<ESavdArquivoUE, std::string> m_arquivos;               // +12
    std::map<ESavdAplicacao, std::string> m_aplicacoes;             // +24 (no accessor in this build; see u22)
};

} // namespace comum
