#include "AnimationSystemCore.h"
#include "Animation/AnimInstance.h"
#include "Animation/PoseSearch/PoseSearchDatabase.h"
#include "IKRigDefinition.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UAnimationSystemCore::UAnimationSystemCore()
{
    // Initialize default settings
}

void UAnimationSystemCore::InitializeMotionMatchingDatabases()
{
    // Initialize Motion Matching databases for different creature types and behaviors
    // This will be populated with actual database assets during content creation
    
    UE_LOG(LogTemp, Log, TEXT("Animation System: Initializing Motion Matching databases..."));
    
    // Player locomotion database
    // CreatureLocomotionDatabase for various dinosaur species
    // CombatDatabase for predator behaviors
    // DomesticationDatabase for tamed creature behaviors
    
    SetupMotionMatchingSchemas();
}

void UAnimationSystemCore::SetActiveDatabase(const FString& DatabaseName)
{
    if (MotionMatchingDatabases.Contains(DatabaseName))
    {
        UE_LOG(LogTemp, Log, TEXT("Animation System: Switching to database: %s"), *DatabaseName);
        // Implementation will connect to Motion Matching node in Animation Blueprint
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation System: Database not found: %s"), *DatabaseName);
    }
}

void UAnimationSystemCore::ApplyCreaturePersonality(USkeletalMeshComponent* SkeletalMesh, const FCreaturePersonality& Personality)
{
    if (!SkeletalMesh || !SkeletalMesh->GetAnimInstance())
    {
        return;
    }

    UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance();
    
    // Apply size variation
    FVector CurrentScale = SkeletalMesh->GetComponentScale();
    FVector NewScale = CurrentScale * Personality.SizeVariation;
    SkeletalMesh->SetWorldScale3D(NewScale);
    
    // Apply behavioral parameters to animation instance
    // These will be read by the Animation Blueprint to modify behavior
    
    // Set animation variables through the Animation Instance
    if (AnimInstance->GetClass()->FindPropertyByName(TEXT("Aggressiveness")))
    {
        AnimInstance->GetClass()->FindPropertyByName(TEXT("Aggressiveness"))->SetFloatPropertyValue(AnimInstance, Personality.Aggressiveness);
    }
    
    if (AnimInstance->GetClass()->FindPropertyByName(TEXT("Nervousness")))
    {
        AnimInstance->GetClass()->FindPropertyByName(TEXT("Nervousness"))->SetFloatPropertyValue(AnimInstance, Personality.Nervousness);
    }
    
    if (AnimInstance->GetClass()->FindPropertyByName(TEXT("MovementSpeedMultiplier")))
    {
        AnimInstance->GetClass()->FindPropertyByName(TEXT("MovementSpeedMultiplier"))->SetFloatPropertyValue(AnimInstance, Personality.MovementSpeedMultiplier);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Animation System: Applied personality - Aggression: %f, Nervousness: %f"), 
           Personality.Aggressiveness, Personality.Nervousness);
}

void UAnimationSystemCore::UpdateCreatureAnimationState(USkeletalMeshComponent* SkeletalMesh, ECreatureAnimationState NewState)
{
    if (!SkeletalMesh || !SkeletalMesh->GetAnimInstance())
    {
        return;
    }

    UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance();
    
    // Update animation state in the Animation Blueprint
    if (AnimInstance->GetClass()->FindPropertyByName(TEXT("CreatureState")))
    {
        AnimInstance->GetClass()->FindPropertyByName(TEXT("CreatureState"))->SetBytePropertyValue(AnimInstance, (uint8)NewState);
    }
    
    // Apply state-specific behaviors
    switch (NewState)
    {
        case ECreatureAnimationState::Alert:
            // Increase animation speed, add tension to posture
            if (AnimInstance->GetClass()->FindPropertyByName(TEXT("AlertnessLevel")))
            {
                AnimInstance->GetClass()->FindPropertyByName(TEXT("AlertnessLevel"))->SetFloatPropertyValue(AnimInstance, 1.0f);
            }
            break;
            
        case ECreatureAnimationState::Hunting:
            // Lower posture, stalking movements
            if (AnimInstance->GetClass()->FindPropertyByName(TEXT("HuntingMode")))
            {
                AnimInstance->GetClass()->FindPropertyByName(TEXT("HuntingMode"))->SetBoolPropertyValue(AnimInstance, true);
            }
            break;
            
        case ECreatureAnimationState::Domesticated:
            // Relaxed posture, friendly behaviors
            if (AnimInstance->GetClass()->FindPropertyByName(TEXT("DomesticationLevel")))
            {
                AnimInstance->GetClass()->FindPropertyByName(TEXT("DomesticationLevel"))->SetFloatPropertyValue(AnimInstance, 1.0f);
            }
            break;
            
        default:
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Animation System: Creature state changed to: %d"), (int32)NewState);
}

void UAnimationSystemCore::UpdatePlayerAnimationState(USkeletalMeshComponent* PlayerMesh, EPlayerAnimationState NewState)
{
    if (!PlayerMesh || !PlayerMesh->GetAnimInstance())
    {
        return;
    }

    UAnimInstance* AnimInstance = PlayerMesh->GetAnimInstance();
    
    // Update player animation state
    if (AnimInstance->GetClass()->FindPropertyByName(TEXT("PlayerState")))
    {
        AnimInstance->GetClass()->FindPropertyByName(TEXT("PlayerState"))->SetBytePropertyValue(AnimInstance, (uint8)NewState);
    }
    
    // Apply state-specific modifiers
    switch (NewState)
    {
        case EPlayerAnimationState::Fearful:
            // Trembling, quick head movements, defensive posture
            if (AnimInstance->GetClass()->FindPropertyByName(TEXT("FearLevel")))
            {
                AnimInstance->GetClass()->FindPropertyByName(TEXT("FearLevel"))->SetFloatPropertyValue(AnimInstance, 1.0f);
            }
            break;
            
        case EPlayerAnimationState::Hiding:
            // Crouched, minimal movement, tense
            if (AnimInstance->GetClass()->FindPropertyByName(TEXT("HidingIntensity")))
            {
                AnimInstance->GetClass()->FindPropertyByName(TEXT("HidingIntensity"))->SetFloatPropertyValue(AnimInstance, 1.0f);
            }
            break;
            
        case EPlayerAnimationState::Exhausted:
            // Slower movements, heavy breathing, slumped posture
            if (AnimInstance->GetClass()->FindPropertyByName(TEXT("ExhaustionLevel")))
            {
                AnimInstance->GetClass()->FindPropertyByName(TEXT("ExhaustionLevel"))->SetFloatPropertyValue(AnimInstance, 1.0f);
            }
            break;
            
        default:
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Animation System: Player state changed to: %d"), (int32)NewState);
}

void UAnimationSystemCore::ApplyFearResponse(USkeletalMeshComponent* PlayerMesh, float FearIntensity)
{
    if (!PlayerMesh || !PlayerMesh->GetAnimInstance())
    {
        return;
    }

    UAnimInstance* AnimInstance = PlayerMesh->GetAnimInstance();
    
    // Clamp fear intensity
    FearIntensity = FMath::Clamp(FearIntensity, 0.0f, 1.0f);
    
    // Apply fear-based animation modifiers
    if (AnimInstance->GetClass()->FindPropertyByName(TEXT("FearIntensity")))
    {
        AnimInstance->GetClass()->FindPropertyByName(TEXT("FearIntensity"))->SetFloatPropertyValue(AnimInstance, FearIntensity);
    }
    
    // Increase movement speed when fear is high (fight or flight)
    float SpeedMultiplier = 1.0f + (FearIntensity * 0.5f);
    if (AnimInstance->GetClass()->FindPropertyByName(TEXT("MovementSpeedMultiplier")))
    {
        AnimInstance->GetClass()->FindPropertyByName(TEXT("MovementSpeedMultiplier"))->SetFloatPropertyValue(AnimInstance, SpeedMultiplier);
    }
    
    // Add trembling effect
    if (AnimInstance->GetClass()->FindPropertyByName(TEXT("TremblingIntensity")))
    {
        AnimInstance->GetClass()->FindPropertyByName(TEXT("TremblingIntensity"))->SetFloatPropertyValue(AnimInstance, FearIntensity * 0.3f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Animation System: Applied fear response with intensity: %f"), FearIntensity);
}

void UAnimationSystemCore::EnableFootIK(USkeletalMeshComponent* SkeletalMesh, bool bEnable)
{
    if (!SkeletalMesh || !SkeletalMesh->GetAnimInstance())
    {
        return;
    }

    UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance();
    
    // Enable/disable foot IK in the Animation Blueprint
    if (AnimInstance->GetClass()->FindPropertyByName(TEXT("bEnableFootIK")))
    {
        AnimInstance->GetClass()->FindPropertyByName(TEXT("bEnableFootIK"))->SetBoolPropertyValue(AnimInstance, bEnable);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Animation System: Foot IK %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UAnimationSystemCore::UpdateTerrainAdaptation(USkeletalMeshComponent* SkeletalMesh, const FVector& GroundNormal)
{
    if (!SkeletalMesh || !SkeletalMesh->GetAnimInstance())
    {
        return;
    }

    UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance();
    
    // Calculate terrain slope and apply to animation
    float SlopeAngle = FMath::Acos(FVector::DotProduct(GroundNormal, FVector::UpVector));
    float SlopeIntensity = FMath::Clamp(SlopeAngle / (PI * 0.25f), 0.0f, 1.0f); // Normalize to 0-1 for 45 degree max slope
    
    // Apply terrain adaptation parameters
    if (AnimInstance->GetClass()->FindPropertyByName(TEXT("TerrainSlopeIntensity")))
    {
        AnimInstance->GetClass()->FindPropertyByName(TEXT("TerrainSlopeIntensity"))->SetFloatPropertyValue(AnimInstance, SlopeIntensity);
    }
    
    if (AnimInstance->GetClass()->FindPropertyByName(TEXT("GroundNormal")))
    {
        AnimInstance->GetClass()->FindPropertyByName(TEXT("GroundNormal"))->SetStructPropertyValue(AnimInstance, &GroundNormal);
    }
}

void UAnimationSystemCore::SetupMotionMatchingSchemas()
{
    // This will be implemented when Motion Matching databases are created
    UE_LOG(LogTemp, Log, TEXT("Animation System: Setting up Motion Matching schemas..."));
}

void UAnimationSystemCore::ConfigureIKSolvers()
{
    // Configure IK solvers for different character types
    UE_LOG(LogTemp, Log, TEXT("Animation System: Configuring IK solvers..."));
}

void UAnimationSystemCore::InitializeDefaultPersonalities()
{
    // Initialize default personality presets for different creature types
    UE_LOG(LogTemp, Log, TEXT("Animation System: Initializing default personalities..."));
}