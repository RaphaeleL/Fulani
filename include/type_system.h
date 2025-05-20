#ifndef TYPE_SYSTEM_H
#define TYPE_SYSTEM_H

#include "runtime.h"

int type_check(Value* value, ValueType expected_type);
int type_compatible(ValueType type1, ValueType type2);
Value* type_convert(Value* value, ValueType target_type);
const char* type_to_string(ValueType type);

#endif /* TYPE_SYSTEM_H */ 