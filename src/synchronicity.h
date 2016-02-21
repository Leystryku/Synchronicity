#pragma once

#include "stdafx.h"
#include "utils.h"
#include "vmthook.h"
#include "hooks.h"

namespace Synchronicity
{
	int Init();
	int Detach();

	bool Initiated;
};