//////////////////////////////////////////////////////////////////////////////
// mtdsp.c
// Module descr:        MTdsp common operations.
// (c) 2005 Nevzorov avn@tercom.ru     TEPKOM
//////////////////////////////////////////////////////////////////////////////

#include <dos.h>
#include <stdlib.h>
#include <time.h>
#include ".\DoDbg.h"

#include ".\MODULE\mtdsp.h"




//////////////////////////////////////////////////////////////////////////////
//                     Message Mode Enable procedure
//----------------------------------------------------------------------------
//      Enables the message mode.
//----------------------------------------------------------------------------
// Parameters:
// Return value:
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//                     Message Mode Disable procedure
//----------------------------------------------------------------------------
//      Test MEMORY Commutator
//----------------------------------------------------------------------------
// Parameters:
// Return value:
//////////////////////////////////////////////////////////////////////////////
void writedspmem(int addr,int val)
{
    asm{
                mov dx,im_addr_dsp
                mov ax,addr
                out dx,ax
                mov dx,im_data_dsp
                mov ax,val
                out dx,ax
   }
}


void writedspio(int addr,int val)
{
    asm{
                mov dx,iom_addr_dsp
                mov ax,addr
                out dx,ax
                mov dx,iom_data_dsp
                mov ax,val
                out dx,ax
   }


}





void far MTtestDSP( long addres,long value )
{
// тест работы dsp

   int i;
   int temp2,temp1,temp3,temp4;
   int error;


   error=0;
   temp1=value;

// сброс АДСП

           asm{
                mov dx,3004h
                mov al,80h
                out dx,al
                in  al,dx
                in  al,dx
                in  al,dx
                in  al,dx
                in  al,dx
                in  al,dx
                in  al,dx
                in  al,dx
                in  al,dx
                in  al,dx
                mov al,0h
                out dx,al
               }


// настройка ДСП
           asm{
                cli
                mov dx,5c00h
                mov ax,3802h
                out dx,ax
                mov dx,5800h
                mov al,01h
                out dx,al
                mov dx,5800h
                mov al,0fh
                out dx,al
               }

           writedspio(0x3a02,0x0010);
           writedspio(0x3a04,0x0000);
           writedspio(0x3a06,0x8000);
           writedspio(0x3a08,0x0005);
           writedspio(0x3a02,0x0017);
           writedspmem(0x8000,0x0017);
           writedspmem(0x8002,0x0061);
           writedspmem(0x8004,0x0064);
           writedspmem(0x8006,0x0000);
           writedspmem(0x8008,0x8000);
//           writedspio(0x39a8,0x0001);
                asm{sti};
/*
// открытие семафора
                mov dx,5c00h
                mov ax,1c01h
                out dx,ax
                mov dx,5800h
                mov ax,0h
                out dx,ax

// запись данных в порт ДСП

                mov dx,5c00h
                mov ax,1c01h
                out dx,ax
                mov dx,5800h
                mov ax,0h
                out dx,ax
// читаем IO port
                mov dx,5c00h
                mov ax,1c02h
                out dx,ax
                mov dx,5800h
                in  ax,dx
                mov temp2,ax




// записать данные по адрессу 4000h
                mov dx,5400h
                mov ax,addres
                out dx,ax
                mov dx,5000h
                mov ax,temp1
                out dx,ax
// считать данные по адрессу 4000h
                mov dx,5400h
                mov ax,addres
                out dx,ax
                mov dx,5000h
                in  ax,dx
                mov temp2,ax


*/





//               printf("  %4X %4X \r\n",temp1,temp2);

return(1);


}




void far MemoryDATADSP(void)
{

 unsigned char far *ptr;
 unsigned int data1,data2,addr,data;
 char *endptr ;
 unsigned int data_wr[4096],data_rd[4096];
 int i,j,m,k,error;
 int temp,adrr,testr,tempaddr;


   error=0;
   tempaddr=rand()%0x7FFF;
// заведение данных для тестирования памяти в DSP
      if  (DEBUG_LEVEL==1)
                printf(" =============================== WRITE DSP\r\n");

       for(i=0; i<128; i++){
           data_wr[i]=rand()%0xffFF;
           data_rd[i]=0x01;
           addr=2*(i+tempaddr);
           data=data_wr[i];
           if  (DEBUG_LEVEL==1)
             {
                if ((i%8)==0)
                   {
                      printf("\r\n");
                      printf(" %4X : ",i);
                   }
                printf(" %4X",data_wr[i]);
             }

           asm{
             pushf
             cli
             mov  ax, addr
             mov  dx, 5400h
             out  dx, ax
             mov  ax, data
             mov  dx, 5000h
             out  dx, ax
             popf
           }
       }
// чтение данных для тестирования памяти из DSP
       for(k=0; k<128; k++){
           addr=2*(k+tempaddr);
           asm{
             pushf
             cli
             mov  ax, addr
             mov  dx, 5400h
             out  dx, ax
             mov  dx, 5000h
             in   ax, dx
             mov  data1,ax
             popf
           }
           data_rd[k]=data1;
       }

      if  (DEBUG_LEVEL==1)
           {
                printf(" \r\n");
                printf(" ===============================READDSP \r\n");
           }

        for( j=0; j <128; j++){
             if  (DEBUG_LEVEL==1)
             {
                if (((j)%8)==0)
                    {
                      printf("\r\n");
                      printf(" %4X : ",j);
                     }
                printf(" %4X",data_rd[j]);
             }
             else{
               if (data_rd[j]== data_wr[j]) ;
               else error++;
             };

        };

     if  (DEBUG_LEVEL==0)

         if (error>0) printf("-");
         else printf("+");


};

void far MemoryCODEDSP(int cycle)
{

 unsigned char far *ptr;
 unsigned int data1,data2,addr,data;
 char *endptr ;
 unsigned int data_wr[4096],data_rd[4096];
 int i,j,m,k,error;
 int temp,adrr,testr;


   error=0;

// заведение данных для тестирования памяти в DSP
      if  (cycle==1)
                printf(" =============================== WRITE DSP\r\n");
                delay(100);
       for(i=0; i<124; i++){
           data_wr[i]=rand()%0xffFF;
           data_wr[i+1]=rand()%0xffFF;
           data_rd[i]=0x01;
           addr=4*i;
           data1=data_wr[i];
           data2=data_wr[i+1];
           if  (cycle==1)
             {
                if ((i%8)==0)
                   {
                      printf("\r\n");
                      printf(" %4X : ",i);
                   }
                printf(" %4X %4X",data_wr[i],data_wr[i+1]);
             }

           asm{
             pushf
             cli
             mov  ax, addr
             mov  dx, 5400h
             out  dx, ax
             mov  ax, data1
             mov  dx, 5000h
             out  dx, ax
             mov  ax, data2
             out  dx, ax
             popf
           }
          i++;
       }
// чтение данных для тестирования памяти из DSP
       for(k=0; k<124; k++){
           addr=k*4;
           asm{
             pushf
             cli
             mov  ax, addr
             mov  dx, 5400h
             out  dx, ax
             mov  dx, 5000h
             in   ax, dx
             mov  data1,ax
             in   ax, dx
             mov  data2,ax

             popf
           }
           data_rd[k]=data1;
           data_rd[k+1]=data2;
       }

      if  (cycle==1)
           {
                printf(" \r\n");
                printf(" ===============================READDSP \r\n");
           }

        for( j=0; j <128; j++){
             if  (cycle==1)
             {
                if (((j)%8)==0)
                    {
                      printf("\r\n");
                      printf(" %4X : ",j);
                     }
                printf(" %4X",data_rd[j]);
             }
             else{
               if (data_rd[j]== data_wr[j]) ;
               else error++;
             };

        };

     if  (cycle!=1)

         if (error>0) printf("-");
         else printf("+");


};
void far MemoryDATDSP(int cycle)
{

 unsigned char far *ptr;
 unsigned int data1,data2,addr,data;
 char *endptr ;
 unsigned int data_wr[4096],data_rd[4096];
 int i,j,m,k,error;
 int temp,adrr,testr;


   error=0;

// заведение данных для тестирования памяти в DSP
      if  (cycle==1)
                printf(" =============================== WRITE DSP\r\n");
                delay(100);
       for(i=0; i<128; i++){
           data_wr[i]=rand()%0xffFF;
           data_rd[i]=0x01;
           addr=0x800+2*i;
           data1=data_wr[i];
           if  (cycle==1)
             {
                if ((i%8)==0)
                   {
                      printf("\r\n");
                      printf(" %4X: ",i);
                   }
                printf(" %4X ",data_wr[i]);
             }

           asm{
             pushf
             cli
             mov  ax, addr
             mov  dx, 5400h
             out  dx, ax
             mov  ax, data1
             mov  dx, 5000h
             out  dx, ax
             popf
           }
       }
// чтение данных для тестирования памяти из DSP
       for(k=0; k<128; k++){
           addr=0x800+k*2;
           asm{
             pushf
             cli
             mov  ax, addr
             mov  dx, 5400h
             out  dx, ax
             mov  dx, 5000h
             in   ax, dx
             mov  data1,ax

             popf
           }
           data_rd[k]=data1;
       }

      if  (cycle==1)
           {
                printf(" \r\n");
                printf(" ===============================READDSP \r\n");
           }

        for( j=0; j <128; j++){
             if  (cycle==1)
             {
                if (((j)%8)==0)
                    {
                      printf("\r\n");
                      printf(" %4X : ",j);
                     }
                printf(" %4X",data_rd[j]);
             }
             else{
               if (data_rd[j]== data_wr[j]) ;
               else error++;
             };

        };

     if  (cycle!=1)

         if (error>0) printf("-");
         else printf("+");


};







