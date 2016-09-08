#pragma once

#define EngineAssert(x) (!!(x) || (__debugbreak(), 0))