// Copyright (C) 2026 Vicente Danzmann. All Rights Reserved.

#include "DanzmannLookComponent.h"

#include "DanzmannEnhancedInputComponent.h"
#include "DanzmannGameplayTags_InputActions.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"

FName UDanzmannLookComponent::ComponentName(TEXT("DanzmannLookComponent"));

void UDanzmannLookComponent::BindInputActions(UDanzmannEnhancedInputComponent* EnhancedInputComponent, const UDanzmannInputProfile* InputProfile)
{
	if (!IsValid(EnhancedInputComponent) || !IsValid(InputProfile))
	{
		return;
	}

	// Look is split into two Input Actions because mouse (per-frame delta) and gamepad (analog rate)
	// need different math -- see the handlers. No Completed handler is needed because
	// AddControllerYawInput()/AddControllerPitchInput() is consumed each tick and has nothing to clear
	EnhancedInputComponent->BindNativeInputAction(InputProfile, Danzmann::GameplayTags::InputAction_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouseTriggered);
	EnhancedInputComponent->BindNativeInputAction(InputProfile, Danzmann::GameplayTags::InputAction_Look_Gamepad, ETriggerEvent::Triggered, this, &ThisClass::Input_LookGamepadTriggered);
}

void UDanzmannLookComponent::Input_LookMouseTriggered(const FInputActionValue& Value)
{
	APawn* OwnerPawn = GetPawn<APawn>();
	if (!IsValid(OwnerPawn))
	{
		return;
	}

	// Mouse value is a per-frame delta: already frame-rate independent, so no DeltaTime scaling
	FVector2D LookDelta = Value.Get<FVector2D>();

	// Optional exponential low-pass to strip hand-tremor micro-jitter
	if (bSmoothMouseInput && MouseSmoothingTime > 0.0f)
	{
		const UWorld* World = GetWorld();
		const float DeltaSeconds = IsValid(World) ? World->GetDeltaSeconds() : 0.0f;
		const float Alpha = 1.0f - FMath::Exp(-DeltaSeconds / MouseSmoothingTime);
		SmoothedMouseDelta = FMath::Lerp(SmoothedMouseDelta, LookDelta, Alpha);
		LookDelta = SmoothedMouseDelta;
	}
	
	OwnerPawn->AddControllerYawInput(LookDelta.X * MouseYawSensitivity);
	OwnerPawn->AddControllerPitchInput(LookDelta.Y * MousePitchSensitivity);
}

void UDanzmannLookComponent::Input_LookGamepadTriggered(const FInputActionValue& Value)
{
	APawn* OwnerPawn = GetPawn<APawn>();
	if (!IsValid(OwnerPawn))
	{
		return;
	}

	// Gamepad value is an analog rate (-1..1): scale by DeltaTime
	// so the turn speed is frame-rate independent
	const FVector2D LookRate = Value.Get<FVector2D>();
	const UWorld* World = GetWorld();
	const float DeltaSeconds = IsValid(World) ? World->GetDeltaSeconds() : 0.0f;
	
	OwnerPawn->AddControllerYawInput(LookRate.X * GamepadYawSpeed * DeltaSeconds);
	OwnerPawn->AddControllerPitchInput(LookRate.Y * GamepadPitchSpeed * DeltaSeconds);
}
