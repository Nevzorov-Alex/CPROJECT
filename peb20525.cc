//***************************************************
//**** Copyright (C)  TEPKOM.
//****     Written by Александр Невзоров avn@tepkom.ru
//****                Serge Koriakin (ksnk@bigfoot.com)
//****
//**** This file is part of BETA-OS.
//****
//****  драйвер hdlc контроллера на peb20525 для i386
//****  передача идет практически непрерывно.
//****   Каждый пакет снабжается номером,
//****   Нарушение нумерации влечет сигнал - "Ошибка"
//***************************************************

//******** 1/2-th a number of output buffers
#define OBUF_NUMB   6

//******** if you hate a tracing
#define NO_ANY_TRACE 1

// if you like to use slave interrupt controller
#define SLAVE_INTR

//***********************************************
//#define IO_BETA_TIMEOUT  5
#define IO_BETA_TIMEOUT  30
//***********************************************

//******** if you like to compute some timing
#define ck4_Right  1
#define ck4_Slave  2
#define ck4_ChanA  4
#define ck4_ChanB  8
#define ck4s_Work  3

//******** so let's go!!!!!!!

#include "betatps.h"
#include "betafunc.h"
#include "netdrv\netdrv.h"
#include "os_ext.h"
#include "frames\gls386.h"
#include "netdrv\io_ring.h"

#include "usproc.h"

// команда - послать последний кусок
#define CMD_TRANSMIT_END           NXF+NXME
#define IMASK_VALUE                0xd132 // 0xf832
//#define IMASK_VALUE                0x8030
                               //    0xc832
// tin csc ~xmr xpr ~alls ~xdu ~suex 1 : ~rdo ~rfo pce rsc ~rpf ~rme rfs ~flex

#define OBUFLEN    32

#ifdef NO_ANY_TRACE
#define TRACE(A)          //   TTRACE(IO_BETA_TRACE,A,0,0)
#define TTRACE1(A,B)      //   TTTRACE(IO_BETA_TRACE,A,B,0)
#define TRACE1(A,B)  B    //   TTRACE(IO_BETA_TRACE,A,B,0)
#define TRACE2(A,B,C) B;C //   TTRACE(IO_BETA_TRACE,A,B,C)
#else

#define TRACE(A)             TTRACE(IO_BETA_TRACE,A,0,0)
#define TRACE1(A,B)          TTRACE(IO_BETA_TRACE,A,B,0)
#define TTRACE1(A,B)         TTTRACE(IO_BETA_TRACE,A,B,0)
#define TRACE2(A,B,C)        TTRACE(IO_BETA_TRACE,A,B,C)

extern UINT pascal IO_BETA_TRACE;

#endif

extern "C" {
void pascal PEB_INIT(int port);
}
extern struct multychan OSDATAPTR MULTCH ;

void sccinit(char AddrA,char AddrB);
//extern io_buf OSDATAPTR CurOFree;


//int TEST_COUTER;
//io_ring OSDATAPTR RingA;

//************************************************************

struct hframe_descr:hard_lvl {
//---------------- semaphores
//---0x0E
        UCHAR  SendStopped;
        char   AddrR;
//---------------- channel specific
//---0x10
        UINT  csl_Port  ;  // базовый адрес - NA_STARL
        char GotTo;
//-------------
        char MeetError ;   // got an error on the way
//-------------
//---0x14
//-------------
        io_buf_struct     RBuf;
        io_buf_struct     OBuf;
        struct io_buf OSDATAPTR  Curr;     // some output buffers
//---0x22

//-------------- Только для трассовых нужд
        UINT CMD ;
//---------------- counts
        UINT  TA1,TO1; // to count in and out packets
//---------------- debug only
//---0x28
        UINT  f_RFO;
        UINT  f_Err0;
        UINT  f_Err1;
        UCHAR f_Err2;
        UCHAR f_Err2x;
        UINT  f_Err3;
        UINT  f_Err4;

        UINT  ISR_reg;
        UINT  f_Err5;
        UINT  f_Alive;
        //UINT  f_Err6;
//---0x32//---0x32//---------------- buffers
//---0x34
};

// *************************************
// some usefull definitions

void outportb(unsigned int port,unsigned char C);
unsigned char inportb(unsigned int port);
// *************************************

struct hframe_descr OSDATAPTR AST_A;
struct hframe_descr OSDATAPTR AST_B;
//uint BUS_ALIVE;

#include "semax.inc" ;

asm {
// просто ассемблер!
_DATA   ends

        extrn     DGROUP@:word

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        include   ..\sema.inc

//;;*********************

TraceAsm MACRO A,B,C
#ifdef NO_ANY_TRACE
#else
     Push Ax
     Push [IO_BETA_TRACE]
     Push A
 ifnb <B>
     Push B
 else
     Push 0
 endif
 ifnb <C>
     Push C
 else
     Push 0
 endif
     Call TTRACE
     Pop  Ax
#endif
  endm

//*
chk_bus:
        DX_VALUE = NA_CCR0H
        SET_DX   NA_STARL
        Push    Cx
        Mov     Cx,100h
Again_cb:
        From_portw   ax,dx
        test         ax,NCD*256
        jz           fin
        Loop         Again_cb
fin:
        Pop     Cx
        Or          Ax,Cx
        SET_DX   NA_CMDRL
        jmp short  here //_plus
// */
XCommand_PEB20525:
// PROC     macro   command:req
//        Cx - comand
        jcxz     xc_fin
        mov      Dx,[Bx].csl_Port
        DX_VALUE = NA_STARL
comment #
        From_portb   al,NA_CCR0H

        Cmp      al,90h
        je       chk_bus
;#
// from STARL
        SET_DX   NA_CMDRL
here:
        From_portw  ax,dx
        Or          Ax,Cx
        To_portw     dx,ax
//        SET_DX   NA_STARL
#ifdef NO_ANY_TRACE
#else
//        in      Ax,Dx
        SET_DX   NA_STARL
        in      Ax,Dx
//        TraceAsm   0bbbbh,Ax,Cx
#endif
xc_fin:
        ret

//        .386p
   public    intrX_vect

intrX_vect:
                Mov             byte ptr ds:[0xE1],1
//        inc       word ptr TEST_COUTER
        call            int_self
;// **************************************************

        mov            al,20h
        Mov            Dx,0F020h
        Out            Dx,al             // EOI,0x8000
#ifdef SLAVE_INTR
        Mov            Dx,0F0A0h
        Out            Dx,al             // EOI,0x8000
#endif
;// **************************************************
                Mov             byte ptr ds:[0xE1],2

        .386p
        iretd
        jmp short intrX_vect

        .186
intr_vect:
        pusha
        PUSHREG Ds,Es
        Mov             Ax,CS:DGROUP@
        Mov             Ds,Ax
        Mov             Es,Ax
        call            int_self
;// **************************************************
        mov            al,20h
        Mov            Dx,0F020h
        Out            Dx,al             // EOI,0x8000
#ifdef SLAVE_INTR
        Mov            Dx,0F0A0h
        Out            Dx,al             // EOI,0x8000
#endif
;// **************************************************

        POPREG  Ds,Es
        popa
        iret
;// **************************************************
int_self:
//        inc   TEST_COUTER
//        To_portw            NB_IMR0,0xffff
        Cld
        mov     Bp,0
ch_b:
        From_portw      cx,NA_ISR0                   //NB_ISR0
        jcxz      ch_a
        To_portDXV      NA_IMR0,0xffff
        mov     Bp,1
//        From_portDXV     bx,NA_STARL                   //NB_ISR0
        Mov       Bx,AST_A
//    TraceAsm        0x1111,Cx,Bx ///TRACE!!!!!!!!!!!!!!!
        Call      Check_Channel
        To_portw               NA_IMR0,IMASK_VALUE
ch_a:
        From_portw      cx,NB_ISR0                   //NB_ISR0
        jcxz      ch_fin
        To_portDXV      NB_IMR0,0xffff
        mov     Bp,1
//        From_portDXV     bx,NB_STARL                   //NB_ISR0
        Mov       Bx,AST_B
        Call      Check_Channel
        To_portw               NB_IMR0,IMASK_VALUE
ch_fin:
        Or       Bp,Bp
        mov     Bp,0
        jnz      ch_b
//        To_portw               NB_IMR0,IMASK_VALUE
// so we have two values on the stack
;// **************************************************

// mask all interrupts
// so we have two values on the stack

ddd = 22
        ret
;// **************************************************
hip_Error:

//    mov    word ptr [Bx].f_Err4,Cx
    mov    byte ptr [Bx].MeetError,1
    ret

Check_Channel PROC //    Reg:word,Descr:word
// ready to send?
    or          [bx].ISR_reg,Cx
/*
    Test        cx,256*NXMR
    jz          chk_err
    Xor         Si,Si
    Xchg        Si,[bx].Curr
    Or          Si,Si
    jz          chk_err
     inc         byte ptr [Bx].f_Err1[1]
    Push        Cx
    Call        TransmitNext
    Pop         Cx
chk_err:
//*/
    Test        cx,NRFO+NFLEX+NRPF+(256*(NXDU+NXMR))
    jz          hi_RFOfin
    Call        hip_Error
    Mov         word ptr [Bx].f_Err4,Cx
    inc         word ptr [Bx].f_RFO
    Or          word ptr [Bx].CMD,NXRES+NRRES*256  // reset recv machine
hi_RFOfin:
// ready to read?
    test        cx,NRME // got a valid frame data
    jz          hi_RMEfin
    test        cx,NRFO+NFLEX+NRPF+(256*(NXDU+NXMR))
    jnz         finnn

    mov    dx,[Bx].csl_Port
    DX_VALUE = NA_STARL
    From_portbDXV      Al,NA_RSTA
    Test   Al,NCRCOK
    jz     finnnxx
    Test   Al,NRAB
    jnz    finnn000
    cmp    Dx,NA_RSTA
    Mov    Dx,NA_RBCL
    jz     RBC_fin
    Mov    Dx,NB_RBCL
 RBC_fin:
//    From_portb      CH,dx
//    dec   Dx // so NxRBCL
    From_portw      Cx,dx

           jcxz            finnn0
           dec             Cx
           And             Cx,0x1f
           Call            ReceiveNext
           jmp             finnn
finnn000:
//         inc         byte ptr [Bx].f_Err1
finnn0:
    Or          word ptr [Bx].CMD,NRRES*256
         inc         byte ptr [Bx].f_Err1
finnnxx:
    Call        hip_Error
;//    Or          word ptr [Bx].CMD,NRRES*256  // reset recv machine
    inc         word ptr [Bx].f_Err0
finnn:
    inc  word ptr [Bx].TO1
;    jmp         hi_RMEfin0

;hi_RMEfin0:
    Or          word ptr [Bx].CMD,NRMC*256
hi_RMEfin:
hi_RPFfin:
    Xor         Cx,Cx
    Xchg        Cx,word ptr [Bx].CMD
    jcxz        cc_fin
    Call        XCommand_PEB20525
cc_fin:
        ret
Check_Channel ENDP


;// **************************************************
// аппаратно-зависимая процедура
// выбрать свободный буфер и переложить его в приемные
ReceiveNext: // cx - cnt,
         jcxz  rfin

         mov    dx,[Bx].csl_Port
         DX_VALUE = NA_STARL
         SET_DX   NA_RFIFOL

//         GET_BUF  CurOFree,Di
//         Cmp      word ptr [Bx].RBuf[4],10h
//         ja       rn_Error
         GETFREEBUF  Di
         Or    Di,Di
         jz    rn_Error

         Mov   [Di].(io_buf)Len,Cx
         Push  Di
         Lea   Di,[Di].(io_buf)Buf
         rep   insb
//         Mov   Ax,0ddddh
//         stosw
         pop   Di
/*
// **************************************************
// добавляем информацию о статусных регистрах
// **************************************************
         From_portbDXV      Ah,NA_ISR0
         From_portbDXV      Al,NA_ISR1
         mov   ds:[di].(io_buf)reserved0,ax
         From_portbDXV      Ah,NA_RSTA
         From_portbDXV      Al,NA_STARH
         mov   ds:[di].(io_buf)reserved1,ax

         From_portbDXV      Ah,NA_RBCH
         From_portbDXV      Al,NA_RBCL
         mov   ds:[di].(io_buf)reserved2,ax
*/
         Mov   Ax,di
// so move buffer to CurBuf position
         INSERT_BUF [Bx].RBuf,ax //di

         Mov  word ptr [BX].f_Err3,0
         Mov  word ptr [BX].f_Alive,1
//         mov   BUS_ALIVE,2
//           TraceAsm     1119h,Ax,Bx
  rfin:
         ret
  rn_Error:
                //inc  word ptr [BX].f_Err6
        Call        hip_Error
                ret

// аппаратно-зависимая процедура
 // Si - obuf try to send
   // нужно послать io_buf [Si]
   // Cx == 0  !!!!



// аппаратно-независимая процедура
TransmitNext: // si - io_buf для передачи
        Xor    Cx,Cx
        Push   Si

        Xchg   Cx,word ptr [Si].(io_buf)Len
        Lea    Si,[Si].(io_buf)Buf
//        TraceAsm  0x1112,Cx,[Si][1]
        mov    dx,[Bx].csl_Port
        DX_VALUE = NA_STARL
        SET_DX   NA_XFIFOL
        rep     outsb

        Pop    Si     // посланый буффер нужно освободить

        Or      word ptr [Bx].CMD,CMD_TRANSMIT_END
//XTF+XME
        inc  word ptr [Bx].TA1
        ret

_TEXT   ends
_DATA   segment word public 'DATA'
}

void  hdlc_pebHANDLE(hframe_descr  OSDATAPTR Tran,UINT Reson)
{
  if      (Reson==hc_Idle)
  {
/**/
     io_buf OSDATAPTR RB;
     if (Tran->MeetError!=0) {
    // переложить принятые буфера в свободные
    // Tran->CurBuf->Pen=0;
       Tran->MeetError = 0;
       Tran->hHandle(Tran->hPar,pck_Error,Tran->f_Err4);
     }
     while ((RB=Get_buf(&Tran->RBuf))!=0)
     {
       asm {push RB;};
       asm pop  RB ;
//       TRACE2(0x1118,(UINT)Tran,RB->Len);
       Tran->hHandle(Tran->hPar,pck_GotData,RB->Len,&RB->Buf[0]);
       MakeFree(RB);
     }
     UINT i ; UINT b ;

//CurOFirst - начало списка буферов
//CurOSending - посылаемый буфер
//CurOBuf - заполняемый
//CurOFree - список свободных

     while (Tran->OBuf.Num<OBUF_NUMB) // 2 - число буферов, одновременно передаваемых
     {
        io_buf OSDATAPTR NBuf =GetFreeBuf();
        if (NBuf==0) break ;
        NBuf->Len=Tran->hHandle(Tran->hPar,pck_GiveMeData,OBUFLEN,&NBuf->Buf[0]);
//
// вычисление контрольной суммы
//

        if (NBuf->Len==0)
        {
          MakeFree(NBuf);
          break ;
        }


        NBuf->Next = 0 ;
        Insert_buf(&Tran->OBuf,NBuf);
     };

     if (Tran->GotTo!=0) {
       TRACE1(0x1117,(UINT)Tran);
       Tran->GotTo=0 ;
       asm {
         cli
         Mov     Bx,Tran
         Mov     cx,NXRES+(NRRES*256)
         Call    XCommand_PEB20525//        <NXRES>
         sti
       }

       Tran->ISR_reg|=(NALLS*256);

       if(Tran->f_Err3++> 20) {
         Tran->f_Alive=0;
         Tran->f_Err3=0;
         hdlc_pebHANDLE(Tran,hc_HardInit2);
       }
       Tran->hHandle(Tran->hPar,pck_CountReset,3);
     }

     if (Tran->OBuf.Top!=0)
     {
       Tran->hHandle(Tran->hPar,pck_CountUp);
       asm {
         Mov     Bx,Tran
//*
         mov    dx,[Bx].csl_Port
         DX_VALUE = NA_STARL
         cli
         cld
         In       Ax,Dx
//         TraceAsm  0x1118,Ax,Bx
         Test     Ax,NXFW
         jz      cccc
// */
         Test byte ptr    [bx].ISR_reg[1],NALLS
         jz      cccc
         And  byte ptr    [bx].ISR_reg[1],not NALLS
        Mov     Si,[Bx].Curr
        Or      Si,Si
        jz      ccc_here
        MAKEFREE   Si
      }
ccc_here:
      asm {
        GET_BUF [Bx].OBuf,Si
        Mov     [Bx].Curr,Si
        Or     Si,Si
        jz     cccc   // нету ничего!!!
         Call    TransmitNext

         Xor      Cx,Cx
         Xchg     Cx,word ptr [Bx].CMD
         jcxz     cccc
         Call        XCommand_PEB20525
         jmp short   cccc
       }
     cccc:
       asm sti ;
     } else
       Tran->hHandle(Tran->hPar,pck_CountReset,0);
  }
  else if (Reson == pck_TimeOut)
  {
    Tran->GotTo = 1 ;
  }
  else if (Reson==pck_InitScaners)
  {
    Tran->hHandle(Tran->hPar,pck_InitScaners,255,4);
  }
  else if (Reson==hc_HardInit)
    { // нет приема! Нужно сбросить приемник!
      TRACE(0x4441);
      if(Tran->f_Err2-Tran->f_Err2x>5)
         hdlc_pebHANDLE(Tran,hc_HardInit2);
      else {
//      PEB_INIT(Tran);
//*
      asm {
        cli
        Mov     Bx,Tran

        Mov     cx,NXRES+(NRRES)*256
        Call    XCommand_PEB20525//        <NXRES>
        sti
      }
//*/
           Tran->f_Err2++;
      }
      ffin:
    }
  else if (Reson==hc_HardInit2)
    { // нет приема! Нужно сбросить приемник!
      Tran->f_Err2x=Tran->f_Err2;
      TRACE(0x4441);
    //  if(( AST_A->f_Alive==0)&&( AST_B->f_Alive==0))  {
    //              AST_A->f_Alive=1; AST_B->f_Alive=1;
         PEB_INIT(Tran->csl_Port);
         Tran->ISR_reg |= NALLS*256;
      //sccinit(0,0);
        Tran->f_Err5++;
  /*    } else {
        PEB_INIT(Tran->csl_Port);
        Tran->ISR_reg |= NALLS*256;
        Tran->f_Err2++;
      }  */

    }
}

//obuf OSDTAPTR OBUF_START;

void CHAN_INIT(int CPort,struct hframe_descr OSDATAPTR Tran,char AddrR)
{
  Tran->hIO_HANDLE =(prc_handle OSDATAPTR) hdlc_pebHANDLE;
  if(AddrR!=0) {
    Tran->AddrR = AddrR ;
    GET_IO_BUF(5);
  }
  Tran->csl_Port= CPort ;
}


void pascal PEB_INIT(int port) //struct hframe_descr OSDATAPTR AST)
{
//unsigned char  bell,i;

  asm {   cli
          From_portb  Al,0x20ef

          Mov   Dx,port

          DX_VALUE  = NA_STARL

 // not-visible, 7a clock mode, DPLL on!
NA_CCR0L_visible = 0x80
        mov      Ax,0x0047
        Cmp      Al,20
        je       itsoldver
//        Or       Ax,0x1000

      }
itsoldver:
     asm {
        To_portDXV            <NA_CCR0L>,Ax ;//0x0047
//        To_portbDXV            <NA_CCR0H>,0x10
 // Bus configuration, timing mode 2 (NRZ data encoding)
NA_CCR0H_Bus_tmod2_NRZ         = 0x00
//        To_portbDXV            <NA_CCR0H>,NA_CCR0H_Bus_tmod2_NRZ
//       DIV(2)  - 0B        DIV=_0_ No Data Inversion /1 - Inversion
#ifdef VER22
        To_portDXV            <NA_CCR1L>,0x1602
#else
        To_portDXV            <NA_CCR1L>,0x1642
#endif


 // 16 bit адрес
CCR2L_testloop = 1
CCR2L_AUTOMODE = 0
CCR2L_AMODE_2  = 0x80
CCR2L_mds      = 0x20
// no any preambs // Was $7200+...
        To_portDXV            <NA_CCR2L>,0x6000+CCR2L_AMODE_2+CCR2L_mds //+CCR2L_testloop  // test loop!!!

        To_portDXV            <NA_CCR3L>,0x8


// маскирование неработающих прерываний
        To_portDXV            <NA_IMR0>,0xffff
        To_portbDXV            <NA_IMR2>,0x3

// намерение использовать преамбулу для улучшения передачи
        To_portbDXV            <NA_PREAMBL>,0x55
// делитель частоты
//      BRRL - BRN(5:0)
//      BRRH - BRM(3:0)
//      k=(1+BRN)*(2**BRM)  для 1,1 режима k=4 ; 3,1=8 ; 3,2=12 ;
        To_portbDXV            <NA_BRRL>,0 //0x3 //1
        To_portbDXV            <NA_BRRH>,0 //0x2 //1


// инициализация адреса

        To_portbDXV            <NA_XAD1>,0xFF //0xff
        To_portbDXV            <NA_XAD2>,0xFF //,0xff
        To_portbDXV            <NA_RAH1>,[Bx].AddrR
        To_portbDXV            <NA_RAL1>,[Bx].AddrR
        To_portbDXV            <NA_RAH2>,0xFF
        To_portbDXV            <NA_RAL2>,0xFF

// ограничение по длине приема
        To_portDXV        NA_RLCRL,0h       //NumMod          // 040h
//        To_portbDXV        NA_RLCRH,0h       //NumMod          // 040h


// инициализация режима
//TIMR3  CNT(2:0)   -   000B
//       TMD        -     0B

        To_portbDXV            <NA_TIMR3>,0x0

// инициализация канала передачи


//маскирование неиспользованных прерываний

        To_portDXV            NA_IMR0,IMASK_VALUE
//        To_portbDXV            NA_IMR0,0x02
//        To_portbDXV            NA_IMR1,0xc8
        To_portbDXV            NA_IMR2,0x3

//зачистка статусных регистров прерываний
        From_portbDXV          al,NA_ISR0
        From_portbDXV          al,NA_ISR1
        From_portbDXV          al,NA_ISR2

        To_portbDXV            NA_CMDRL,0x0
        To_portbDXV            NA_CMDRH,0x0


        From_portbDXV          al,NA_CCR0H
        OR                    al,80h
        To_portbDXV            NA_CCR0H,al

//        TraceAsm  0xeee1,DX_VALUE,AST
        Mov                    Cx,NXRES+256*NRRES
        Call                   XCommand_PEB20525 //        <NXRES+256*NRRES>
        TraceAsm  0xeee3,DX_VALUE,AX

      }
fin:
        asm{    sti  }
}

void sccinit(char AddrA,char AddrB){
  if(AST_A==0) {
    GETMEM(hframe_descr, Tran_A);
    AST_A=Tran_A;
  }
  if(AST_B==0){
    GETMEM(hframe_descr, Tran_B);
    AST_B=Tran_B;
  }

  TRACE(0x101);

  asm {
    Push PEB20525_intr
    push offset intr_vect
    Call IBMsetNearIntVect
  }
//*
  asm {
#ifdef NO_ANY_TRACE
        .386p
        smsw    ax
        .186
        test    Ax,1
        jz      Not_A_ProtMode

//_&iV            = 9;
                mov  Bx,offset intrX_vect
                Mov  Dx,Cs
//                Push Es
//                pop  Ds
                Mov  Cx, 0x9 //_&iV

                mov  Ax,9
                int  61h
//
#endif
  }

Not_A_ProtMode:
      asm { cli };
//*/

  TRACE(0x102);
//printf("peb_init end1 \r\n");
// сброс состояния HDLC - контроллера
      outportb(GCMDR,1);
// маскирование неиспользуемых векторов
//GMODE  GIM  (0)   - 0B
// GIM=_1_ Global interrupt mask is set. Pin INT/INT remains inactive.
//       DSHP (1)   - 0B
// DSHP=_0_ Shaper is enabled. Recommended setting if a crystal is
// connected to pins XTAL1/XTAL2.
// DSHP=_0_ Shaper is disabled.
//       OSCPD(3)   - 0B
//  OSCPD=_0_ The internal oscillator is active.
//  OSCPD=_1_ The internal oscillator is in power down mode.
//       IPC(5-4)   - 11b
//  _00_ Open Drain active low
//  _01_ Push/Pull active low
//  _10_ Reserved.
//  _11_ Push/Pull active high
//       EDMA (6)   - 0B      работаем без DMA
      outportb(GMODE,0x32);
//printf("peb_init end2 \r\n");
      outportb(DIMR,0x77);
      outportb(GPIML,0x7);
      outportb(GPIMH,0xFF);

      CHAN_INIT(NA_STARL+0x0,AST_A,AddrA);
      CHAN_INIT(NA_STARL+0x50,AST_B,AddrB);

  TRACE(0x103);
       asm{
          cli
#ifdef SLAVE_INTR
          From_portb      al,OCW1MDOS
          and     al,not  4H
          To_portb        OCW1MDOS,al
          From_portb      al,OCW1SDOS
          and     al,not PEB20525_irq
          To_portb        OCW1SDOS,al
#else
          From_portb      al,OCW1MDOS
          and     al,not PEB20525_irq
          To_portb        OCW1MDOS,al
#endif

          xor     ax,ax

        mov            al,20h
        Mov            Dx,0F020h
        Out            Dx,al             // EOI,0x8000
#ifdef SLAVE_INTR
        Mov            Dx,0F0A0h
        Out            Dx,al             // EOI,0x8000
#endif

 //подключение прерываний через регист gmode

          From_portb      al,GMODE
          and     al,0xFE
          mov             dx,GMODE
          To_portb        dx,al
          To_portb        DIMR,33h

     INTCFG     EQU 0F832H

          From_portb      al,INTCFG
          or              al,2
          To_portb        dx,Al

          sti
                  }
  TRACE(0x104);

 PEB_INIT(AST_A->csl_Port);
 AST_A->ISR_reg |= NALLS*256;
  TRACE(0x105);
 hdlc_pebHANDLE(AST_A,hc_HardInit);
 PEB_INIT(AST_B->csl_Port);
 AST_B->ISR_reg |= NALLS*256;
  TRACE(0x106);
 hdlc_pebHANDLE(AST_B,hc_HardInit);
  TRACE(0x107);
}

// *******************************************************
// to control channals

#define PROC *((unsigned char _ss *)0xd)

void pascal SendMess2All(int cmd)
{
 if (AST_A !=0)
 {
   io_ring_item OSDATAPTR Item = ((io_ring OSDATAPTR)AST_A->hPar)->IO_Ring ;
   io_ring_item OSDATAPTR Item0 = Item ;
   do {
     SEND_OS_MESSAGE((((int)Item->Address)<<8)+0xff,0x80ff,0xf,2,cmd);
     Item = Item->Next;
   } while (Item !=Item0);
 }
}


int pascal _PEB_CONTROL(int Command)
{
  switch (Command)
  {
    case CMD_INITIALIZE_TO:
      asm { Push  Es ; Pop  Ds }
      if (CK4_CURSTATE==0) return 1;
      return -1 ;
/*
    case CMD_INITIALIZE:
      CK4_CURSTATE = 0;
      SetTO(0x1,CMD_PEB_SWITCH_A);
      return -1 ;
*/
    case CMD_PEB_SWITCH_A:
      asm { Push  Es ; Pop  Ds }
      CK4_CURSTATE = 3;
      if (((CK4_FLAG & ck4_ChanA)==0)&& ((CK4_FLAG & ck4_Slave)!=0))
      {
        PEB_SWITCH(ck4_ChanA);
        RegisterQueue(&((io_beta_dscr OSDATAPTR)
                            ((io_ring OSDATAPTR)AST_A->hPar)->IO_Ring->hPar
                       )->Mss.mssInQueue,0);
      }
      break;
    case CMD_PEB_SWITCH_B:
      asm { Push  Es ; Pop  Ds }
      CK4_CURSTATE = 3;
      if (((CK4_FLAG & ck4_ChanB)==0)&& ((CK4_FLAG & ck4_Slave)!=0))
      {
        PEB_SWITCH(ck4_ChanB);
        RegisterQueue(&((io_beta_dscr OSDATAPTR)
                            ((io_ring OSDATAPTR)AST_B->hPar)->IO_Ring->hPar
                       )->Mss.mssInQueue,0);
      }
      break ;
    default:
      return -1 ;
  };
  return 0 ;
};

// *******************************************************

struct routetab_hdlc {
  int  Proc;
  char AddrA;
  UCHAR AddrB;
  char mod[20];
};

#define CH ((routetab_hdlc far *)Ch)

#define PROC *((unsigned char _ss *)0xd)

//pack_lvl OSDATAPTR  TEST_DRV;
//pack_lvl OSDATAPTR  TEST_DRV_B;

int pascal _ROUTE_PEB20525(void far * Ch)
{
// so CH->AddrA - address of A Channal
// so CH->AddrB - address of B Channal
//    if Addr <> 1 then sendto 1 channal,

  int i,j ; UCHAR ADDRESS;UCHAR ADDRESS0;
  io_beta_dscr OSDATAPTR Temp;

#ifdef NO_ANY_TRACE
#else
  //IO_BETA_TRACE = 0x1400;
#endif
  TRACE(0x001);
// Это ck4?
  io_ring OSDATAPTR ring=0 ; // so the master
  if (CH->AddrA==1)
  { // ck4
    sccinit(1,CH->AddrB);
    ring = GetNewRing(1,AST_A);
    i = 0;
    if (ring !=0) {
      for (i=0;(i<0x10)&&(CH->mod[i]!=0);i++)
      {
        Temp = GetNewIO(NewRingItem(ring,CH->mod[i],CH->mod[i]),PROC);
        Temp->CanRegister=0;
        Temp->InfoIn=CH->mod[i];
        Temp->tim_Max1=IO_BETA_TIMEOUT+10;
        RegisterQueue(&Temp->Mss.mssInQueue,CH->mod[i] );
        GET_IO_BUF(3);
      }
      // fill 50 module!!!
      Temp = GetNewIO(NewRingItem(ring,CH->AddrB,CH->AddrB),PROC);
      Temp->tim_Max1=IO_BETA_TIMEOUT;
      REGISTER_QUEUE(&Temp->Mss.mssInQueue);

      Temp->CanRegister=0;
      Temp->InfoIn=PROC;
    }
    else {
      i=2;
      CK4_CURSTATE = 0;
    }

        if ((CK4_FLAG &&ck4_Right) ==0)
        {
/*         Temp = GetNewIO(NewRingItem(ring,CH->AddrB,CH->AddrB),PROC);
         Temp->CanRegister=0;*/
         RegisterQueue(&Temp->Mss.mssInQueue,CH->AddrB );
         ring = GetNewRing(0,AST_B);
         if (ring !=0)
         {
            Temp=GetNewIO(NewRingItem(ring,CH->AddrB,1),PROC);
            Temp->tim_Max1=IO_BETA_TIMEOUT;
            REGISTER_QUEUE(&Temp->Mss.mssInQueue);
            Temp->CanRegister=0;
            Temp->InfoIn=PROC;
         }
        } else {
         ring = GetNewRing(0,AST_B);
   //      TEST_DRV_B = GetNewTestIO(NewRingItem(ring,CH->AddrB,CH->AddrB)); /*
         if (ring !=0)
         {
            Temp=GetNewIO(NewRingItem(ring,CH->AddrB,1),PROC);
            Temp->tim_Max1=IO_BETA_TIMEOUT;
            REGISTER_QUEUE(&Temp->Mss.mssInQueue);
            Temp->CanRegister=0;
            Temp->InfoIn=PROC;
         }
        }
/**/
    j = CH->AddrB;
    asm {
      Xor       Ax,Ax
      Push      Ax
      Mov       Ax,j
      Mov       Ah,SS:[0xd]
//      Mov       SS:[0x40],Ax
      Push      Ax
      Push      Ax
      Mov       Bx,Sp
      Push      SS
      Push      Bx
      Call _ROUTE_MULTICH
      Add       Sp,6
//;  Al - ModuleNumber
//;    -> Bx - элемент таблицы
   }
   MULTCH->Dst=0;
   asm {
      Mov       Ax,j
     Extrn pascal FindModuleTranItem:proc
      Call      FindModuleTranItem
      Mov       Ax,Ds:[0x300][Bx]
      Mov       Ds:[STRANGE_QUEUE],Ax
    };
//    STRANGE_QUEUE  = (UINT) &Temp->Mss.mssInQueue;
    STRANGE_SWITCH = (((UINT)PROC)<<8)+CH->AddrB;

    return i+5;
  }
  else if (CH->AddrA==2)
  {
    CK4_FLAG |=ck4_Slave;
    sccinit(PROC,PROC);
    GET_IO_BUF(5);
//    TEST_DRV   = GetNewTestIO(AST_A); /*
    ring = GetNewRing(0,AST_A);
//    RegisterQueue(&GetNewIO(AST_A,PROC)->Mss.mssInQueue,CH->mod);
    if (ring !=0)
    {
      Temp=GetNewIO(NewRingItem(ring,PROC,1),PROC);
      Temp->tim_Max1=IO_BETA_TIMEOUT;
      REGISTER_QUEUE(&Temp->Mss.mssInQueue);
      MessQueue  OSDATAPTR ttt = (MessQueue OSDATAPTR)(*((UINT _ds *)0x300));
      RegisterQueue(&Temp->Mss.mssInQueue,0);
      RegisterQueue(ttt,0x1f);
      Temp->CanRegister=0;
    } /**/
    ring = GetNewRing(0,AST_B);
//    TEST_DRV_B = GetNewTestIO(NewRingItem(ring,PROC,1)); /*
    if (ring !=0)
    {
      Temp=GetNewIO(NewRingItem(ring,PROC,1),PROC);
      Temp->tim_Max1=IO_BETA_TIMEOUT;
      REGISTER_QUEUE(&Temp->Mss.mssInQueue);
      Temp->CanRegister=0;
    }
    /**/
//********* only one driver!!!!!!!!!!!!!!!!
    return 4;
  }
  else // 3 - csl8 как центральный пуп
  {
    CK4_FLAG |=ck4_Slave;
    sccinit(PROC,PROC);
;//    sccinit(1,1);

    GET_IO_BUF(5);
//    TEST_DRV   = GetNewTestIO(AST_A); /*
    ring = GetNewRing(0,AST_A);
    if (ring !=0) {
     if (PROC==0x71)
        Temp = GetNewIO(NewRingItem(ring,CH->AddrB,CH->AddrB),PROC);
     else
        Temp = GetNewIO(NewRingItem(ring,PROC,CH->AddrB),PROC);
        Temp->CanRegister=0;
        Temp->InfoIn=CH->AddrB;
        Temp->tim_Max1=IO_BETA_TIMEOUT+10;
        GET_IO_BUF(3);

      CK4_CURSTATE = 3;
//      SetTO(0x1,CMD_PEB_SWITCH_A);

//   CallExt SendMess2All

    }

    ring = GetNewRing(0,AST_B);
    if (ring !=0)
    {
     if (PROC==0x70)
        Temp = GetNewIO(NewRingItem(ring,CH->AddrB,CH->AddrB),PROC);
     else
        Temp = GetNewIO(NewRingItem(ring,PROC,CH->AddrB),PROC);
        Temp->CanRegister=0;
        Temp->InfoIn=CH->AddrB;
        Temp->tim_Max1=IO_BETA_TIMEOUT+10;
        GET_IO_BUF(3);
    }
    j = CH->AddrB;
    asm {
      Xor       Ax,Ax
      Push      Ax
      Mov       Ax,j
      Mov       Ah,Al
//      Mov       Ah,SS:[0xd]
      Push      Ax
      Push      Ax
      Mov       Bx,Sp
      Push      SS
      Push      Bx
      Call _ROUTE_MULTICH
      Add       Sp,6
//;  Al - ModuleNumber
//;    -> Bx - элемент таблицы
   }

    /**/
//********* only one driver!!!!!!!!!!!!!!!!
    return 4;
  }

}
