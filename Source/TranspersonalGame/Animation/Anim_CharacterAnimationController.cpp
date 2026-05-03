#include "Anim_CharacterAnimationController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"

UAnim_CharacterAnimationController::UAnim_CharacterAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize default values
    BlendSpaceInput = FVector2D::ZeroVector;
    StaminaSpeedModifier = 1.0f;
    FearTremblingIntensity = 0.0f;
    InjuryLimpIntensity = 0.0f;
    
    OwnerMesh = nullptr;
    AnimInstance = nullptr;
    MovementBlendSpace = nullptr;
}

void UAnim_CharacterAnimationController::BeginPlay()
{
    Super::BeginPlay();
    
    CacheAnimationReferences();
    
    // Initialize animation state
    CurrentAnimationState = FAnim_AnimationState();
    PreviousAnimationState = CurrentAnimationState;
    
    UE_LOG(LogTemp, Log, TEXT("Animation Controller initialized for %s"), 
           *GetOwner()->GetName());
}

void UAnim_CharacterAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerMesh || !AnimInstance)
    {
        return;
    }
    
    // Update blend space from current state
    UpdateBlendSpaceFromState();
    
    // Apply survival modifiers
    ApplySurvivalModifiers();
}

void UAnim_CharacterAnimationController::UpdateAnimationState(const FAnim_AnimationState& NewState)
{
    if (!ShouldTransitionState(NewState))
    {
        return;
    }
    
    FAnim_AnimationState OldState = CurrentAnimationState;
    PreviousAnimationState = CurrentAnimationState;
    CurrentAnimationState = NewState;
    
    HandleStateTransition(OldState, NewState);
    
    // Broadcast event
    OnAnimationStateChanged(CurrentAnimationState);
    
    UE_LOG(LogTemp, Log, TEXT("Animation state changed: Speed=%.2f, Direction=%.2f, State=%d"), 
           CurrentAnimationState.Speed, CurrentAnimationState.Direction, 
           (int32)CurrentAnimationState.MovementState);
}

void UAnim_CharacterAnimationController::SetMovementState(EDir_MovementState NewMovementState)
{
    if (CurrentAnimationState.MovementState != NewMovementState)
    {
        FAnim_AnimationState NewState = CurrentAnimationState;
        NewState.MovementState = NewMovementState;
        UpdateAnimationState(NewState);
    }
}

void UAnim_CharacterAnimationController::SetSpeed(float NewSpeed)
{
    if (!FMath::IsNearlyEqual(CurrentAnimationState.Speed, NewSpeed, 0.1f))
    {
        FAnim_AnimationState NewState = CurrentAnimationState;
        NewState.Speed = FMath::Max(0.0f, NewSpeed);
        UpdateAnimationState(NewState);
    }
}

void UAnim_CharacterAnimationController::SetDirection(float NewDirection)
{
    // Normalize direction to [-180, 180] range
    float NormalizedDirection = FMath::UnwindDegrees(NewDirection);
    
    if (!FMath::IsNearlyEqual(CurrentAnimationState.Direction, NormalizedDirection, 5.0f))
    {
        FAnim_AnimationState NewState = CurrentAnimationState;
        NewState.Direction = NormalizedDirection;
        UpdateAnimationState(NewState);
    }
}

bool UAnim_CharacterAnimationController::PlayMontage(const FAnim_MontageSettings& MontageSettings)
{
    if (!AnimInstance || !MontageSettings.Montage)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot play montage: Invalid AnimInstance or Montage"));
        return false;
    }
    
    // Stop current montage if playing
    if (IsPlayingMontage())
    {
        StopMontage(MontageSettings.BlendOutTime);
    }
    
    // Play new montage
    float MontageLength = AnimInstance->Montage_Play(
        MontageSettings.Montage, 
        MontageSettings.PlayRate,
        EMontagePlayReturnType::MontageLength,
        0.0f,
        true
    );
    
    if (MontageLength > 0.0f)
    {
        OnMontageStarted(MontageSettings.Montage);
        UE_LOG(LogTemp, Log, TEXT("Playing montage: %s (Length: %.2f)"), 
               *MontageSettings.Montage->GetName(), MontageLength);
        return true;
    }
    
    return false;
}

void UAnim_CharacterAnimationController::StopMontage(float BlendOutTime)
{
    if (AnimInstance && IsPlayingMontage())
    {
        UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();
        AnimInstance->Montage_Stop(BlendOutTime);
        
        if (CurrentMontage)
        {
            OnMontageEnded(CurrentMontage, true);
            UE_LOG(LogTemp, Log, TEXT("Stopped montage: %s"), *CurrentMontage->GetName());
        }
    }
}

bool UAnim_CharacterAnimationController::IsPlayingMontage() const
{
    return AnimInstance && AnimInstance->IsAnyMontagePlaying();
}

float UAnim_CharacterAnimationController::GetMontagePosition() const
{
    if (AnimInstance && IsPlayingMontage())
    {
        return AnimInstance->Montage_GetPosition(AnimInstance->GetCurrentActiveMontage());
    }
    return 0.0f;
}

void UAnim_CharacterAnimationController::SetBlendSpaceInput(float X, float Y)
{
    BlendSpaceInput.X = FMath::Clamp(X, -1.0f, 1.0f);
    BlendSpaceInput.Y = FMath::Clamp(Y, -1.0f, 1.0f);
}

void UAnim_CharacterAnimationController::ApplyStaminaModifier(float StaminaLevel)
{
    CurrentAnimationState.StaminaLevel = FMath::Clamp(StaminaLevel, 0.0f, 1.0f);
    
    // Lower stamina = slower movement
    StaminaSpeedModifier = FMath::Lerp(0.5f, 1.0f, CurrentAnimationState.StaminaLevel);
}

void UAnim_CharacterAnimationController::ApplyFearModifier(float FearLevel)
{
    CurrentAnimationState.FearLevel = FMath::Clamp(FearLevel, 0.0f, 1.0f);
    
    // Higher fear = more trembling
    FearTremblingIntensity = CurrentAnimationState.FearLevel * 0.5f;
}

void UAnim_CharacterAnimationController::ApplyInjuryModifier(float InjuryLevel)
{
    InjuryLimpIntensity = FMath::Clamp(InjuryLevel, 0.0f, 1.0f);
}

void UAnim_CharacterAnimationController::CacheAnimationReferences()
{
    if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
    {
        OwnerMesh = OwnerCharacter->GetMesh();
        if (OwnerMesh)
        {
            AnimInstance = OwnerMesh->GetAnimInstance();
        }
    }
    
    if (!OwnerMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation Controller: No SkeletalMeshComponent found on owner"));
    }
    
    if (!AnimInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation Controller: No AnimInstance found"));
    }
}

void UAnim_CharacterAnimationController::UpdateBlendSpaceFromState()
{
    if (!MovementBlendSpace)
    {
        return;
    }
    
    // Convert speed and direction to blend space coordinates
    float SpeedNormalized = CurrentAnimationState.Speed / 600.0f; // Assume max speed 600
    float DirectionRadians = FMath::DegreesToRadians(CurrentAnimationState.Direction);
    
    float BlendX = SpeedNormalized * FMath::Sin(DirectionRadians);
    float BlendY = SpeedNormalized * FMath::Cos(DirectionRadians);
    
    SetBlendSpaceInput(BlendX, BlendY);
}

void UAnim_CharacterAnimationController::ApplySurvivalModifiers()
{
    // Apply stamina modifier to current speed
    if (StaminaSpeedModifier < 1.0f)
    {
        FAnim_AnimationState ModifiedState = CurrentAnimationState;
        ModifiedState.Speed *= StaminaSpeedModifier;
        
        // Update blend space with modified speed
        float SpeedNormalized = ModifiedState.Speed / 600.0f;
        float DirectionRadians = FMath::DegreesToRadians(ModifiedState.Direction);
        
        float BlendX = SpeedNormalized * FMath::Sin(DirectionRadians);
        float BlendY = SpeedNormalized * FMath::Cos(DirectionRadians);
        
        SetBlendSpaceInput(BlendX, BlendY);
    }
}

bool UAnim_CharacterAnimationController::ShouldTransitionState(const FAnim_AnimationState& NewState) const
{
    // Don't transition if states are too similar
    if (CurrentAnimationState.MovementState == NewState.MovementState &&
        FMath::IsNearlyEqual(CurrentAnimationState.Speed, NewState.Speed, 10.0f) &&
        FMath::IsNearlyEqual(CurrentAnimationState.Direction, NewState.Direction, 10.0f))
    {
        return false;
    }
    
    // Always allow transitions if movement state changes
    if (CurrentAnimationState.MovementState != NewState.MovementState)
    {
        return true;
    }
    
    // Allow transitions for significant speed or direction changes
    return true;
}

void UAnim_CharacterAnimationController::HandleStateTransition(const FAnim_AnimationState& OldState, const FAnim_AnimationState& NewState)
{
    // Handle specific state transitions
    if (OldState.MovementState != NewState.MovementState)
    {
        // Check if we have a montage for this state
        if (UAnimMontage** FoundMontage = StateMontages.Find(NewState.MovementState))
        {
            if (*FoundMontage)
            {
                FAnim_MontageSettings MontageSettings;
                MontageSettings.Montage = *FoundMontage;
                MontageSettings.PlayRate = 1.0f;
                MontageSettings.BlendInTime = 0.2f;
                MontageSettings.BlendOutTime = 0.2f;
                
                PlayMontage(MontageSettings);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Animation state transition: %d -> %d"), 
           (int32)OldState.MovementState, (int32)NewState.MovementState);
}