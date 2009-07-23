/*
 *  This file is part of Skeldal project
 * 
 *  Skeldal is free software: you can redistribute 
 *  it and/or modify it under the terms of the GNU General Public 
 *  License as published by the Free Software Foundation, either 
 *  version 3 of the License, or (at your option) any later version.
 *
 *  OpenSkeldal is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Skeldal.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  --------------------
 *
 *  Project home: https://sourceforge.net/projects/skeldal/
 *  
 *  Last commit made by: $Id$
 */
//#include <skeldal_win.h>
#include <stdlib.h>
#include "libs/types.h"
#include "libs/bgraph.h"
//#include <debug.h>


void bar32(int x1,int y1, int x2, int y2)
  {
  word *begline;
  int i,j;

  int mx =  Screen_GetXSize() - 1;
  int my =  Screen_GetYSize() - 1;

  if (x1>x2) swap_int(x1,x2);
  if (y1>y2) swap_int(y1,y2);
  if (x1<0) x1=0;
  if (y1<0) y1=0;
  if (x2>mx) x2=mx;
  if (y2>my) y2=my;
  for (i=y1,begline=Screen_GetAddr()+scr_linelen2*i;i<=y2;i++,begline+=scr_linelen2)
    {
    for (j=x1;j<=x2;j++) begline[j]=curcolor;
    }
  }

void hor_line32(int x1,int y1,int x2)
  {
  word *begline;
  int i;
  unsigned long curcolor2=curcolor | (curcolor<<16);

  int mx =  Screen_GetXSize() - 1;
  int my =  Screen_GetYSize() - 1;

  if (y1<0 || y1>my) return;
  if (x1>x2) swap_int(x1,x2);  
  if (x1<0) x1=0;
  if (x2>mx) x2=mx;
  begline=Screen_GetAddr()+scr_linelen2*y1;
  for (i=x1;i<x2;i+=2) *(unsigned long *)(begline+i)=curcolor2;
  if (i==x2) begline[i]=curcolor;
  }

void ver_line32(int x1,int y1,int y2)
  {
  int mx =  Screen_GetXSize() - 1;
  int my =  Screen_GetYSize() - 1;

  word *begline;
  int i;
  if (y1>y2) swap_int(y1,y2);
  if (x1<0 || x1>mx) return;
  if (y1<0) y1=0;
  if (y2>my) y2=my;
  begline=Screen_GetAddr()+scr_linelen2*y1+x1;
  for (i=y1;i<=y2;i++,begline+=scr_linelen2) *begline=curcolor;
  }
  
void hor_line_xor(int x1,int y1,int x2)
  {
  word *begline;
  int i;
  unsigned long curcolor2=curcolor | (curcolor<<16);

  int mx =  Screen_GetXSize() - 1;
  int my =  Screen_GetYSize() - 1;

  if (y1<0 || y1>my) return;
  if (x1>x2) swap_int(x1,x2);  
  if (x1<0) x1=0;
  if (x2>mx) x2=mx;
  begline=Screen_GetAddr()+scr_linelen2*y1;
  for (i=x1;i<x2;i+=2) *(unsigned long *)(begline+i)^=curcolor2;
  if (i==x2) begline[i]^=curcolor;
  }

void ver_line_xor(int x1,int y1,int y2)
  {
  word *begline;
  int i;

  int mx =  Screen_GetXSize() - 1;
  int my =  Screen_GetYSize() - 1;

  if (y1>y2) swap_int(y1,y2);
  if (x1<0 || x1>mx) return;
  if (y1<0) y1=0;
  if (y2>my) y2=my;
  begline=Screen_GetAddr()+scr_linelen2*y1+x1;
  for (i=y1;i<=y2;i++,begline+=scr_linelen2) *begline^=curcolor;
  }

void line_32(int x,int y,int xs,int ys)
  {
  int xp,yp,ly;
  if (xs==0) {ver_line32(x,y,y+ys);return;}
  if (ys==0) {hor_line32(x,y,x+xs);return;}
  if (xs<0)
	{
	x=x+xs;
	y=y+ys;
	ys=-ys;
	xs=-xs;
	}
  ly=y;
  for (xp=0;xp<=xs;xp++)
	{
	yp=(xp*ys+(xs>>1))/xs+y;
	  {
	  ver_line32(xp+x,ly,yp);
	  ly=yp+(ys>0)-(ys<0);
	  }
	}
  }

void char_32(word *posit,word *font,char znak)
//#pragma aux char_32 parm [edi] [esi] [eax] modify [eax ebx ecx edx]
  {
// FIXME: rewrite
/*
  __asm
    {
        mov     edi,posit;
        mov     esi,font;
        mov     al,znak
                        ;edi - pozice na obrazovce
                        ;esi - ukazatel na font
                        ;al - znak
        and     eax,0ffh
        mov     ax,[esi][eax*2]
        or      ax,ax
        jz      chrend
        add     esi,eax
        lodsw
        xor     dl,dl   ;dl - je citac transparetnich pozic
        mov     cx,ax  ;cl - XRES, ch - YRES
chr6:   mov     ebx,edi ;ebx - ukazuje po radcich v jednom sloupci
        mov     dh,ch   ;dh - bude citac radku
chr5:   or      dl,dl   ;pokud je dl = 0 pak se cte dalsi bajt
        jnz     chr1    ;jinak je dalsi bod jenom transparetni
        lodsb           ;cti barvu
        or      al,al   ;pokud je 0 pak je transparetni
        jz      chr2    ;preskoc kresleni
        cmp     al,8    ;8 a vice jsou informace o opakovanych transparetnich bodech
        jnc     chr3    ;(viz FONTEDIT.DOC). Pak se podle toho zarid
        and     eax,0ffh;v eax jen dolnich 8 bitu
        dec     al
        mov     ax,short ptr charcolors[EAX*2] ;vyjmi barvu
        cmp     ax,0xffff ;0xffff je barva ktera se nekresli;
        jz      chr4    ;
        mov     [ebx],ax;zobraz ji na obrazovce
        jmp     chr4    ;a skoc na konec smycky
chr3:   cmp     al,0ffh ;pokud je al=255 pak jsme narazily na terminator.
        jz      chrend  ;V tom pripade KONEC
        sub     al,6    ;odecti do al 6. Ziskas pocet transparetnich pozic
        mov     dl,al   ;uloz je do citace
chr1:   dec     dl      ;pro kazdou pozici to dl odecti dokud neni 0
chr2:
chr4:   add     ebx,scr_linelen;dalsi radka
        dec     dh      ;odecti citac radek
        jnz     chr5    ;dokud neni nula
        add     edi,2   ;dalsi sloupec
        dec     cl      ;odecti citac sloupcu
        jnz     chr6    ;dokud neni nula
chrend:                 ;konec
    }
*/
  }
void char2_32(word *posit,word *font,char znak)
//#pragma aux char2_32 parm [edi] [esi] [eax] modify [eax ebx ecx edx]
  {
// FIXME: rewrite
/*
  __asm
    {
        mov edi,posit
        mov esi,font
        mov al,znak
    
                        ;edi - pozice na obrazovce
                        ;esi - ukazatel na font
                        ;al - znak
        and     eax,0ffh
        mov     ax,[esi][eax*2]
        or      ax,ax
        jz      chr2end
        add     esi,eax
        lodsw
        xor     dl,dl   ;dl - je citac transparetnich pozic
        mov     cx,ax  ;cl - XRES, ch - YRES
chr26:   mov     ebx,edi ;ebx - ukazuje po radcich v jednom sloupci
        mov     dh,ch   ;dh - bude citac radku
chr25:   or      dl,dl   ;pokud je dl = 0 pak se cte dalsi bajt
        jnz     chr21    ;jinak je dalsi bod jenom transparetni
        lodsb           ;cti barvu
        or      al,al   ;pokud je 0 pak je transparetni
        jz      chr22    ;preskoc kresleni
        cmp     al,8    ;8 a vice jsou informace o opakovanych transparetnich bodech
        jnc     chr23    ;(viz FONTEDIT.DOC). Pak se podle toho zarid
        and     eax,0ffh;v eax jen dolnich 8 bitu
        dec     al
        mov     ax,charcolors[EAX*2] ;vyjmi barvu
        push    ebx
        mov     [ebx],ax;zobraz ji na obrazovce
        mov     [ebx+2],ax;zobraz ji na obrazovce
        add     ebx,scr_linelen
        mov     [ebx],ax;zobraz ji na obrazovce
        mov     [ebx+2],ax;zobraz ji na obrazovce
        pop     ebx
        jmp     chr24    ;a skoc na konec smycky
chr23:   cmp     al,0ffh ;pokud je al=255 pak jsme narazily na terminator.
        jz      chr2end  ;V tom pripade KONEC
        sub     al,6    ;odecti do al 6. Ziskas pocet transparetnich pozic
        mov     dl,al   ;uloz je do citace
chr21:   dec     dl      ;pro kazdou pozici to dl odecti dokud neni 0
chr22:
chr24:  add     ebx,scr_linelen;dalsi radka
        add     ebx,scr_linelen
        dec     dh      ;odecti citac radek
        jnz     chr25    ;dokud neni nula
        add     edi,4   ;dalsi sloupec
        dec     cl      ;odecti citac sloupcu
        jnz     chr26    ;dokud neni nula
chr2end:              ;konec
    }
*/
  }

word charsize(word *font,char znak)
  {
//#pragma aux charsize parm [esi] [eax]
// FIXME: rewrite
/*
  __asm
    {
    mov     esi,font
    mov     al,znak

              ;esi - ukazatel na font
                        ;al - znak
        and     eax,0ffh
        mov     ax,[esi][eax*2]
        or      ax,ax
        jz      chsend
        add     esi,eax
        lodsw
chsend: and     eax,0ffffh
    }
*/
  }

void put_picture(word x,word y,void *p)
//#pragma aux put_picture parm [esi] [eax] [edi] modify [ebx ecx edx]
  {
  word *adr=Screen_GetAddr()+scr_linelen2*y+x;
  word *data=p;
  word xs=data[0];
  word ys=data[1];
  word mode=data[2];
  word xss=xs;
  word yss=ys;

  if (x+xss>=Screen_GetXSize()) xss=Screen_GetXSize()-x;
  if (y+yss>=Screen_GetYSize()) yss=Screen_GetYSize()-y;

  data+=3;

  if (mode==15)
    {
    int i;
    int j;
    
    for (i=0;i<yss;i++,adr+=scr_linelen2,data+=(xs-xss))
      for (j=0;j<xss;j++) 
        {
        adr[j]=((*data & ~0x1f)<<1) | (*data & 0x1f);
        data++;
        }
    }
  if (mode==16)
    {
    int i;
    int j;
    
    for (i=0;i<yss;i++,adr+=scr_linelen2,data+=(xs-xss))
      for (j=0;j<xss;j++) 
        {
        adr[j]=*data;
        data++;
        }
    }
  if (mode==8 || mode==264)
    {
    word *table=data;
    char *cdata=(char *)(data+(mode==264?10*256:256));
    int i;
    int j;
    
    for (i=0;i<yss;i++,adr+=scr_linelen2,cdata+=(xs-xss))
      for (j=0;j<xss;j++)
        {
        if (*cdata)
        adr[j]=table[*cdata];
        cdata++;
        }
    }
  else if (mode==512 )
    {
    word *table=data;
    char *cdata=(char *)(data+256);
    int i;
    int j;
    
    for (i=0;i<yss;i++,adr+=scr_linelen2,cdata+=(xs-xss))
      for (j=0;j<xss;j++)
        {
        if (*cdata)
        adr[j]=table[*cdata]+(table[*cdata] & ~0x1F);
        cdata++;
        }
    }
  }
void get_picture(word x,word y,word xs,word ys,void *p)
  {
  word *adr=Screen_GetAddr()+scr_linelen2*y+x;
  word *data=p;
  word xss=xs;
  word yss=ys;

  if (x+xss>=Screen_GetXSize()) xss=Screen_GetXSize()-x;
  if (y+yss>=Screen_GetYSize()) yss=Screen_GetYSize()-y;
  
  data[0]=xss;
  data[1]=yss;
  data[2]=16;
  data+=3;
    {
    int i;
    int j;
    
    for (i=0;i<yss;i++,adr+=scr_linelen2)
      for (j=0;j<xss;j++) 
        {
        *data=adr[j];
        data++;
        }
    }
  }

void put_image(word *image,word *target,int start_line,int sizex,int sizey)
//#pragma aux put_image parm [ESI][EDI][EAX][EBX][EDX] modify [ECX]
  {
// FIXME: rewrite
/*
  __asm
    {
        mov     esi,image
        mov     edi,target
        mov     eax,start_line
        mov     ebx,sizex
        mov     edx,sizey
             ;ESI - obrazek
                        ;EDI - obrazovka
                        ;EAX - startline
                        ;EBX - velikostx
                        ;EDX - velikosty
        shl     eax,1
        xor     ecx,ecx
        mov     cx,[esi]
        imul    eax,ecx
        add     esi,eax
        add     esi,6
        mov     eax,ecx
puti_lp:mov     ecx,ebx
        shr     ecx,1
        rep     movsd
        rcl     ecx,1
        rep     movsw
        mov     ecx,eax
        sub     ecx,ebx
        add     esi,ecx
        sub     edi,ebx
        sub     edi,ebx
        add     edi,scr_linelen
        dec     edx
        jnz     puti_lp
    }
*/
  }

void put_8bit_clipped(void *src,void *trg,int startline,int velx,int vely)
//#pragma aux put_8bit_clipped parm [ESI][EDI][EAX][EBX][EDX] modify [ECX];
  {
  if (src==NULL) return;
// FIXME: rewrite
/*
  _asm
    {
        mov     esi,src
        mov     edi,trg
        mov     eax,startline
        mov     ebx,velx
        mov     edx,vely
                          ;ESI - obrazek
                         ;EDI - obrazovka
                         ;EAX - startline
                         ;EBX - velikostx
                         ;EDX - velikosty

        push    ebp
        mov     ebp,ebx
        xor     ecx,ecx
        mov     cx,[esi]
        imul    eax,ecx
        lea     ebx,[esi+6];
        add     esi,6+512
        add     esi,eax
        mov     eax,ecx
put8_lp:mov     ecx,ebp
        push    eax
put8lp2:xor     eax,eax
        lodsb
        or      eax,eax
        jz      put8_trns
        mov     eax,[ebx+eax*2]
        stosw
put8nxt:dec     ecx
        jnz     put8lp2
        pop     eax
        mov     ecx,eax
        sub     ecx,ebp
        add     esi,ecx
        sub     edi,ebp
        sub     edi,ebp
        add     edi,scr_linelen
        dec     edx
        jnz     put8_lp
        pop     ebp
        jmp     ende
put8_trns:
        add     edi,2
        jmp     put8nxt
ende:
    }
*/
  }

void put_textured_bar_(void *src,void *trg,int xsiz,int ysiz,int xofs,int yofs)
//#pragma aux put_textured_bar_ parm [EBX][EDI][EDX][ECX][ESI][EAX];
  {
// FIXME: rewrite
/*
  __asm
    {
    mov    ebx,src
    mov    edi,trg
    mov    edx,xsiz
    mov    ecx,ysiz
    mov    esi,xofs
    mov    eax,yofs
                        ;zobrazi texturovany obdelnik tvoreny cyklickou texturou
                        ;vstup
                        ;    EDI - pozice na obrazovce
                        ;    EBX - textura
                        ;    ECX - velikosty
                        ;    EDX - velikostx
                        ;    EAX - yofs
                        ;    ESI - xofs

        push    ebp             ;uchovej EBP - bude pouzit jako univerzalni registr
        mov     ebp,eax         ;zacneme o EAX radku v texture niz
        shl     edx,16          ;nachvili uchovej dx v horni pulce
        mul     short ptr [ebx]  ;zjisti pocatecni adresu v texture
        shl     eax,16
        shrd    eax,edx,16      ;EAX=AX*word ptr[ebx]
        shr     edx,16
        xchg    esi,eax         ;napln esi pocatkem
        lea     esi,[esi+512]   ;vypocti zacatek v texture+xlat
        lea     ebx,[ebx+6]     ;postav ebx na zacatek xlat
        add     esi,ebx         ;pricti zacatek textury
        add     esi,eax         ;pricti k ukazateli ofset v x
ptb_l2: push    eax             ;uchovej xofset
        push    edx             ;uchovej velikost v x
        push    esi             ;uchovej zacatek radky v texture
        shl     ebp,16          ;odsun citac offset do horni pulky ebp
        add     bp,ax           ;pricti k bp offset v x
ptb_l1: xor     eax,eax         ;pred kazdym bodem vynuluj eax
        lodsb                   ;nacti barvu
        or      al,al
        jz      ptb_s1
        mov     ax,[ebx+eax*2];vyhledej barvu v palete
        mov     [edi],ax        ;zapis hicolor barvu
ptb_s1: add     edi,2
        inc     bp              ;zvys counter v x
        cmp     bp,[ebx-6]      ;byl dosazen pravy roh textury?
        jc      ptb_skip1       ;ne pokracuj
        mov     ax,bp           ;nacti eax counterem
        sub     esi,eax         ;odexti to cele od esi - tj spatky doleva
        xor     bp,bp
ptb_skip1:
        dec     edx
        jnz     ptb_l1
        shr     ebp,16          ;vrat counter pro y
        pop     esi             ;obnov pocatek v texture
        mov     ax,[ebx-6]      ;nalouduj do eax delku x (horni pulka je zarucene prazdna)
        add     esi,eax         ;pricti k esi
        inc     ebp             ;zvys y counter
        cmp     bp,[ebx-4]      ;test zda jsme dosahli dolniho rohu textury
        jc      ptb_skip2       ;ne pokracuj
        mov     eax,ebp         ;ano nalouduj eax
        mul     short ptr [ebx-6];vynasob ho delkou v x
        shl     eax,16
        shrd    eax,edx,16      ;eax=ax*word ptr [ebx-4]
        sub     esi,eax         ;jsme na zacatku
        xor     ebp,ebp
ptb_skip2:
        pop     edx             ;obnov zbyvajici registry
        pop     eax
        add     edi,scr_linelen;
        sub     edi,edx
        sub     edi,edx
        dec     ecx             ;odecti 1 od globalniho citace radek
        jnz     ptb_l2          ;konec velke smycky
        POP     EBP        
    }    
*/
  }

#define MIXTRANSP(a,b) ((((a) & 0xF7DE)+((b) & 0xF7DE))>>1)

void trans_bar(int x,int y,int xs,int ys,int barva)
  {
  word *begline;
  int x1=x;
  int y1=y;
  int x2=x+xs-1;
  int y2=y+ys-1;
  int i,j;

  int mx =  Screen_GetXSize() - 1;
  int my =  Screen_GetYSize() - 1;

  if (x1>x2) swap_int(x1,x2);
  if (y1>y2) swap_int(y1,y2);
  if (x1<0) x1=0;
  if (y1<0) y1=0;
  if (x2>mx) x2=mx;
  if (y2>my) y2=my;
  for (i=y1,begline=Screen_GetAddr()+scr_linelen2*i;i<=y2;i++,begline+=scr_linelen2)
    {
    for (j=x1;j<=x2;j++) begline[j]=MIXTRANSP(begline[j],barva);
    }
  }

void trans_line_x(int x,int y,int xs,int barva)
  {
  trans_bar(x,y,xs,1,barva);
  }

void trans_line_y(int x,int y,int ys,int barva)
  {
  trans_bar(x,y,1,ys,barva);
  }

void trans_bar25(int x,int y,int xs,int ys)
  {
  word *begline;
  int x1=x;
  int y1=y;
  int x2=x+xs-1;
  int y2=y+ys-1;
  int i,j;

  int mx =  Screen_GetXSize() - 1;
  int my =  Screen_GetYSize() - 1;

  if (x1>x2) swap_int(x1,x2);
  if (y1>y2) swap_int(y1,y2);
  if (x1<0) x1=0;
  if (y1<0) y1=0;
  if (x2>mx) x2=mx;
  if (y2>my) y2=my;
  for (i=y1,begline=Screen_GetAddr()+scr_linelen2*i;i<=y2;i++,begline+=scr_linelen2)
    {
    for (j=x1;j<=x2;j++) begline[j]=MIXTRANSP(begline[j],MIXTRANSP(begline[j],0));
    }
  }

void wait_retrace()
  {

  }


#define pic_start (2+2+2+512*5+512*5)

void put_picture2picture(word *source,word *target,int xp,int yp)
//#pragma aux put_picture2picture parm [ESI][EDI][EAX][EDX] modify [ECX]
  {
// FIXME: rewrite
/*
  __asm
    {
    mov   esi,source
    mov   edi,target
    mov   eax,xp
    mov   edx,yp
                             ;ESI - obrazek 256c
                             ;EDI - obrazek 256
                             ;EAX - Xpos
                             ;EDX - Ypos
        movzx   ecx,short ptr [edi]              ;vyzvedni sirku obrazku
        imul    edx,ecx                         ;nova adresa=edi+(Xpos+sirka*Ypos)
        add     eax,edx
        mov     edx,ecx
        lea     edi,[edi+eax+pic_start]         ;edi obsahuje novou adresu
        mov     ecx,[esi]                       ;ecx obsahuje Xsize a Ysize obrazku
        sub     ecx,10000h                      ;Ysize-1
        lea     esi,[esi+pic_start]             ;nastav esi na zacatek bitmapy
ppp_lp2:push    ecx                             ;uchovej velikosti obrazku v zasobniku
ppp_lp1:lodsb                                   ;nacti bod ze zdroje
        or      al,al                           ;zkontroluj zda to neni transparentni barva
        jz      ppp_trn                         ;pokud je tak ji prezskoc
        mov     [edi],al                        ;zapis barvu
ppp_trn:inc     edi                             ;dalsi bod
        dec     cx                              ;odecitej x sirku
        jnz     ppp_lp1                         ;dokud to neni vse
        pop     ecx                             ;obnov ecx
        movzx   eax,cx                          ;vem jeste jednou sirku
        sub     eax,edx                         ;spocitej kolik bajtu je nutne
        sub     edi,eax                         ;prezkocit k dosazeni dalsi radky
        sub     ecx,10000h                      ;opakuj pro y radku
        jnc     ppp_lp2
    }
*/
  }

