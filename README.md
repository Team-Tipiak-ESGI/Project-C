# Projet-C Cloud et Partage de fichiers

# Compilation
- Installer les dépendances avec la commande suivante :
```sh
sudo apt install libncurses5-dev libncursesw5-dev libssl-dev gcc cmake
```
- Compiler avec :
```sh
cmake CMakeLists.txt
```

# Fonctionnalités

## MVP

### Gestion d'utilisateurs
- Authentification par clé privée (mot de passe) et publique

### Envoi de fichiers
Les utilisateurs inscrits sur le site pourront envoyer des fichiers depuis leur machine à partir d'un client de leur choix.

### Téléchargement de fichiers
Les fichiers envoyés par des utilisateurs seront téléchargeables à tout moment.

### Partage de fichiers
Une fois les fichiers envoyés sur le réseau, ces derniers pourront être rendus accessibles à d'autres utilisateurs.

- Entre utilisateurs par défaut avec tous les droits (écriture, lecture et suppression)

### Chiffrement des données
Pour garantir une sécurité complète, toutes les données privées (non partagées) seront chiffrées.

### Accès client
- Accès via client (développé en C)

### Serveur de stockage
Le client se connecte au serveur pour envoyer et télécharger ses données

---

## Projet complet

### Gestion d'utilisateurs
- Authentification par clé privée (mot de passe) et publique
- Offres payante et freemium (octroie plus ou moins de stockage)

### Envoi de fichiers
Les utilisateurs inscrits sur le site pourront envoyer des fichiers depuis leur machine à partir d'un client de leur choix.

### Téléchargement de fichiers
Les fichiers envoyés par des utilisateurs seront téléchargeables à tout moment.

### Partage de fichiers
Une fois les fichiers envoyés sur le réseau, ces derniers pourront être rendus accessibles à d'autres utilisateurs.

- Gestion des droits  
Le propriétaire peut choisir de donner un accès en lecture ou écriture sur un de ses fichiers à un autre utilisateur.
- Entre utilisateurs
- A des personnes non inscrites (ou utilisateurs anonymes)  
Pour ceux qui n'ont pas de compte sur la plateforme, il leur sera tout de même possible d'accéder aux fichiers partagés par un utilisateur inscrit.
- De façon publique (sans chiffrement)
Les données publiques sont accéssible sans aucun chiffrement par n'importe qui.
    - Utilisation du service comme CDN  
    Les données seront distribuées sur plusieurs nodes à travers le monde pour accélerer le transfert de ces dernières.
    - Mode "réseau social"  
    Les fichiers partagés en mode publique (avec une option supplémentaire) seront répertoriés et visible par tous les utilisateurs inscrits, un peu comme sur un réseau social.
    - Modération des fichiers partagés  
    Pour éviter tout litige, les fichiers partagés de façon publique devont être analysés et devront être du contenu légal.

### Chiffrement des données
Pour garantir une sécurité complète, toutes les données privées (non partagées) seront chiffrées.

### Réseau de serveurs de stockage (nodes)
Une node stocke les données des utilisateurs et sont contrôlées par le serveur maitre. Elles peuvent également communiquer entre elles.

- Découpage des données  
Les données sont découpées en petit morceaux et réparties sur plusieurs nodes pour garantir la sécurité.
- Redondance des données  
En cas d'indisponibilité d'une node, les données sont stockées en double sur une ou plusieurs autres nodes.
- Répartition des données en fonction des nodes les plus proches de l'utilisateur  
Pour garantir un débit suffisant, les nodes sont choisies en fonction de leur position géographique par rapport à l'utilisateur.

### Statistiques
Quelques statistiques seront collectés sur les nodes et les utilisateurs pour améliorer le service et garantir la sécurité de ce dernier.

- Position géographique
- Machine/client utilisé
- Heure et date de connexion
- Quantité de fichiers envoyés/reçus

### Accès client
- Accès en CLI/SSH/SFTP
- Accès via client (développé en C)
- Interface web
- Application mobile

### Serveur maitre
Le serveur maitre possède une base de donnée.
- Indexe les nodes
    - Répartie la charge sur les nodes (load balancing)  
    Il ne faut pas qu'une node ait plus de demande qu'une autre, le serveur maitre se doit de répartir la charge sur toutes les nodes.
- Contient la liste des utilisateurs inscrits
