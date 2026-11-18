#include "src/tini.h"
#include <string.h>
#include <stdlib.h>

int main() {
  char str[20] = "section.name";
  strVec splitTable = splitTableNameSections(str);
  for(int i = 0; i < splitTable.size; i++) {  
    char* section = splitTable.vec[i];
    printf("%s\n", section);
    free(section);
  }
  free(splitTable.vec);
  return 0;
}
