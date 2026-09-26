#pragma once

// Terminality.hpp must come before Windows.h (the framework enforces this).
#include <terminality/Terminality.hpp>
#include <winsock2.h>
#include <Windows.h>
#undef MessageBox
#undef TRANSPARENT