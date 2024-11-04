# Projet LP25 - A2024

## Contexte

Le projet consiste à construire une solution de sauvegarde incrémentale d'un répertoire source (le répertoire à sauvegarder) vers un répertoire cible (le répertoire de sauvegarde). Il s'agit de développer un programme en langage C pour créer un outil de sauvegarde inspiré de Borg Backup, avec un accent sur la déduplication des données et la possibilité de réaliser des sauvegardes sur un serveur distant via des sockets.

## Architecture

Le projet comprend trois exécutables :

- backup : réalise la sauvegarde
- restore : restaure une sauvegarde
- list : liste les éléments de la sauvegarde, notamment leurs versions

```bash
projet_lp25/
│
├── src/
│   ├── main.c
│   ├── file_handler.c
│   ├── file_handler.h
│   ├── deduplication.c
│   ├── deduplication.h
│   ├── backup_manager.c
│   ├── backup_manager.h
│   ├── network.c
│   └── network.h
├── Makefile
└── README.md

```

## Options du programme
- `--backup` : crée une nouvelle sauvegarde du répertoire source, localement ou sur le serveur distant
- `--restore` : restaure une sauvegarde à partir du chemin, localement ou depuis le serveur
- `--list-backups` : liste toutes les sauvegardes existantes, localement ou sur le serveur
- `--dry-run` : test une sauvegarde ou une restauration sans effectuer de réelles copies
- `--d-server` : spécifie l'adresse IP du serveur à utiliser comme destination
- `--d-port` : spécifie l'e port du serveur de destination
- `--s-server` : spécifie l'adresse IP du serveur à utiliser comme source
- `--s-port` : spécifie le port du serveur source
- `--dest` : spécifie le chemin de destination de la sauvegarde ou de la restauration
- `--source` : spécifie le chemin source de la sauvegarde ou de la restauration
- `--verbose` ou `v` : affiche plus d'informations sur l'exécution du programme

### L'option `--backup`

`backup` commence par effectuer une copie complète de la dernière sauvegarde, par liens durs, avec la date et l'heure actuelles comme nom, sous le format `"YYYY-MM-DD-hh:mm:ss.sss"` où :

- `YYYY` est l'année sur 4 chiffres
- `MM` est le mois, entre 01 et 12
- `DD` est le jour, entre 01 et 31
- `hh` est l'heure, entre 00 et 23
- `mm` sont les minutes, entre 00 et 59
- `ss.sss` sont les secondes et les millisecondes, entre 00.000 et 59.999

S'il n'existe pas de sauvegarde précédente (i.e. c'est la première sauvegarde), le programme crée simplement un répertoire avec ce nom.

Cette première étape a donc pour effet que la sauvegarde de la source `/path/to/source` dans `/path/to/destination` sera en réalité située dans le répertoire `/path/to/destination/YYYY-MM-DD-hh:mm:ss.sss` où les champs du dernier répertoire sont remplacés par la date et l'heure réelles.

Le programme fait ensuite la sauvegarde en suivant les règles ci-dessous :

- un dossier dans la source est créé quand il n'existe pas dans la destination
- un dossier dans la destination est supprimé quand il n'existe pas dans la source
- un fichier dans la source, mais pas dans la destination, est copié dans la destination
- un fichier dans la source et dans la destination est copié si :
	- la date de modification est postérieure dans la source et le contenu est différent
	- la taille est différente et le contenu est différent
- un fichier de la destination est supprimé s'il n'existe plus dans la source

### L'option `--restore`

### L'option `--list-backups`

## Points notables

- copie avec `sendfile`
- suppression avec `unlink`
- copie par lien dur avec `link`
- date : combinaison de `gettimeofday` avec `localtime` et `strftime`


# Modalités d'évaluation

L'évaluation porte sur 3 éléments :

- Le rapport
	- un canvas vous est fourni sur la plateforme moodle ; vous devrez détailler plus précisément ce qui sera présenté à la soutenance, ainsi que des résultats quantitatifs et/ou qualitatifs de votre travail.
- La soutenance
	- une présentation de 10 minutes de tout le groupe (en équilibrant les temps de parole) dans laquelle :
		- vous rappellerez le contexte du projet, les grandes lignes de son fonctionnement (2 minutes)
		- vous expliquerez les choix que vous avez eu à faire et pourquoi (3 minutes)
		- vous présenterez les difficultés les plus importantes rencontrées et vos analyse à ce sujet (2 minutes)
		- vous proposerez un RETEX (retour d'expérience) dans lequel vous répondrez du mieux que possible à la question suivante : "_fort(e)s de notre expérience sur le projet, que ferions-nous différemment si nous devions le recommencer aujourd'hui ?_"
		- vous ferez une brève conclusion (1 minute)
	- vous répondrez ensuite à des questions posées par les enseignants pendant une dizaine de minutes.
- Le code
	- fourni dans un dépôt git avec l'historique des contributions de tous les membres,
	- avec un Makefile qui permet la compilation simplement avec la commande `make`
	- capacité à effectuer les traitements demandés dans le sujet,
	- capacité à traiter les cas particuliers sujets à erreur (pointeurs NULL, etc.)
	- Respect des conventions d'écriture de code
	- Documentation du code
		- Avec des commentaires au format doxygen en entêtes de fonction (si nécessaire)
		- Des commentaires pertinents sur le flux d'une fonction (astuces, cas limites, détails de l'algorithme, etc.)
	- Absence ou faible quantité de fuites mémoire (vérifiables avec `valgrind`)
	- **ATTENTION !** le code doit compiler sous Linux ! Un code non compatible avec un système Linux sera pénalisé de 5 points sur 20.

# Annexes

## Convention de code

Il est attendu dans ce projet que le code rendu satisfasse un certain nombre de conventions (ce ne sont pas des contraintes du langages mais des choix au début d'un projet) :

- indentations : les indentations seront faites sur un nombre d'espaces à votre discrétion, mais ce nombre **doit être cohérent dans l'ensemble du code**.
- Déclaration des pointeurs : l'étoile du pointeur est séparée du type pointé par un espace, et collée au nom de la variable, ainsi :
	- `int *a` est correct
	- `int* a`, `int*a` et `int * a` sont incorrects
- Nommage des variables, des types et des fonctions : vous utiliserez le *snake case*, i.e. des noms sans majuscule et dont les parties sont séparées par des underscores `_`, par exemple :
	- `ma_variable`, `variable`, `variable_1` et `variable1` sont corrects
	- `maVariable`, `Variable`, `VariableUn` et `Variable1` sont incorrects
- Position des accolades : une accolade s'ouvre sur la ligne qui débute son bloc (fonction, if, for, etc.) et est immédiatement suivie d'un saut de ligne. Elle se ferme sur la ligne suivant la dernière instruction. L'accolade fermante n'est jamais suivie d'instructions à l'exception du `else` ou du `while` (structure `do ... while`) qui suit l'accolade fermante. Par exemple :

```c
for (...) {
	/*do something*/
}

if (true) {
	/*do something*/
} else {
	/*do something else*/
}

int max(int a, int b) {
	return a;
}
```

sont corrects mais :

```c
for (int i=0; i<5; ++i)
{ printf("%d\n", i);
}

for (int i=0; i<5; ++i) {
	printf("%d\n", i); }

if () {/*do something*/
}
else
{
	/*do something else*/}
```

sont incorrects.
- Espacement des parenthèses : la parenthèse ouvrante après `if`, `for`, et `while` est séparée de ces derniers par un espace. Après un nom de fonction, l'espace est collé au dernier caractère du nom. Il n'y a pas d'espace après une parenthèse ouvrante, ni avant une parenthèse fermante :
	- `while (a == 3)`, `for (int i=0; i<3; ++i)`, `if (a == 3)` et `void ma_fonction(void)` sont corrects
	- `while(a == 3 )`, `for ( i=0;i<3 ; ++i)`, `if ( a==3)` et `void ma_fonction (void )` sont incorrects
- Basé sur les exemples ci dessus, également, les opérateurs sont précédés et suivis d'un espace, sauf dans la définition d'une boucle `for` où ils sont collés aux membres de droite et de gauche.
- Le `;` qui sépare les termes de la boucle `for` ne prend pas d'espace avant, mais en prend un après.
