// Reconstructed from vota_web_wasm.wasm (unit u05). Original: uenux2/src/app/comum/dados/crespostas.h
#pragma once

#include <string>

#include "api/io/cdatamap.h"
#include "md/processoeleitoral/crespostaconsulta.h"   // ? comum::md::CRespostaConsulta

namespace comum {

// singleton storage: mutex @1838960, instance pointer @1838984
class CRespostas : public api::CDataMap<unsigned, md::CRespostaConsulta> {
public:
    static CRespostas& GetInst();   // crespostas.cpp:28
    static void CreateInst();       // crespostas.cpp:33 (inlined into 7787)
};

}  // namespace comum
