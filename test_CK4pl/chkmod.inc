/*
  SendMess($21FF,$6b,$21);
  SendMess($20FF,$6b,$20);
  SendMess($40FF,$6b,$40);
  SendMess($0dFF,$6b,$0d);
  SendMess($0eFF,$6b,$0e);
  SendMess($41FF,$6b,$41);
//  SendMess($21FF,$6b,$21);
//  SendMess($41FF,$6b,$41);
  if recvmess(a,b,mod,d,e,f)=0 then
*/
     SetMask($B);SendMess($8000,$FF01,2,$D,$40);
     recvmess(a,b,c,d,e,mod)
