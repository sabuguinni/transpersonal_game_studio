#include "AnimationSystemCore.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearchDatabase.h"

DEFINE_LOG_CATEGORY(LogAnimationSystem);

UAnimationSystemCore::UAnimationSystemCore()
{
    CurrentContext = EMovementContext::Exploration;
    CurrentEmotionalState = ECharacterEmotionalState::Calm;
    CurrentFearLevel = 0.0f;
    CurrentExhaustion = 0.0f;

    // Initialize default character profile for paleontologist
    FCharacterAnimationProfile PaleontologistProfile;
    PaleontologistProfile.CharacterName = TEXT("Dr_Paleontologist");
    PaleontologistProfile.FearThreshold = 0.6f; // Lower threshold - gets scared easier
    PaleontologistProfile.ExhaustionRate = 0.15f; // Gets tired faster
    PaleontologistProfile.RecoveryRate = 0.03f; // Recovers slower
    
    // Contextual speed modifiers for a scientist, not an athlete
    PaleontologistProfile.ContextualSpeedModifiers[EMovementContext::Exploration] = 0.9f;
    PaleontologistProfile.ContextualSpeedModifiers[EMovementContext::Stealth] = 0.4f;
    PaleontologistProfile.ContextualSpeedModifiers[EMovementContext::Escape] = 1.3f; // Adrenaline helps
    PaleontologistProfile.ContextualSpeedModifiers[EMovementContext::Interaction] = 0.7f;
    PaleontologistProfile.ContextualSpeedModifiers[EMovementContext::Combat] = 0.8f; // Not a fighter
    PaleontologistProfile.ContextualSpeedModifiers[EMovementContext::Observation] = 0.05f; // Very slow, careful observation
    
    CharacterProfiles.Add(TEXT("Dr_Paleontologist"), PaleontologistProfile);
}

void UAnimationSystemCore::InitializeForCharacter(ACharacter* Character)
{
    if (!Character)
    {
        UE_LOG(LogAnimationSystem, Error, TEXT("Cannot initialize animation system: Character is null"));
        return;
    }

    UE_LOG(LogAnimationSystem, Log, TEXT("Initializing animation system for character: %s"), *Character->GetName());

    // Set default animation profile
    if (CharacterProfiles.Contains(TEXT("Dr_Paleontologist")))
    {
        UE_LOG(LogAnimationSystem, Log, TEXT("Applied paleontologist animation profile"));
    }
}

void UAnimationSystemCore::UpdateAnimationContext(EMovementContext NewContext, ECharacterEmotionalState EmotionalState)
{
    EMovementContext PreviousContext = CurrentContext;
    CurrentContext = NewContext;
    CurrentEmotionalState = EmotionalState;

    // Update fear level based on context
    switch (NewContext)
    {
        case EMovementContext::Escape:
            CurrentFearLevel = FMath::Clamp(CurrentFearLevel + 0.3f, 0.0f, 1.0f);
            break;
        case EMovementContext::Stealth:
            CurrentFearLevel = FMath::Clamp(CurrentFearLevel + 0.1f, 0.0f, 1.0f);
            break;
        case EMovementContext::Observation:
            CurrentFearLevel = FMath::Clamp(CurrentFearLevel - 0.05f, 0.0f, 1.0f);
            break;
        case EMovementContext::Exploration:
            CurrentFearLevel = FMath::Clamp(CurrentFearLevel - 0.02f, 0.0f, 1.0f);
            break;
    }

    UE_LOG(LogAnimationSystem, Log, TEXT("Animation context changed from %d to %d. Fear level: %f"), 
           (int32)PreviousContext, (int32)NewContext, CurrentFearLevel);
}

float UAnimationSystemCore::CalculateBlendTime(EMovementContext FromContext, EMovementContext ToContext)
{
    // Quick transitions for fear responses
    if (ToContext == EMovementContext::Escape)
    {
        return 0.1f; // Immediate fear response
    }

    // Slower transitions when calming down
    if (FromContext == EMovementContext::Escape && ToContext == EMovementContext::Exploration)
    {
        return 0.8f; // Takes time to calm down
    }

    // Stealth requires careful transitions
    if (ToContext == EMovementContext::Stealth || FromContext == EMovementContext::Stealth)
    {
        return 0.4f;
    }

    // Default blend time
    return 0.25f;
}

FVector UAnimationSystemCore::CalculateFootPlantingOffset(const FVector& GroundNormal, const FVector& FootLocation)
{
    // Calculate IK offset for foot placement on uneven terrain
    FVector UpVector = FVector::UpVector;
    FVector Offset = FVector::ZeroVector;

    // Project foot location onto ground plane
    float DotProduct = FVector::DotProduct(GroundNormal, UpVector);
    
    if (DotProduct > 0.1f) // Valid ground normal
    {
        // Calculate offset to align foot with ground
        FVector GroundPoint = FootLocation - FVector::DotProduct(FootLocation - FVector::ZeroVector, GroundNormal) * GroundNormal;
        Offset = GroundPoint - FootLocation;
        
        // Limit offset to reasonable values
        Offset = Offset.GetClampedToMaxSize(20.0f); // Max 20cm offset
    }

    return Offset;
}