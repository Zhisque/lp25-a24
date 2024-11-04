# Projet LP25 - A2024

## Contexte

Le projet consiste à construire une solution de sauvegarde incrémentale d'un répertoire source (le répertoire à sauvegarder) vers un répertoire cible (le répertoire de sauvegarde). La sauvegarde incrémentale reposera sur des liens symboliques.

## Architecture

Le projet comprend trois exécutables :

- backup : réalise la sauvegarde
- restore : restaure une sauvegarde
- list : liste les éléments de la sauvegarde, notamment leurs versions

### La commande `backup`

La commande `backup` prend en paramètres (dans cet ordre) la source et la destination de la sauvegarde. Elle accepte les options suivantes :

- `--dry-run` pour tester quelles copies seront effectuées
- `--verbose` ou `-v` pour afficher plus d'informations sur l'exécution du programme.

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

### La commande `restore`

### La commande `list`

## Points notables

- copie avec `sendfile`
- suppression avec `unlink`
- copie par lien dur avec `link`
- date : combinaison de `gettimeofday` avec `localtime` et `strftime`

## Éléments d'évaluation
