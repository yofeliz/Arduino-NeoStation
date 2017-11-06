/*******************************************************************************
 * Librerias de funcionamiento.
 ******************************************************************************/
   // Libreria no usada, pero puesta como prevencion de errores de compilacion de RTCLib.
   #include <SPI.h>
   // Libreria usada para las comunicaciones serie.
   #include <Wire.h>
   // Libreria usada para el funcionamiento del RTC.
   #include <RTClib.h>
   // Libreria usada para el funcionamiento de los NeoPixels.
   #include <Adafruit_NeoPixel.h>

/*******************************************************************************
 * Definiciones de constantes.
 ******************************************************************************/
   // Pin de lectura de sonido para activacion de strings durante X segundos.
   #define PINSA         13
   // Pin de uso para la string de NeoPixels de segundos y minutos.
   #define PINSM          6
   // Pin de uso para la string de NeoPixels de horas.
   #define PINH           7
   // Pin de uso para el boton de seteo de las horas.
   #define PINBH         10
   // Pin de uso para el boton de seteo de los minutos.
   #define PINBM         12
   // Numero de NeoPixels en la string de segundos y minutos.
   #define NUMPIXELSSM   60
   // Numero de NeoPixels en la string de horas.
   #define NUMPIXELSH    12
   // Luminosidad
   #define lum          255
   // Tiempo de iluminacion al hacer sonido.
   #define tiempoSonido  10
   // Modo depuracion. Descomentar para activar.
   //#define DEBUG

/*******************************************************************************
 * Definición de variables globales.
 ******************************************************************************/
   // Variable del RTC (Real Time Clock).
   RTC_DS3231 RTC;
   // Variable de la string de NeoPixels de los segundos y minutos.
   // Referencia: https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library
   Adafruit_NeoPixel pixelsSM = Adafruit_NeoPixel(NUMPIXELSSM, PINSM, NEO_GRB + NEO_KHZ800);
   // Variable de la string de NeoPixels de las horas.
   // Referencia: https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library
   Adafruit_NeoPixel pixelsH = Adafruit_NeoPixel(NUMPIXELSH, PINH, NEO_GRB + NEO_KHZ800);
   // Variable de pausa de refresco de las strings de NeoPixels.
   int refresco = 1000;
   // Variable de pausa de refresco de las strings de NeoPixels en la inicizalizacion.
   int refrescoInicializacion = 10;
   // Variables temporales para bucles.
   int x, y;
   // Variable de la fecha y hora.
   DateTime fechaHora;
   // Variables de la fecha y hora actuales.
   int diaActual, mesActual, anoActual, horaActual, minutoActual, segundoActual;

/*******************************************************************************
 * Inicialización de programa.
 ******************************************************************************/
void setup () {
   #ifdef DEBUG
      // Seteo del puerto de comunicaciones serie, velocidad en baudios.
      inicializacionPuertoSerie();
   #endif
   
   // Inicializacion del puerto de comunicaciones I2C para las tiras LED.
   inicializacionI2C();
   // Inicializacion del RTC.
   inicializacionRTC();
   
   #ifdef DEBUG
      // Ajuste del RTC con la hora del sistema.
      seteoFechaHora();
   #endif
   
   // Inicializacion de alarma del RTC. (DESHABILITADA)
   //inicializacionAlarmaRTC();

   // Inicializacion de pin de sensor de sonido.
   inicializacionSensorSonido();
   
   // Inicializacion de botones de seteo.
   inicializacionBotones();
   
   // Inicializacion de las strings.
   inicializacionPixeles();
   
   // Reseteo inicial a apagado de las strings LED.
   resetPixels();
   
   // Comprobacion de todos los LEDs de las strings de horas, minutos y segundos.
   comprobacionPixels();
}//setup
   
/*******************************************************************************
 * [DEBUG] Seteo del puerto de comunicaciones serie (velocidad en baudios).
 ******************************************************************************/
void inicializacionPuertoSerie() {
   Serial.begin(9600);
}//inicializacionPuertoSerie

/*******************************************************************************
 * Inicializacion del puerto de comunicaciones I2C para las tiras LED.
 ******************************************************************************/
void inicializacionI2C() {
   Wire.begin();
}//inicializacionI2C

/*******************************************************************************
 * Inicializacion del RTC.
 ******************************************************************************/
void inicializacionRTC() {
   RTC.begin();
}//inicializacionRTC

/*******************************************************************************
 * [DEBUG] Ajuste del RTC con la hora del sistema.
 ******************************************************************************/
void seteoFechaHora() {
   // Ajuste del RTC con la hora del sistema.
   // En caso de no haber ningun ordenador conectado,
   // toma la hora de la ultima compilacion del programa.
   // Otro posible seteo: RTC.adjust(DateTime("DEC 31 2011","12:59:45"));
   RTC.adjust(DateTime(__DATE__, __TIME__));
   // En caso de no detectarse RTC conectado...
   if (!RTC.isrunning()) {
      // ... se envia un mensaje serie de desconexion.
      #ifdef DEBUG
         Serial.println("El modulo RTC no esta conectado. Revise la conexion.");
      #endif
      // ... toma la hora de la ultima compilacion del programa.
      RTC.adjust(DateTime(__DATE__, __TIME__));
   }//if
}//seteoFechaHora

/*******************************************************************************
 * Codigo para setear y comprobar la activacion de la alarma del RTC.
 ******************************************************************************/
void inicializacionAlarmaRTC() {
   // Linea para tomar la hora del RTC.
   //DateTime now = RTC.now();
   // Linea de seteo de alarma.
   RTC.setAlarm1Simple(21, 58);
   // Linea de activacion de alarma.
   RTC.turnOnAlarm(1);
   // Comprobacion de alarma activada.
   if (RTC.checkAlarmEnabled(1)) {
      #ifdef DEBUG
         Serial.println("Alarma habilitada.");
      #endif
   }//if
}//inicializacionAlarmaRTC

/*******************************************************************************
 * Inicializacion de pin de sensor de sonido.
 ******************************************************************************/
void inicializacionSensorSonido() {
   pinMode(PINSA, INPUT);
}//inicializacionSensorSonido

/*******************************************************************************
 * Inicializacion de botones de seteo.
 ******************************************************************************/
void inicializacionBotones() {
   pinMode(PINBH, INPUT);
   pinMode(PINBM, INPUT);
}//inicializacionBotones

/*******************************************************************************
 * Inicializacion de las strings.
 ******************************************************************************/
void inicializacionPixeles() {
  // Inicializacion de la string de segundos y minutos.
  pixelsSM.begin();
  // Inicializacion de la string de horas.
  pixelsH.begin();
}//inicializacionPixeles

/*******************************************************************************
 * Reseteo inicial de las strings LED.
 ******************************************************************************/
void resetPixels() {
   // Apaga todos los LEDs de la string segundos y minutos.
   for(x=0; x<NUMPIXELSSM; x++) {
      pixelsSM.setPixelColor(x, 0);
   }//for
   pixelsSM.show();
   
   // Apaga todos los LEDs de la string horas.
   for(x=0; x<NUMPIXELSH; x++) {
      pixelsH.setPixelColor(x, 0);
   }//for
   pixelsH.show();
}//resetPixels
    
/*******************************************************************************
 * Comprobacion de todos los LEDs de las strings y luego apagado.
 ******************************************************************************/
void comprobacionPixels() {
   for(x=0; x<NUMPIXELSSM; x++) {
      pixelsSM.setPixelColor(x, pixelsSM.Color(lum,0,0));
      pixelsH.setPixelColor(x/5, pixelsH.Color(lum,0,0));
      pixelsSM.show();
      pixelsH.show();
      delay(refrescoInicializacion);
   }//for
   for(x=0; x<NUMPIXELSSM; x++) {
      pixelsSM.setPixelColor(x, pixelsSM.Color(0,lum,0));
      pixelsH.setPixelColor(x/5, pixelsH.Color(0,lum,0));
      pixelsSM.show();
      pixelsH.show();
      delay(refrescoInicializacion);
   }//for
   for(x=0; x<NUMPIXELSSM; x++) {
      pixelsSM.setPixelColor(x, pixelsSM.Color(0,0,lum));
      pixelsH.setPixelColor(x/5, pixelsH.Color(0,0,lum));
      pixelsSM.show();
      pixelsH.show();
      delay(refrescoInicializacion);
   }//for
   for(x=0; x<NUMPIXELSSM; x++) {
      pixelsSM.setPixelColor(x, pixelsSM.Color(lum,lum,lum));
      pixelsH.setPixelColor(x/5, pixelsH.Color(lum,lum,lum));
      pixelsSM.show();
      pixelsH.show();
      delay(refrescoInicializacion);
   }//for
   resetPixels();
}//comprobacionPixels

/*******************************************************************************
 * Bucle de programa.
 ******************************************************************************/
void loop() {
   // Obtencion de la hora del RTC.
   //fechaHora = obtenerFechaHora();
   
   // Comprobacion de botones para seteo.
   //comprobacionBotones();
   
   // Copia de fecha y hora a variables actuales para futuro uso.
   //copiaFechaHoraActuales();
   
   // Comprobacion que la hora obtenida del RTC es correcta.
   // Si no es correcta, la hora se seteara a 00:00:30.
   //comprobacionFechaHora();
   
   //#ifdef DEBUG
      // Impresion serie de la hora y la temperatura.
      //imprimirFechaHoraTemperatura();
   //#endif
   
   // Comprobar si la alarma ha saltado. (DESHABILITADO)
   //comprobacionAlarmaRTC();

   // Mostrar hora en los LEDs.
   //mostrarHora();

   // Comprueba si se ha producido un sonido y se muestra la hora X segundos.
   comprobarSonido();
}//loop

/*******************************************************************************
 * Obtencion de la fecha y la hora del RTC.
 ******************************************************************************/
DateTime obtenerFechaHora() {
   return RTC.now();
}//obtenerFechaHora

/*******************************************************************************
 * Comprobacion de botones para seteo.
 ******************************************************************************/
void comprobacionBotones() {
   //boolean botonHorasPresionado      = false;
   //boolean botonMinutosPresionado    = false;
   int     botonHorasPresionadoInt   = digitalRead(PINBH);
   int     botonMinutosPresionadoInt = digitalRead(PINBM);
   int     horasTemp                 = fechaHora.hour();
   int     minutosTemp               = fechaHora.minute();
   String  cadenaFecha, cadenaHora;
   
   #ifdef DEBUG
      Serial.print("PINBH > ");
      Serial.println(botonHorasPresionadoInt);
      Serial.print("PINBM > ");
      Serial.println(botonMinutosPresionadoInt);
   #endif
   
   // Si se presiona el boton de las horas...
   if (botonHorasPresionadoInt == 1) {
      if (horasTemp < 23)
         horasTemp++;
      if (horasTemp == 23)
         horasTemp = 0;
   }//if
   
   // Si se presiona el boton de los minutos...
   if (botonMinutosPresionadoInt == 1) {
      if (minutosTemp < 59)
         minutosTemp++;
      if (minutosTemp == 59)
         minutosTemp = 0;
   }//if
   
   RTC.adjust(DateTime(anoActual, mesActual, diaActual, horasTemp, minutosTemp, segundoActual));
   
   #ifdef DEBUG
      Serial.println(cadenaHora);
      Serial.println("Horas Temporales   > " + String(horasTemp));
      Serial.println("Minutos Temporales > " + String(minutosTemp));
   #endif
}//comprobacionBotones

/*******************************************************************************
 * Copia de fecha y hora a variables actuales para futuro uso.
 ******************************************************************************/
void copiaFechaHoraActuales() {
   diaActual     = fechaHora.day();
   mesActual     = fechaHora.month();
   anoActual     = fechaHora.year();
   horaActual    = fechaHora.hour();
   minutoActual  = fechaHora.minute();
   segundoActual = fechaHora.second();
}//copiaFechaHoraActuales
   
/*******************************************************************************
 * Comprobacion que la hora obtenida del RTC es correcta.
 ******************************************************************************/
void comprobacionFechaHora() {
   // Si la hora no es correcta, la pone a 0. 
   if (horaActual < 0 || horaActual > 23) {
      horaActual = 0;
   }//if
   // Si los minutos no son correctas, los pone a 0. 
   if (minutoActual < 0 || minutoActual > 59) {
      minutoActual = 0;
   }//if
   // Si los segundos no son correctas, los pone a 30. 
   if (segundoActual < 0 || segundoActual > 59) {
      segundoActual = 30;
   }//if
}//comprobacionFechaHora

/*******************************************************************************
 * [DEBUG] Impresion serie de la hora y la temperatura.
 ******************************************************************************/
void imprimirFechaHoraTemperatura() {
   #ifdef DEBUG
      Serial.print(fechaHora.hour(), DEC);
      Serial.print(':');
      Serial.print(fechaHora.minute(), DEC);
      Serial.print(':');
      Serial.print(fechaHora.second(), DEC);
      Serial.print(' ');
      Serial.print(fechaHora.day(), DEC);
      Serial.print('/');
      Serial.print(fechaHora.month(), DEC);
      Serial.print('/');
      Serial.print(fechaHora.year(), DEC);
      Serial.print(' ');
      Serial.print(RTC.getTemperature());
      Serial.print('C');
      Serial.println();
   #endif
}//imprimirFechaHoraTemperatura

/*******************************************************************************
 * Comprobar si la alarma ha saltado.
 ******************************************************************************/
void comprobacionAlarmaRTC() {
   if (RTC.checkIfAlarm(1)) {
      #ifdef DEBUG
         Serial.println("Ha saltado la alarma.");
      #endif
   }//if
}//comprobacionAlarmaRTC

/*******************************************************************************
 * Comprueba si se hay sonido y se muestra la hora "tiempoSonido" segundos.
 ******************************************************************************/
void comprobarSonido() {
   #ifdef DEBUG
      Serial.println(digitalRead(PINSA));
   #endif
   
   if(digitalRead(PINSA)) {
      for(y=0; y<tiempoSonido; y++) {
         // Mostrar hora en los LEDs.
         mostrarHora();
      }//for
   }//if
   resetPixels();
}//comprobarSonido

/*******************************************************************************
 * Mostrar hora en los LEDs.
 ******************************************************************************/
void mostrarHora() {
   // Obtencion de la hora del RTC.
   fechaHora = obtenerFechaHora();
   
   // Copia de fecha y hora a variables actuales para futuro uso.
   copiaFechaHoraActuales();
   
   // Comprobacion que la hora obtenida del RTC es correcta.
   // Si no es correcta, la hora se seteara a 00:00:30.
   comprobacionFechaHora();

   // Comprobacion de botones para seteo.
   comprobacionBotones();
   
   #ifdef DEBUG
      // Impresion serie de la hora y la temperatura.
      imprimirFechaHoraTemperatura();
   #endif
   
   // Comprobar si la alarma ha saltado. (DESHABILITADO)
   //comprobacionAlarmaRTC();
   
   // Configuracion de pixeles de segundos en string de segundos y minutos (LED encendido, color).
   pixelsSM.setPixelColor(segundoActual, pixelsSM.Color(lum,lum,lum));
   // Configuracion de pixeles de minutos en string de segundos y minutos (LED encendido, color).
   pixelsSM.setPixelColor(minutoActual, pixelsSM.Color(0,0,lum));
   // Configuracion de pixeles de horas en string de horas, formato 12h (LED encendido, color).
   pixelsH.setPixelColor(formato12Horas(horaActual), pixelsH.Color(lum,0,0));
   // Refresca la string de segundos y minutos y muestra los cambios en la configuracion.
   pixelsSM.show();
   // Refresca la string de horas y muestra los cambios en la configuracion.
   pixelsH.show();
   
   // Recorre la string de pixeles de segundos y minutos y setea todos los LEDs a apagados.
   // Nota: no se apagan, pues es necesario llamar a ".show()" para ello.
   for(x=0; x<NUMPIXELSSM; x++) {
      pixelsSM.setPixelColor(x, 0);
   }//for
    
   // Recorre la string de pixeles de horas y setea todos los LEDs a apagados.
   // Nota: no se apagan, pues es necesario llamar a ".show()" para ello.
   for(x=0; x<NUMPIXELSH; x++) {
      pixelsH.setPixelColor(x, 0);
   }//for
   
   // Pausa de refresco, normalmente seteado a 1 segundo.
   delay(refresco);
}//mostrarHora

/*******************************************************************************
 * Devuelve la hora en formato 12H.
 ******************************************************************************/
int formato12Horas(int horas) {
  if(horas<12) {
    return horas;
  } else {
    return horas-12;
  }//if-elseif
}//formato12Horas
