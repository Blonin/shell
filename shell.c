//unix shell en C


int main(int argc, char **argv){
  //chargement de fichiers de configuration a faire ici
  //comme l'apparence du shell


  //lancement de la comande loop
  shell_loop();

  // lancement de cleanup si nécessaire a faire ici
  
  return EXIT_SUCCESS;
}

//il faut que je vérifie les possibilités de mettre les diférentes fonctions dans différents fichiers pour possiblement moins se perdre
//

void shell_loop(void){
  char *line;
  char **args;
  int status;
  
  do {
    printf("> ");
    line = shell_read_line();
    args = shell_split_line(line);
    status = shell_execute(args);

    free(line);
    free(args);

  }while(status);
}


// on alloue un gros bloc au cas où la commande serait grande si elle depasse il faudra realoué de l'espace en temps réel :/
#define SHELL_RL_BUFSIZE 1024

char *shell_read_line(void){
  int bufsize = SHELL_RL_BUFSIZE;
  int position = 0;
  char *buffer =malloc(sizeof(char)* bufsize);
  int c;

  if (!buffer){
    fprintf(stderr, " ERREUR D'ALLOCATION DU BUFFER\n");
    exit(EXIT_FAILURE);
  }

  //faire une condition d'arret?
  while(1){
    //lecture d'un caractère
    c = getchar();

    //si on arrive a la fin de la ligne, \n, ou a la fin d'un fichier, EOF, on dois le remplacer par un caractère nul
    //ne pas oublier que EOF est un int et pas un carac du coup la comparaison est différente
    if ( c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    }else{
      buffer[position] = c;
    }

    position++;
    //on vérifie avant de repartir dans la boucle
    //si on a dépassé la capacité du buffer, si oui, on doit le réalouer
    
    if(position >= bufsize){
      bufsize += SHELL_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer){
        fprintf(stderr, " ERREUR D'ALLOCATION DU BUFFER\n");
        exit(EXIT_FAILURE);
      }

    }
    // possibilité d'améliorer le code avec la commande getline() 
    // a voir comment l'utiliser et l'implémenter a la place
  
  }
}

//On se dit que les "" ne seront pas utilisé ainsi que les \
//On part du principe que les espace seront les délimiteurs de chaque arguments
//On enregistre chaque token avec un pointeur pour chaque puis dans une liste
#define SHELL_TOK_BUFFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"
char **shell_split_line(char *line){
  int bufsize = SHELL_TOK_BUFFSIZE, position=0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if(!tokens){
    fprintf(stderr, "Shell tokens : erreur d'allocation\n");
    exit(EXIT_FAILURE);
  }
  token = strtok(line, SHELL_TOK_DELIM);

  while (token != NULL) {
    tokens[position] = token;
    position++;

    if(position >= bufsize) {
      bufsize += SHELL_TOK_BUFFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if(!tokens) {
        fprintf (stderr, "SHELL tokens : erreur de REalloc");
        exit(EXIT_FAILURE);
      }
    }

    token =strtok(NULL, SHELL_TOK_DELIM);
  }
  tokens[position]=NULL;
  return tokens;
}





