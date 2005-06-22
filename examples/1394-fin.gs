% This file contains a description of the physical layer service
% of the 1394 or firewire protocol and also a description of the link
% layer protocol entities. This decription has been published 
% as S.P. Luttik in $\mu$CRL, and adapted to mCRL. Description 
% and formal specification of the link layer
% of P1394. Technical Report SEN-R9706, CWI, Amsterdam, 1997.
% The state space of the description as given, with 2 link
% protocol entities and the domains DATA, ACK and HEADER all having
% two elements has approximately 380.000 states. For 3 link protocols
% and the domains DATA, ACK and HEADER having only one element
% the state space has less than 300.000 states.
%
% This description has been analysed in M. Sighireanu and R. Mateescu, 
%  Verification of the Link Layer Protocol of the IEEE-1394 Serial Bus 
% (``FireWire''): an Experiment with E-LOTOS. Springer International
% Journal on Software Tools for Technology Transfer (STTT), 1998. 
% 1996, where they showed a mistake in the description, which could
% be trace back to a mistake in the standard.
%
% The description in this file differs on a few points from the
% description by Luttik to facilitate the generation of the state
% space. 


 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %%% DATA/CONTROL/ACKNOWLEDGE ELEMENTS AND THERE CRC COMPUTATION %%%
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

sort CHECK = struct bottom | check;

sort DATA = struct d1 | d2;

map  crc : DATA -> CHECK;

eqn  crc(d1)=check;
     crc(d2)=check;

sort HEADER = struct h1 | h2;


map  crc : HEADER -> CHECK;

eqn  crc(h1)=check;
     crc(h2)=check;

sort ACK = struct a1 | a2;

map  crc : ACK -> CHECK;
eqn  crc(a1)=check;
     crc(a2)=check;

sort SIGNAL = struct sig(getdest:Nat) ? is_dest |
                     sig(gethead:HEADER,gethcrc:CHECK) ? is_header |
                     sig(getdata:DATA,getdcrc:CHECK) ? is_data |
                     sig(getack:ACK,getacrc:CHECK) ? is_ack |
                     Start ? is_start | 
                     End ? is_end | 
                     Prefix ? is_prefix | 
                     subactgap ? is_sagap | 
                     dhead ? is_dhead | 
                     Dummy ? is_dummy;

map  is_physig,is_terminator : SIGNAL -> Bool;
     getcrc : SIGNAL -> CHECK;
var  s : SIGNAL;
eqn  is_physig(s) = is_start(s) || is_end(s) || is_prefix(s) || is_sagap(s);
     is_terminator(s)=is_end(s) || is_prefix(s);
     getcrc(s)=if(is_header(s),gethcrc(s),
               if(is_data(s),getdcrc(s),
               if(is_ack(s),getacrc(s),
                            bottom)));

map  is_hda : SIGNAL -> Bool;
     valid_hpart, valid_ack : SIGNAL -> Bool;
var  s : SIGNAL;
eqn  is_hda(s)=is_header(s) || is_data(s) || is_ack(s);
     valid_ack(s)=if(is_ack(s),getacrc(s)==check,false);
     valid_hpart(s)=if(is_header(s),getdcrc(s)==check,false);

map  corrupt : SIGNAL -> SIGNAL;
var  h : HEADER;
     d : DATA;
     a : ACK;
     c : CHECK;
eqn  corrupt(sig(h,c)) = sig(h,bottom);
     corrupt(sig(d,c)) = sig(d,bottom);
     corrupt(sig(a,c)) = sig(a,bottom);

sort SIG_TUPLE = 
       struct quadruple (first:SIGNAL,
                         second:SIGNAL,
                         third:SIGNAL,
                         fourth:SIGNAL)
        | void ? is_void;

sort PAR = struct fair | immediate;

sort PAC = struct won | lost;

sort LDC = struct ackrec(ACK) 
            |     ackmiss
            |     broadsent;

sort LDI = struct good (HEADER,DATA)
            |     broadrec (HEADER,DATA)
            |     dcrc_err (HEADER);

sort BOC = struct release | hold;

act
  LDreq : Nat#Nat#HEADER#DATA;
  LDcon : Nat#LDC;
  LDind : Nat#LDI;
  LDres : Nat#ACK#BOC;

  sPDreq,rPDind : Nat#SIGNAL;
  sPAreq : Nat#PAR;
  rPAcon : Nat#PAC;
  rPCind : Nat;

proc Link0(n:Nat,id:Nat,buffer:SIG_TUPLE)=
        sum dest:Nat,h:HEADER,d:DATA.
           is_void(buffer) ->
              LDreq(id,dest,h,d).
                 Link0(n,id,quadruple(dhead,
                                 sig(dest),
                                 sig(h,crc(h)),
                                 sig(d,crc(d)))),
              sPAreq(id,fair).Link1(n,id,buffer) +
        sum p:SIGNAL.
           rPDind(id,p).
              (is_start(p) -> Link4(n,id,buffer) , Link0(n,id,buffer));

     Link1(n:Nat,id:Nat,p:SIG_TUPLE)=
        rPAcon(id,won).Link2req(n,id,p) +
        rPAcon(id,lost).Link0(n,id,p);


     Link2req(n:Nat,id:Nat,p:SIG_TUPLE)=
        rPCind(id).sPDreq(id,Start).
        rPCind(id).sPDreq(id,first(p)).
        rPCind(id).sPDreq(id,second(p)) .
        rPCind(id).sPDreq(id,third(p)).
        rPCind(id).sPDreq(id,fourth(p)).
        rPCind(id).sPDreq(id,End).
        ( (getdest(second(p))==n) ->
            LDcon(id,broadsent).Link0(n,id,void),
            Link3(n,id,void));

     Link3(n:Nat,id:Nat,buffer:SIG_TUPLE)=
        sum p:SIGNAL.
           rPDind(id,p).
           ( is_prefix(p) -> Link3(n,id,buffer),
           ( is_start(p) ->  Link3RA(n,id,buffer),
           ( is_sagap(p) ->  LDcon(id,ackmiss).Link0(n,id,buffer),
                             LDcon(id,ackmiss).LinkWSA(n,id,buffer,n)
           )));

     Link3RA(n:Nat,id:Nat,buffer:SIG_TUPLE)=
        sum a:SIGNAL.
           rPDind(id,a).
           ( is_sagap(a) ->  LDcon(id,ackmiss).Link0(n,id,buffer),
             ( is_physig(a) -> LDcon(id,ackmiss).LinkWSA(n,id,buffer,n),
                               Link3RE(n,id,buffer,a)));

     Link3RE(n:Nat,id:Nat,buffer:SIG_TUPLE,a:SIGNAL)=
        sum e:SIGNAL.
           rPDind(id,e).
           ((valid_ack(a) && is_terminator(e)) ->
                   LDcon(id,ackrec(getack(a))).LinkWSA(n,id,buffer,n),
             ( is_sagap(e) ->
                   LDcon(id,ackmiss).Link0(n,id,buffer),
                   LDcon(id,ackmiss).LinkWSA(n,id,buffer,n)
           ) );

     Link4(n:Nat,id:Nat,buffer:SIG_TUPLE)=
        sum dh:SIGNAL.
           rPDind(id,dh).
           ( is_physig(dh) ->
             ( is_sagap(dh) ->
                Link0(n,id,buffer),
                LinkWSA(n,id,buffer,n)),
             Link4DH(n,id,buffer));

     Link4DH(n:Nat,id:Nat,buffer:SIG_TUPLE)=
        sum dest:SIGNAL.rPDind(id,dest).
           ( is_dest(dest) ->
             ( (getdest(dest)==id) ->
                  sPAreq(id,immediate).Link4RH(n,id,buffer,id),
                  ( (getdest(dest)==n) ->
                      Link4RH(n,id,buffer,n),
                      LinkWSA(n,id,buffer,n)
                  )
             ),
             ( is_sagap(dest) ->
                  Link0(n,id,buffer),
                  LinkWSA(n,id,buffer,n)
           ) );

     Link4RH(n:Nat,id:Nat,buffer:SIG_TUPLE,dest:Nat)=
        sum h:SIGNAL.rPDind(id,h).
           ( valid_hpart(h) ->
                Link4RD(n,id,buffer,dest,h),
                LinkWSA(n,id,buffer,dest)
           );

     Link4RD(n:Nat,id:Nat,buffer:SIG_TUPLE,dest:Nat,h:SIGNAL)=
        sum d:SIGNAL.
           rPDind(id,d).
             ( is_data(d) ->
                  Link4RE(n,id,buffer,dest,h,d),
                  LinkWSA(n,id,buffer,dest)
             );

     Link4RE(n,id:Nat,buffer:SIG_TUPLE,dest:Nat,h:SIGNAL,d:SIGNAL)=
        sum e:SIGNAL.
           rPDind(id,e).
             ( is_terminator(e) ->
                  ( (dest==id) ->
                       Link4DRec(n,id,buffer,h,d),
                       Link4BRec(n,id,buffer,h,d)
                  ),
                  LinkWSA(n,id,buffer,dest)
             );

     Link4DRec(n:Nat,id:Nat,buffer:SIG_TUPLE,h:SIGNAL,d:SIGNAL)=
        (getcrc(d)==check) ->
           LDind(id,good(gethead(h),getdata(d))).rPAcon(id,won).Link5(n,id,buffer),
           LDind(id,dcrc_err(gethead(h))).rPAcon(id,won).Link5(n,id,buffer);

     Link4BRec(n:Nat,id:Nat,buffer:SIG_TUPLE,h:SIGNAL,d:SIGNAL)=
        (getcrc(d)==check) ->
           LDind(id,broadrec(gethead(h),getdata(d))).Link0(n,id,buffer),
           Link0(n,id,buffer);

     Link5(n,id:Nat,buffer:SIG_TUPLE)=
        sum a:ACK,b:BOC.LDres(id,a,b).Link6(n,id,buffer,sig(a,crc(a)),b) +
        rPCind(id).sPDreq(id,Prefix).Link5(n,id,buffer);

     Link6(n:Nat,id:Nat,buffer:SIG_TUPLE,p:SIGNAL,b:BOC)=
        rPCind(id).sPDreq(id,Start).rPCind(id).sPDreq(id,p).rPCind(id).
           ( (b==release) ->
                sPDreq(id,End).Link0(n,id,buffer),
                sPDreq(id,Prefix).Link7(n,id,buffer)
           );
 

     Link7(n,id:Nat,buffer:SIG_TUPLE)=
        rPCind(id).sPDreq(id,Prefix).Link7(n,id,buffer) +
        sum dest:Nat,h:HEADER,d:DATA.
             LDreq(id,dest,h,d). Link2resp(n,id,buffer,
                      quadruple(dhead,sig(dest),sig(h,crc(h)),sig(d,crc(d))));

     Link2resp(n:Nat,id:Nat,buffer:SIG_TUPLE,p:SIG_TUPLE)=
        rPCind(id).sPDreq(id,Start).
        rPCind(id).sPDreq(id,first(p)).
        rPCind(id).sPDreq(id,second(p)).
        rPCind(id).sPDreq(id,third(p)).
        rPCind(id).sPDreq(id,fourth(p)).
        rPCind(id).sPDreq(id,End).
        ( (getdest(second(p))==n) ->
             LDcon(id,broadsent).Link0(n,id,buffer),
             Link3(n,id,buffer)
        );


     LinkWSA(n:Nat,id:Nat,buffer:SIG_TUPLE,dest:Nat)=
        sum p:SIGNAL.rPDind(id,p).
           ( is_sagap(p) ->
                Link0(n,id,buffer),
                LinkWSA(n,id,buffer,dest) 
           ) +
        (dest==id) -> rPAcon(id,won).rPCind(id).sPDreq(id,End).Link0(n,id,buffer);


 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %%%                AUXILIARY SPECIFICATION OF BUS               %%%
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


sort BoolTABLE = List(struct pair(Nat,getbool:Bool));

map  inita : Nat -> BoolTABLE;
     invert : Nat#BoolTABLE -> BoolTABLE;
     get : Nat#BoolTABLE -> Bool;
var  n,m : Nat;
     b : Bool;
     bt1,bt2 : BoolTABLE;
eqn  inita(0)=[];
     n>0 -> inita(n)=pair(Int2Nat(n-1),false)|>inita(Int2Nat(n-1));

     invert(n,[])=[];
     invert(n,pair(m,b)|>bt1)=
        if(n==m,pair(m,!b)|>bt1,pair(m,b)|>invert(n,bt1));
     get(n,[])=false;
     get(n,pair(m,b)|>bt1)=if(n==m,b,get(n,bt1));

map  zero,one,more: BoolTABLE -> Bool;
var  n  : Nat;
     bt : BoolTABLE;
eqn  zero([])=true;
     zero(pair(n,true)|>bt)=false;
     zero(pair(n,false)|>bt)=zero(bt);
     one([])=false;
     one(pair(n,true)|>bt)=zero(bt);
     one(pair(n,false)|>bt)=one(bt);
     more(bt)=!zero(bt) && !one(bt);

act  rPAreq: Nat#PAR;
     rPDreq,sPDind: Nat#SIGNAL;
     sPAcon: Nat#PAC;
     sPCind: Nat;
     arbresgap;
     losesignal;

proc BusIdle(n:Nat,t:BoolTABLE)=
        sum id:Nat,astat:PAR.rPAreq(id,astat).DecideIdle(n,t,id,astat) +
        !zero(t)->arbresgap.BusIdle(n,inita(n));



     DecideIdle(n:Nat,t:BoolTABLE,id:Nat,astat:PAR)=
        (!get(id,t)) ->
          sPAcon(id,won).BusBusy(n,invert(id,t),inita(n),inita(n),id),
          sPAcon(id,lost).BusIdle(n,t);

     BusBusy(n:Nat,t,next,destfault:BoolTABLE,busy:Nat)=
        (busy<n) -> 
           ( sPCind(busy).
                (sum p:SIGNAL.rPDreq(busy,p).Distribute(n,t,next,destfault,busy,p,0))
           ),
           ( zero(next) ->
                SubactionGap(n,t,0),
                  Resolve(n,t,next,0)
           ) +
       sum j:Nat.rPAreq(j,fair).sPAcon(j,lost).BusBusy(n,t,next,destfault,busy) +
       sum j:Nat.rPAreq(j,immediate).
             (!get(j,next) -> BusBusy(n,t,invert(j,next),destfault,busy));

     SubactionGap(n:Nat,t:BoolTABLE,i:Nat)=
        (i==n) ->
           BusIdle(n,t),
           sPDind(i,subactgap).SubactionGap(n,t,i+1);

     Resolve(n:Nat,t,next:BoolTABLE,i:Nat)=
        (i<n) ->
        (get(i,next) ->
            sPAcon(i,won).sPCind(i).Resolve(n,t,next,i+1),
            tau.Resolve(n,t,next,i+1) 
        ),
        Resolve2(n,t,next);

     Resolve2(n:Nat,t:BoolTABLE,next:BoolTABLE)=
        more(next) ->
           (sum j:Nat.rPDreq(j,End).(get(j,next) -> Resolve2(n,t,invert(j,next)))),
           (sum j:Nat,p:SIGNAL.
              rPDreq(j,p).
                 (is_end(p) ->
                     SubactionGap(n,t,0),
                     Distribute(n,t,inita(n),inita(n),j,p,0)
           )     );

     Distribute(n:Nat,t,next,destfault:BoolTABLE,busy:Nat,p:SIGNAL,i:Nat)=
        (i<n) ->
        ( (i!=busy) ->
          ( %% Signals can be handed over correctly
            (!is_header(p) || !get(i,destfault)) ->
               sPDind(i,p).Distribute(n,t,next,destfault,busy,p,i+1) +
            %% Destination signals may be corrupted
            sum dest:Nat.is_dest(p)->
               sPDind(i,sig(dest)).
                  Distribute(n,t,next,invert(i,destfault),busy,p,i+1) +
            %% Headers/Data/Acks may be corrupted
            is_hda(p) ->  
               sPDind(i,corrupt(p)).
                  Distribute(n,t,next,destfault,busy,p,i+1) +
            %% Headers/Data/Acks may get lost
            is_hda(p) ->
               losesignal.Distribute(n,t,next,destfault,busy,p,i+1) +
            %% Packets may be too large
            is_data(p) ->
               sPDind(i,p).sPDind(i,Dummy).
                  Distribute(n,t,next,destfault,busy,p,i+1) +
            (!get(i,next)) ->
               rPAreq(i,immediate).
                  Distribute(n,t,invert(i,next),destfault,busy,p,i)
         ),
         %% i==busy
         tau.Distribute(n,t,next,destfault,busy,p,i+1)
       ),
       %% i>=n
       ( is_end(p) ->
            BusBusy(n,t,next,destfault,n),
            BusBusy(n,t,next,destfault,busy)
       );


 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %%%      SPECIFICATION OF n LINK LAYERS CONNECTED BY A BUS      %%%
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  
proc LINK(n:Nat,i:Nat)=Link0(n,i,void);

     BUS(n:Nat)=BusIdle(n, inita(n));

     P1394(n:Nat)=
        allow({LDreq,LDcon,LDind,LDres},
           hide({arbresgap,losesignal},
              comm({rPDind|sPDind,rPDreq|sPDreq,rPAcon|sPAcon,
                    rPAreq|sPAreq,rPCind|sPCind},
                  allow({LDreq,LDcon,LDind,LDres,arbresgap,losesignal,rPDind|sPDind,rPDreq|sPDreq,rPAcon|sPAcon,
                    rPAreq|sPAreq,rPCind|sPCind},
                          BUS(2) || LINK(2,0) || LINK(2,1)))));

init P1394(2);

