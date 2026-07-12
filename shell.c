// unix shell en C

// commande pour compiler
// gcc -o shell shell.c
// puis lancer la commande ./shell pour lance le programme
// man 3p pour approfondir sur les syscall
// aussi POSIX specification section 13
//
// Ce Shell n'a pour le moment pas beaucoup de fonction implémenté

// déclaration des includes
#include <signal.h> // afin de gerer ctrl+c et les SIGINT
#include <stdio.h>  // printf, fprintf, perror
#include <stdlib.h> // malloc, realloc, free, exit, EXIT_SUCCESS, EXIT_FAILURE
#include <string.h> // strtok, strcmp
#include <sys/types.h> // pid_t
#include <sys/wait.h>  // waitpid, WUNTRACED, WIFEXITED, WIFSIGNALED
#include <unistd.h>    // fork, execvp, chdir

// déclaration des fonctions
char *shell_read_line(void);
char **shell_split_line(char *line);
int shell_execute(char **args);
void shell_loop(void);
int shell_launch(char **args);
// déclaration des fonctions construit/intégré pour le shell directement sans
// passer par fork
int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);

// utilisation de syscall, fork(), pou dupliquer

int shell_launch(char **args) {

  pid_t pid, wpid;
  int status;
  pid = fork();

  if (pid == 0) {
    // nouveau proc fils
    // on fait en sort que le fils ne change pas son comportement au ctrl+c
    signal(SIGINT, SIG_DFL);

    if (execvp(args[0], args) == -1) {
      perror("SHELL");
    }
    exit(EXIT_FAILURE);

  } else if (pid < 0) {
    // erreur sur la création du proc
    perror("SHELL");

  } else {
    // proc parent
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
      // si waitpid renvoi -1 erreur fixer avec la verification qui nous fais
      // sortir de la boucle
    } while (!WIFEXITED(status) && !WIFSIGNALED(status) && wpid != -1);
  }

  return 1;
}

// Liste des fonctions intégré
char *list_func_integre_str[] = {"cd", "help", "exit"};

int (*integre_func[])(char **) = {&shell_cd, &shell_help, &shell_exit};

int shell_num_funcinte() {
  return sizeof(list_func_integre_str) / sizeof(char *);
}

// Maintenant du coup on créé les fonctions

int shell_cd(char **args) {

  if (args[1] == NULL) {
    fprintf(stderr, "Erreur dans les arguments attendu,\"cd\" \n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("shell_cd");
    }
  }

  return 1;
}

int shell_help(char **args) {
  int i;
  printf("C'est mon shell, Blonin'shell ");
  printf("Il suffit de taper le nom d'une commande avec les arguments et "
         "appuyer sur entrer\n");
  printf("Les commandes pré-intégré sont les suivant :\n");
  for (i = 0; i < shell_num_funcinte(); i++) {
    printf(" %s\n", list_func_integre_str[i]);
  }
  printf("Utilisez la command man pour une information sur les autre commands");

  return 1;
}

// la plus facile pour la fin
int shell_exit(char **args) { return 0; }

// Crétion de la func exec

int shell_execute(char **args) {
  int i;

  // verification que la commande demandé n'est pas vide
  // Si elle est vide on renvoi 1 et rien ne se passe
  if (args[0] == NULL) {
    return 1;
  }

  for (i = 0; i < shell_num_funcinte(); i++) {
    if (strcmp(args[0], list_func_integre_str[i]) == 0) {
      return (*integre_func[i])(args);
    }
  }

  return shell_launch(args);
}

int main(int argc, char **argv) {
  // chargement de fichiers de configuration a faire ici
  // comme l'apparence du shell

  // le shell principal ignore le ctrl+c
  signal(SIGINT, SIG_IGN);

  // lancement de la comande loop
  shell_loop();

  // lancement de cleanup si nécessaire a faire ici

  return EXIT_SUCCESS;
}

// il faut que je vérifie les possibilités de mettre les diférentes fonctions
// dans différents fichiers pour possiblement moins se perdre
//

void shell_loop(void) {
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = shell_read_line();
    if (line == NULL) {
      printf("\n");
      break;
    }

    args = shell_split_line(line);
    status = shell_execute(args);

    free(line);
    free(args);

  } while (status);
}

// on alloue un gros bloc au cas où la commande serait grande si elle depasse il
// faudra realoué de l'espace en temps réel :/
#define SHELL_RL_BUFSIZE 1024

char *shell_read_line(void) {
  int bufsize = SHELL_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, " ERREUR D'ALLOCATION DU BUFFER\n");
    exit(EXIT_FAILURE);
  }

  // faire une condition d'arret?
  while (1) {
    // lecture d'un caractère
    c = getchar();

    // si on arrive a la fin de la ligne, \n, ou a la fin d'un fichier, EOF, on
    // dois le remplacer par un caractère nul ne pas oublier que EOF est un int
    // et pas un carac du coup la comparaison est différente
    if (c == EOF) {
      if (position == 0) {
        free(buffer);
        return NULL;

      } else {
        buffer[position] = '\0';
        return buffer;
      }
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;

    } else {
      buffer[position] = c;
    }

    position++;
    // on vérifie avant de repartir dans la boucle
    // si on a dépassé la capacité du buffer, si oui, on doit le réalouer

    if (position >= bufsize) {
      bufsize += SHELL_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, " ERREUR D'ALLOCATION DU BUFFER\n");
        exit(EXIT_FAILURE);
      }
    }
    // possibilité d'améliorer le code avec la commande getline()
    // a voir comment l'utiliser et l'implémenter a la place
  }
}

// On se dit que les "" ne seront pas utilisé ainsi que les \
// On part du principe que les espace seront les délimiteurs de chaque arguments
// On enregistre chaque token avec un pointeur pour chaque puis dans une liste
#define SHELL_TOK_BUFFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"
char **shell_split_line(char *line) {
  int bufsize = SHELL_TOK_BUFFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token;

  if (!tokens) {
    fprintf(stderr, "Shell tokens : erreur d'allocation\n");
    exit(EXIT_FAILURE);
  }
  token = strtok(line, SHELL_TOK_DELIM);

  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += SHELL_TOK_BUFFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char *));
      if (!tokens) {
        fprintf(stderr, "SHELL tokens : erreur de REalloc");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, SHELL_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}
