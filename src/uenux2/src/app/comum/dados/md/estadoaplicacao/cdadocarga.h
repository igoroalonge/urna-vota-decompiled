// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/estadoaplicacao/cdadocarga.h
// md side of ModuloEstadoGeralUrna::DadoCarga {turno, tipoUrnaT1, tipoUrnaT2, modelo, fase}.
#pragma once

namespace comum::md::estadoaplicacao {

enum class EUrnaModelo : int { UE2013 = 2013, UE2015 = 2015, UE2020 = 2020, UE2022 = 2022 };  // ?
enum class EFase : int { OFICIAL = '1', SIMULADO = '2', TREINAMENTO = '3' };                 // ?

class CDadoCarga {
public:
    CDadoCarga(int turno, int tipoUrnaT1, int tipoUrnaT2, EUrnaModelo modelo, EFase fase);   // func 5633
    static void ValidaModelo(const EUrnaModelo modelo);   // cdadocarga.cpp:84 (inlined into 5633)
    char GetFaseChar() const;                             // func 2253 (:108)

private:
    int m_turno;           // +0
    int m_tipoUrnaT1;      // +4
    int m_tipoUrnaT2;      // +8
    EUrnaModelo m_modelo;  // +12
    EFase m_fase;          // +16
};

}  // namespace comum::md::estadoaplicacao
