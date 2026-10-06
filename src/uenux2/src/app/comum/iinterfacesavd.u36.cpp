// uenux2/src/app/comum/iinterfacesavd.cpp  --  FRAGMENT written by unit u36 (file owned by u23).
// Reconstructed from vota_web_wasm.wasm.  The file is inferred from the relatives: ESavdArquivoUE is
// declared in iinterfacesavd.h and both callers print the name next to a SAVD signing error.   (path inferred)
#include "comum/iinterfacesavd.h"

#include <array>
#include <string>

namespace comum {

namespace {
// @1551420: 96 const char* (ids 25..120), MI = memória interna (internal flash), ME/MV = memória externa
// / de votação (the removable card), MR = memória de resultado, SA = sistema de apuração, RED = sistema
// recuperador de dados, (res) = copy in the "resultado" directory. Ids 48..53 and 54..59 repeat the same
// six names (two families of the same files, e.g. the WSQ packages and their signatures ?).
constexpr std::array<const char*, 96> NOMES_ARQUIVOS_SAVD = {
    "EG Geral MI",                                  //  25  (@328306)
    "EG Geral ME",                                  //  26  (@330959)
    "EG GAP 1 MI",                                  //  27  (@328862)
    "EG GAP 2 MI",                                  //  28  (@328850)
    "EG GAP 1 ME",                                  //  29  (@331105)
    "EG GAP 2 ME",                                  //  30  (@331093)
    "EG VOTA MI",                                   //  31  (@328830)
    "EG VOTA ME",                                   //  32  (@331054)
    "EG SA MI",                                     //  33  (@328841)
    "Tabela de correspondência",                    //  34  (@229784)
    "BU do VOTA MI (res)",                          //  35  (@383639)
    "BU do SA MI (res)",                            //  36  (@383750)
    "RDV MI (res)",                                 //  37  (@383602)
    "Res. RDV SA",                                  //  38  (@334853)
    "Justif. de Seção MI (res)",                    //  39  (@383546)
    "Justif. SA (res)",                             //  40  (@384136)
    "Justif. de MRJ MI (res)",                      //  41  (@383615)
    "Justif. de MRJ ME (res)",                      //  42  (@384025)
    "BU imp. do VOTA MI (res)",                     //  43  (@383659)
    "ZE imp. do VOTA MI (res)",                     //  44  (@383684)
    "BU imp. do SA MI (res)",                       //  45  (@383892)
    "Hashes MI (res)",                              //  46  (@383530)
    "Hashes do SA MI (res)",                        //  47  (@383709)
    "Habilitados MI (res)",                         //  48  (@383509)
    "Não Habilitados MI (res)",                     //  49  (@383505)
    "Mesario MI (res)",                             //  50  (@383572)
    "Habilitados ME (res)",                         //  51  (@383919)
    "Não Habilitados ME (res)",                     //  52  (@383915)
    "Mesario ME (res)",                             //  53  (@383982)
    "Habilitados MI (res)",                         //  54  (@383509)
    "Não Habilitados MI (res)",                     //  55  (@383505)
    "Mesario MI (res)",                             //  56  (@383572)
    "Habilitados ME (res)",                         //  57  (@383919)
    "Não Habilitados ME (res)",                     //  58  (@383915)
    "Mesario ME (res)",                             //  59  (@383982)
    "Log MI (res)",                                 //  60  (@383589)
    "Log do SA MI (res)",                           //  61  (@383731)
    "Log da MR do SA MI (res)",                     //  62  (@383867)
    "BU do VOTA ME (res)",                          //  63  (@384049)
    "Justif. de Seção ME (res)",                    //  64  (@383956)
    "RDV ME (res)",                                 //  65  (@384012)
    "BU imp. do VOTA ME (res)",                     //  66  (@384069)
    "ZE imp. do VOTA ME (res)",                     //  67  (@384094)
    "Hashes ME (res)",                              //  68  (@383940)
    "Log ME (res)",                                 //  69  (@383999)
    "Arquivo de versões na MI",                     //  70  (@328446)
    "Arquivo de versões na ME",                     //  71  (@330971)
    "Arquivo de versões do SA",                     //  72  (@334828)
    "Justif. de Seção MI (rec. Para SA)",           //  73  (@400677)
    "RDV MI (rec. Para SA)",                        //  74  (@400734)
    "BU imp. MI (rec. Para SA)",                    //  75  (@400756)
    "ZE imp. MI (rec. Para SA)",                    //  76  (@400782)
    "Log MI (rec. Para SA)",                        //  77  (@400712)
    "Justif. de Seção ME (rec. Para SA)",           //  78  (@400808)
    "RDV ME (rec. Para SA)",                        //  79  (@400865)
    "BU imp. ME (rec. Para SA)",                    //  80  (@400887)
    "ZE imp. ME (rec. Para SA)",                    //  81  (@400913)
    "Log ME (rec. Para SA)",                        //  82  (@400843)
    "RDV MI",                                       //  83  (@328736)
    "BU imp. de Justif. Seção MI",                  //  84  (@328278)
    "BU imp. de Justif. Seção ME",                  //  85  (@330931)
    "BU imp. do VOTA MI",                           //  86  (@328811)
    "BU imp. do VOTA ME",                           //  87  (@331035)
    "Zerésima MI",                                  //  88  (@328650)
    "Resumo da Zerésima MI",                        //  89  (@328640)
    "BU imp. do RED. MI",                           //  90  (@328874)
    "BUJ imp. do RED MI",                           //  91  (@328762)
    "BIM imp. do RED MI",                           //  92  (@328743)
    "BEHB imp. do RED MI",                          //  93  (@328781)
    "BU imp. do RED ME",                            //  94  (@331017)
    "BUJ imp. do RED. ME",                          //  95  (@331137)
    "BIM imp. do RED. ME",                          //  96  (@331117)
    "BEHB imp. do RED. ME",                         //  97  (@331157)
    "RDV SA INT",                                   //  98  (@322643)
    "ZE SA INT",                                    //  99  (@322704)
    "BU SEÇÃO SA INT",                              // 100  (@322654)
    "SARAIZ SA INT",                                // 101  (@322629)
    "SASECAO SA INT",                               // 102  (@322689)
    "BU DIGITADO SA INT",                           // 103  (@322670)
    "REG CERTIFICAÇÕES SIECO",                      // 104  (@326965)
    "REG AUTENTICAÇÕES SIECO",                      // 105  (@326941)
    "BIM da MI",                                    // 106  (@328715)
    "BIM da ME",                                    // 107  (@330996)
    "BEHB da MI",                                   // 108  (@328725)
    "BEHB da ME",                                   // 109  (@331006)
    "UENUXDB INT",                                  // 110  (@322617)
    "UENUXDB EXT",                                  // 111  (@322457)
    "Arquivo de local",                             // 112  (@157430)
    "Dados disp. carga",                            // 113  (@230391)
    "UENUX CFG",                                    // 114  (@329485)
    "EG RED MI",                                    // 115  (@328801)
    "ZE imp. SA (res)",                             // 116  (@384119)
    "Habilitados da MR do SA MI (res)",             // 117  (@383805)
    "Não habilitados da MR do SA MI (res)",         // 118  (@383768)
    "Mesário da MR do SA MI (res)",                 // 119  (@383838)
    "",                                             // 120  (@450187: an empty literal, suffix-merged)
};
}  // namespace

// wasm func 5905 - executed (entry counter: 5 calls in votaInit)       // name inferred (u07/u23 use it)
// Human-readable name of a SAVD file id, for the application-context text "Ocorreu um erro durante a
// assinatura do arquivo: <nome>". Callers: CAssinador::Assina (1277) and vota::CGravaResultado::StartState
// (12098). CAssinador::Assina computes it BEFORE calling SAVD, to push a CApplicationContextGuard ("Erro na
// assinatura") that is displayed only if the signature fails, so this runs on every signature.
// Ids outside 25..120 -> "Arquivo não identificado".
// Ids used by VOTA in this unit's code: 83 "RDV MI" (rdv.dat) and 110 "UENUXDB INT" (uenux.db).
std::string NomeArquivoSavd(const ESavdArquivoUE arquivo)
{
    const auto indice = static_cast<ueint32>(arquivo) - 25u;          // unsigned: ids < 25 wrap around
    if (indice > 95)
        return "Arquivo não identificado";
    return NOMES_ARQUIVOS_SAVD[indice];
}

}  // namespace comum
