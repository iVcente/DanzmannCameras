// Copyright (C) 2026 Vicente Danzmann. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FDanzmannCamerasModule : public IModuleInterface
{
	public:
		/**
		 * @see more info in ModuleInterface.h.
		 */
		virtual void StartupModule() override;

		/**
		 * @see more info in ModuleInterface.h.
		 */
		virtual void ShutdownModule() override;
};
