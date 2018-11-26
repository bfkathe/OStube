#include <stdio.h>
#include <string.h>

int main () {
   FILE *fp;
   int i = 0;
   char total[9999];
   char str[200] = {'\0'};

   /* opening file for reading */
   fp = fopen("index.html" , "r+");
   if(fp == NULL) {
      perror("Error opening file");
      return(-1);
   }
   while( fgets (str, 60, fp)!=NULL ) {
      /* writing content to stdout */
	for(int n = 0; str[n]!=10;n++){
		printf("i: %d\n",i);
		total[i]=str[n];
		i++;
	}
	total[i] = 10;
	i++;
	if(str[17] == '/'){
		if(str[18]=='u'){
			printf("ya\n");
		}
	}
      puts(str);
      char str[200] = {'\0'};
   }
   fputs( total, fp );
   fclose(fp);
   printf("total: %s\n",total);
	printf("letra: %c\n",total[1006]);
   
   return(0);
}
