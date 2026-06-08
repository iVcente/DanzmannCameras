# DanzmannCameras

Modular camera support for player-controlled Pawns. A Look Component translates look input into the Pawn's Control Rotation -- the single value the engine's Gameplay Camera Component rig and the Mover Component both read to orient the view and the character. The Component sources its binding from a Danzmann Input Profile and the owning Pawn applies it automatically.

Pawn-scoped by design and built to sit beside a stock `UGameplayCameraComponent`, not replace it. The plugin is also the intended home for shared Camera Rig assets authored for Epic's Gameplay Cameras framework.

> Depends on the sibling Danzmann plugins `DanzmannInput` and `DanzmannExperiences` (technically not really required, but highly suggested). Look binding rides `DanzmannInput`'s apply pipeline, so the project must set `DefaultInputComponentClass=/Script/DanzmannInput.DanzmannEnhancedInputComponent` in `Config/DefaultInput.ini` and the active Input Profile must pair `InputAction.Look.Mouse` and `InputAction.Look.Gamepad` with `UInputAction`s.

## Concepts

- `UDanzmannLookComponent` (`UPawnComponent`, implements `IDanzmannInputBinderInterface`): binds the mouse and gamepad `Look` Input Actions and feeds the owning Pawn's Control Rotation.
  - `BindInputActions(EIC, Profile)`: binds `InputAction.Look.Mouse` and `InputAction.Look.Gamepad` on `ETriggerEvent::Triggered` to `Input_LookMouse()` / `Input_LookGamepad()`. There is no `Completed` handler -- look is per-frame accumulation consumed by the engine each tick, so there is nothing to clear on release;
  - `Input_LookMouse(Value)`: the mouse value is a per-frame delta scaled by `MouseYaw/PitchSensitivity` and optionally low-pass smoothed (`bSmoothMouseInput`/`MouseSmoothingTime`) to strip hand-tremor micro-jitter;
  - `Input_LookGamepad(Value)`: the stick value is an analog rate [`-1..1`], so it is scaled by a turn speed (`GamepadYawSpeed/GamepadPitchSpeed`) and delta time;
  - Both handlers call `AddControllerYawInput()`/`AddControllerPitchInput()` on the owning Pawn and never own or write Control Rotation themselves -- `APlayerController::UpdateRotation()` applies (and pitch-clamps) the accumulated input each tick, after which both the Gameplay Camera rig and the Mover Component (via `APlayerController::GetControlRotation()`) read the result.
- Binding is automatic. The Component implements `IDanzmannInputBinderInterface`, so the owning Pawn's `UDanzmannEnhancedInputComponent` discovers it during the apply scan and calls `BindInputActions()` -- the Pawn never references the Component to wire it up.

### Why a separate Component (and not a camera subclass)

The natural design would be a `UGameplayCameraComponent` subclass that also binds look input. That is not possible in C++ across a module boundary: `UGameplayCameraComponent` and its base `UGameplayCameraComponentBase` are declared `MinimalAPI`, so their virtuals and `AddReferencedObjects()` are not DLL-exported and the linker cannot resolve a subclass's vtable from another module. Look input therefore lives in its own `UPawnComponent`, paired with a stock `UGameplayCameraComponent` on the Pawn.

### Pitch limits live in data, not here

`UDanzmannLookComponent` deliberately does not clamp pitch. The clamp is the Player Camera Manager's `ViewPitchMin` and `ViewPitchMax`, which `UpdateRotation()` applies to the Control Rotation. These values are authored per-Pawn on `UDanzmannPawnData` (from [`DanzmannExperiences`](https://github.com/iVcente/DanzmannExperiences)) and pushed onto the local Player Camera Manager by `UDanzmannPawnDataComponent::ApplyPawnData()`. Keeping the limits as data (and off the Component) preserves the split: the Component owns the input, the Pawn Data owns the tuning.

## Usage

```cpp
// 1. Give the player Pawn a stock Gameplay Camera Component and a Look Component.
//    The Look Component needs no configuration -- it sources its binding from the active profile.
AMyPlayerPawn::AMyPlayerPawn(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    GameplayCamera = CreateDefaultSubobject<UGameplayCameraComponent>(FName("GameplayCamera"));
    GameplayCamera->SetupAttachment(MainSkeletalMesh);

    LookComponent = CreateDefaultSubobject<UDanzmannLookComponent>(UDanzmannLookComponent::ComponentName);
}

// 2. The active UDanzmannInputProfile must map InputAction.Look.Mouse and
//    InputAction.Look.Gamepad to UInputActions (authored in the Editor). DanzmannInput's apply
//    pipeline then calls UDanzmannLookComponent::BindInputActions() automatically on possession.
```

That is the whole integration. Whether look pitch feels inverted is an authoring choice on the `Look` Input Actions' Mapping Context (a `Negate` modifier on the Y axis) -- the Component applies sensitivity/smoothing but never inverts the axis itself.

## Flow at a glance

```
Look Input Actions (Triggered)   [Mouse = delta | Gamepad = rate]
  └─ UDanzmannLookComponent::Input_LookMouse / Input_LookGamepad(Value)
       ├─ Pawn->AddControllerYawInput()              Mouse: delta * sensitivity (optional smoothing) | Gamepad: rate  * speed * delta time
       └─ Pawn->AddControllerPitchInput()             
APlayerController::UpdateRotation()  (each tick)         -> applies input, clamps pitch (ViewPitchMin/ViewPitchMax)
  └─ Control Rotation
       ├─ UGameplayCameraComponent Camera Rig                   -> orients the view
       └─ UDanzmannMoverComponent (GetControlRotation()) -> orient-to-control / aim
```

---

Part of the Danzmann plugin suite. See also [`DanzmannInput`](https://github.com/iVcente/DanzmannInput) and [`DanzmannExperiences`](https://github.com/iVcente/DanzmannExperiences).
