#include "src/tini.h"


int main() {
  const char* itemLine = "name: str = \"true\"";
  IniErr err;
  IniItem ival = parseIniItemLine(itemLine, &err);
  if (err != TINI_SUCCESS) {
    printf("error validating double string: %d\n", err);
    return 1;
  }
  printf("%s\n", ival.typedValue.string);
  freeIniItem(&ival);
  return 0;
}
