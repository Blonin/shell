


int main(int argc, char **argv){
  //chargement de fichiers de configuration a faire ici
  //comme l'apparence du shell


  //lancement de la comande loop
  shell_loop();

  // lancement de cleanup si nécessaire a faire ici
  
  return EXIT_SUCCESS;
}

//possibilité de mettre les diférentes void dans diférents fichiers pour possiblement moins se perdre
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

    //si on arrive a la fin de la ligne, EOF, on dois le remplacer par un caractère nul
    if ( c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    }else{
      buffer[position] = c;
    }
    position++;

    //si on a dépassé la capacité du buffer, on doit le réalouer
    if(position >= bufsize){
      bufsize += SHELL_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer){
        fprintf(stderr, " ERREUR D'ALLOCATION DU BUFFER\n");
        exit(EXIT_FAILURE);
      }

    }

  }

}
