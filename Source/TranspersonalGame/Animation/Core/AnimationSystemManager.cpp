#include "AnimationSystemManager.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"

UAnimationSystemManager::UAnimationSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // Initialize default character profiles
    FAnimationProfile PlayerProfile;
    PlayerProfile.CharacterType = ECharacterType::Player;
    PlayerProfile.MovementSpeed = 1.0f;
    PlayerProfile.AnimationIntensity = 1.0f;
    PlayerProfile.FearLevel = 0.3f; // Player starts with some fear
    PlayerProfile.ConfidenceLevel = 0.4f; // Low confidence initially
    PlayerProfile.InjuryLevel = 0.0f;
    PlayerProfile.ExhaustionLevel = 0.0f;
    CharacterProfiles.Add(ECharacterType::Player, PlayerProfile);

    FAnimationProfile SmallDinosaurProfile;
    SmallDinosaurProfile.CharacterType = ECharacterType::DinosaurSmall;
    SmallDinosaurProfile.MovementSpeed = 1.2f;
    SmallDinosaurProfile.AnimationIntensity = 1.1f;
    SmallDinosaurProfile.FearLevel = 0.6f; // Small dinosaurs are more fearful
    SmallDinosaurProfile.ConfidenceLevel = 0.3f;
    CharacterProfiles.Add(ECharacterType::DinosaurSmall, SmallDinosaurProfile);

    FAnimationProfile MediumDinosaurProfile;
    MediumDinosaurProfile.CharacterType = ECharacterType::DinosaurMedium;
    MediumDinosaurProfile.MovementSpeed = 0.9f;
    MediumDinosaurProfile.AnimationIntensity = 1.0f;
    MediumDinosaurProfile.FearLevel = 0.2f;
    MediumDinosaurProfile.ConfidenceLevel = 0.7f;
    CharacterProfiles.Add(ECharacterType::DinosaurMedium, MediumDinosaurProfile);

    FAnimationProfile LargeDinosaurProfile;
    LargeDinosaurProfile.CharacterType = ECharacterType::DinosaurLarge;
    LargeDinosaurProfile.MovementSpeed = 0.7f;
    LargeDinosaurProfile.AnimationIntensity = 0.8f;
    LargeDinosaurProfile.FearLevel = 0.1f; // Large dinosaurs fear little
    LargeDinosaurProfile.ConfidenceLevel = 0.9f;
    CharacterProfiles.Add(ECharacterType::DinosaurLarge, LargeDinosaurProfile);

    CurrentProfile = PlayerProfile;
}

void UAnimationSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize animation system
    SetMovementState(EMovementState::Idle);
    SetEmotionalState(EEmotionalState::Cautious);
}

void UAnimationSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, 
                                           FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update fear response timer
    UpdateFearResponse(DeltaTime);
    
    // Apply current emotional state to animation profile
    ApplyEmotionalStateToProfile();
}

void UAnimationSystemManager::SetMovementState(EMovementState NewState)
{
    if (CurrentMovementState != NewState)
    {
        CurrentMovementState = NewState;
        
        // Adjust animation intensity based on movement state
        switch (NewState)
        {
            case EMovementState::Sneaking:
                CurrentProfile.AnimationIntensity = 0.6f;
                CurrentProfile.MovementSpeed = 0.3f;
                break;
            case EMovementState::Running:
                CurrentProfile.AnimationIntensity = 1.4f;
                CurrentProfile.MovementSpeed = 1.8f;
                break;
            case EMovementState::Afraid:
                CurrentProfile.AnimationIntensity = 1.6f;
                CurrentProfile.FearLevel = FMath::Clamp(CurrentProfile.FearLevel + 0.3f, 0.0f, 1.0f);
                break;
            case EMovementState::Injured:
                CurrentProfile.AnimationIntensity = 0.7f;
                CurrentProfile.MovementSpeed = 0.5f;
                break;
            case EMovementState::Exhausted:
                CurrentProfile.AnimationIntensity = 0.5f;
                CurrentProfile.MovementSpeed = 0.4f;
                break;
            default:
                CurrentProfile.AnimationIntensity = 1.0f;
                CurrentProfile.MovementSpeed = 1.0f;
                break;
        }
    }
}

void UAnimationSystemManager::SetEmotionalState(EEmotionalState NewState)
{
    if (CurrentEmotionalState != NewState)
    {
        CurrentEmotionalState = NewState;
        ApplyEmotionalStateToProfile();
    }
}

void UAnimationSystemManager::UpdateAnimationProfile(const FAnimationProfile& NewProfile)
{
    CurrentProfile = NewProfile;
}

FAnimationProfile UAnimationSystemManager::GetCurrentAnimationProfile() const
{
    return CurrentProfile;
}

void UAnimationSystemManager::TriggerFearResponse(float FearIntensity, float Duration)
{
    OriginalFearLevel = CurrentProfile.FearLevel;
    CurrentProfile.FearLevel = FMath::Clamp(FearIntensity, 0.0f, 1.0f);
    FearResponseTimer = 0.0f;
    FearResponseDuration = Duration;
    
    // Trigger immediate fear animation state
    SetEmotionalState(EEmotionalState::Fearful);
    SetMovementState(EMovementState::Afraid);
}

void UAnimationSystemManager::TriggerInjuryAnimation(float InjurySeverity)
{
    CurrentProfile.InjuryLevel = FMath::Clamp(InjurySeverity, 0.0f, 1.0f);
    
    if (InjurySeverity > 0.5f)
    {
        SetMovementState(EMovementState::Injured);
        SetEmotionalState(EEmotionalState::Injured);
    }
}

void UAnimationSystemManager::UpdateExhaustion(float ExhaustionLevel)
{
    CurrentProfile.ExhaustionLevel = FMath::Clamp(ExhaustionLevel, 0.0f, 1.0f);
    
    if (ExhaustionLevel > 0.7f)
    {
        SetMovementState(EMovementState::Exhausted);
        SetEmotionalState(EEmotionalState::Exhausted);
    }
}

void UAnimationSystemManager::UpdateFearResponse(float DeltaTime)
{
    if (FearResponseTimer < FearResponseDuration)
    {
        FearResponseTimer += DeltaTime;
        
        // Gradually return fear level to original
        float Alpha = FearResponseTimer / FearResponseDuration;
        CurrentProfile.FearLevel = FMath::Lerp(CurrentProfile.FearLevel, OriginalFearLevel, Alpha);
        
        if (FearResponseTimer >= FearResponseDuration)
        {
            // Fear response complete, return to appropriate emotional state
            if (CurrentProfile.FearLevel < 0.3f)
            {
                SetEmotionalState(EEmotionalState::Cautious);
            }
            else
            {
                SetEmotionalState(EEmotionalState::Fearful);
            }
        }
    }
}

void UAnimationSystemManager::ApplyEmotionalStateToProfile()
{
    switch (CurrentEmotionalState)
    {
        case EEmotionalState::Fearful:
            CurrentProfile.AnimationIntensity = FMath::Clamp(CurrentProfile.AnimationIntensity * 1.3f, 0.5f, 2.0f);
            CurrentProfile.ConfidenceLevel = FMath::Max(CurrentProfile.ConfidenceLevel - 0.2f, 0.0f);
            break;
        case EEmotionalState::Cautious:
            CurrentProfile.AnimationIntensity = FMath::Clamp(CurrentProfile.AnimationIntensity * 1.1f, 0.7f, 1.5f);
            break;
        case EEmotionalState::Confident:
            CurrentProfile.AnimationIntensity = FMath::Clamp(CurrentProfile.AnimationIntensity * 0.9f, 0.8f, 1.2f);
            CurrentProfile.ConfidenceLevel = FMath::Min(CurrentProfile.ConfidenceLevel + 0.1f, 1.0f);
            break;
        case EEmotionalState::Exhausted:
            CurrentProfile.AnimationIntensity = FMath::Max(CurrentProfile.AnimationIntensity * 0.6f, 0.3f);
            CurrentProfile.MovementSpeed = FMath::Max(CurrentProfile.MovementSpeed * 0.5f, 0.2f);
            break;
        case EEmotionalState::Injured:
            CurrentProfile.AnimationIntensity = FMath::Max(CurrentProfile.AnimationIntensity * 0.7f, 0.4f);
            CurrentProfile.MovementSpeed = FMath::Max(CurrentProfile.MovementSpeed * 0.6f, 0.3f);
            break;
        default:
            // Neutral state - no modifications
            break;
    }
}