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
#include <string.h>
#include "libs/types.h"
#include "game/engine1.h"
#include "libs/bgraph.h"
#include "game/globals.h"


typedef ZOOMINFO tzoom;

extern ZOOMINFO zoom;
extern word *screen;

void sikma_zleva(void)
  {
/*  __asm
	{
        mov     edi,zoom       ;nacti ukazatel do obrazovky
  	    mov     ebx,[zoom]tzoom.palette ;ukazatel na paletu
        mov     cx,short ptr [zoom]tzoom.ycount ;velikost textury na y
        shl     ecx,16  ;vloz do horni pulky ecx
        mov     esi,[zoom]tzoom.texture ;nacti ukazatel na texturu
skzl3:  mov     edx,[zoom]tzoom.xtable ;nacti ukazetel na zvetsovaci tabulku x
        push    esi ;uchovej esi
        push    edi ;uchovej edi
        mov     cx,[zoom]tzoom.xmax
skzl1:  xor     eax,eax ;vynuluj eax pro spravny vypocet
        lodsb       ;nacti bod
        add     esi,[edx] ;posun se od nekolik pozic v texture podle hodnoty v tabulce x
        add     edx,4 ;posun se v tabulce x o dalsi polozku
        or      al,al ;test bodu na nulu
        jz      skz1  ;preskoc transparetni barvu
        cmp     al,1  ;test bodu na jedna
        jz      skz2  ;ukonci kresleni linky pokud narazi na 1
        mov     ax,[eax*2+ebx] ;konverze barvy podle palety
        mov     [edi],ax ;nakresli bod na obrazovce
skz1:   add     edi,2 ;dalsi pozice
        dec     cx
        jnz     skzl1 ;opakuj dokola
skz2:   pop     edi ;obnov edi
        pop     esi ;obnov esi
        mov     edx,[zoom]tzoom.ytable ;vyzvedni ukazatel na ytable
        mov     cx,[edx] ;cx - o kolik pozic se mam v texture posunout dolu
        or      cx,cx
        jz      skzskp
skzl2:  add     esi,[zoom]tzoom.texture_line ;posun o jednu pozici
        dec     cx ;sniz citac
        jnz     skzl2 ;dokud neni nula
skzskp:add     edx,2 ;dalsi hodnota v tabulce
        mov     [zoom]tzoom.ytable,edx ;uloaz na puvodni misto
        sub     edi,[zoom]tzoom.line_len ;odecti tolik, kolik odpovida lince na obrazovce
        sub     ecx,10000h ;sniz horni pulku ecx o jedna
        jnz     skzl3 ;opakuj dokud neni nula        
	}
*/

	// TODO: needs testing
	int x, y;
	byte *src = zoom.texture, *tmp;
	word *dst = zoom.startptr;

	for (y = 0; y < zoom.ycount; y++) {
		tmp = src;
		for (x = 0; x < zoom.xmax; x++) {
			if (*src == 1) {
				src += zoom.xtable[x] + 1;
				break;
			} else if (*src) {
				dst[x-y*zoom.line_len] = zoom.palette[*src];
			}
			src += zoom.xtable[x] + 1;
		}

		src = tmp + *zoom.ytable++ * zoom.texture_line;
	}
  }

void sikma_zprava(void)
  {
/*  __asm
	{
        mov     edi,zoom       ;nacti ukazatel do obrazovky
        mov     ebx,[zoom]tzoom.palette ;ukazatel na paletu
        mov     cx,short ptr [zoom]tzoom.ycount ;velikost textury na y
        shl     ecx,16  ;vloz do horni pulky ecx
        mov     esi,[zoom]tzoom.texture ;nacti ukazatel na texturu
skzp3:  mov     edx,[zoom]tzoom.xtable ;nacti ukazetel na zvetsovaci tabulku x
        push    esi ;uchovej esi
        push    edi ;uchovej edi
        mov     cx,[zoom]tzoom.xmax
skzp1:  xor     eax,eax ;vynuluj eax pro spravny vypocet
        lodsb       ;nacti bod
        add     esi,[edx] ;posun se od nekolik pozic v texture podle hodnoty v tabulce x
        add     edx,4 ;posun se v tabulce x o dalsi polozku
        or      al,al ;test bodu na nulu
        jz      skz3  ;preskoc transparetni barvu
        cmp     al,1  ;test bodu na jedna
        jz      skz4  ;ukonci kresleni linky pokud narazi na 1
        mov     ax,[eax*2+ebx] ;konverze barvy podle palety
        mov     [edi],ax ;nakresli bod na obrazovce
skz3:   sub     edi,2 ;dalsi pozice
        dec     cx
        jnz     skzp1 ;opakuj dokola
skz4:   pop     edi ;obnov edi
        pop     esi ;obnov esi
        mov     edx,[zoom]tzoom.ytable ;vyzvedni ukazatel na ytable
        mov     cx,[edx] ;cx - o kolik pozic se mam v texture posunout dolu
        or      cx,cx
        jz      skpskp
skzp2:  add     esi,[zoom]tzoom.texture_line ;posun o jednu pozici
        dec     cx ;sniz citac
        jnz     skzp2 ;dokud neni nula
skpskp: add     edx,2 ;dalsi hodnota v tabulce
        mov     [zoom]tzoom.ytable,edx ;uloaz na puvodni misto
        sub     edi,[zoom]tzoom.line_len ;odecti tolik, kolik odpovida lince na obrazovce
        sub     ecx,10000h ;sniz horni pulku ecx o jedna
        jnz     skzp3 ;opakuj dokud neni nula
    }
*/

	// TODO: needs testing
	int x, y;
	byte *src = zoom.texture, *tmp;
	word *dst = zoom.startptr;

	for (y = 0; y < zoom.ycount; y++) {
		tmp = src;
		for (x = 0; x < zoom.xmax; x++) {
			if (*src == 1) {
				src += zoom.xtable[x] + 1;
				break;
			} else if (*src) {
				dst[-x-y*zoom.line_len] = zoom.palette[*src];
			}
			src += zoom.xtable[x] + 1;
		}

		src = tmp + *zoom.ytable++ * zoom.texture_line;
	}
  }

/*
void zooming_dx(void *source,void *target,void *background,void *xlat_tab,long xysize)
//#pragma aux zooming_dx parm [ESI][EDI][EAX][EBX][ECX] modify [EDX]
  {
  __asm
    {
    mov esi,source
    mov edi,target
    mov eax,background
    mov ecx,xysize
    mov ebx,xlat_tab
    
                        ;esi - zdrojovy blok
                        ;edi - cil
                        ;eax - pozadi
                        ;zoom.xtable - tabulka pro x
                        ;zoom.xtable - tabulka pro y
                        ;zoom.textline - rozdil mezi pravym okrajem a levym okrajem
                        ;               pri prechodu na novou radku
                        ;ecx ysize:xsize
                        ;ebx xlat
        push    ebp
        mov     ebp,eax
        mov     ebx,[zoom]tzoom.ytable
z32d:   push    cx
        mov     edx,[zoom]tzoom.xtable
        push    esi
z32c:   mov     al,[edx]
        inc     edx
        or      al,al
        jz      z32a
        lodsw
        test    ax,BGSWITCHBIT 
        jnz      z32e
        stosw
        stosw
        jmp     z32b
z32f:   test    eax,BGSWITCHBIT 
        jz      z32g
        test    eax,BGSWITCHBIT<<16
        jz      z32h
z32e:   mov     eax,ds:[ebp]
        stosd
        jmp     z32b
z32g:   rol     eax,16
        mov     ax,ds:[ebp+2]
        rol     eax,16
        stosd
        jmp     z32b
z32h:   mov     ax,ds:[ebp]
        stosd
        jmp     z32b
z32a:   lodsd
        test    eax,BGSWITCHBIT | (BGSWITCHBIT <<16)
        jnz      z32f
        stosd
z32b:   add     ebp,4
        dec     cx
        jnz     z32c
        pop     esi
        mov     eax,[ebx]
        and     eax,0ffffh
        add     esi,eax
        add     ebx,2
        add     edi,[zoom]tzoom.texture_line
        xor     cx,cx
        sub     ecx,10000h
        pop     cx
        jnz     z32d
        pop     ebp
    }
  }
*/

/*
void scroll_support_dx(void *lbuf,void *src1,void *src2,int size1)
//#pragma aux scroll_support_dx parm [EDI][ESI][EDX][ECX] modify [EAX]
  {
  __asm
    {
        mov     edi,lbuf
        mov     esi,src1
        mov     edx,scr2	; mov edx,src2?
        mov     ecx,size1

        push    ebp             ;uchovej ebp
        mov     ebp,360         ;ebp pro tuto chvili predstavuje citac
        mov     eax,ecx         ;uchovej ecx jeste v eac - zachova citac sloupcu
scrl1:  push    esi             ;uchovej esi
        shr     ecx,1           ;presun ecx bloku
        rep     movsd
        rcl     ecx,1
        rep     movsw
        mov     ecx,640         ;dopocitej ecx do 640
        sub     ecx,eax
        pop     esi             ;obnov esi
        xchg    esi,edx         ;esi je nyni novy blok
        push    esi             ;uchovek esi
        shr     ecx,1           ;presun
        rep     movsd
        rcl     ecx,1
        rep     movsw
        pop     esi             ;obnov esi
        xchg    esi,edx         ;vrat edx a esi do puvodniho stavu
        mov     ecx,eax         ;obnov zase ecx z eax
        add     esi,scr_linelen        ;dalsi radek
        add     edx,scr_linelen
        dec     ebp             ; dokud nejsme na konci
        add     edi,scr_linelen;
        sub     edi,scr_linelen
        jnz     scrl1           ;opakuj
        pop     ebp
    }
  }
*/

/*
void lodka_dx(void *source,void *target,void *background,void *xlat_tab,long xysize)
//#pragma aux lodka_dx parm [ESI][EDI][EAX][EBX][ECX] modify [EDX]
  {
  __asm
    {
      mov esi,source
      mov edi,target
      mov eax,background
      mov ecx,xysize
      mov ebx,xlat_tab
                        ;esi - zdrojovy blok
                        ;edi - cil
                        ;eax - lodka
                        ;zoom.xtable - tabulka pro x
                        ;zoom.xtable - tabulka pro y
                        ;zoom.textline - rozdil mezi pravym okrajem a levym okrajem
                        ;               pri prechodu na novou radku
                        ;ecx ysize:xsize
        push    ebp
        mov     ebp,eax
        mov     ebx,[zoom]tzoom.ytable
l32d:   push    cx
        mov     edx,[zoom]tzoom.xtable
        push    esi
l32c:   mov     eax,dword ptr [ebp]
        or      eax,eax
        jnz     l32e
        mov     al,[edx]
        inc     edx
        or      al,al
        jz      l32a
        lodsw
        stosw
        stosw
        jmp     l32b
l32e:   stosd
        movzx   eax,byte ptr[edx]       ;0 - o 4 1 - o 2
        xor     eax,1                   ;0->1  1->0
        inc     eax                     ;1+1=2 0+1=1
        shl     eax,1                   ;2*2=4 1*2=2
        add     esi,eax
        inc     edx
        jmp     l32b
l32a:   lodsd
        stosd
l32b:   add     ebp,4
        dec     cx
        jnz     l32c
        pop     esi
        mov     eax,[ebx]
        and     eax,0ffffh
        add     esi,eax
        add     ebx,2
        add     edi,[zoom]tzoom.texture_line
        xor     cx,cx
        cmp     ecx,60*65536
        cmc
        sbb     eax,eax
        and     eax,scr_linelen
        sub     ebp,eax
        sub     ecx,10000h
        pop     cx
        jnz     l32d
        pop     ebp

    }
  }
*/

void fcdraw(void *source,void *target, T_FLOOR_MAP *table) {
	do {
		memcpy(target + table->lineofs, source + table->txtrofs, table->linesize * sizeof(word));
	} while ((table++)->counter);
}

static void klicovani_anm_back(word *target,word *source);
void klicovani_anm(word *target,word *source,char mirror)
  {
	if (mirror) {
		klicovani_anm_back(target,source);
		return;
	}

	// TODO: needs testing
	int x, y;
	word color;
	for (y = 0; y < 180; y++) {
		for (x = 0; x < 320; x++) {
			color = source[x+y*320];
			if (color & 0x8000) {
				continue;
			}

			// red and green are shifted by one bit to the right
			// to make space for skip bit 0x8000
			color = Screen_RGB((color >> 10) & 0x1f, (color >> 5) & 0x1f, color & 0x1f);
			target[2*x+2*y*Screen_GetXSize()] = color;
			target[2*x+1+2*y*Screen_GetXSize()] = color;
			target[2*x+(2*y+1)*Screen_GetXSize()] = color;
			target[2*x+1+(2*y+1)*Screen_GetXSize()] = color;
		}
	}
  }

static void klicovani_anm_back(word *target,word *source)
//#pragma aux klicovani_anm parm [edi][esi][eax] modify [ecx edx ebx]
  {
/*  __asm
    {
        mov     edi,target
        mov     esi,source
      
        mov     ecx,180
        add     edi,1280
kba_lp2: mov     ebx,320
kba_lp1: lodsw
        sub     edi,4
        movzx   eax,ax
        test    eax,0x8000
        jnz     kba_skip        
        mov     edx,eax
        and     edx,0x7FE0
        add     eax,edx
        mov     edx,eax
        shl     edx,16
        add     edi,scr_linelen
        or      eax,edx
        mov     [edi],eax
        sub     edi,scr_linelen
        mov     [edi],eax
kba_skip:dec     ebx
        jnz     kba_lp1
        add     edi,scr_linelen
        add     edi,scr_linelen
        add     edi,1280
        dec     ecx
        jnz     kba_lp2
    }
*/

	// TODO: needs testing
	int x, y;
	word color;
	for (y = 0; y < 180; y++) {
		for (x = 1; x <= 320; x++) {
			color = source[x+y*320];
			if (color & 0x8000) {
				continue;
			}

			// red and green are shifted by one bit to the right
			// to make space for skip bit 0x8000
			color = Screen_RGB((color >> 10) & 0x1f, (color >> 5) & 0x1f, color & 0x1f);
			target[640-2*x+2*y*Screen_GetXSize()] = color;
			target[640-2*x+1+2*y*Screen_GetXSize()] = color;
			target[640-2*x+(2*y+1)*Screen_GetXSize()] = color;
			target[640-2*x+1+(2*y+1)*Screen_GetXSize()] = color;
		}
	}
  }

#define zobraz_1 \
        __asm lodsb \
        __asm movzx   eax,al  \
        __asm movzx   eax,short ptr [eax*2+ebx]  \
        __asm stosw \




void small_anm_buff(word *target,byte *buff,word *paleta)
//#pragma aux small_anm_buff parm[edi][esi][ebx] modify [eax ecx]
  {
/*  __asm
    {
    mov   edi,target;
    mov   esi,buff;
    mov   ebx,paleta

        mov     ecx,179*10000h
shmab4: mov     cx,320
shmab3: zobraz_1
        dec     cx
        jnz     shmab3
        add     edi,640
        sub     ecx,010000h
        jnc     shmab4
    }
*/

	// TODO: needs testing
	int x, y;
	for (y = 0; y < 180; y++) {
		for (x = 0; x < 320; x++) {
			target[x+640*y] = paleta[buff[x+320*y]];
		}
	}
  }

// small_anm_delta() is similar to show_delta_lfb12e in libs/mgifplaya.c - merge?
void small_anm_delta(word *target,byte *buff,word *paleta)
//#pragma aux small_anm_delta parm[edi][esi][ebx] modify [eax ecx]
  {
/*  __asm
    {
    mov   edi,target;
    mov   esi,buff;
    mov   ebx,paleta


        mov     ch,180          ;180 radek
        mov     eax,[esi]       ;vem offset na delta data
        lea     edx,[esi+eax+4]     ;ebp obsahuje tento offset
        lea     esi,[esi+4]     ;esi obsahuje offset na delta control
shmad5: push    edi             ;uchovej zacatek radky
shmad3: lodsb                   ;vem skip hodnotu
        mov     ah,al           ;uchovej ji ah - kvuli destruktivnimu testu
        and     al,not 3fh      ;na posledni 2 bity
        cmp     al,0c0h         ;C0 a vyssi znamenaji ze se preskakuje radek
        jz      shmad1          ;pri pozitivnim testu je preskoc
        movzx   eax,ah          ;vem skip hodnotu do eax
        shl     eax,2           ;na obrazovce *2 bodu *1 zoom *2 za bod (x4)
        add     edi,eax         ;pricti k edi
        lodsb                   ;vem copy hodnotu
        mov     cl,al           ;zaved counter
        xchg    esi,edx         ;prohod ukazatele control a data
shmad2: zobraz_1                ;zobraz bod
        zobraz_1                ;zobraz bod
        dec     cl              ;opakuj tolikrat kolik je copy hodnota
        jnz     shmad2
        xchg    esi,edx         ;prohod ukazatele control a data
        jmp     shmad3          ;a precti dalsi skip hodnotu
shmad1: pop     edi
        and     ah,03fh         ;maskuj spodnich 6 bitu
        inc     ah              ;+1
shmad4: add     edi,scr_linelen
        dec     ch              ;odecti counter radek
        dec     ah              ;odecti counter
        jnz     shmad4          ;a opakuj ah krat
        or      ch,ch           ;je li counter nulovy tak padame
        jnz     shmad5
    }
*/

	// TODO: needs testing
	int i, j, k;
	byte *map = buff + 4;
	word *tmp;
	buff += 4 + *(int*)buff;

	for (i = 0; i < 180; i += j) {
		tmp = target;
		for (j = *map++; j < 0xc0; j = *map++) {
			target += 2 * j;
			for (k = 0; k < *map; k++) {
				*target++ = paleta[*buff++];
				*target++ = paleta[*buff++];
			}
			map++;
		}

		j -= 0xbf;
		target = tmp + j * Screen_GetXSize();
	}
  }


void scroll_and_copy(word *pic, word *slide, word *scr, int _size, int shift, int lineinfo[][2])
//#pragma aux scroll_and_copy parm[esi][ebx][edi][ecx][edx][eax]
{  

/*  __asm
    {
        mov     esi,pic
        mov     ebx,slide
        mov     edi,scr
        mov     ecx,_size
        mov     edx,shift
        mov     eax,lineinfo
                ;odscroluje a kopiruje obrazovku do screenu
                                 ;pouzitelne pro titulky
                                 ;0x8000 je transparentni
                                 ;edi screen
                                 ;ebx buffer
                                 ;esi back_picture
                                 ;ecx velikost bufferu (pocet_radku)
                                 ;edx pocet bajtu o kolik je nutne posouvat (*1280)
                                 ;eax - ukazatel na tittle lines
        push    ebp
        mov     ebp,eax         ;uloz ukazatel na titlelines
        mov     eax,edx
        imul    eax,scr_linelen
sac_lp1:push    ecx
        push    eax
        mov     eax,[ebp]       ;nacti zac
        mov     ecx,[ebp+edx*8]
        cmp     eax,ecx
        jc      sac_sk1
        mov     eax,ecx
sac_sk1:shl     eax,1           ;adresa
        add     esi,eax         ;pricti k pointrum
        add     edi,eax
        add     ebx,eax
        add     ebp,4           ;presun se na kon
        shr     eax,1           ;eax je zacatek v bodech
        mov     ecx,[ebp+edx*8] ;vem konec
        cmp     ecx,[ebp]
        jnc     sac_sk2
        mov     ecx,[ebp]
sac_sk2:push    ecx
        sub     ecx,eax         ;kon-zac+1=celk pocet
        xchg    edx,[esp+4]
sac_lp2:mov     eax,[ebx+edx]   ;Vem data na dalsim radku
        mov     [ebx],eax       ;Zapis je sem
        add     ebx,4           ;dalsi pozice
        test    eax,~BGSWITCHBIT ;je to transparentni?
        jz      sac_all         ;pokud ano, zobraz_obrazek
        bt      eax,5          ;test bitu 15
        jnc     sac_1           ;Neni nastaven - zobraz primo barvu
        mov     ax,[esi]        ;jinak vem barvu z obrazku
sac_1:  stosw                   ;zapis barvu
        add     esi,2           ;dalsi pozice
        rol     eax,16          ;presun eax na dolni slovo
        bt      eax,5           ;test bitu 15
        jnc     sac_2           ;neni nastaven - zobraz primo barvu
        mov     ax,[esi]        ;jinak vem barvu z obrazku
sac_2:  stosw                   ;zapis barvu
        add     esi,2           ;dalsi pozice
        jmp     sac_end
sac_all:movsd                   ;presun celeho slova
sac_end:sub     ecx,2           ;odecti counter
        ja      sac_lp2         ;je li nad opakuj
        add     ecx,640         ;pricti sirku, - kolik jsme prejeli
        pop     eax
        sub     ecx,eax       ;odecti konec -> kolik preskocit
        add     ebp,4           ;aktualizuj ukazatele
        shl     ecx,1           ;v adresach
        add     esi,ecx
        push    edx
        mov     edx,scr_linelen
        sub     edx,1280
        add     ecx,edx
        add     edi,ecx
        add     ebx,ecx
        pop     edx
        pop     eax
        xchg    edx,eax
        pop     ecx             ;obnov ecx
        dec     ecx             ;sniz citac radku
        jnz     sac_lp1         ;dokud neni nula
        pop     ebp             ;obnov ebp

    }
*/

	// TODO: needs testing, testing and more testing
	int i, j, info0, info1, tmp;

	for (i = 0; i < _size; i++) {
		info0 = min(lineinfo[i][0], lineinfo[shift+i][0]);
		tmp = max(lineinfo[i][1], lineinfo[shift+i][1]);
		info1 = tmp - info0;

		scr += info0;
		pic += info0;
		slide += info0;

		do {
			slide[0] = slide[shift*Screen_GetXSize()];
			slide[1] = slide[shift*Screen_GetXSize()+1];

			if (((slide[1] << 16) | slide[0]) & ~BGSWITCHBIT) {
				*scr++ = (slide[0] & 0x20) ? *pic : slide[0];
				pic++;
				*scr++ = (slide[1] & 0x20) ? *pic : slide[1];
				pic++;
			} else {
				*scr++ = *pic++;
				*scr++ = *pic++;
			}

			slide += 2;
			info1 -= 2;
		} while (info1 > 0);

		info1 += 640 - tmp;
		pic += info1;
		info1 += Screen_GetXSize() - 640;
		scr += info1;
		slide += info1;
	}
}

#define pic_start 2+2+2+512*5+512*5
#define ed_stack 800*4+600*4
#define ed_stk1 600*4


void enemy_draw(byte *src,word *trg,int shade,int scale,int maxspace,int clip)
//#pragma aux enemy_draw parm[ESI][EDI][EBX][EDX][EAX][ECX]
  {
/*  __asm
    {
    mov esi,src
    mov edi,trg
    mov ebx,shade
    mov edx,scale
    mov eax,maxspace
    mov ecx,clip

        cmp     eax,470
        jc      ed_ok1
        ret
ed_ok1: push    ebp
        sub     esp,ed_stack ;vyhrad zasobnik
        mov     ebp,esp
        push    ecx          ;uchovej ecx               +1
        push    edi          ;uchovej edi               +2
        push    ebx          ;uchovej ebx               +3
        mov     ebx,eax      ;volne misto je ted v ebx
        mov     edi,ebp      ;nastav edi na zacatek tabulek
        push    ebp          ;                          +4
        movzx   ecx,short ptr[esi]     ;precti xs
        movzx   eax,short ptr[esi+2]   ;precti ys
        imul    eax,ecx      ;xs*ys
        sub     eax,ecx      ;minus 1 radek
        xor     ebp,ebp
ed_lp1b:stosd                ;zapis hodnotu do tabulky
        add     ebp,320      ;zmensovani
ed_lp1a:cmp     ebp,edx
        jc      ed_nxt1      ;pri ebp<edx pokracuj dal
        sub     ebp,edx
        sub     eax,ecx      ;odecti radku
        jbe     ed_end1      ;pokud je to posledni tak konec1
        jmp     ed_lp1a
ed_nxt1:dec     ebx
        jnz     ed_lp1b
ed_end1:xor     eax,eax
        dec     eax
        stosd                ;tabulka pro y je hotova
        pop     ebp          ;                          +3
        lea     edi,[ebp+ed_stk1] ;nastav se na druhou tabulku
        xor     eax,eax      ;vynuluj vypoctove registry
        xor     ebx,ebx
        movzx   ecx,short ptr[esi]  ;ecx je velikost obrazku v x
ed_nxt2:stosd                ;zapis x souradnici
        add     ebx,320      ;pocitej zmenseni
ed_lp2a:cmp     ebx,edx
        jc      ed_nxt2
        sub     ebx,edx
        inc     eax
        cmp     eax,ecx      ;pokud je x>=velikosti obrazku tak konec
        jc     ed_lp2a
        xor     eax,eax
        dec     eax          ;zapis -1 na konec tabulky
        pop     ebx          ;                          +2
        stosd
        mov     eax,edi      ;konecnou pozici pro tabulku x zapis do eax
        pop     edi          ;obnov registry edi exc ebx+1
        add     ebx,esi      ;najdi adresu prekladove tabulky barev
        add     ebx,2        ;preskoc transparentni barvu
        add     esi,pic_start;presun se na zacatek obrazku
        pop     ecx          ;                          +0
        movzx   edx,cx       ;vem levy okraj
        lea     edx,[ebp+edx*4]
        lea     edx,[edx+ed_stk1];edx ukazuje na sloupce
        cmp     eax,edx      ;je-li levy okraj za platnou tabulkou tak konec
        jbe     ed_err
        shr     ecx,16       ;ecx ted obsahuje pravy okraj
ed_lp4: push    esi          ;uchovej esi               +1
        push    edi          ;uchovej edi               +2
        add     esi,[ebp]    ;spocitej spravnou hodnotu esi
        push    ecx          ;uchovej pravy okraj       +3
        push    edx          ;uchovej ukazatel na tabulku+4
ed_lp3: mov     eax,[edx]
        cmp     eax,-1       ;testuj konec tabulky
        jz      ed_end3      ;pri pozitivnim vysledku => konec radky
        movzx   eax,byte ptr[esi+eax];vem barvu
        or      al,al
        jz      ed_skp1      ;preskoc transparentni barvu
        dec     al
        jz      ed_shd       ;1=shadow
        mov     eax,[ebx+eax*2];vyzvedni hicolor
        stosw                ;zapis barvu
        jmp     ed_skp2
ed_shd: movzx   eax,short ptr[edi];vem barvu
        and     eax,0xF7DE   ;stmav
        shr     eax,1
        stosw               ;zapis
        jmp     ed_skp2     ;skok na konec
ed_skp1:add     edi,2       ;preskoc bod
ed_skp2:add     edx,4       ;dalsi pozice
        dec     ecx         ;dokud nedosahneme praveho okraje obrazku
        jnz     ed_lp3
ed_end3:pop     edx         ;obnov vse ulozene
        pop     ecx
        pop     edi
        pop     esi         ;                           +0
        add     ebp,4       ;dalsi y souradnice
        sub     edi,scr_linelen ;dalsi radka
        cmp     dword ptr [ebp],-1 ;test na konec tabulky
        jnz     ed_lp4      ;jinak pokracuj
ed_err: add     esp,ed_stack;vymaz tabulku
        pop     ebp         ;obnov ebp
     }
*/


	// TODO: needs testing, testing and more testing
	if (maxspace >= 470) {
		return;
	}

	int i, j, pos, tmp, array1[600], array2[800];
	word *pal = (word*)src;
	byte idx;

	for (i = 0, pos = 1, tmp = 0; i < maxspace && pos < pal[1]; i++) {
		array1[i] = pal[0] * (pal[1] - pos);
		tmp += 320;
		pos += tmp / scale;
		tmp %= scale;
	}
	array1[i] = -1;

	for (i = 0, pos = 0, tmp = 0; pos < pal[0]; i++) {
		array2[i] = pos;
		tmp += 320;
		pos += tmp / scale;
		tmp %= scale;
	}
	array2[i] = -1;

	if (i <= clip % 0x10000) {
		return;
	}

	pal = (word*)(src + shade);
	src += pic_start;

	for (i = 0; array1[i] != -1; i++) {
		for (j = 0; j < (clip / 0x10000); j++) {
			if (array2[j + clip % 0x10000] == -1) {
				break;
			}

			idx = src[array1[i] + array2[j + clip % 0x10000]];
			if (!idx) {
				continue;
			}

			if (idx == 1) {
//				trg[j-i*Screen_GetXSize()] = (trg[j-i*Screen_GetXSize()] & 0xf7de) >> 1;
				trg[j-i*Screen_GetXSize()] = Screen_ColorAvg(trg[j-i*Screen_GetXSize()], 0);
			} else {
				trg[j-i*Screen_GetXSize()] = pal[idx];
			}
		}
	}
  }
void enemy_draw_transp(byte *src,word *trg,palette_t shade,int scale,int maxspace,int clip)
//#pragma aux enemy_draw_transp parm[ESI][EDI][EBX][EDX][EAX][ECX]
  {
/*  __asm
    {
    mov esi,src
    mov edi,trg
    mov ebx,shade
    mov edx,scale
    mov eax,maxspace
    mov ecx,clip

        cmp     eax,470
        jc      et_ok1
        ret
et_ok1: push    ebp
        sub     esp,ed_stack ;vyhrad zasobnik
        mov     ebp,esp
        push    ecx          ;uchovej ecx               +1
        push    edi          ;uchovej edi               +2
        push    ebx          ;uchovej ebx               +3
        mov     ebx,eax      ;volne misto je ted v ebx
        mov     edi,ebp      ;nastav edi na zacatek tabulek
        push    ebp          ;                          +4
        movzx   ecx,short ptr[esi]     ;precti xs
        movzx   eax,short ptr[esi+2]   ;precti ys
        imul    eax,ecx      ;xs*ys
        sub     eax,ecx      ;minus 1 radek
        xor     ebp,ebp
et_lp1b:stosd                ;zapis hodnotu do tabulky
        add     ebp,320      ;zmensovani
et_lp1a:cmp     ebp,edx
        jc      et_nxt1      ;pri ebp<edx pokracuj dal
        sub     ebp,edx
        sub     eax,ecx      ;odecti radku
        jbe     et_end1      ;pokud je to posledni tak konec1
        jmp     et_lp1a
et_nxt1:dec     ebx
        jnz     et_lp1b
et_end1:xor     eax,eax
        dec     eax
        stosd                ;tabulka pro y je hotova
        pop     ebp          ;                          +3
        lea     edi,[ebp+ed_stk1] ;nastav se na druhou tabulku
        xor     eax,eax      ;vynuluj vypoctove registry
        xor     ebx,ebx
        movzx   ecx,short ptr[esi]  ;ecx je velikost obrazku v x
et_nxt2:stosd                ;zapis x souradnici
        add     ebx,320      ;pocitej zmenseni
et_lp2a:cmp     ebx,edx
        jc      et_nxt2
        sub     ebx,edx
        inc     eax
        cmp     eax,ecx      ;pokud je x>=velikosti obrazku tak konec
        jc     et_lp2a
        xor     eax,eax
        dec     eax          ;zapis -1 na konec tabulky
        pop     ebx          ;                          +2
        stosd
        mov     eax,edi      ;konecnou pozici pro tabulku x zapis do eax
        pop     edi          ;obnov registry edi exc ebx+1
        cmp     byte ptr [esi+5],2      ;obrazek bez palety?
        jz      et_pl1
        add     esi,pic_start;presun se na zacatek obrazku (za paletu)
        jmp     et_pl2
et_pl1: add     esi,6        ;(obrazek bez palety, presun se za hlavicku)
et_pl2: pop     ecx          ;                          +0
        movzx   edx,cx       ;vem levy okraj
        lea     edx,[ebp+edx*4]
        lea     edx,[edx+ed_stk1];edx ukazuje na sloupce
        cmp     eax,edx      ;je-li levy okraj za platnou tabulkou tak konec
        jbe     et_err
        shr     ecx,16       ;ecx ted obsahuje pravy okraj
et_lp4: push    esi          ;uchovej esi               +1
        push    edi          ;uchovej edi               +2
        add     esi,[ebp]    ;spocitej spravnou hodnotu esi
        push    ecx          ;uchovej pravy okraj       +3
        push    edx          ;uchovej ukazatel na tabulku+4
et_lp3: mov     eax,[edx]
        cmp     eax,-1       ;testuj konec tabulky
        jz      et_end3      ;pri pozitivnim vysledku => konec radky
        movzx   eax,byte ptr[esi+eax];vem barvu
        test    al,80h
        jnz     et_shd
        or      al,al
        jz      et_skp1      ;preskoc transparentni barvu
        mov     eax,[ebx+eax*2];vyzvedni hicolor
        stosw                ;zapis barvu
        jmp     et_skp2
et_shd: and     short ptr[edi],0xF7DE ;1111 0111 1101 1110
        mov     eax,[ebx+eax*2];vyzvedni hicolor
        and     eax,0xF7DE  ;stmav
        add     ax,short ptr[edi]
        rcr     ax,1
        stosw               ;zapis
        jmp     et_skp2     ;skok na konec
et_skp1:add     edi,2       ;preskoc bod
et_skp2:add     edx,4       ;dalsi pozice
        dec     ecx         ;dokud nedosahneme praveho okraje obrazku
        jnz     et_lp3
et_end3:pop     edx         ;obnov vse ulozene
        pop     ecx
        pop     edi
        pop     esi         ;                           +0
        add     ebp,4       ;dalsi y souradnice
        sub     edi,scr_linelen    ;dalsi radka
        cmp     dword ptr [ebp],-1 ;test na konec tabulky
        jnz     et_lp4      ;jinak pokracuj
et_err: add     esp,ed_stack;vymaz tabulku
        pop     ebp         ;obnov ebp
     }
*/

	// TODO: needs testing, testing and more testing
	if (maxspace >= 470) {
		return;
	}

	int i, j, pos, tmp, array1[600], array2[800];
	word *header = (word*)src;
	byte idx;

	for (i = 0, pos = 1, tmp = 0; i < maxspace && pos < header[1]; i++) {
		array1[i] = header[0] * (header[1] - pos);
		tmp += 320;
		pos += tmp / scale;
		tmp %= scale;
	}
	array1[i] = -1;

	for (i = 0, pos = 0, tmp = 0; pos < header[0]; i++) {
		array2[i] = pos;
		tmp += 320;
		pos += tmp / scale;
		tmp %= scale;
	}
	array2[i] = -1;

	if (i <= clip % 0x10000) {
		return;
	}

	if (src[5] == 2) {
		src += 6;
	} else {
		src += pic_start;
	}

	for (i = 0; array1[i] != -1; i++) {
		for (j = 0; j < (clip / 0x10000); j++) {
			if (array2[j + clip % 0x10000] == -1) {
				break;
			}

			idx = src[array1[i] + array2[j + clip % 0x10000]];
			if (!idx) {
				continue;
			}

			if (idx & 0x80) {
//				trg[j-i*Screen_GetXSize()] = ((trg[j-i*Screen_GetXSize()] & 0xf7de) + (shade[idx] & 0xf7de)) >> 1;
				trg[j-i*Screen_GetXSize()] = Screen_ColorAvg(trg[j-i*Screen_GetXSize()], shade[idx]);
			} else {
				trg[j-i*Screen_GetXSize()] = shade[idx];
			}
		}
	}
  }

void enemy_draw_mirror_transp(byte *src,word *trg,palette_t shade,int scale,int maxspace,int clip)
//#pragma aux enemy_draw_mirror_transp parm[ESI][EDI][EBX][EDX][EAX][ECX]
  {
/*  __asm
    {
    mov esi,src
    mov edi,trg
    mov ebx,shade
    mov edx,scale
    mov eax,maxspace
    mov ecx,clip

       push    ebp
        sub     esp,ed_stack ;vyhrad zasobnik
        mov     ebp,esp
        push    ecx          ;uchovej ecx               +1
        push    edi          ;uchovej edi               +2
        push    ebx          ;uchovej ebx               +3
        mov     ebx,eax      ;volne misto je ted v ebx
        mov     edi,ebp      ;nastav edi na zacatek tabulek
        push    ebp          ;                          +4
        movzx   ecx,short ptr[esi]     ;precti xs
        movzx   eax,short ptr[esi+2]   ;precti ys
        imul    eax,ecx      ;xs*ys
        sub     eax,ecx      ;minus 1 radek
        xor     ebp,ebp
etmlp1b:stosd                ;zapis hodnotu do tabulky
        add     ebp,320      ;zmensovani
etmlp1a:cmp     ebp,edx
        jc      etmnxt1      ;pri ebp<edx pokracuj dal
        sub     ebp,edx
        sub     eax,ecx      ;odecti radku
        jbe     etmend1      ;pokud je to posledni tak konec1
        jmp     etmlp1a
etmnxt1:dec     ebx
        jnz     etmlp1b
etmend1:xor     eax,eax
        dec     eax
        stosd                ;tabulka pro y je hotova
        pop     ebp          ;                          +3
        lea     edi,[ebp+ed_stk1] ;nastav se na druhou tabulku
        xor     ebx,ebx
        movzx   ecx,short ptr[esi]  ;ecx je velikost obrazku v x
        mov     eax,ecx      ;eax je velikost obrazku v x
        dec     eax
etmnxt2:stosd                ;zapis x souradnici
        add     ebx,320      ;pocitej zmenseni
etmlp2a:cmp     ebx,edx
        jc      etmnxt2
        sub     ebx,edx
        sub     eax,1
        jnc     etmlp2a
        xor     eax,eax
        dec     eax          ;zapis -1 na konec tabulky
        pop     ebx          ;                          +2
        stosd
        mov     eax,edi      ;konecnou pozici pro tabulku x zapis do eax
        pop     edi          ;obnov registry edi exc ebx+1
        cmp     byte ptr [esi+5],2      ;obrazek bez palety?
        jz      etmpl1
        add     esi,pic_start;presun se na zacatek obrazku (za paletu)
        jmp     etmpl2
etmpl1: add     esi,6        ;(obrazek bez palety, presun se za hlavicku)
etmpl2: pop     ecx          ;                          +0
        movzx   edx,cx       ;vem levy okraj
        lea     edx,[ebp+edx*4]
        lea     edx,[edx+ed_stk1];edx ukazuje na sloupce
        cmp     eax,edx      ;je-li levy okraj za platnou tabulkou tak konec
        jbe     etmerr
        shr     ecx,16       ;ecx ted obsahuje pravy okraj
etmlp4: push    esi          ;uchovej esi               +1
        push    edi          ;uchovej edi               +2
        add     esi,[ebp]    ;spocitej spravnou hodnotu esi
        push    ecx          ;uchovej pravy okraj       +3
        push    edx          ;uchovej ukazatel na tabulku+4
etmlp3: mov     eax,[edx]
        cmp     eax,-1       ;testuj konec tabulky
        jz      etmend3      ;pri pozitivnim vysledku => konec radky
        movzx   eax,byte ptr[esi+eax];vem barvu
        test    al,80h
        jnz     etmshd
        or      al,al
        jz      etmskp1      ;preskoc transparentni barvu
        mov     eax,[ebx+eax*2];vyzvedni hicolor
        stosw                ;zapis barvu
        jmp     etmskp2
etmshd: and     short ptr[edi],0xF7DE
        mov     eax,[ebx+eax*2];vyzvedni hicolor
        and     eax,0xF7DE   ;stmav
        add     ax,short ptr[edi]
        rcr     ax,1
        stosw               ;zapis
        jmp     etmskp2     ;skok na konec
etmskp1:add     edi,2       ;preskoc bod
etmskp2:add     edx,4       ;dalsi pozice
        dec     ecx         ;dokud nedosahneme praveho okraje obrazku
        jnz     etmlp3
etmend3:pop     edx         ;obnov vse ulozene
        pop     ecx
        pop     edi
        pop     esi         ;                           +0
        add     ebp,4       ;dalsi y souradnice
        sub     edi,scr_linelen    ;dalsi radka
        cmp     dword ptr [ebp],-1 ;test na konec tabulky
        jnz     etmlp4      ;jinak pokracuj
etmerr: add     esp,ed_stack;vymaz tabulku
        pop     ebp         ;obnov ebp
     }
*/

	// TODO: needs testing, testing and more testing
	int i, j, pos, tmp, array1[600], array2[800];
	word *header = (word*)src;
	byte idx;

	for (i = 0, pos = 1, tmp = 0; i < maxspace && pos < header[1]; i++) {
		array1[i] = header[0] * (header[1] - pos);
		tmp += 320;
		pos += tmp / scale;
		tmp %= scale;
	}
	array1[i] = -1;

	// this loop is the only difference from enemy_draw_transp() - merge?
	for (i = 0, pos = header[0] - 1, tmp = 0; pos >= 0; i++) {
		array2[i] = pos;
		tmp += 320;
		pos -= tmp / scale;
		tmp %= scale;
	}
	array2[i] = -1;

	if (i <= clip % 0x10000) {
		return;
	}

	if (src[5] == 2) {
		src += 6;
	} else {
		src += pic_start;
	}

	for (i = 0; array1[i] != -1; i++) {
		for (j = 0; j < (clip / 0x10000); j++) {
			if (array2[j + clip % 0x10000] == -1) {
				break;
			}

			idx = src[array1[i] + array2[j + clip % 0x10000]];
			if (!idx) {
				continue;
			}

			if (idx & 0x80) {
//				trg[j-i*Screen_GetXSize()] = ((trg[j-i*Screen_GetXSize()] & 0xf7de) + (shade[idx] & 0xf7de)) >> 1;
				trg[j-i*Screen_GetXSize()] = Screen_ColorAvg(trg[j-i*Screen_GetXSize()], shade[idx]);
			} else {
				trg[j-i*Screen_GetXSize()] = shade[idx];
			}
		}
	}
  }
void enemy_draw_mirror(byte *src,word *trg,int shade,int scale,int maxspace,int clip)
//#pragma aux enemy_draw_mirror parm[ESI][EDI][EBX][EDX][EAX][ECX]
//clip je v poradi vpravo - vlevo (HiLo)
  {
/*  __asm
    {
    mov esi,src
    mov edi,trg
    mov ebx,shade
    mov edx,scale
    mov eax,maxspace
    mov ecx,clip

        push    ebp
        sub     esp,ed_stack ;vyhrad zasobnik
        mov     ebp,esp
        push    ecx          ;uchovej ecx               +1
        push    edi          ;uchovej edi               +2
        push    ebx          ;uchovej ebx               +3
        mov     ebx,eax      ;volne misto je ted v ebx
        mov     edi,ebp      ;nastav edi na zacatek tabulek
        push    ebp          ;                          +4
        movzx   ecx,short ptr[esi]     ;precti xs
        movzx   eax,short ptr[esi+2]   ;precti ys
        imul    eax,ecx      ;xs*ys
        sub     eax,ecx      ;minus 1 radek
        xor     ebp,ebp
edmlp1b:stosd                ;zapis hodnotu do tabulky
        add     ebp,320      ;zmensovani
edmlp1a:cmp     ebp,edx
        jc      edmnxt1      ;pri ebp<edx pokracuj dal
        sub     ebp,edx
        sub     eax,ecx      ;odecti radku
        jbe     edmend1      ;pokud je to posledni tak konec1
        jmp     edmlp1a
edmnxt1:dec     ebx
        jnz     edmlp1b
edmend1:xor     eax,eax
        dec     eax
        stosd                ;tabulka pro y je hotova
        pop     ebp          ;                          +3
        lea     edi,[ebp+ed_stk1] ;nastav se na druhou tabulku
        xor     ebx,ebx
        movzx   ecx,short ptr[esi]  ;ecx je velikost obrazku v x
        mov     eax,ecx      ;eax je velikost obrazku v x
        dec     eax
edmnxt2:stosd                ;zapis x souradnici
        add     ebx,320      ;pocitej zmenseni
edmlp2a:cmp     ebx,edx
        jc      edmnxt2
        sub     ebx,edx
        sub     eax,1
        jnc     edmlp2a
        xor     eax,eax
        dec     eax          ;zapis -1 na konec tabulky
        pop     ebx          ;                          +2
        stosd
        mov     eax,edi      ;konecnou pozici pro tabulku x zapis do eax
        pop     edi          ;obnov registry edi exc ebx+1
        add     ebx,esi      ;najdi adresu prekladove tabulky barev
        add     ebx,2        ;preskoc transparentni barvu
        add     esi,pic_start;presun se na zacatek obrazku
        pop     ecx          ;                          +0
        movzx   edx,cx       ;vem levy okraj
        lea     edx,[ebp+edx*4]
        lea     edx,[edx+ed_stk1];edx ukazuje na sloupce
        cmp     eax,edx      ;je-li levy okraj za platnou tabulkou tak konec
        jbe     edmerr
        shr     ecx,16       ;ecx ted obsahuje pravy okraj
edmlp4: push    esi          ;uchovej esi               +1
        push    edi          ;uchovej edi               +2
        add     esi,[ebp]    ;spocitej spravnou hodnotu esi
        push    ecx          ;uchovej pravy okraj       +3
        push    edx          ;uchovej ukazatel na tabulku+4
edmlp3: mov     eax,[edx]
        cmp     eax,-1       ;testuj konec tabulky
        jz      edmend3      ;pri pozitivnim vysledku => konec radky
        movzx   eax,byte ptr[esi+eax];vem barvu
        or      al,al
        jz      edmskp1      ;preskoc transparentni barvu
        dec     al
        jz      edmshd       ;1=shadow
        mov     eax,[ebx+eax*2];vyzvedni hicolor
        stosw                ;zapis barvu
        jmp     edmskp2
edmshd: movzx   eax,short ptr[edi];vem barvu
        and     eax,0xF7DE   ;stmav
        shr     eax,1
        stosw               ;zapis
        jmp     edmskp2     ;skok na konec
edmskp1:add     edi,2       ;preskoc bod
edmskp2:add     edx,4       ;dalsi pozice
        dec     ecx         ;dokud nedosahneme praveho okraje obrazku
        jnz     edmlp3
edmend3:pop     edx         ;obnov vse ulozene
        pop     ecx
        pop     edi
        pop     esi         ;                           +0
        add     ebp,4       ;dalsi y souradnice
        sub     edi,scr_linelen  ;dalsi radka
        cmp     dword ptr [ebp],-1 ;test na konec tabulky
        jnz     edmlp4      ;jinak pokracuj
edmerr: add     esp,ed_stack;vymaz tabulku
        pop     ebp         ;obnov ebp
    }
*/

	// TODO: needs testing, testing and more testing
	int i, j, pos, tmp, array1[600], array2[800];
	word *pal = (word*)src;
	byte idx;

	for (i = 0, pos = 1, tmp = 0; i < maxspace && pos < pal[1]; i++) {
		array1[i] = pal[0] * (pal[1] - pos);
		tmp += 320;
		pos += tmp / scale;
		tmp %= scale;
	}
	array1[i] = -1;

	// this loop is the only difference from enemy_draw() - merge?
	for (i = 0, pos = pal[0] - 1, tmp = 0; pos >= 0; i++) {
		array2[i] = pos;
		tmp += 320;
		pos -= tmp / scale;
		tmp %= scale;
	}
	array2[i] = -1;

	if (i <= clip % 0x10000) {
		return;
	}

	pal = (word*)(src + shade);
	src += pic_start;

	for (i = 0; array1[i] != -1; i++) {
		for (j = 0; j < (clip / 0x10000); j++) {
			if (array2[j + clip % 0x10000] == -1) {
				break;
			}

			idx = src[array1[i] + array2[j + clip % 0x10000]];
			if (!idx) {
				continue;
			}

			if (idx == 1) {
//				trg[j-i*Screen_GetXSize()] = (trg[j-i*Screen_GetXSize()] & 0xf7de) >> 1;
				trg[j-i*Screen_GetXSize()] = Screen_ColorAvg(trg[j-i*Screen_GetXSize()], 0);
			} else {
				trg[j-i*Screen_GetXSize()] = pal[idx];
			}
		}
	}
  }
