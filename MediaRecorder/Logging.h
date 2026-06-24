#pragma once
#include "pch.h"

#ifdef _DEBUG
#include <iostream>
#define log(...) printf("%s [%s:%d] ", __FILE__, __func__, __LINE__);printf(__VA_ARGS__)
#else
#define log(...) (0)
#endif

void InitializeLogging();
