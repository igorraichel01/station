/*
 * i18n.c
 *
 *  Created on: Mar 3, 2026
 *      Author: igor
 */

#include "i18n.h"

const char* I18N(TextId_t id, uint8_t lang) {
    if (lang >= LANG_TOTAL) lang = LANG_PT;

    switch (id) {
        case TXT_USB_DETECTED:
            return (lang == LANG_ES) ? "  USB DETECTADO!     " :
                   (lang == LANG_EN) ? "   USB DETECTED!     " :
                                       "  USB DETECTADO!     ";

        case TXT_EXPORTING:
            return (lang == LANG_ES) ? "  EXPORTANDO...      " :
                   (lang == LANG_EN) ? "  EXPORTING...       " :
                                       "  EXPORTANDO...      ";

        case TXT_PLEASE_WAIT:
            return (lang == LANG_ES) ? "Espere...            " :
                   (lang == LANG_EN) ? "Please wait...       " :
                                       "Aguarde...           ";

        case TXT_SUCCESS:
            return (lang == LANG_ES) ? "   EXITO!            " :
                   (lang == LANG_EN) ? "   SUCCESS!          " :
                                       "   SUCESSO!          ";

        case TXT_USB_ERROR:
            return (lang == LANG_ES) ? "   ERROR USB!        " :
                   (lang == LANG_EN) ? "   USB ERROR!        " :
                                       "   ERRO USB!         ";

        case TXT_NO_DATA:
            return (lang == LANG_ES) ? "   SIN DATOS!        " :
                   (lang == LANG_EN) ? "   NO DATA!          " :
                                       "   SEM DADOS!        ";

        case TXT_SYSTEM_READY:
            return (lang == LANG_ES) ? "  Sistema Listo!     " :
                   (lang == LANG_EN) ? "  System Ready!      " :
                                       "  Sistema Pronto!    ";

        case TXT_FIRST_ACQ:
            return (lang == LANG_ES) ? " Primera adquisicion " :
                   (lang == LANG_EN) ? " First acquisition   " :
                                       " Primeira aquisicao  ";

        case TXT_WAIT:
            return (lang == LANG_ES) ? "    Espere...        " :
                   (lang == LANG_EN) ? "    Please wait...   " :
                                       "    Aguarde...       ";

        case TXT_TEST_MODE_TITLE:
            return (lang == LANG_ES) ? "=== MODO PRUEBA ===  " :
                   (lang == LANG_EN) ? "=== TEST MODE ===    " :
                                       "=== MODO TESTE ===   ";

        case TXT_TEST_MODE_SUBTITLE:
            return (lang == LANG_ES) ? "10 muestras x 5s     " :
                   (lang == LANG_EN) ? "10 samples x 5s      " :
                                       "10 amostras x 5s     ";

        case TXT_STARTING:
            return (lang == LANG_ES) ? "Iniciando...         " :
                   (lang == LANG_EN) ? "Starting...          " :
                                       "Iniciando...         ";

        case TXT_TEST_DONE:
            return (lang == LANG_ES) ? "  Prueba Finalizada! " :
                   (lang == LANG_EN) ? "   Test Finished!    " :
                                       "  Teste Concluido!   ";

        case TXT_TEST_OK_10:
            return (lang == LANG_ES) ? "  10 muestras OK     " :
                   (lang == LANG_EN) ? "  10 samples OK      " :
                                       "  10 amostras OK     ";

        // TXT_SAMPLE_FMT é formato (usado com sprintf), então não retorna string “fixa” 20 chars
        case TXT_SAMPLE_FMT:
            return (lang == LANG_ES) ? "Muestra:  %d/10" :
                   (lang == LANG_EN) ? "Sample:   %d/10" :
                                       "Amostra:  %d/10";
            // ===== MENU PRINCIPAL =====
        case TXT_MENU_ITEM_DATETIME:
                return (lang == LANG_ES) ? "> Config Fecha/Hora  " :
                       (lang == LANG_EN) ? "> Set Date/Time      " :
                                           "> Config Data/Hora   ";

        case TXT_MENU_ITEM_ACQ:
                return (lang == LANG_ES) ? "> Config Adquisicion " :
                       (lang == LANG_EN) ? "> Set Acquisition    " :
                                           "> Config Aquisicao   ";

        case TXT_MENU_ITEM_LANGUAGE:
                return (lang == LANG_ES) ? "> Config Idioma      " :
                       (lang == LANG_EN) ? "> Set Language       " :
                                           "> Config Idioma      ";

         case TXT_MENU_ITEM_TEST:
                return (lang == LANG_ES) ? "> Ejecutar Prueba    " :
                       (lang == LANG_EN) ? "> Run Test           " :
                                           "> Executar Teste     ";

        case TXT_MENU_FOOTER_MAIN:
                return (lang == LANG_ES) ? "SET=OK  UP/DOWN=Mov  " :
                       (lang == LANG_EN) ? "SET=OK  UP/DOWN=Move " :
                                           "SET=OK  UP/DOWN=Mov  ";

            // ===== MENU DATA/HORA =====
        case TXT_MENU_DATETIME_TITLE:
                return (lang == LANG_ES) ? "Config Fecha/Hora:   " :
                       (lang == LANG_EN) ? "Set Date/Time:       " :
                                           "Config Data/Hora:    ";

        case TXT_MENU_DATETIME_CANCEL:
                return (lang == LANG_ES) ? " SALIR" :
                       (lang == LANG_EN) ? " EXIT" :
                                           " SAIR";

        case TXT_MENU_DATETIME_SAVE:
                return (lang == LANG_ES) ? " GUARDAR" :
                       (lang == LANG_EN) ? " SAVE" :
                                           " SALVAR";

            // ===== MENU AQUISICAO =====
        case TXT_MENU_ACQ_TITLE:
                return (lang == LANG_ES) ? "Tiempo Adquisicion:  " :
                       (lang == LANG_EN) ? "Acquisition Time:    " :
                                           "Tempo de Aquisicao:  ";


       case TXT_MENU_ACQ_SAVED:
                return (lang == LANG_ES) ? "  Tiempo Guardado!   " :
                       (lang == LANG_EN) ? "  Time Saved!        " :
                                           "  Tempo Salvo!       ";

            // ===== MENU IDIOMA =====
       case TXT_MENU_LANG_TITLE:
                return (lang == LANG_ES) ? "Config Idioma:       " :
                       (lang == LANG_EN) ? "Set Language:        " :
                                           "Config Idioma:       ";

       case TXT_MENU_LANG_FOOTER:
                return (lang == LANG_ES) ? "SET=GUARDAR <- SALIR " :
                       (lang == LANG_EN) ? "SET=SAVE   <- EXIT   " :
                                           "SET=SALVAR  <- SAIR  ";

       case TXT_MENU_LANG_SAVED:
                return (lang == LANG_ES) ? "  Idioma Guardado!   " :
                       (lang == LANG_EN) ? "  Language Saved!    " :
                                           "  Idioma Salvo!      ";
       case TXT_USB_MENU_TRANSFER:
                return (lang == LANG_ES) ? "  [TRANSFERIR]      " :
                       (lang == LANG_EN) ? "  [TRANSFER]        " :
                                           "  [TRANSFERIR]      ";

       case TXT_USB_MENU_CANCEL:
                return (lang == LANG_ES) ? "  [CANCELAR]        " :
                       (lang == LANG_EN) ? "  [CANCEL]          " :
                                           "  [CANCELAR]        ";

       case TXT_USB_MENU_TRANSFER_SEL:
                return (lang == LANG_ES) ? " >[TRANSFERIR]      " :
                       (lang == LANG_EN) ? " >[TRANSFER]        " :
                                           " >[TRANSFERIR]      ";

       case TXT_USB_MENU_CANCEL_SEL:
                return (lang == LANG_ES) ? " >[CANCELAR]        " :
                       (lang == LANG_EN) ? " >[CANCEL]          " :
                                           " >[CANCELAR]        ";

            // Mensagens do editor de Data/Hora
       case TXT_DATETIME_SAVED:
                return (lang == LANG_ES) ? "  Fecha/Hora OK!    " :
                       (lang == LANG_EN) ? " Date/Time Saved!   " :
                                           "  Data/Hora Salva!  ";

       case TXT_DATETIME_CANCELED:
                return (lang == LANG_ES) ? "   Cancelado!       " :
                       (lang == LANG_EN) ? "   Canceled!        " :
                                           "   Cancelado!       ";


       case TXT_MENU_ACQ_FOOTER:
                return (lang == LANG_ES) ? "UP/DOWN SET=GUARDAR  " :
                       (lang == LANG_EN) ? "UP/DOWN SET=SAVE     " :
                                           "UP/DOWN  SET=SALVAR  ";



       case TXT_MENU_LANG_LINE1:
                return (lang == LANG_ES) ? "UP/DOWN p/ elegir    " :
                       (lang == LANG_EN) ? "UP/DOWN to choose    " :
                                           "UP/DOWN p/ escolher  ";
       case TXT_MAIN_LINE2_FMT:
                // T, H, B (temperatura, umidade, bateria)
                // args: tempStr, humidity(int), battery(int)
                return (lang == LANG_ES) ? "T:%sC H:%d%% B:%d%%" :
                       (lang == LANG_EN) ? "T:%sC H:%d%% B:%d%%" :
                                           "T:%sC H:%d%% B:%d%%";

       case TXT_MAIN_LINE3_FMT:
                // P (pressao ao nivel do mar) e A (absoluta)
                // args: seaStr, absStr
                return (lang == LANG_ES) ? "P:%s A:%s" :
                       (lang == LANG_EN) ? "P:%s A:%s" :
                                           "P:%s A:%s";

       case TXT_MAIN_LINE4_FMT:
                // “F” hoje é seu contador total_samples_saved + modo (30m/1h)
                // args: total(unsigned long), modeStr
                // Sugestão: em ES/EN usar "N:" de "Num" ou "S:" de "Samples".
                return (lang == LANG_ES) ? "MUESTRAS:%lu %s" :
                       (lang == LANG_EN) ? "SAMPLES:%lu %s" :
                                           "AMOSTRAS:%lu %s";
       case TXT_BOOT_LOADING_CONFIG:
                return (lang == LANG_ES) ? "Cargando config...   " :
                       (lang == LANG_EN) ? "Loading config...    " :
                                           "Carregando config... ";

       case TXT_BOOT_MODE_FMT:
                // arg: display_text ("30m"/"1h")
                return (lang == LANG_ES) ? " Modo: %s            " :
                       (lang == LANG_EN) ? " Mode: %s            " :
                                           " Modo: %s            ";

       case TXT_BOOT_INIT_RTC:
                return (lang == LANG_ES) ? "  Init DS3231...     " :
                       (lang == LANG_EN) ? "  Init DS3231...     " :
                                           "  Init DS3231...     ";

       case TXT_BOOT_RTC_OK:
                return (lang == LANG_ES) ? "  DS3231 OK!         " :
                       (lang == LANG_EN) ? "  DS3231 OK!         " :
                                           "  DS3231 OK!         ";

       case TXT_BOOT_RTC_ERR:
                return (lang == LANG_ES) ? "Error DS3231!        " :
                       (lang == LANG_EN) ? "DS3231 Error!        " :
                                           "Erro DS3231!         ";

       case TXT_BOOT_INIT_BMP:
                return (lang == LANG_ES) ? "  Init BMP180...     " :
                       (lang == LANG_EN) ? "  Init BMP180...     " :
                                           "  Init BMP180...     ";

       case TXT_BOOT_BMP_OK:
                return (lang == LANG_ES) ? "  BMP180 OK!        " :
                       (lang == LANG_EN) ? "  BMP180 OK!        " :
                                           "  BMP180 OK!        ";

       case TXT_BOOT_BMP_ERR:
                return (lang == LANG_ES) ? " Error BMP180!       " :
                       (lang == LANG_EN) ? " BMP180 Error!       " :
                                           "  Erro BMP180!       ";

      case TXT_BOOT_INIT_DHT:
                return (lang == LANG_ES) ? " Init DHT11...       " :
                       (lang == LANG_EN) ? " Init DHT11...       " :
                                           " Init DHT11...       ";

      case TXT_BOOT_DHT_OK:
                return (lang == LANG_ES) ? "  DHT11 OK!          " :
                       (lang == LANG_EN) ? "  DHT11 OK!          " :
                                           "  DHT11 OK!          ";

      case TXT_BOOT_FIRST_ACQ_OK:
                return (lang == LANG_ES) ? "      OK!            " :
                       (lang == LANG_EN) ? "      OK!            " :
                                           "      OK!            ";
      case TXT_MENU_ITEM_CLEAR_DATA:
                return (lang == LANG_ES) ? "> Borrar datos       " :
                       (lang == LANG_EN) ? "> Clear data         " :
                                           "> Apagar dados       ";

      case TXT_CLEAR_TITLE:
                return (lang == LANG_ES) ? "   Borrar datos       " :
                       (lang == LANG_EN) ? "   Clear data         " :
                                           "   Apagar dados       ";

      case TXT_CLEAR_QUESTION:
                return (lang == LANG_ES) ? "Borrar NORMAL+TEST?  " :
                       (lang == LANG_EN) ? "Clear NORMAL+TEST?   " :
                                           "Apagar NORMAL+TEST?  ";

      case TXT_CLEAR_NO:
                return (lang == LANG_ES) ? "NAO" :
                       (lang == LANG_EN) ? "NO"  :
                                           "NAO";

      case TXT_CLEAR_YES:
                return (lang == LANG_ES) ? "SIM" :
                       (lang == LANG_EN) ? "YES" :
                                           "SIM";

      case TXT_CLEAR_DONE:
                return (lang == LANG_ES) ? "   Listo!            " :
                       (lang == LANG_EN) ? "   Done!             " :
                                           "   Pronto!           ";
      case TXT_CLEAR_ERASING:
                return (lang == LANG_ES) ? "Borrando...          " :
                       (lang == LANG_EN) ? "Erasing...           " :
                                           "Apagando...          ";

     case TXT_CLEAR_FOOTER:
                return (lang == LANG_ES) ? "SET=OK  <- Cancelar  " :
                       (lang == LANG_EN) ? "SET=OK  <- Cancel    " :
                                           "SET=OK  <- Cancelar  ";
     case TXT_MENU_ITEM_ALTITUDE:
         return (lang == LANG_ES) ? "> Config Altitud    " :
                (lang == LANG_EN) ? "> Set Altitude      " :
                                    "> Config Altitude   ";

     case TXT_MENU_ALT_TITLE:
         return (lang == LANG_ES) ? "Configurar Altitud   " :
                (lang == LANG_EN) ? "Set Altitude         " :
                                    "Configurar Altitude  ";

     case TXT_MENU_ALT_LINE1:
         return (lang == LANG_ES) ? "Use UP/DOWN ajustar  " :
                (lang == LANG_EN) ? "Use UP/DOWN to set   " :
                                    "Use UP/DOWN ajustar  ";

     case TXT_MENU_ALT_FOOTER:
         return (lang == LANG_ES) ? "SET=Guardar  LR=Salir" :
                (lang == LANG_EN) ? "SET=Save   LR=Exit   " :
                                    "SET=Salvar  LR=Sair  ";

     case TXT_MENU_ALT_SAVED:
         return (lang == LANG_ES) ? " Altitud guardada!   " :
                (lang == LANG_EN) ? " Altitude saved!     " :
                                    " Altitude salva!     ";

        default:
            return "";
    }
}
