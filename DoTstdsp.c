//////////////////////////////////////////////////////////////////////////////
// DoTstdsp.c
// Module descr:        MTmux common operations.
// (c) 2005 Nevzorov avn@tercom.ru
//////////////////////////////////////////////////////////////////////////////

#include <dos.h>
#include <stdlib.h>
#include "MODULE\mtdsp.h"
#include ".\DoDbg.h"
#include "DoTstdsp.h"

//////////////////////////////////////////////////////////////////////////////
//              DoMTmux(...) procedure
//----------------------------------------------------------------------------
// This procedure reads parameters from command line string and calls
// MT90826 testing procedure.
//----------------------------------------------------------------------------
// Parameters:
//      char *STR = pointer to the command line string
//              ("[cycles]").
// Return value:
//      none
//////////////////////////////////////////////////////////////////////////////
void far DoDSP(char *STR)
{
         long cycles,addres;
         int i;

         addres=ReadHexDef(STR, &CurCmdStr,0);
         cycles=1;

printf("CTDSP>");


         if (cycles==0)
              while ( 1 )
              {
                  MTtestDSP(addres,0xaabb);
                  if(kbhit()) { getch(); printf("\r\n"); break; }
              }
         else
            for (i=0;i<cycles;i++){
               MTtestDSP(addres,0xaabb);
               if(kbhit()) { getch(); printf("\r\n"); break; }
             }

printf("  \r\n");


};

void    far     iomwrite(unsigned int ADDR,unsigned int VALUE)
{
  outport(iom_addr_dsp, ADDR);
  outport(iom_data_dsp, VALUE);

}
unsigned int iomread(unsigned int ADDR)
{
unsigned int val;
  outport(iom_addr_dsp, ADDR);
  val=inport(iom_data_dsp);
  return val;
}

unsigned int imread(unsigned int ADDR)
{
  unsigned int val;
  asm{cli};

  outport(im_addr_dsp, ADDR);
  val=inport(im_data_dsp);
  asm{sti};

  return val;
}

void far writecode(unsigned int ADDR,unsigned int VAL)
{
// write code fro code
  asm{cli};
  outport(im_addr_dsp, ADDR);
  outport(im_data_dsp, VAL);
  asm{sti};

}


void    far     dmadata(unsigned int VAL)
{
  outport(im_data_dsp, VAL);
}
// считка данных  с DSP в режиме DMA
unsigned int dmadataread(void)
{
  unsigned int val;   //=inport(im_data_dsp);
  asm{
       mov dx,5000H
       in  ax,dx
       mov val,ax
  }

  return val;
}


void far READDSP(char *STR)
{

 int i,j;
 unsigned char far *ptr;
 unsigned int temp1,temp2;
 unsigned long port,readport;
 char *endptr ;

 i=0;

 port=ReadHex(STR, &endptr);
 if (port==-1) port=0x100;

 printf("== %4X ++== \r\n",port);

/*
  iomwrite(0x3a02,0x0);
  iomwrite(0x3804,0x0);

 if (port>0x7fff)
   {
     outport(0x3004,0x40);
     port=port*2;
     printf("== %4X == \r\n",port);
   }
*/


 printf(" ===================================================== \r\n");


 while (i<0x8)
        {
         printf("%4X   ",port);
         for (j=0; j<8; j++)
            {
               readport=(port+j)*2;
           asm{
                cli
                mov dx,im_addr_dsp
                mov ax,readport
                out dx,ax
                mov dx,im_data_dsp
                in  ax,dx
                mov temp1,ax
                sti
           }

                printf("%04X ",temp1);

             }

         printf("\r\n");
         port=port+0x8;
         i++;
        }


printf("  \r\n");


};

void far READDSPC(char *STR)
{

 int i,j;
 unsigned char far *ptr;
 unsigned int temp1,temp2;
 unsigned long port,readport;
 char *endptr ;

 i=0;

 port=ReadHex(STR, &endptr);
 if (port==-1) port=0x100;


 printf("== %4X ++== \r\n",port);

  iomwrite(0x3a02,0x0);
  iomwrite(0x3804,0x0);

 printf(" ===================================================== \r\n");


 while (i<0x8)
        {
         printf("%4X ",port);
         for (j=0; j<8; j++)
            {
               readport=(port+j)*4;
           asm{
                cli
                mov dx,im_addr_dsp
                mov ax,readport
                out dx,ax
                mov dx,im_data_dsp
                in  ax,dx
                mov temp1,ax
                in  ax,dx
                mov temp2,ax
                sti
           }

                printf("%04X %04X ",temp1,temp2);

             }

         printf("\r\n");
         port=port+0x8;
         i++;
        }


printf("  \r\n");


};

void DSPSTOP(void )
{

     asm{
         cli
         mov     dx, 3004H    //   сброс ADSP
         in      al, dx
         or      al, 080h
         out     dx, al
         in      al,dx
         in      al,dx
         in      al,dx
         in      al,dx
         in      al,dx
         in      al,dx
         in      al,dx
         in      al,dx
         in      al,dx
         in      al,dx
         in      al,dx
         in      al,dx
         in      al,dx
         in      al,dx
         in      al,dx
         in      al,dx
         in      al,dx
         in      al,dx
     }
//        delay(100);
    asm{
         mov     dx, 3004H    //   сброс ADSP
         mov     al, 0h
         out     dx, al
     }
// переход в 2 байтный режим
     asm{
         mov     dx,5c00h         //iom_addr_dsp
         mov     ax,3802h
         out     dx,ax
         mov     dx,5800h        //iom_data_dsp
         mov     al,01h
         out     dx,al
         mov     al,0fh
         out     dx,al
         mov     dx,5c00h         //iom_addr_dsp
         mov     ax,3804h
         out     dx,ax
         mov     dx,5800h        //iom_data_dsp
         mov     ax,0h
         out     dx,ax
         mov     dx,3004h        //iom_data_dsp
         in      al,dx
         or      al,40h
         out     dx,al
         sti
         }

//  iomwrite(0x408,0x83c4);

}

unsigned char ADSPDATA[]=
        {
        #include "MODULE\dt.dat"
        };
int ADSPDATASize = sizeof(ADSPDATA);

unsigned char LOAD[]=
        {
        #include "MODULE\pmu.dat"
        };
int LOADSize = sizeof(LOAD);

unsigned char ADSPCODE[]=
        {
        #include "MODULE\di.dat"
        };
int ADSPCODESize = sizeof(ADSPCODE);

//////////////////////////////////////////////////////////////////////////////
//                      LoadADSPImage(...) procedure
//----------------------------------------------------------------------------
//      Load image from PC memory into ADSP program segment.
//----------------------------------------------------------------------------
// Parameters:
//      unsigned char far *ADSPIMAGE - pointer to image
//      int SIZE - size (in bytes) of ADSPIMAGE
// Return value:
//      = 0 if ADSP PM values is equals to ADSPIMAGE;
//      < 0 if there is a difference.
//////////////////////////////////////////////////////////////////////////////

char LoadADSPDATA(unsigned char far *ADSPDATA, int ADSPDATASize )
{
  unsigned long count_dat,addr_dat;
  int val,i,j,status;


  printf("DATASIZE= %04x",ADSPDATASize);


  for (i=0; i<ADSPDATASize; i++)
         {
// считка данных о количестве передаваемых данных
           addr_dat=((unsigned int)( ADSPDATA[i+1]*256 + (unsigned int)ADSPDATA[i+0])-0x4000) & 0xFFFF;
           printf(" %04x ",addr_dat);
           if (addr_dat!=0xffff)
              {
                 count_dat=(unsigned int)( ADSPDATA[i+3]*256 + (unsigned int)ADSPDATA[i+2]);
                 printf(" %04x \r\n",count_dat);

                 asm{ cli};
                 iomwrite(0x3A02,0x10);
                 iomwrite(0x3A04,0x0);
                 iomwrite(0x3A06,addr_dat);
                 iomwrite(0x3A08,count_dat/2); //     Size/2);
                 iomwrite(0x3A02,0x17);
                 asm{ sti};
                 for (j=4;j<4+count_dat*2;j++)
                   {
                     dmadata((unsigned int)( ADSPDATA[i+j+1]*256 + (unsigned int)ADSPDATA[i+j]) );
                     j++;
                   }
                 i=i+j-1;
                 iomwrite(0x3A02,0x0);

               }
           else break;
         };
    return 1;

//printf(" data end /r/n",addr_dat);

}

void far READADSPDATA(char *STR)
{
  int val,i,j,status;
  unsigned int temp,temp1,port;
  char *endptr ;


  int dmarda[400];

 port=ReadHex(STR, &endptr);
 if (port==0xFFFF) port=0x100;

 printf("== %4X ++== \r\n",port);

  asm{ cli};
  iomwrite(0x3802,0x0f11);
  iomwrite(0x3804,0x03);
  iomwrite(0x3A02,0x10);
  iomwrite(0x3A04,0x0);
  iomwrite(0x3A06,port);
  iomwrite(0x3A08,0x80);       //   ADSPDATASize/2);
  iomwrite(0x3A02,0x19);

  asm{ sti};

  i=0;

  for (i=0; i<0x80 /*ADSPDATASize*/; i++)
     {
      asm{ cli
           mov dx,5000H
           in  ax,dx
           mov temp,ax
             };
          dmarda[i]=temp;   //         dmadataread();

     };
   asm{ sti};


  for (j=0; j<0x80; j++)
         {
          if ((j%8)==0)
             {
                printf(" \r\n");
                printf(" %04x : ",j+port);
             }
          printf(" %04x  ",dmarda[j]);

         };


//  return ADSPCodeCmp(ADSPIMAGE, SIZE);
printf(" \r\n");


}

char LoadADSPCODE(unsigned char far *ADSPCODE, int Size )
{
  int val,i,j;
  unsigned long count_cod,addr_cod,temp;

  DSPSTOP();
  printf("CODESIZE= %04x",Size);
//  iomwrite(0x3802,0x0f51);
//    iomwrite(0x3804,0x0002);

     asm{ cli};
     iomwrite(0x3A02,0x10);
     iomwrite(0x3A04,0x0);
     iomwrite(0x3A06,0x140); // addr_cod);
     iomwrite(0x3A08,0x8);            //count_cod*3/2); //     Size/2);
     iomwrite(0x3A02,0x1b);
     asm{ sti};
     for (i=4;i<17;i++)
         {

                asm{ cli};
                dmadata((unsigned int)( (unsigned int)ADSPCODE[i]*256) );
                dmadata((unsigned int)( ADSPCODE[i+1] + (unsigned int)ADSPCODE[i+2]*256) );
                asm{ sti};

                i++;
                i++;
         }





/*
  for (i=0; i<Size; i++)
         {
// считка данных о количестве передаваемых данных
           addr_cod=(unsigned int)( ADSPCODE[i+1]*256 + (unsigned int)ADSPCODE[i+0]);
           printf(" %04x ",addr_cod);

           if (addr_cod!=0xffff)
              {
                 count_cod=(unsigned int)( ADSPCODE[i+3]*256 + (unsigned int)ADSPCODE[i+2]);
                  asm{ cli};
                 iomwrite(0x3A02,0x10);
                 iomwrite(0x3A04,0x0);
                 iomwrite(0x3A06,addr_cod);
                 iomwrite(0x3A08,count_cod*2); //     Size/2);
                 iomwrite(0x3A02,0x1b);
                 asm{ sti};

                 for (j=3;j<3+count_cod*3;j++)
                   {
                    asm{ cli}
                     dmadata((unsigned int)( ADSPCODE[i+j]*256) );
                     dmadata((unsigned int)( ADSPCODE[i+j+1] + (unsigned int)ADSPCODE[i+j+2]*256) );
//                     printf(" %04x",(unsigned int)( ADSPCODE[i+j+1] + (unsigned int)ADSPCODE[i+j+2]*256));
                    asm{ sti}
                     j++;
                     j++;
//                     printf("\r\n");
//                     temp=iomread(0x3A02);
//                     printf("temp= %04X \r\n",temp);

                   }

                 i=i+j;
               }
           else break;
         };
*/


iomwrite(0x3A02,0x0);

return 1;


}


void DSPSTART(void )
{

DSPSTOP();

// загрузка кода
/*
if (LoadADSPCODE(ADSPCODE, sizeof(ADSPCODE))<0)
         puts ("\r\n ERROR LOADING ADSP PROGRAM IN CODE SEGMENT !!!\r\n");

// загрузка данных



if (LoadADSPDATA(ADSPDATA, sizeof(ADSPDATA))<0)
         puts ("\r\n ERROR LOADING ADSP PROGRAM IN CODE SEGMENT !!!\r\n");
*/
iomwrite(0x3a02,0x00);
//iomread(0x39f8);

//iomwrite(0x39f8,0x0001);


}

void far writeiom(char *STR)
{
  long cycles;
  long adrr,val;
  char *endptr ;
  char *endptr1 ;


 adrr =ReadHex(STR, &endptr);
 if (adrr<0) goto fine ;
 val =ReadHex(endptr, &endptr);
 if (val<0) goto fine;
 iomwrite(adrr,val);

goto end;
fine:
printf("Syntax ERROR");
end:


}


void far readiom(char *STR)
{
  long cycles;
  long adrr,val;
  char *endptr ;

 adrr =ReadHex(STR, &endptr);
 if (adrr<0) goto fine1 ;
 val=iomread(adrr);
printf(" %04X",val);
goto end1;
fine1:
printf("Syntax ERROR");
end1:


}


void DATADSP(void)
{
unsigned int a,b,c,d,e;
unsigned int i;

outport(0x5c00,0x3a02);
outport(0x5800,0x0);
outportb(0x3004,0x40);

for (i=0;i<10;i++)
{
   a=imread(0x8002);
   b=imread(0x4001);
   c=imread(0x8004);
//   printf("8002= %04x 4001= %04x  8004= %04x \r\n",a,b,c);
   printf("%04x  %04x  %04x \r\n",a,b,c);
delay(100);
}

}

void far testiom(char *STR)
{
int i,cycles;

         delay(100);
         printf("MEMDSP \r\n");

         cycles=ReadHexDef(STR, &CurCmdStr,0);


         if (cycles==0)
              while ( 1 )
              {
                  DATADSP();
                  if(kbhit()) { getch(); printf("\r\n"); break; }
              }
         else
            for (i=0;i<cycles;i++){
                  DATADSP();
               if(kbhit()) { getch(); printf("\r\n"); break; }
             }
        printf("  \r\n");

}


// тестирование памяти ДСП

void far TESTDATADSP(char *STR)
{
         long cycles;
         int i;

// остановить процессор
         DSPSTOP();


         printf("MEMDSP \r\n");

         cycles=ReadHexDef(STR, &CurCmdStr,0);

         if (cycles==0)
              while ( 1 )
              {
                  MemoryDATADSP();
                  if(kbhit()) { getch(); printf("\r\n"); break; }
              }
         else
            for (i=0;i<cycles;i++){
               MemoryDATADSP();
               if(kbhit()) { getch(); printf("\r\n"); break; }
             }

        printf("  \r\n");

// стартовать процессор
//        DSPSTART();

};

void far codered(char *STR)
{
         long cycles;
         int i;

// остановить процессор
         DSPSTOP();

// переход в режим передачи для данных кода
         iomwrite(0x3804,0x2);

         printf("ucode \r\n");
         delay(100);
         cycles=ReadHexDef(STR, &CurCmdStr,0);

         if (cycles==0)
              while ( 1 )
              {
                  MemoryCODEDSP(cycles);
                  if(kbhit()) { getch(); printf("\r\n"); break; }
              }
         else
            for (i=0;i<cycles;i++){
               MemoryCODEDSP(cycles);
               if(kbhit()) { getch(); printf("\r\n"); break; }
             }

        printf("  \r\n");

// стартовать процессор
//        DSPSTART();

};


void far datared(char *STR)
{
         long cycles;
         int i;

// остановить процессор
         DSPSTOP();

// переход в режим передачи для данных кода
         iomwrite(0x3804,0x0);

         printf("ucode \r\n");
         delay(100);
         cycles=ReadHexDef(STR, &CurCmdStr,0);

         if (cycles==0)
              while ( 1 )
              {
                  MemoryDATDSP(cycles);
                  if(kbhit()) { getch(); printf("\r\n"); break; }
              }
         else
            for (i=0;i<cycles;i++){
               MemoryDATDSP(cycles);
               if(kbhit()) { getch(); printf("\r\n"); break; }
             }

        printf("  \r\n");

// стартовать процессор
//        DSPSTART();

};
void write_com(int symbol)
{
int status;
status=inportb(0x2fd);

while ( (status & 0x20) == 0 )
   {
            status=inportb(0x2fd);

   }

outportb(0x2f8,symbol);

}

void far startcom(char *STR)
{
long cycles;
int i,k,temp,status,u;
int key1,key2,dlinnatwo,addrtwo;
int j,addr,addrDM,dlinna,point;

beigin:

DSPSTOP();
//outportb(0x3004,0x40);
writecode(0,0xaa);
status=imread(0);


if (status!=0xaa)
  {
//     printf("readflag1 %04X \r\n",status);
     goto beigin;
  }

// проиницировать COM2  скорость 19200, 8N1

status=inportb(0x2fd);
while (  (status & 0x20) == 0 )
     {
     status=inportb(0x2fd);
//     printf("readstat1 %04X \r\n",status);
     }

write_com(0xaa);

// получить ответ
status=inportb(0x2fd);
while (  (status & 0x1) == 0 )
     {
     status=inportb(0x2fd);
//     printf("readstat1 %04X \r\n",status);
     }
key1=inportb(0x2f8);
/*
status=inportb(0x2fd);
while (  (status & 0x1) == 0)
     {
     status=inportb(0x2fd);
     printf("readstat2 %04X \r\n",status);
     }
*/
key2=0x4B;           //inportb(0x2f8);

printf("key1= %04X key2= %04X \r\n",key1,key2);

if ((key1==0x4B) && (key2==0x4B)) printf("key bed") ;
else
   {
//   printf("status= %04X \r\n",status) ;
   goto beigin;
   }
// закачать загрузчик
for (i=0; i<LOADSize; i++)
   {
     write_com(LOAD[i]);
   }

//printf("\r\n");
//printf("CODELoad= %04X",ADSPCODESize);
//printf("\r\n");


//delay(100);

status=imread(0);

while  (status != 0xF)
     {
       status=imread(0);
//       printf("loadcom= %04X \r\n",status);
     }



// начало копирования кода
    addrDM=1;
    point=0;
    for (k=0;k<ADSPCODESize-29;k++)
       {
        addr=ADSPCODE[k+1]+ADSPCODE[k]*256;
        writecode(addrDM*2,addr);
        if (addr==0xFFFF)
           {
              k=k+3;
              addrDM=1;
              writecode(0,0);
              status=imread(0);
//              printf("===point= %04X k= %04X\r\n",addr,k);
              while ( (status & 0x2) == 0 )  status=imread(0);

           }
        else
           {
              addrDM++;
              dlinna=ADSPCODE[k+3]+ADSPCODE[k+2]*256;
//              printf("Caddr= %04X dlinna= %04X\r\n",addr,dlinna);
              writecode(addrDM*2,dlinna);
              addrDM++;
              for (j=0;j<dlinna;j++)
                   {
                   writecode(addrDM*2,ADSPCODE[k+5+j*3]+ADSPCODE[k+4+j*3]*256);
                   addrDM++;
                   writecode(addrDM*2,ADSPCODE[k+6+j*3]); //*256);
                   addrDM++;
                   }
               k=(k-1)+dlinna*3+4;
           }
       }


status=imread(0);
while (  (status & 0x2) == 0 )
     {
     status=imread(0);
     printf("readdsp %04X \r\n",status);
     }

// копирование данных

   for (k=0;k<ADSPDATASize;k++)
     {
      addr=(ADSPDATA[k+1]+ADSPDATA[k]*256);
      if (addr==0xFFFF) break;
      addr=(addr-0x8000) & 0xFFFF;
      dlinna=ADSPDATA[k+3]+ADSPDATA[k+2]*256;
//      printf("addr= %04X dlinna= %04X\r\n",addr,dlinna);
         for (j=0;j<dlinna;j++)
            {
            writecode(addr*2,ADSPDATA[k+5+j*2]+ADSPDATA[k+4+j*2]*256);
            addr++;
            }
      k=k-1+dlinna*2+4;
     }


status=imread(0);

while (  (status & 0x2) == 0 )
     {
     status=imread(0);
     printf("readdsp %04X \r\n",status);
     }

delay(400);

writecode(0,3);
/*
status=imread(0x5190);
printf("stat1 %04X \r\n",status);
delay(100);
status=imread(0x5190);
printf("stat2 %04X \r\n",status);
*/
}







