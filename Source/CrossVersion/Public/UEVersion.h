﻿#pragma once

#include "Misc/EngineVersionComparison.h"

#define UE_VERSION_START(MajorVersion, MinorVersion, PatchVersion) \
	UE_GREATER_SORT(ENGINE_MAJOR_VERSION, MajorVersion, UE_GREATER_SORT(ENGINE_MINOR_VERSION, MinorVersion, UE_GREATER_SORT(ENGINE_PATCH_VERSION, PatchVersion, true)))

#define UE_T_IS_T_SUB_CLASS_OF !UE_VERSION_START(5, 0, 0)

#define UE_T_IS_T_SET !UE_VERSION_START(5, 0, 0)

#define UE_T_IS_T_MAP !UE_VERSION_START(5, 0, 0)

#define UE_T_IS_T_MAP !UE_VERSION_START(5, 0, 0)

#define UE_T_BASE_STRUCTURE_F_MATRIX !UE_VERSION_START(5, 0, 0)

#define UE_LINEAR_COLOR_COMPUTE_LUMINANCE !UE_VERSION_START(5, 0, 0)

#define UE_TUPLE_EXPLICIT_CONSTRUCTOR !UE_VERSION_START(5, 0, 0)

#define UE_LINEAR_COLOR_QUANTIZE_FLOOR UE_VERSION_START(5, 0, 0)

#define UE_INTERFACE_PROPERTY_SET_PROPERTY_VALUE !UE_VERSION_START(5, 0, 0)

#define UE_OBJECT_PROPERTY_STATIC_IDENTICAL UE_VERSION_START(5, 0, 0)

#define UE_ARRAY_IS_EMPTY UE_VERSION_START(5, 0, 0)

#define UE_HASH_COMBINE_FAST UE_VERSION_START(5, 0, 0)

#define UE_OBJECT_PTR UE_VERSION_START(5, 0, 0)

#define UE_OBJECT_BASE_UTILITY_MARK_AS_GARBAGE UE_VERSION_START(5, 0, 0)

#define UE_PLATFORM_PROCESS_GET_SYNCH_EVENT_FROM_POOL UE_VERSION_START(5, 0, 0)

#define UE_CLASS_ADD_REFERENCED_OBJECTS !UE_VERSION_START(5, 1, 0)

#define UE_FILTER_CLASS_PATHS UE_VERSION_START(5, 1, 0)

#define STD_CPP_11 __cplusplus >= 201103L

#define STD_CPP_14 __cplusplus >= 201402L

#define STD_CPP_17 __cplusplus >= 201703L

#define STD_CPP_20 __cplusplus >= 202002L

#define STD_CPP_CPP_NON_TYPE_TEMPLATE_PARAMETERS STD_CPP_17

#define STD_CPP_DEFAULT_NEW_ALIGNMENT UE_VERSION_START(5, 0, 0)
