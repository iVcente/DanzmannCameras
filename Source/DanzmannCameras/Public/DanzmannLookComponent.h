// Copyright (C) 2026 Vicente Danzmann. All Rights Reserved.

#pragma once

#include "Components/PawnComponent.h"
#include "CoreMinimal.h"
#include "DanzmannInputBinderInterface.h"

#include "DanzmannLookComponent.generated.h"

class UDanzmannEnhancedInputComponent;
class UDanzmannInputProfile;
struct FInputActionValue;

/**
 * Pawn Component that consumes the "Look" Input Actions and forwards them into the owning Pawn's
 * Control Rotation via AddControllerYawInput()/AddControllerPitchInput() -- the Component never owns
 * the rotation itself. The PlayerController's UpdateRotation() applies (and pitch-clamps) it each
 * tick, after which both the Gameplay Camera rig and the Mover Component (via
 * APlayerController::GetControlRotation()) consume it.
 *
 * Mouse and gamepad look are bound as two separate Input Actions. Binding is automatic: the Component
 * implements IDanzmannInputBinderInterface, so the owning Pawn's UDanzmannEnhancedInputComponent
 * discovers it during input setup and calls BindInputActions().
 * 
 * This lives in this Component (rather than on UGameplayCameraComponent) because that engine class
 * is MinimalAPI and cannot be subclassed across a module boundary. Pairing this Component with a
 * stock UGameplayCameraComponent on a player Pawn keeps the Pawn thin and the look-input concern self-owned.
 *
 * Scope is the look-to-Control-Rotation paradigm (FPS/third-person). Camera paradigms that don't use
 * Control Rotation (e.g., RTS pan/zoom) belong in separate sibling Components, not here.
 */
UCLASS(ClassGroup = "Danzmann", HideCategories = ("Activation", "AssetUserData", "ComponentTick", "ComponentReplication", "Cooking", "Navigation", "Replication"), Meta = (BlueprintSpawnableComponent))
class DANZMANNCAMERAS_API UDanzmannLookComponent : public UPawnComponent, public IDanzmannInputBinderInterface
{
	GENERATED_BODY()

	public:
		/**
		 * Bind the "Look" Input Actions (mouse + gamepad).
		 * @see more info in DanzmannInputBinderInterface.h.
		 */
		virtual void BindInputActions(UDanzmannEnhancedInputComponent* EnhancedInputComponent, const UDanzmannInputProfile* InputProfile) override;
		
		/**
		 * FName the Component is created under. Pass to FObjectInitializer::SetDefaultSubobjectClass<>()
		 * to swap in a subclass on derived Pawn classes.
		 */
		static FName ComponentName;

	protected:
		/**
		 * Callback for Input Action "Look (Mouse)" for Triggered event. The value is a per-frame mouse
		 * delta, scaled by mouse sensitivity and optionally smoothed to avoid hand-tremor micro-jitter.
		 * @param Value Input Action value.
		 */
		virtual void Input_LookMouseTriggered(const FInputActionValue& Value);

		/**
		 * Callback for Input Action "Look (Gamepad)" for Triggered event. The value is an analog stick
		 * rate (-1..1), scaled by a turn speed and DeltaTime.
		 * @param Value Input Action value.
		 */
		virtual void Input_LookGamepadTriggered(const FInputActionValue& Value);

		/**
		 * Mouse yaw sensitivity. Raw multiplier on the per-frame mouse delta.
		 */
		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mouse")
		float MouseYawSensitivity = 1.0f;

		/**
		 * Mouse pitch sensitivity. Raw multiplier on the per-frame mouse delta.
		 */
		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mouse")
		float MousePitchSensitivity = 1.0f;

		/**
		 * Whether to apply a low-pass to the mouse delta to remove micro-jitter.
		 */
		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mouse")
		bool bSmoothMouseInput = true;

		/**
		 * Time constant (seconds) of the mouse low-pass. Higher = smoother but more latency. 0 disables.
		 */
		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mouse", Meta = (EditCondition = bSmoothMouseInput, ClampMin = 0.0f))
		float MouseSmoothingTime = 0.025f;

		/**
		 * Gamepad yaw turn speed.
		 */
		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gamepad")
		float GamepadYawSpeed = 1.0f;

		/**
		 * Gamepad pitch turn speed .
		 */
		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gamepad")
		float GamepadPitchSpeed = 1.0f;

	private:
		/**
		 * Running low-pass state for the mouse delta. Transient -- only meaningful while the mouse moves.
		 */
		FVector2D SmoothedMouseDelta = FVector2D::ZeroVector;
};
