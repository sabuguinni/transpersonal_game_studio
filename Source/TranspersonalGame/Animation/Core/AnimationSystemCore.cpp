#include "AnimationSystemCore.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UAnimationSystemCore::UAnimationSystemCore()
{
    CurrentTerrorLevel = ETerrorLevel::Uneasy;
    EnvironmentalTension = 0.5f;
    LastTerrorUpdateTime = 0.0f;
    bIsInStartleState = false;
    StartleRecoveryTime = 0.0f;
    
    // Default personality for a paleontologist - cautious but determined
    PlayerPersonality.CautiousnessFactor = 1.2f;
    PlayerPersonality.ReactivitySpeed = 0.8f;
    PlayerPersonality.HeadMovementIntensity = 0.8f;
    PlayerPersonality.ShoulderTension = 0.7f;
}

void UAnimationSystemCore::InitializeAnimationSystem(USkeletalMeshComponent* TargetMesh)
{
    if (!TargetMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("AnimationSystemCore: TargetMesh is null"));
        return;
    }

    // Initialize Motion Matching databases
    if (!LocomotionSchema)
    {
        UE_LOG(LogTemp, Warning, TEXT("AnimationSystemCore: LocomotionSchema not assigned"));
    }

    if (!LocomotionDatabase)
    {
        UE_LOG(LogTemp, Warning, TEXT("AnimationSystemCore: LocomotionDatabase not assigned"));
    }

    // Set initial terror level based on environment
    UpdateTerrorLevel(ETerrorLevel::Uneasy);
    
    UE_LOG(LogTemp, Log, TEXT("AnimationSystemCore: Initialized for character with survival-focused movement"));
}

void UAnimationSystemCore::UpdateTerrorLevel(ETerrorLevel NewLevel)
{
    if (CurrentTerrorLevel != NewLevel)
    {
        ETerrorLevel PreviousLevel = CurrentTerrorLevel;
        CurrentTerrorLevel = NewLevel;
        LastTerrorUpdateTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        
        // Adjust movement personality based on terror level
        switch (CurrentTerrorLevel)
        {
            case ETerrorLevel::Calm:
                PlayerPersonality.CautiousnessFactor = 0.8f;
                PlayerPersonality.ReactivitySpeed = 0.6f;
                PlayerPersonality.HeadMovementIntensity = 0.4f;
                PlayerPersonality.ShoulderTension = 0.3f;
                break;
                
            case ETerrorLevel::Uneasy:
                PlayerPersonality.CautiousnessFactor = 1.0f;
                PlayerPersonality.ReactivitySpeed = 0.8f;
                PlayerPersonality.HeadMovementIntensity = 0.6f;
                PlayerPersonality.ShoulderTension = 0.5f;
                break;
                
            case ETerrorLevel::Nervous:
                PlayerPersonality.CautiousnessFactor = 1.2f;
                PlayerPersonality.ReactivitySpeed = 1.0f;
                PlayerPersonality.HeadMovementIntensity = 0.8f;
                PlayerPersonality.ShoulderTension = 0.7f;
                break;
                
            case ETerrorLevel::Afraid:
                PlayerPersonality.CautiousnessFactor = 1.4f;
                PlayerPersonality.ReactivitySpeed = 1.2f;
                PlayerPersonality.HeadMovementIntensity = 0.9f;
                PlayerPersonality.ShoulderTension = 0.8f;
                break;
                
            case ETerrorLevel::Terrified:
                PlayerPersonality.CautiousnessFactor = 1.6f;
                PlayerPersonality.ReactivitySpeed = 1.5f;
                PlayerPersonality.HeadMovementIntensity = 1.0f;
                PlayerPersonality.ShoulderTension = 0.9f;
                break;
                
            case ETerrorLevel::Panic:
                PlayerPersonality.CautiousnessFactor = 2.0f;
                PlayerPersonality.ReactivitySpeed = 2.0f;
                PlayerPersonality.HeadMovementIntensity = 1.0f;
                PlayerPersonality.ShoulderTension = 1.0f;
                break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Terror level changed from %d to %d"), 
               (int32)PreviousLevel, (int32)CurrentTerrorLevel);
    }
}

void UAnimationSystemCore::TriggerStartleReaction(FVector ThreatDirection)
{
    LastKnownThreatDirection = ThreatDirection.GetSafeNormal();
    bIsInStartleState = true;
    StartleRecoveryTime = GetWorld() ? GetWorld()->GetTimeSeconds() + 2.0f : 2.0f;
    
    // Temporarily increase terror level
    ETerrorLevel TemporaryLevel = static_cast<ETerrorLevel>(
        FMath::Min(static_cast<int32>(CurrentTerrorLevel) + 2, 
                   static_cast<int32>(ETerrorLevel::Panic))
    );
    
    UpdateTerrorLevel(TemporaryLevel);
    
    UE_LOG(LogTemp, Log, TEXT("Startle reaction triggered from direction: %s"), 
           *ThreatDirection.ToString());
}

float UAnimationSystemCore::GetMovementSpeedModifier() const
{
    float BaseModifier = 1.0f;
    
    // Terror affects movement speed
    switch (CurrentTerrorLevel)
    {
        case ETerrorLevel::Calm:
            BaseModifier = 1.0f;
            break;
        case ETerrorLevel::Uneasy:
            BaseModifier = 0.9f; // Slightly slower, more careful
            break;
        case ETerrorLevel::Nervous:
            BaseModifier = 0.8f; // Noticeably more cautious
            break;
        case ETerrorLevel::Afraid:
            BaseModifier = 1.1f; // Starting to move faster
            break;
        case ETerrorLevel::Terrified:
            BaseModifier = 1.3f; // Moving quickly
            break;
        case ETerrorLevel::Panic:
            BaseModifier = 1.5f; // Full sprint
            break;
    }
    
    // Apply personality modifier
    BaseModifier *= PlayerPersonality.CautiousnessFactor;
    
    // Startle state affects speed temporarily
    if (bIsInStartleState && GetWorld())
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime < StartleRecoveryTime)
        {
            float StartleIntensity = (StartleRecoveryTime - CurrentTime) / 2.0f;
            BaseModifier *= (1.0f + StartleIntensity * 0.5f); // Up to 50% speed boost when startled
        }
    }
    
    return FMath::Clamp(BaseModifier, 0.3f, 2.0f);
}

float UAnimationSystemCore::GetAnimationBlendWeight(EMovementState State) const
{
    float BaseWeight = 0.0f;
    
    // Calculate blend weights based on current terror level and movement state
    switch (State)
    {
        case EMovementState::Idle_Cautious:
            BaseWeight = (CurrentTerrorLevel <= ETerrorLevel::Nervous) ? 1.0f : 0.3f;
            break;
            
        case EMovementState::Walk_Careful:
            BaseWeight = (CurrentTerrorLevel <= ETerrorLevel::Afraid) ? 0.8f : 0.2f;
            break;
            
        case EMovementState::Run_Panicked:
            BaseWeight = (CurrentTerrorLevel >= ETerrorLevel::Afraid) ? 1.0f : 0.0f;
            break;
            
        case EMovementState::Crouch_Stealth:
            BaseWeight = (CurrentTerrorLevel >= ETerrorLevel::Nervous) ? 0.7f : 0.3f;
            break;
            
        case EMovementState::React_Startle:
            BaseWeight = bIsInStartleState ? 1.0f : 0.0f;
            break;
            
        case EMovementState::React_Fear:
            BaseWeight = (CurrentTerrorLevel >= ETerrorLevel::Terrified) ? 0.9f : 0.1f;
            break;
            
        default:
            BaseWeight = 0.5f;
            break;
    }
    
    return FMath::Clamp(BaseWeight, 0.0f, 1.0f);
}