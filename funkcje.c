
// funkcja zamieniająca jedną z 4 połówek (to którą określa zmienna nr <0, 3>)
// bajta 2 bajtowego shorta na znak
char bythalf(int in, short nr){
	char ret = in>>4*nr & 15;
	return (ret < 10) ? ret + 48 : ret + 87;
}

// funkcja zmieniająca 4 znaki na liczbę short (2 bajtowy)
unsigned short pr2(char* a){
	int i;
	unsigned short w = 0;
	for(i = 0; i < 4; i++){
		w |= ((a[i] < 58) ? a[i] - 48 : a[i] - 87) << (3-i)*4;
	}
	return w;
}