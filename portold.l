%{

#include <stdio.h>
#include <string.h>
#include "portold.tab.h"
void showError();
%}


numbers     ([0-9])+
alpha       ([a-zA-Z])+
WhS          ([ \r\n\t])

%%



"hei de ser"       {return (STATEIF);}
"quando nao"       {return (STATEELSE);}
"enquanto hei-de estar-te fazendo"  {return (STATEWHILE);}

"permuta" 		   {return (SEMICOLLCOLON);}
"comensurado"      {return (IGUAL);}
"mor"                {return (MENOR);}
"hei de acrescentar-te" {return (MAIS);}
"hei de decrementar-te" {return (MENOS);}
"{"                 {return (ACHAVE);}
"}"                 {return (BCHAVE);}
"("                 {return (CCHAVE);}
")"                 {return (DCHAVE);}
{alpha}			   {return (STRING);}
{numbers}          {return (INT);}
" "                 {printf("S");}
.      		     {showError(); return(OTHER);}

%%


void showError(){
    printf("Err");
}