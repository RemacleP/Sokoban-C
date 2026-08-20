#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>

typedef struct { int x, y; } Position;

// Affichage avec la hauteur (h) et la largeur (w) dynamiques
void afficherCarte(char** carte, int h) {
    system("cls");
    for (int i = 0; i < h; i++) {
        printf("%s\n", carte[i]);
    }
    printf("\nZ:Haut, S:Bas, Q:Gauche, D:Droite, ESC:Quitter\n");
}

Position trouverJoueur(char** carte, int h, int w) {
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            if (carte[y][x] == '@') return (Position) { x, y };
    return (Position) { -1, -1 };
}

int verifierVictoire(char** carte, int h, int w) {
    int caissesSimples = 0;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (carte[y][x] == '$') caissesSimples++;
        }
    }
    return (caissesSimples == 0);
}

int estCoinceeDansCoin(char** carte, int x, int y, int h, int w) {
    if (carte[y][x] == '*') return 0;
    // Vérifier les limites pour éviter les dépassements de tableau
    int haut = (y > 0 && carte[y - 1][x] == '#');
    int bas = (y < h - 1 && carte[y + 1][x] == '#');
    int gauche = (x > 0 && carte[y][x - 1] == '#');
    int droite = (x < w - 1 && carte[y][x + 1] == '#');
    return ((haut || bas) && (gauche || droite));
}

// Fonction pour compter les niveaux et récupérer leurs tailles
int compterTotalNiveaux() {
    FILE* f = fopen("niveaux.txt", "r");
    if (!f) return 0;
    int h, l, total = 0;
    char buffer[256];
    while (fscanf(f, "%d %d", &h, &l) != EOF) {
        fgets(buffer, sizeof(buffer), f);
        for (int i = 0; i < h; i++) {
            fgets(buffer, sizeof(buffer), f);
        }
        total++;
    }
    fclose(f);
    return total;
}

// Charger un niveau de taille variable en allouant dynamiquement la mémoire
int chargerNiveau(int index, char*** carte, int* h, int* w) {
    FILE* f = fopen("niveaux.txt", "r");
    if (!f) return 0;

    int currentH, currentW, n = 0;
    char buffer[256];

    while (fscanf(f, "%d %d", &currentH, &currentW) != EOF) {
        fgets(buffer, sizeof(buffer), f);

        if (n == index) {
            *h = currentH;
            *w = currentW;

            // Allocation dynamique de la grille (tableau de pointeurs)
            *carte = (char**)malloc((*h) * sizeof(char*));
            for (int i = 0; i < *h; i++) {
                (*carte)[i] = (char*)malloc((*w + 1) * sizeof(char));
            }

            // Lecture des lignes du niveau
            for (int i = 0; i < *h; i++) {
                if (fgets(buffer, sizeof(buffer), f) != NULL) {
                    int j = 0;
                    while (j < *w && buffer[j] != '\n' && buffer[j] != '\r' && buffer[j] != '\0') {
                        (*carte)[i][j] = buffer[j];
                        j++;
                    }
                    (*carte)[i][j] = '\0';
                }
            }
            fclose(f);
            return 1;
        }
        else {
            for (int i = 0; i < currentH; i++) {
                fgets(buffer, sizeof(buffer), f);
            }
        }
        n++;
    }
    fclose(f);
    return 0;
}

// Libérer la mémoire de la carte à la fin de chaque niveau
void libererCarte(char** carte, int h) {
    for (int i = 0; i < h; i++) {
        free(carte[i]);
    }
    free(carte);
}

int main() {
    char** carte = NULL;
    int h = 0, w = 0;
    int niveau = 0;
    int totalNiveaux = compterTotalNiveaux();

    if (totalNiveaux == 0) {
        printf("Erreur : Impossible de lire le fichier niveaux.txt !\n");
        _getch();
        return 1;
    }

    while (chargerNiveau(niveau, &carte, &h, &w)) {
        int enJeu = 1;
        char derniereCase = ' ';

        while (enJeu) {
            afficherCarte(carte, h);
            if (verifierVictoire(carte, h, w)) {
                afficherCarte(carte, h);

                if (niveau >= totalNiveaux - 1) {
                    printf("\nBravo ! Niveau termine !\n");
                    printf("\nFelicitations ! Vous avez fini tous les niveaux !\n");
                    printf("Fermeture automatique dans 3 secondes...");
                    Sleep(3000);
                    libererCarte(carte, h);
                    return 0;
                }

                printf("\nBravo ! Niveau termine ! Passage au niveau suivant dans 3 secondes...");
                Sleep(3000);
                libererCarte(carte, h); // On libère l'ancien niveau avant de passer au suivant
                niveau++;
                enJeu = 0;
                break;
            }

            Position j = trouverJoueur(carte, h, w);
            char t = _getch();
            if (t == 27) {
                libererCarte(carte, h);
                return 0; // ESC pour quitter
            }

            int nx = j.x, ny = j.y;
            if (t == 'z') ny--; else if (t == 's') ny++;
            else if (t == 'q') nx--; else if (t == 'd') nx++;

            // Sécurité pour ne pas sortir des limites du tableau
            if (nx < 0 || nx >= w || ny < 0 || ny >= h) continue;

            char caseVisee = carte[ny][nx];

            if (caseVisee == ' ' || caseVisee == '.') {
                carte[j.y][j.x] = derniereCase;
                derniereCase = (caseVisee == '.') ? '.' : ' ';
                carte[ny][nx] = '@';
            }
            else if (caseVisee == '$' || caseVisee == '*') {
                int cx = nx + (nx - j.x);
                int cy = ny + (ny - j.y);

                // Vérifier que la caisse poussée reste dans les limites
                if (cx >= 0 && cx < w && cy >= 0 && cy < h) {
                    char caseArriereCaisse = carte[cy][cx];

                    if (caseArriereCaisse == ' ' || caseArriereCaisse == '.') {
                        char nouveauSymboleCaisse = (caseArriereCaisse == '.') ? '*' : '$';

                        carte[cy][cx] = nouveauSymboleCaisse;
                        carte[ny][nx] = '@';

                        if (caseVisee == '*') {
                            carte[j.y][j.x] = '.';
                        }
                        else {
                            carte[j.y][j.x] = derniereCase;
                        }

                        derniereCase = ' ';

                        if (nouveauSymboleCaisse == '$' && estCoinceeDansCoin(carte, cx, cy, h, w)) {
                            afficherCarte(carte, h);
                            printf("\nDefaite ! Caisse coincee. Fermeture dans 3 secondes...");
                            Sleep(3000);
                            libererCarte(carte, h);
                            return 0;
                        }
                    }
                }
            }
        }
    }
    return 0;
}