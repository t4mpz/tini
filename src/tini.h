#ifndef TINI_H 
#define TINI_H
#include <stdio.h>
#include <stdbool.h>
constexpr char INIT_TABLE_SUB_OPEN_BOUND  = '[';
constexpr char INIT_TABLE_SUB_CLOSE_BOUND = ']';
constexpr char  INIT_TYPECAST_SIGN = ':';
constexpr char INIT_VALUESIGN = '=';
constexpr char INIT_ITEM_TYPE_STRING[] = "str";
constexpr char INIT_ITEM_TYPE_UINT[] = "uint";
constexpr char INIT_ITEM_TYPE_INT[] = "int";
constexpr char INIT_ITEM_TYPE_DOUBLE[] = "double";
constexpr char INIT_ITEM_TYPE_FLOAT[] = "float";
constexpr char INIT_ITEM_TYPE_BOOL[] = "bool";
constexpr char INIT_ITEM_TYPE_UNDEF[] = "unknown";
constexpr char INIT_BOOLEAN_FALSE[] = "false";
constexpr char INIT_BOOLEAN_TRUE[] = "true";
constexpr unsigned int TINI_ERR_MESSAGE_MAX_LEN = 1024;


constexpr char INIT_TABLE_SUB_DELIMITER = '.';

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
  size_t capacity;
} IniItemV;

typedef struct IniTable IniTable;

typedef struct {
  IniTable* tables;
  size_t count;
  size_t capacity;
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
  TINI_INVALID_SIGNED_NUMERIC_VALUE = -9,
  TINI_OUT_OF_MEMORY = -10,
} TINI_ERR_CODE;

typedef struct {
  TINI_ERR_CODE code;
  char msg[1024];
} IniErr;

IniErr
IniAsString(const IniItem item, char* out); 

IniErr 
IniAsInt(const IniItem item, int* out); 

IniErr 
IniAsUInt(const IniItem item, unsigned int* out); 

IniErr 
IniAsDouble(const IniItem item, double* out); 

IniErr 
IniAsFloat(const IniItem item, float* ierr); 

IniErr
IniAsBoolean(const IniItem item, bool* out);

IniErr 
parseTini(FILE* file);

IniErr
parseTableNameFromFileLine(const char* line, char* out);

IniErr
parseIniItemLine(const char* line, IniItem* out);

void
setTableSubsections(IniTable target, IniTableV tables, IniErr* tini_err);

void
freeIniItem(IniItem* item);

void
freeIniTable(IniTable* table);

void
freeIniTableV(IniTableV tables);

void
freeIniItemV(IniItemV items);

IniErr
parseTiniContent(char* content, IniTableV* out);

#endif
