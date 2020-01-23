SetMask($F);
SendMess(Mod*256+$FF,$6c,0,0);
SendMess(BASE*256+$FF,$6c,0,0);

MainDsp := $c;

SetMask($d);
SendMess(BASE*256,$ff01,06,$14+MainDsp,MainSeg,0,0,0);
recvMess(a,b,c,d,e);
/*
if Wave<10 then
evalwrite('module $~cd(d[0],h) w ~cd(d[4],d)(~cd(d[8],d)) ---- ',mod,Wave,Length);
else
evalwrite('module $~cd(d[0],h) w~cd(d[4],d)(~cd(d[8],d)) ---- ',mod,Wave,Length);
fi ;
*/
setreturnaddress(mod*256+$FF);
for i from 1 to wave do
if Length=100 then
   SendMess(BASE*256+$FF,$6b
        ,0,0,0,0,0,0 //2 - 14
        ,0,0,0,0,0,0 //3,
        ,0,0,0,0,0,0
        ,0,0,0,0,0,0
        ,0,0,0,0,0,0
//        ,0,0,0,0,0,0
//        ,0,0,0,0,0,0
        ,0
{}
   );
else
   SendMess(BASE*256+$FF,$6b  //10 +
       ,0,0,0,0,0,0 // 12 = 22 +
{        ,0,0,0,0,0,0 // 12 = 34 + 32
        ,0,0,0,0,0,0 // 12 = 46 +
        ,0,0,0,0,0,0 // 12 = 58 +
        ,0,0,0,0,0,0 // 12 = 70 + 64
        ,0,0,0,0,0,0 // 12 = 82 +
        ,0,0,0,0,0,0 // 12 = 94 +
        ,0           // 2 = 96        96
{}
   );
fi ;
od;

setreturnaddress(-1);

trcseg := $b80;
if DoSomething then
 setmask($d); SendMess(base*256,$ff01,6,0,trcseg,08,$f000,$aaa2);recvmess(a);
 for mmm from $40 to $41 do
   setmask($d); SendMess(mmm*256,$ff01,6,0,trcseg,08,$f000,$aaa2);recvmess(a)
 od
fi ;
if recvmess(a) then delay(time*1000) else delay((time-3)*1000); fi ;
if DoSomething then setmask($d); SendMess(base*256,$ff01,2,4,trcseg,$aaaA);recvmess(a);
 for mmm from $40 to $41 do
   setmask($d); SendMess(mmm*256,$ff01,2,4,trcseg,$aaaa);recvmess(a)
 od
fi ;

SetMask($b);
SendMess(BASE*256,$ff01,02,$18+MainDsp,MainSeg);
recvMess(a,b,c,d,e,f);
//evalwrite('~cd(d[0]) messages per ~cd(d[4]) sec',f,time);
if (f>600) then write('+');else write('-'); fi;



SendMess(BASE*256+$FF,$6c,0,1);

delay(3000);
SetMask($b);
SendMess(BASE*256,$ff01,02,$14+MainDsp,MainSeg);
recvMess(a,b,c,d,e,f);
/*
if f = wave then
  write('.')
else
  evalwrite(' ~cd(d[0]) waves instead ~cd(d[4])', f,wave); 
//  goto fin;
fi ;
*/
SendMess(BASE*256+$FF,$6c,0,0);
