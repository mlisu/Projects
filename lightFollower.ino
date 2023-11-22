/* Projekt wieżyczki obracającej się w stronę źródła światła, przesyłającej informację
o kierunku padania (kierunek z prawej lub z lewej) do uC poprzez podczerwień.
Na wieżyczce są 2 diody IR. Pod wieżyczką jest 6 fototranzystorów ustawionych
w regularnych odstępach na okręgu. W danym momecie następuje porównanie między dwoma
przeciwległymi tranzystorami (decyzja o obrocie prawo/lewo) oraz porównanie tranzystora następnego
(patrząc w aktualnym kierunku obrotu) względem tego z którego informacja decyduje
aktualnie o obrocie*/

#include <M5Core2.h>
#include <ESP32Servo.h>

#define EPS1 50   // epsilon 1 - histereza dla obrotu w prawo/lewo
#define EPS2 100  // epsilon 2 - histereza dla przełączania aktywnego czujnika na czujnik następny
#define INC 1     // przyrost pozycji w przypadku decyzji o obrocie
#define S_PIN 19  // pin do PWM dla servo
#define DL 15     // opóźnienie miedzy kolejnymi inkrementacjami pozycji
#define LIM 20    // pozycja serva możliwa w zakresie <LIM, 180 - LIM> stopni

Servo theser;

short tab[6] = {14, 34, 27, 35, 36, 25}; /*numery pinów do analogRead. Pierwsze 3 odpowiadają za obrót w prawo, pozostałe
                                          za obrót w lewo. Rozmieszczenie tranzystorów podłączonych do wejść analogowych
                                          (dla lewej połowy kartezjańskiego układu współrzędnych patrząc z góry wieżyczki
                                          tranzystory są rozmieszczone wokół osi obrotu w przybliżeniu w pozycjach
                                          zdefiniowanych kątami -110, -180, -250; dla prawej strony -70, 0, 70) odpowiada
                                          indeksom w tablicy (rosnący kąt - rosnący indeks tablicy)*/
short idx;
short valLeft, valRight;
short *gt, *lt;
short delta;
short left, right;
short active;
short gtChange;
short c;
short pos;

void fn(short inc, short val, short idxFn, short *gtSide, short *ltSide)
{
    pos += inc;
    if(pos > 180 - LIM)
      pos = 180 - LIM;
    else if(pos < LIM)
      pos = LIM;
    
    theser.write(pos);
    active = val;                       // val to odczyt z aktywnego tranzystora (tzn wywołującego w danym momecnie obrót).
                                        // zmienna activ służy do decyzji o przełączeniu na następny (po tej samej stronie) tranzystor
                                        // w przypadku większej wartości na nim odczytanej w wyniku obrotu

    gtChange = tab[idxFn + c % 2 + 1];  // gtChange - numer pinu do którego podłączony jest następny tranzystor w kierunku obrotu.
                                        // idxFn to indeks bazowy pod którym jest pierwszy z pinów dla obrotu w lewo albo w prawo
                                        // (0 dla prawo, 3 dla lewo). Zmienna c to informacja o indeksie pinu w tablicy, pod którym jest tranzystor
                                        // z którego może nastąpić przełączenie na następny tranzystor

    idx = (idxFn + 3) % 6;              // idx po tym przypisaniu to indeks pierwszego tranzystora w tablicy dla strony przeciwnej do strony
                                        // po której leży czujnik w danym momencie odczytywany
   
    gt = gtSide;                        // gt - greater than - dla obrotu w prawo przypisywany jest mu adres zmiennej left (nomenklatura odwrotna
    lt = ltSide;                        // bo obrót w prawo następuje przy świeceniu z lewej strony; dla odwrotu w lewo - zmiennej right)
                                        // w celu ewentualnego wpisania tam pinu gtChange. Do lt przypisywany jest adres drugiej ze zmiennych left/right
    
}

void setup() {
  M5.begin();
  Serial.begin(115200);

  analogSetAttenuation(ADC_0db);

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  theser.setPeriodHertz(50);
  theser.attach(S_PIN, 450, 2450);

  left = 34;              // wieżyczka na początku obracana jest do pozycji 90 stopni. Przypisanie odpowiednich pinów, aktywnych w tej pozycji.
  right = 36;             // Piny te mają indeksy w tablicy 1 (0 + c) oraz 4 (3 + c).
  gtChange = 27;
  c = 1;

  theser.write(pos = 90);

  delay(500);
}

void loop() {

  valLeft = analogRead(left);
  valRight = analogRead(right);
  delta = valLeft - valRight;

  if(delta > EPS1) //left > right
    fn(-INC, valLeft, 0, &left, &right);
  else if(delta < -EPS1)
    fn(INC, valRight, 3, &right, &left);
    
  if(analogRead(gtChange) > active + EPS2){ //sprawdzenie czy przełączyć na następny tranzystor
    *gt = gtChange;
    *lt = tab[idx + ++c % 2];
  }
  
  delay(DL);
}


