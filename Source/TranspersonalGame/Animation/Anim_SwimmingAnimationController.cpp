#include "Anim_SwimmingAnimationController.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"

UAnim_SwimmingAnimationController::UAnim_SwimmingAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize swimming data
    SwimmingData = FAnim_SwimmingAnimData();
    MontageSettings = FAnim_SwimmingMontageSettings();
    
    CachedAnimInstance = nullptr;
    CurrentPlayingMontage = nullptr;
    StateTransitionTimer = 0.0f;
    LastSwimSpeedUpdate = 0.0f;
    bIsTransitioning = false;
}

void UAnim_SwimmingAnimationController::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache animation instance from owner character
    if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
    {
        if (USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh())
        {
            CachedAnimInstance = MeshComp->GetAnimInstance();
            if (CachedAnimInstance)
            {
                UE_LOG(LogTemp, Log, TEXT("Swimming Animation Controller: Cached animation instance for %s"), *GetOwner()->GetName());
            }
        }
    }
}

void UAnim_SwimmingAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!CachedAnimInstance)
    {
        return;
    }
    
    // Update transition timer
    if (bIsTransitioning)
    {
        StateTransitionTimer += DeltaTime;
        if (StateTransitionTimer >= MontageSettings.BlendInTime)
        {
            bIsTransitioning = false;
            StateTransitionTimer = 0.0f;
        }
    }
    
    // Update swim speed based animations
    UpdateSwimSpeedBasedAnimation();
    
    // Handle breath holding visual effects
    HandleBreathHoldingEffects();
    
    // Update last swim speed tracking
    LastSwimSpeedUpdate += DeltaTime;
}

void UAnim_SwimmingAnimationController::UpdateSwimmingAnimation(EAnim_SwimmingAnimState NewState, float SwimSpeed, float WaterDepth)
{
    // Update swimming data
    SwimmingData.SwimSpeed = SwimSpeed;
    SwimmingData.WaterDepth = WaterDepth;
    SwimmingData.bIsUnderwater = WaterDepth > 100.0f; // Character head underwater
    SwimmingData.bIsDiving = NewState == EAnim_SwimmingAnimState::Diving;
    SwimmingData.bIsSurfacing = NewState == EAnim_SwimmingAnimState::Surfacing;
    
    // Transition to new state if different
    if (SwimmingData.CurrentSwimState != NewState)
    {
        TransitionToSwimState(NewState);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Swimming Animation: State=%s, Speed=%.1f, Depth=%.1f"), 
           *UEnum::GetValueAsString(NewState), SwimSpeed, WaterDepth);
}

void UAnim_SwimmingAnimationController::PlaySwimmingMontage(EAnim_SwimmingAnimState SwimState)
{
    if (!CachedAnimInstance)
    {
        return;
    }
    
    UAnimMontage* MontageToPlay = nullptr;
    
    // Select appropriate montage based on swim state
    switch (SwimState)
    {
        case EAnim_SwimmingAnimState::SurfaceSwim:
            MontageToPlay = MontageSettings.SurfaceSwimMontage;
            break;
        case EAnim_SwimmingAnimState::Underwater:
            MontageToPlay = MontageSettings.UnderwaterSwimMontage;
            break;
        case EAnim_SwimmingAnimState::Diving:
            MontageToPlay = MontageSettings.DivingMontage;
            break;
        case EAnim_SwimmingAnimState::Surfacing:
            MontageToPlay = MontageSettings.SurfacingMontage;
            break;
        case EAnim_SwimmingAnimState::BreathHolding:
            MontageToPlay = MontageSettings.BreathHoldMontage;
            break;
        case EAnim_SwimmingAnimState::Treading:
            MontageToPlay = MontageSettings.TreadingWaterMontage;
            break;
        default:
            break;
    }
    
    if (MontageToPlay)
    {
        // Stop current montage if playing
        if (CurrentPlayingMontage && CachedAnimInstance->Montage_IsPlaying(CurrentPlayingMontage))
        {
            CachedAnimInstance->Montage_Stop(MontageSettings.BlendOutTime, CurrentPlayingMontage);
        }
        
        // Play new montage
        float PlayLength = CachedAnimInstance->Montage_Play(MontageToPlay, MontageSettings.MontagePlayRate);
        CurrentPlayingMontage = MontageToPlay;
        
        UE_LOG(LogTemp, Log, TEXT("Swimming Animation: Playing montage for state %s (Length: %.2f)"), 
               *UEnum::GetValueAsString(SwimState), PlayLength);
    }
}

void UAnim_SwimmingAnimationController::StopCurrentSwimmingMontage()
{
    if (CachedAnimInstance && CurrentPlayingMontage)
    {
        CachedAnimInstance->Montage_Stop(MontageSettings.BlendOutTime, CurrentPlayingMontage);
        CurrentPlayingMontage = nullptr;
        
        UE_LOG(LogTemp, Log, TEXT("Swimming Animation: Stopped current montage"));
    }
}

void UAnim_SwimmingAnimationController::UpdateBreathHoldingAnimation(float BreathPercentage)
{
    SwimmingData.BreathPercentage = FMath::Clamp(BreathPercentage, 0.0f, 100.0f);
    
    // Adjust animation playback rate based on breath level
    if (SwimmingData.CurrentSwimState == EAnim_SwimmingAnimState::BreathHolding)
    {
        float BreathStress = 1.0f - (BreathPercentage / 100.0f);
        float AnimationRate = FMath::Lerp(0.8f, 1.5f, BreathStress); // Faster when low on breath
        
        if (CachedAnimInstance && CurrentPlayingMontage)
        {
            CachedAnimInstance->Montage_SetPlayRate(CurrentPlayingMontage, AnimationRate);
        }
    }
}

void UAnim_SwimmingAnimationController::SetSwimmingDirection(FVector Direction)
{
    SwimmingData.SwimDirection = Direction.GetSafeNormal();
}

void UAnim_SwimmingAnimationController::SetStrokeIntensity(float Intensity)
{
    SwimmingData.StrokeIntensity = FMath::Clamp(Intensity, 0.1f, 2.0f);
    
    // Adjust montage playback rate based on stroke intensity
    if (CachedAnimInstance && CurrentPlayingMontage)
    {
        float PlayRate = MontageSettings.MontagePlayRate * SwimmingData.StrokeIntensity;
        CachedAnimInstance->Montage_SetPlayRate(CurrentPlayingMontage, PlayRate);
    }
}

void UAnim_SwimmingAnimationController::TransitionToSwimState(EAnim_SwimmingAnimState NewState)
{
    EAnim_SwimmingAnimState PreviousState = SwimmingData.CurrentSwimState;
    SwimmingData.CurrentSwimState = NewState;
    
    // Start transition
    bIsTransitioning = true;
    StateTransitionTimer = 0.0f;
    
    // Play appropriate montage for new state
    PlaySwimmingMontage(NewState);
    
    UE_LOG(LogTemp, Log, TEXT("Swimming Animation: Transitioned from %s to %s"), 
           *UEnum::GetValueAsString(PreviousState), *UEnum::GetValueAsString(NewState));
}

void UAnim_SwimmingAnimationController::UpdateSwimSpeedBasedAnimation()
{
    if (LastSwimSpeedUpdate < 0.1f) // Update every 100ms
    {
        return;
    }
    
    LastSwimSpeedUpdate = 0.0f;
    
    // Adjust animation playback based on swim speed
    if (SwimmingData.SwimSpeed > 0.1f && CachedAnimInstance && CurrentPlayingMontage)
    {
        // Scale animation rate with swim speed (normalized to expected max speed)
        float NormalizedSpeed = SwimmingData.SwimSpeed / 400.0f; // Max swim speed from character system
        float SpeedBasedRate = FMath::Lerp(0.5f, 1.5f, FMath::Clamp(NormalizedSpeed, 0.0f, 1.0f));
        
        float FinalRate = MontageSettings.MontagePlayRate * SpeedBasedRate * SwimmingData.StrokeIntensity;
        CachedAnimInstance->Montage_SetPlayRate(CurrentPlayingMontage, FinalRate);
    }
}

void UAnim_SwimmingAnimationController::HandleBreathHoldingEffects()
{
    if (SwimmingData.bIsUnderwater && SwimmingData.BreathPercentage < 50.0f)
    {
        // Add subtle animation adjustments for low breath
        // This could trigger additional animation layers or facial expressions
        
        if (SwimmingData.BreathPercentage < 20.0f)
        {
            // Critical breath level - more urgent animations
            UE_LOG(LogTemp, Warning, TEXT("Swimming Animation: Critical breath level %.1f%%"), SwimmingData.BreathPercentage);
        }
    }
}