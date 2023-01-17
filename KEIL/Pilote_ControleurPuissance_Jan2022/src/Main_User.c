
/*
	!!!! NB : ALIMENTER LA CARTE AVANT DE CONNECTER L'USB !!!

VERSION 16/12/2021 :
- ToolboxNRJ V4
- Driver version 2021b (synchronisation de la mise à jour Rcy -CCR- avec la rampe)
- Validé Décembre 2021

*/


/*
STRUCTURE DES FICHIERS

COUCHE APPLI = Main_User.c : 
programme principal à modifier. Par défaut hacheur sur entrée +/-10V, sortie 1 PWM
Attention, sur la trottinette réelle, l'entrée se fait sur 3V3.
Attention, l'entrée se fait avec la poignée d'accélération qui va de 0.6V à 2.7V !

COUCHE SERVICE = Toolbox_NRJ_V4.c
Middleware qui configure tous les périphériques nécessaires, avec API "friendly"

COUCHE DRIVER =
clock.c : contient la fonction Clock_Configure() qui prépare le STM32. Lancée automatiquement à l'init IO
lib : bibliothèque qui gère les périphériques du STM : Drivers_STM32F103_107_Jan_2015_b
*/



#include "ToolBox_NRJ_v4.h"




//=================================================================================================================
// 					USER DEFINE
//=================================================================================================================





// Choix de la fréquence PWM (en kHz)
#define FPWM_Khz 20.0
#define Te 0.000200
#define Tm 0.002
#define Ti 0.001442*2.3
#define b1 ((2*Tm+Te)/(2*Ti))
#define b0 ((Te-2*Tm)/(2*Ti))

#define facteur 2

#define Te2 facteur*Te
#define Tc2 0.01592
#define Ti2 0.003606
#define b21 ((Te2)+(2*Tc2))
#define b20 ((Te2)-(2*Tc2))
#define a21 (2*Ti2)
#define a20 (-2*Ti2)



//==========END USER DEFINE========================================================================================

// ========= Variable globales indispensables et déclarations fct d'IT ============================================

void IT_Principale(void);
//=================================================================================================================


/*=================================================================================================================
 					FONCTION MAIN : 
					NB : On veillera à allumer les diodes au niveau des E/S utilisée par le progamme. 
					
					EXEMPLE: Ce progamme permet de générer une PWM (Voie 1) à 20kHz dont le rapport cyclique se règle
					par le potentiomètre de "l'entrée Analogique +/-10V"
					Placer le cavalier sur la position "Pot."
					La mise à jour du rapport cyclique se fait à la fréquence 1kHz.

//=================================================================================================================*/


float Te_us;

// Nos variables
float potentiometre_actuel, capteur_courant_actuel, entree_correcteur_actuel, capteur_fem_actuel, consigne_vit_actuel, entree_c2_actuel, consigne_c1_actuel,entree_c2_old, consigne_c1_old;
float entree_correcteur_old;
float alpha, alpha_old; 
int indexTe;

float a; 

int main (void)
{
	// !OBLIGATOIRE! //	
	Conf_Generale_IO_Carte();	
		

		
	// ------------- Discret, choix de Te -------------------	
	Te_us=Te*1000000.0; // conversion en µs pour utilisation dans la fonction d'init d'interruption
	indexTe = 0;
	
	
	// Initialisation k-1
	entree_correcteur_old = 0;
	alpha_old = 0;
	entree_c2_old = 0;
	consigne_c1_old = 0;

	//______________ Ecrire ici toutes les CONFIGURATIONS des périphériques ________________________________	
	// Paramétrage ADC pour entrée analogique
	Conf_ADC();
	// Configuration de la PWM avec une porteuse Triangle, voie 1 & 2 activée, inversion voie 2
	Triangle (FPWM_Khz);
	Active_Voie_PWM(1);	
	Active_Voie_PWM(2);	
	Inv_Voie(2);

	Start_PWM;
	R_Cyc_1(2048);  // positionnement à 50% par défaut de la PWM
	R_Cyc_2(2048);

	// Activation LED
	LED_Courant_On;
	LED_PWM_On;
	LED_PWM_Aux_Off;
	LED_Entree_10V_Off;
	LED_Entree_3V3_On;
	LED_Codeur_Off;

	a = b0;
	a = b1;

	// Conf IT
	Conf_IT_Principale_Systick(IT_Principale, Te_us);

	while(1) {
	
	}	
}





//=================================================================================================================
// 					FONCTION D'INTERRUPTION PRINCIPALE SYSTICK
//=================================================================================================================
int Courant_1,Cons_In;


void IT_Principale(void)
{
	// Recuperer valeurs ADC
	capteur_courant_actuel = (float)I1()/4095.0*3.3;
	//potentiometre_actuel = (float)Entree_3V3()/4095.0*3.3; // a modifier
	
	// Te2
	if (indexTe++ == facteur)
	{
		indexTe = 0;
		entree_c2_old = entree_c2_actuel;
		consigne_c1_old = consigne_c1_actuel;
		capteur_fem_actuel = (float)Entree_10V()/4095.0*3.3;
		consigne_vit_actuel = (float)Entree_3V3()/4095.0*3.3;
		
		entree_c2_actuel = consigne_vit_actuel - capteur_fem_actuel;
		consigne_c1_actuel = -a20/a21*consigne_c1_old + b21/a21*entree_c2_actuel + b20/a21*entree_c2_old;
		
		if (consigne_c1_actuel < 0) {
			consigne_c1_actuel = 0;
		} else if (consigne_c1_actuel > 3.3) {
			consigne_c1_actuel = 3.3;
		}
	}
	
	entree_correcteur_actuel = consigne_c1_actuel - capteur_courant_actuel;//potentiometre_actuel - capteur_courant_actuel;
	
	
	//Te
	// Calcul de alpha à partir de C(s)
	alpha = alpha_old + b1*entree_correcteur_actuel + b0*entree_correcteur_old;
	
	if (alpha > 1) {
		alpha = 1;
	} else if (alpha < 0) {
		alpha = 0;
	}	
		
	// Mise à jour des valeurs
	entree_correcteur_old = entree_correcteur_actuel;
	alpha_old = alpha;
	
	// Lancement de la PWM
	R_Cyc_1((int)(4095*(alpha)));
	R_Cyc_2((int)(4095*(alpha))); // Pas besoin d'inverser alpha

}

