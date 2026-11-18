#include "./tini.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

const char*
parseTableNameFromFileLine(const char* line, IniErr* tini_err) {
  // checks if the line contains the open and closing table name
  size_t openBoundSize  = strlen(INIT_TABLE_SUB_OPEN_BOUND);
  size_t closeBoundSize = strlen(INIT_TABLE_SUB_CLOSE_BOUND);
  if (strncmp(&line[0], INIT_TABLE_SUB_OPEN_BOUND, openBoundSize) != 0) {
    printf("Invalid initial line char %s", &line[0]);
    *tini_err = TINI_INVALID_TINI_TABLE_NOTATION;
    return NULL;
  }
  if (strncmp(&line[strlen(line)-1], INIT_TABLE_SUB_CLOSE_BOUND, closeBoundSize) != 0) {
    printf("Invalid end line char %s", &line[strlen(line)]);
    *tini_err = TINI_INVALID_TINI_TABLE_NOTATION;
    return NULL; // invalid table name without closing bounds
  }
  size_t tableNameSize = (strlen(line) - (openBoundSize + closeBoundSize));
  char* tableName = (char*) malloc(sizeof(char) * (tableNameSize+1));
  strncpy(tableName, &line[openBoundSize], tableNameSize);
  *tini_err = TINI_SUCCESS;
  return tableName;
}

static IniType
parseTypeToIniType(const char* type, IniErr* tini_err) {
  unsigned int isString = strcmp(type, INIT_ITEM_TYPE_STRING) == 0;
  unsigned int isInt = strcmp(type, INIT_ITEM_TYPE_INT) == 0;
  unsigned int isUInt = strcmp(type, INIT_ITEM_TYPE_UINT) == 0;
  unsigned int isDouble = strcmp(type, INIT_ITEM_TYPE_DOUBLE) == 0;
  unsigned int isFloat = strcmp(type, INIT_ITEM_TYPE_FLOAT) == 0;
  unsigned int isBool = strcmp(type, INIT_ITEM_TYPE_BOOL) == 0;


  if (isString)
    return TINI_String;
  if (isInt)
    return TINI_Integer;
  if (isUInt)
    return TINI_UInteger;
  if (isFloat)
    return TINI_Float;
  if (isDouble)
    return TINI_Double;
  if (isBool)
    return TINI_Bool;
  return TINI_Unknown;
}

static bool
isDigit(char c) {
  return c >= '0' && c <= '9';
}

static int
strToInt(const char* value, IniErr* tini_err) {
  int result = 0;
  int fold10 = 1;
  for(int i = strlen(value)-1; i >= 0; i--) {
    if (value[i] == '_') {
      continue;
    }
    if (!isDigit(value[i])) {
        *tini_err = TINI_INVALID_NUMERIC_CHAR;
        return 0;
    }
    result += (value[i] - '0') * fold10;
    fold10 *= 10;
  }
  *tini_err = TINI_SUCCESS;
  return result;
}

static double
tiniPow(double base, int power) {
  if (power == 0) {
    return 1;
  }
  if (power < 0) {
    double result = 1;
    for (int i = power; i < 0; i++) {
      result /= base;
    }
    return result;
  }
  double result = base;
  for (int i = 1; i < power; i++) {
    result *= base;
  }
  return result;
}

static double
strToDouble(const char* value, IniErr* tini_err) {
  double result = 0;
  int fold10 = 1;
  const char* dot = strstr(value, ".");
  if (dot == NULL) {
    return (double) strToInt(value, tini_err);
  }
  int unfold10 = 0;
  for(int j = 0; j <= strlen(&dot[1]); j++) {
    if (dot[j] == '.' || dot[j] == '_') {
      continue;
    }
    unfold10++;
  }
  for(int i = strlen(value)-1; i >= 0; i--) {
    if (value[i] == '.' || value[i] == '_') {
      continue;
    }
    if (!isDigit(value[i])) {
      *tini_err = TINI_INVALID_NUMERIC_CHAR;
      return 0;
    }
    result += (value[i] - '0') * fold10;
    fold10 *= 10;
  }
  *tini_err = TINI_SUCCESS;
  return result * tiniPow(10, -unfold10);
}

static bool
strToBool(const char* str, IniErr* tini_err) {
  *tini_err = TINI_SUCCESS;
  if (strcmp(str, INIT_BOOLEAN_TRUE) == 0)
      return true;
  else if (strcmp(str, INIT_BOOLEAN_FALSE) == 0)
      return false;
  else  {
    *tini_err = TINI_INVALID_BOOL_VALUE;
    return false;
  }
}

static IniValue
parseIniItemValue(const char* value, IniType type, IniErr* tini_err) {
  IniValue val = {};
  *tini_err = TINI_SUCCESS;
  switch(type) {
    case TINI_Integer:
      val.integer = strToInt(value, tini_err);
      break;
    case TINI_UInteger: 
      val.uint = (unsigned int)strToInt(value, tini_err);
      break;
    case TINI_Float:
      val._float = (float)strToDouble(value, tini_err);
      break;
    case TINI_Double:
      val._double = strToDouble(value, tini_err);
      break;
    case TINI_Bool:
      val.boolean = strToBool(value, tini_err);
      break;
    case TINI_String:
      val.string = (char*) malloc(sizeof(char) * (strlen(value)+1));
      strcpy(val.string, value);
      break;
    default:
      *tini_err = TINI_UNKNOWN_VALUE_PARSE;
  }
  return val;
}

void
ltrim(char* c) {
  int nonspc = -1;
  for(int i = 0; i <= strlen(c); i++) {
    if (c[i] != ' ') {
      nonspc = i;
      break;
    }
  }
  if (nonspc == -1) {
    c[0] = '\0';
    return;
  }
  for(int j = nonspc; j < strlen(c); j++) {
    c[j - nonspc] = c[j];
  }
  c[strlen(c)-nonspc] = '\0';
}

void
rtrim(char* c) {
  int i;
  for (i = strlen(c)-1; i > 0; i--) {
    if (c[i] != ' ') {
      break;
    }
  }
  c[i+1] = '\0';
}

const IniItem
parseIniItemLine(const char* line, IniErr* tini_err) {
  IniItem item = {};
  const char* typesign = strstr(line, INIT_TYPECAST_SIGN);
  if (typesign == NULL) {
    *tini_err = TINI_INVALID_TINI_ITEM_TYPE_NOTATION;
    printf("type cast sign not found");
    return item;
  }
  const char* valsign = strstr(line, INIT_VALUESIGN);
  if (valsign == NULL) {
    *tini_err = TINI_INVALID_TINI_ITEM_VALUE_NOTATION;
    printf("value cast sign not found");
    return item;
  }

  char* name = (char*) malloc(sizeof(char) * (typesign-line+1));
  strncpy(name, line, (typesign-line));
  name[typesign-line] = '\0';
  ltrim(name);
  rtrim(name);
  
  char* value = (char*) malloc(sizeof(char) * (valsign-line+1));
  strncpy(value, valsign+1, (valsign-line));
  value[valsign-line-1] = '\0';
  ltrim(value);
  rtrim(value);

  char* rawType = (char*) malloc(sizeof(char) * (valsign-typesign+1));
  strncpy(rawType, typesign+1, (valsign-typesign-1));
  rawType[valsign-typesign-1] = '\0';
  ltrim(rawType);
  rtrim(rawType);

  IniType type = parseTypeToIniType(rawType, tini_err);
  free(rawType);

  IniValue typedValue = parseIniItemValue(value, type, tini_err);

  // parse item value
  item = (IniItem){
    .name = name,
    .rawValue = value,
    .type = type,
    .typedValue = typedValue,
  };
  
  return item;
}

void
freeIniValue(IniValue val, IniType type) {
  if (type == TINI_String)
    free(val.string);
}

void
freeIniItem(IniItem *item) {
  free(item->name);
  free(item->rawValue);
  freeIniValue(item->typedValue, item->type);
}
typedef struct {
  char** vec;
  size_t size;
} strVec;

static strVec
splitTableNameSections(char* tableName) {
  int sectionsCount = 0;
  int* sectionsPositions = (int*) malloc(sizeof(int) * strlen(tableName));
  for(int j = 0; j < strlen(tableName); j++) {
    if(tableName[j] == '.') {
      sectionsPositions[sectionsCount] = j;
      sectionsCount++;
    }
  }
  int lastPos = 0;
  char** result = (char**)malloc(sizeof(char*) * (sectionsCount+1));
  for(int i = 0; i < sectionsCount; i++) {
    int pos = sectionsPositions[i];
    char* sectionName = (char*)malloc(sizeof(char) * (pos - lastPos + 1));
    strncpy(sectionName, &tableName[lastPos], pos - lastPos);
    sectionName[pos-lastPos] = '\0';
    result[i] = sectionName;
    lastPos = pos+1;
  }
  free(sectionsPositions);
  if (lastPos != strlen(tableName)) {
    char* sectionName = (char*)malloc(sizeof(char) * (strlen(tableName) - lastPos + 1));
    strncpy(sectionName, &tableName[lastPos], strlen(tableName) - lastPos);
    sectionName[strlen(tableName)-lastPos] = '\0';
    result[sectionsCount] = sectionName;
    sectionsCount++;
  }
  return (strVec){ .vec=result, .size=sectionsCount };
}

void
setTableSubsections(IniTable target, IniTableV tables, IniErr* tini_err) {
  
}
