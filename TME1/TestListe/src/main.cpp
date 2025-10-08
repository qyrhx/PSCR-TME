#include "List.h"
#include <string>
#include <iostream>
#include <cstring>

int main() {
  std::string abc = "abc";
  // FAUTE: ajout d'une 4eme pour le 0 a la fin du string "abc"
  char *str       = new char[4];
  str[0]          = 'a';
  str[1]          = 'b';
  str[2]          = 'c';
  str[3]          = 0;
  // FAUTE: size_t est tjrs > 0 donc la boucle d'affichage ne fonctionne pas
  int i           = 0;

  if (! strcmp(str, abc.c_str())) {
    std::cout << "Equal!";
  }

  pr::List list;
  list.push_front(abc);
  list.push_front(abc);

  std::cout << "Liste : " << list << std::endl;
  std::cout << "Taille : " << list.size() << std::endl;

  // Affiche à l'envers
  for (i = list.size() - 1; i >= 0; i--) {
    std::cout << "elt " << i << ": " << list[i] << std::endl;
  }

  // FAUTE: on utilise delete[] et on libere pas chaque element de la liste ici
  delete[] str;
}
