#pragma once
#include <stdio.h>
#include <stdbool.h>
#define INIT_TABLE_SUB_DELIMITER "."
#define INIT_TABLE_SUB_OPEN_BOUND "["
#define INIT_TABLE_SUB_CLOSE_BOUND "]"
#define INIT_TYPECAST_SIGN ":"
#define INIT_VALUESIGN "="
#define INIT_ITEM_TYPE_STRING "str"
#define INIT_ITEM_TYPE_UINT "uint"
#define INIT_ITEM_TYPE_INT "int"
#define INIT_ITEM_TYPE_DOUBLE "double"
#define INIT_ITEM_TYPE_FLOAT "float"
#define INIT_ITEM_TYPE_BOOL "bool"
#define INIT_ITEM_TYPE_UNDEF "unknown"
#define INIT_BOOLEAN_FALSE "false"
#define INIT_BOOLEAN_TRUE "true"


typedef enum {
  TINI_String = 0,
  TINI_Integer = 1,
  TINI_UInteger = 2,
  TINI_Double = 3,
  TINI_Float = 4,
  TINI_Bool = 5,
  TINI_Unknown = 6, // invalid type reserved enum item
} IniType;

typedef union {
  char* string;
  int integer;
  unsigned int uint;
  double _double;
  float _float;
  bool boolean;
} IniValue;

typedef struct {
  char* name;
  IniType type;
  char* rawValue;
  IniValue typedValue;
} IniItem;

typedef struct {
  IniItem* items;
  size_t count;
} IniItemV;

typedef struct IniTable IniTable;

typedef struct {
  IniTable* tables;
  size_t count;
} IniTableV;

struct IniTable {
  char* name;
  IniItemV items;
  IniTableV subs;
};

typedef enum {
  TINI_SUCCESS = 0,
  TINI_TYPECAST_MISMATCH = -1,
  TINI_INVALID_TINI_FILE = -2,
  TINI_INVALID_TINI_TABLE_NOTATION = -3,
  TINI_INVALID_TINI_ITEM_TYPE_NOTATION = -4,
  TINI_INVALID_TINI_ITEM_VALUE_NOTATION = -5,
  TINI_INVALID_NUMERIC_CHAR = -6,
  TINI_INVALID_BOOL_VALUE = -7,
  TINI_UNKNOWN_VALUE_PARSE = -8,
} IniErr;

char* 
IniAsString(const IniItem item, IniErr* ierr); 

int 
IniAsInt(const IniItem item, IniErr* ierr); 

unsigned int 
IniAsUInt(const IniItem item, IniErr* ierr); 

double 
IniAsDouble(const IniItem item, IniErr* ierr); 

float 
IniAsFloat(const IniItem item, IniErr* ierr); 

bool
IniAsBoolean(const IniItem item, IniErr* ierr);

IniErr 
parseTini(FILE* file);

const char*
parseTableNameFromFileLine(const char* line, IniErr* tini_err);

const IniItem
parseIniItemLine(const char* line, IniErr* tini_err);

void
freeIniItem(IniItem* item);

void
freeIniTable(IniTable* table);
