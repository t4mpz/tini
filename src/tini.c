#include "./tini.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

static void
okTini(IniErr* tini_err) {
  tini_err->code = TINI_SUCCESS;
  memset(tini_err->msg, '\0', 1024);
}


IniErr
parseTableNameFromFileLine(const char* line, char* out) {
  // checks if the line contains the open and closing table name
  size_t openBoundSize  = strlen(INIT_TABLE_SUB_OPEN_BOUND);
  size_t closeBoundSize = strlen(INIT_TABLE_SUB_CLOSE_BOUND);
  IniErr tini_err;
  if (strncmp(&line[0], INIT_TABLE_SUB_OPEN_BOUND, openBoundSize) != 0) {
    printf("Invalid initial line char %s", &line[0]);
    tini_err.code = TINI_INVALID_TINI_TABLE_NOTATION;
    sprintf(tini_err.msg, "Invalid Table Line notation %s, use notation %stable-name%s", line, INIT_TABLE_SUB_OPEN_BOUND, INIT_TABLE_SUB_CLOSE_BOUND);
  }
  if (strncmp(&line[strlen(line)-1], INIT_TABLE_SUB_CLOSE_BOUND, closeBoundSize) != 0) {
    printf("Invalid end line char %s", &line[strlen(line)]);
    tini_err.code = TINI_INVALID_TINI_TABLE_NOTATION;
  }
  size_t tableNameSize = (strlen(line) - (openBoundSize + closeBoundSize));
  char* tableName = (char*) malloc(sizeof(char) * (tableNameSize+1));
  strncpy(tableName, &line[openBoundSize], tableNameSize);
  okTini(&tini_err);
  out = tableName;
  return tini_err; 
}

static void
parseTypeToIniType(const char* type, IniType* out) {
  unsigned int isString = strcmp(type, INIT_ITEM_TYPE_STRING) == 0;
  unsigned int isInt = strcmp(type, INIT_ITEM_TYPE_INT) == 0;
  unsigned int isUInt = strcmp(type, INIT_ITEM_TYPE_UINT) == 0;
  unsigned int isDouble = strcmp(type, INIT_ITEM_TYPE_DOUBLE) == 0;
  unsigned int isFloat = strcmp(type, INIT_ITEM_TYPE_FLOAT) == 0;
  unsigned int isBool = strcmp(type, INIT_ITEM_TYPE_BOOL) == 0;


  if (isString)
    * out = TINI_String;
  if (isInt)
    *out = TINI_Integer;
  if (isUInt)
    *out = TINI_UInteger;
  if (isFloat)
    *out = TINI_Float;
  if (isDouble)
    *out = TINI_Double;
  if (isBool)
    *out = TINI_Bool;
  *out = TINI_Unknown;

}

static bool
isDigit(char c) {
  return c >= '0' && c <= '9';
}

static IniErr
strToInt(const char* value, int* out) {
  int result = 0;
  int fold10 = 1;
  IniErr tini_err;
  for(int i = strlen(value)-1; i >= 0; i--) {
    if (value[i] == '_') {
      continue;
    }
    if (!isDigit(value[i])) {
        tini_err.code = TINI_INVALID_NUMERIC_CHAR;
        sprintf(tini_err.msg, "Invalid non numeric char received at value %s", value);
    }
    result += (value[i] - '0') * fold10;
    fold10 *= 10;
  }
  *out = result;
  okTini(&tini_err);
  return tini_err;
}

static IniErr
strToUInt(const char* value, unsigned int* out) {
  int result = 0;
  int fold10 = 1;
  IniErr tini_err;
  for(int i = strlen(value)-1; i >= 0; i--) {
    if (value[i] == '_') {
      continue;
    }
    if (!isDigit(value[i])) {
        tini_err.code = TINI_INVALID_NUMERIC_CHAR;
        sprintf(tini_err.msg, "Invalid non numeric char received at value %s", value);
    }
    result += (value[i] - '0') * fold10;
    fold10 *= 10;
  }
  if (result < 0) {
    tini_err.code = TINI_INVALID_SIGNED_NUMERIC_VALUE;
    sprintf(tini_err.msg, "Invalid signed numeric char received at value %d expected unsigned integer", result);
    return tini_err;
  }
  *out = result;
  okTini(&tini_err);
  return tini_err;
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

static IniErr
strToDouble(const char* value, double* out) {
  double result = 0;
  int fold10 = 1;
  const char* dot = strstr(value, ".");
  IniErr tini_err;
  if (dot == NULL) {
    int iout;
    tini_err = strToInt(value, &iout);
    if (tini_err.code != TINI_SUCCESS) {
      return tini_err;
    }
    *out = (double) iout;
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
      tini_err.code = TINI_INVALID_NUMERIC_CHAR;
      sprintf(
          tini_err.msg, 
          " Invalid non numeric (double) character '%c' received for numerically typed field",
          value[i]
        );
      return tini_err;
    }
    result += (value[i] - '0') * fold10;
    fold10 *= 10;
  }
  okTini(&tini_err);
  *out =  result * tiniPow(10, -unfold10);
  return tini_err;
}

static IniErr
strToFloat(const char* value, float* out) {
  double result = 0;
  int fold10 = 1;
  const char* dot = strstr(value, ".");
  IniErr tini_err;
  if (dot == NULL) {
    int iout;
    tini_err = strToInt(value, &iout);
    if (tini_err.code != TINI_SUCCESS) {
      return tini_err;
    }
    *out = (double) iout;
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
      tini_err.code = TINI_INVALID_NUMERIC_CHAR;
      sprintf(
          tini_err.msg, 
          " Invalid non numeric (double) character '%c' received for numerically typed field",
          value[i]
        );
      return tini_err;
    }
    result += (value[i] - '0') * fold10;
    fold10 *= 10;
  }
  okTini(&tini_err);
  *out =  (float)(result * tiniPow(10, -unfold10));
  return tini_err;
}

static IniErr
strToBool(const char* str, bool* out) {
  IniErr tini_err;
  if (strcmp(str, INIT_BOOLEAN_TRUE) == 0)
      *out = true;
  else if (strcmp(str, INIT_BOOLEAN_FALSE) == 0)
      *out = false;
  else  {
    tini_err.code = TINI_INVALID_BOOL_VALUE;
    sprintf(tini_err.msg, "Invalid tini boolean value received %s", str);
    return tini_err;
  }
  okTini(&tini_err);
  return tini_err;
}

static IniErr
parseIniItemValue(const char* value, IniType type, IniValue* out) {
  IniErr tini_err;
  okTini(&tini_err);
  switch(type) {
    case TINI_Integer:
      tini_err = strToInt(value, &out->integer);
      break;
    case TINI_UInteger: 
      tini_err = strToUInt(value, &out->uint);
      break;
    case TINI_Float:
      tini_err = strToFloat(value, &out->_float);
      break;
    case TINI_Double:
      tini_err = strToDouble(value, &out->_double);
      break;
    case TINI_Bool:
      tini_err = strToBool(value, &out->boolean);
      break;
    case TINI_String:
      out->string = strdup(value);
      if (out->string == nullptr) {
        tini_err.code = TINI_OUT_OF_MEMORY;
        sprintf(tini_err.msg, " parseIniItemValue :> Invalid null string received ");
      }
      break;
    default:
      tini_err.code = TINI_UNKNOWN_VALUE_PARSE;
      sprintf(tini_err.msg, " parseIniItemValue :> Invalid value received %s ", value);
  }
  return tini_err;
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

IniErr
parseIniItemLine(const char* line, IniItem* out) {
  IniErr tini_err;
  okTini(&tini_err);
  const char* typesign = strstr(line, INIT_TYPECAST_SIGN);
  if (typesign == NULL) {
    tini_err.code = TINI_INVALID_TINI_ITEM_TYPE_NOTATION;
    sprintf(
        tini_err.msg, 
        " Invalid untyped line %s, use %s to cast types in a field like field %s:string %s yourvalue ", 
        line,
        INIT_TYPECAST_SIGN,
        INIT_TYPECAST_SIGN,
        INIT_VALUESIGN
    );
    return tini_err;
  }
  const char* valsign = strstr(line, INIT_VALUESIGN);
  if (valsign == NULL) {
    tini_err.code = TINI_INVALID_TINI_ITEM_VALUE_NOTATION;
    sprintf(
        tini_err.msg, 
        " Invalid value notation line %s, use %s to assign values to a field like field %s:string %s yourvalue ", 
        line,
        INIT_TYPECAST_SIGN,
        INIT_TYPECAST_SIGN,
        INIT_VALUESIGN
    );
    return tini_err;
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
  IniType type;

  parseTypeToIniType(rawType, &type);
  free(rawType);

  IniValue typedValue; 
  tini_err = parseIniItemValue(value, type, &typedValue);

  if(tini_err.code != TINI_SUCCESS) {
    return tini_err;
  }

  // parse item value
  *out = (IniItem){
    .name = name,
    .rawValue = value,
    .type = type,
    .typedValue = typedValue,
  };
  
  return tini_err;
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

