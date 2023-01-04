clear all
close all
% Définition des constantes 
Tm = 0.002;                     %Tm = L/R 
T1 = 7.43e-5;                   %T1 = (R1//R2)*C1
T2 = 4.84e-6;                   %T2 = (R5*C2)
Km = 1;                         %Km = 1/R
Kca = 0.10416;                  %Gain du capteur
Kf = 1.45;                      %Gain du filtre Kf = (R2+R3)/(R1+R2)
Vbat = 12;
K = 2*Vbat*Kf*Kca;              %Gain système 

%Fonctions de transfert
G1=tf(2*Vbat);                  %TF du hacheur
G2=tf(Km,[Tm 1]);               %TF du moteur
F=tf(Kca*Kf,[T1*T2 T1+T2 1]);   %TF du filtre
T=G1*G2*F;                      %TF du système en BO

%Calcul des coefficients de C
Ti = K/(2*pi*400)
C=tf([Tm 1],[Ti 0])             %Tm correspond à la fc à 80Hz que l'on 
                                %souhaite éliminer
H = T*C                         %TF avec le correcteur
%bode(H)
%datacursormode
%grid

%Simulation avant discrétisation 
%out = sim('sim1', 2)           %Simulation sans saturateur
%out = sim('sim_satur',0.05)     %Simulation avec saturateur
%plot(out.simout)
%figure 
%plot(out.simout1)

%Discrétisation
Te = 1/5000                   %Période d'échantillonage 
D = c2d(C,Te)                   %TF discrète 
out = sim('sim_satur_disc',0.05)
plot(out.simout1)

b1 = ((2*Tm+Te)/(2*Ti));
b0 = ((Te-2*Tm)/(2*Ti));

