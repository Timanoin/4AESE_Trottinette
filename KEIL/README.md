## KEIL

Lorsque le projet KEIL est lancé, vous pouvez choisir d'exécuter soit le programme de la commande en couple, soit de la commande en vitesse. Pour cela, ouvrez le projet `Pilote_Controleur_Puissance.uvprojx` ou bien accédez au code c depuis `src/Main_User.c`, et modifiez la ligne suivante selon vos souhaits :
```c
#define MODE COUPLE
```
ou
```c
#define MODE VITESSE
```
