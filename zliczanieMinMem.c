#include <stdio.h>
#include <time.h>

//	funkcje pomocnicze:
unsigned long long losuj();				// slowo o losowej liczbie jedynek (rozklad normalny)
unsigned long long losujIle(int ile1);	// slowo o zadanej liczbie jedynek

/*
	Pod komentarzem funkcja zliczajaca jedynki.
	Wywolywana zawsze z argumentem b = 32. 

	Ponizsza tabela pokazuje z jaka liczb¹ porownywac
	argument b (lewa kolumna), zeby algorytm dzielil slowo 
	wejsciowe na slowa o pozadanej ilosci bitow (prawa kolumna):

	b:	  bity:
	32	- 64
	16	- 32
	8	- 16
	4	- 8
	2	- 4
	1	- 2
	0	- 1
	
	Przykladowo dla parametru = 16, szybciej wykonywany jest 
	"test szybkosci 3" (testy w funkcji main) niz dla
	parametru = 32

	Na koncu pliku podalem przyklad implementacji ktora na sztywno
	dzieli slowo wejœciowe na 2 slowa 32 - bitowe (funkcja licz32)
	oraz implementacji nie dzielacej slowa wejsciowego (licz64)
*/
int licz(unsigned long long liczba, int b)
{
	if(!liczba)
		return 0;
	else if(b == 32){ //mozliwa zmiana parametru, zmienic tez ponizej
		while(liczba){
			b += liczba & 1;
			liczba >>= 1;
		}
		return b - 32; //wpisac ten sam parametr, co powyzej
	}else
		//podzial slowa wejsciowego na wzor drzewa binarnego:
		return licz(liczba >> b, b >> 1) + licz(liczba & 0xffffffff >> 32 - b, b >> 1);
		
}

main()
{
	unsigned long long num = 0;
	unsigned long long k = 0;
	clock_t t;
	srand(time(NULL));
	
	//	test poprawnosci:
	for(k = 0; k < 65; k++){
		num = losujIle(k);
		if(licz(num, 32) != k){
			printf("blad dla liczby %d\n", k);
			return;
		}		
	}
	printf("test poprawnosci: ok\n");
	
	/*	
		w ponizszych testach zmienna k jest iterowana rozn¹ ilosc razy,
		bo wyniki sluza do porownania roznych algorytmow a nie wynikow
		jednego algorytmu dla roznych testow
	*/
	
	//	test szybkosci dla losowego slowa:
	t = clock();
	for(k = 0; k < 0x1ffffff; k++)
		licz(losuj(), 32);
	t = clock() - t;
	printf("test szybkosci 1 -> liczba cykli: %u\n", t);
	
	//	test szybkosci dla slowa bitowego o losowej (rozklad jednostajny) liczbie jedynek:
	t = clock();
	for(k = 0; k < 0xffffff; k++){
		num = losujIle(rand() % 65);
		licz(num, 32);
	}
	t = clock() - t;
	printf("test szybkosci 2 -> liczba cykli: %u\n", t);
	
	//	test szybkosci dla jedynek tylko w jednej polowie slowa:
	t = clock();
	for(k = 0x100000000; k < 0x3ffffff00000000; k += 0x100000000)
		licz(k, 32);
	t = clock() - t;
	printf("test szybkosci 3 -> liczba cykli: %u\n", t);
	
	
}

unsigned long long losuj()
{
	unsigned long long liczba;
	int i;
	unsigned char *bajt = (unsigned char*)&liczba;
	
	for(i = 0; i < 8; i++){
		*bajt++ = rand() % 256;
	}
	
	return liczba;
}

unsigned long long losujIle(int ile1)
{
	unsigned long long liczba1, liczba2;
	
	if(ile1 > 32){
		liczba1 = 0xffffffffffffffff;
		while(64 - ile1++){
			liczba2 = ~((unsigned long long)1 << rand() % 64);
			if(liczba1 == (liczba1 & liczba2))
				ile1--;
			liczba1 &= liczba2;
		}
	}else{
		liczba1 = 0;
		while(ile1--){
			liczba2 = (unsigned long long)1 << rand() % 64;
			if(liczba1 == (liczba1 | liczba2))
				ile1++;
			liczba1 |= liczba2;
		}
	}
	return liczba1;
}

int licz64(unsigned long long liczba)
{
	int ile = 0;
	while(liczba){
		ile += liczba & 1;
		liczba >>= 1;
	}
	return ile;
}

int podLicz(unsigned long liczba) //funkcja wywolywana przez licz32
{
	int ile = 0;
	while(liczba){
		ile += liczba & 1;
		liczba >>= 1;
	}
	return ile;
}

int licz32(unsigned long *liczba)
{	
	if(!*(unsigned long long*)liczba){
		return 0;
	}
		
	return podLicz(*liczba++) + podLicz(*liczba);
}

