
//slot:=16;           //  слот
//tr  :=1;            //  физический тракт

muxbase :=$8000;
crval   :=inword(muxbase);
dmrena  :=crval ||$0010;
port1    :=muxbase +$2000 +(tr <<8)+ (slot<<1);


        setmask($16);

                SendMess((mod*256) +$ff,$AA51,0, // save script from disp 0
                                $0100,  datack4,
                                $0100,  port1,          $5509,  // Outword(port); // Idle
                                $100,$4444,0,1,$5506,
                                $FFFF                                                                                   // Finish
                                );

        setmask($16);

                SendMess((mod*256) +$ff,$AA52,0);       // Run the script

                RecvMess(msg,val1);

//writeln(mod,'write tract ', tr, 'slot', slot, datack4 && $FF );


