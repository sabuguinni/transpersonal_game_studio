#include "AnimationAgent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/PoseSearchDatabase.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "IK/IKRigDefinition.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

void UAnimationAgent::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Animation Agent initialized - Ready to bring characters to life"));
    
    // Initialize archetype animation configurations
    InitializeArchetypeAnimations();
}

void UAnimationAgent::Deinitialize()
{
    // Clean up any active animation states
    ActiveCharacterStates.Empty();
    ArchetypeConfigurations.Empty();
    
    Super::Deinitialize();
}

void UAnimationAgent::InitializeArchetypeAnimations()
{
    // Initialize default configurations for each archetype
    for (int32 i = 0; i < static_cast<int32>(ECharacterArchetype::MAX); ++i)
    {
        ECharacterArchetype Archetype = static_cast<ECharacterArchetype>(i);
        FArchetypeAnimationData& Config = ArchetypeConfigurations.FindOrAdd(Archetype);
        
        Config.Archetype = Archetype;
        Config.bUseMotionMatching = true;
        Config.bUseAdaptiveIK = true;
        Config.bUseProceduralGestures = true;
        Config.BlendWeight = 1.0f;
        
        // Configure body language based on archetype
        SetupBodyLanguageForArchetype(Config.BodyLanguage, Archetype);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Animation Agent: Initialized %d archetype configurations"), ArchetypeConfigurations.Num());
}

void UAnimationAgent::SetupCharacterAnimation(ACharacter* Character, ECharacterArchetype Archetype)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation Agent: Cannot setup animation for null character"));
        return;
    }
    
    // Get or create animation configuration for this archetype
    FArchetypeAnimationData* Config = ArchetypeConfigurations.Find(Archetype);
    if (!Config)
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation Agent: No configuration found for archetype %d"), static_cast<int32>(Archetype));
        return;
    }
    
    // Setup Motion Matching if enabled
    if (Config->bUseMotionMatching)
    {
        SetupMotionMatchingForCharacter(Character, *Config);
    }
    
    // Setup Adaptive IK if enabled
    if (Config->bUseAdaptiveIK)
    {
        SetupAdaptiveIKForArchetype(Character, Archetype);
    }
    
    // Initialize character animation state
    FCharacterAnimationState& AnimState = ActiveCharacterStates.FindOrAdd(Character);
    AnimState.CurrentArchetype = Archetype;
    AnimState.LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Animation Agent: Setup animation for character with archetype %d"), static_cast<int32>(Archetype));
}

void UAnimationAgent::UpdateCharacterBodyLanguage(ACharacter* Character, float DeltaTime)
{
    if (!Character)
    {
        return;
    }
    
    FCharacterAnimationState* AnimState = ActiveCharacterStates.Find(Character);
    if (!AnimState)
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation Agent: No animation state found for character"));
        return;
    }
    
    // Update animation state based on character condition
    UpdateAnimationStateFromCharacter(Character, *AnimState, DeltaTime);
    
    // Apply body language modifications
    ApplyBodyLanguageToCharacter(Character, *AnimState, DeltaTime);
    
    // Update procedural gestures if enabled
    FArchetypeAnimationData* Config = ArchetypeConfigurations.Find(AnimState->CurrentArchetype);
    if (Config && Config->bUseProceduralGestures)
    {
        UpdateProceduralGestures(Character, *AnimState, Config->BodyLanguage, DeltaTime);
    }
}

UPoseSearchDatabase* UAnimationAgent::GetOptimalDatabaseForArchetype(ECharacterArchetype Archetype, ECharacterMovementState MovementState)
{
    FArchetypeAnimationData* Config = ArchetypeConfigurations.Find(Archetype);
    if (!Config)
    {
        return nullptr;
    }
    
    // Find the most appropriate database for current movement state
    TSoftObjectPtr<UPoseSearchDatabase>* DatabasePtr = Config->MotionMatchingDatabases.Find(MovementState);
    if (DatabasePtr && DatabasePtr->IsValid())
    {
        return DatabasePtr->LoadSynchronous();
    }
    
    // Fallback to default database if specific one not found
    DatabasePtr = Config->MotionMatchingDatabases.Find(ECharacterMovementState::Walking);
    if (DatabasePtr && DatabasePtr->IsValid())
    {
        return DatabasePtr->LoadSynchronous();
    }
    
    return nullptr;
}

void UAnimationAgent::UpdateMotionMatchingWeights(ACharacter* Character, const FBodyLanguageConfig& BodyLanguage)
{
    if (!Character || !Character->GetMesh())
    {
        return;
    }
    
    UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
    if (!AnimInstance)
    {
        return;
    }
    
    // Update motion matching weights based on body language configuration
    // This would typically involve setting parameters on the animation blueprint
    
    // Example: Set movement speed multiplier
    if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
    {
        float BaseSpeed = MovementComp->MaxWalkSpeed;
        MovementComp->MaxWalkSpeed = BaseSpeed * BodyLanguage.MovementSpeed;
    }
    
    // Set animation blueprint variables for body language
    // These would be consumed by the animation blueprint to blend animations appropriately
    AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);
}

float UAnimationAgent::CalculateBlendTimeForPersonality(EMovementPersonality Personality, ECharacterMovementState FromState, ECharacterMovementState ToState)
{
    float BaseBlendTime = 0.2f;
    
    // Adjust blend time based on personality
    switch (Personality)
    {
        case EMovementPersonality::Nervous:
            return BaseBlendTime * 0.5f; // Quick, jittery transitions
            
        case EMovementPersonality::Relaxed:
            return BaseBlendTime * 1.5f; // Slower, more fluid transitions
            
        case EMovementPersonality::Aggressive:
            return BaseBlendTime * 0.7f; // Sharp, decisive transitions
            
        case EMovementPersonality::Graceful:
            return BaseBlendTime * 1.2f; // Smooth, elegant transitions
            
        case EMovementPersonality::Tired:
            return BaseBlendTime * 2.0f; // Slow, labored transitions
            
        case EMovementPersonality::Alert:
            return BaseBlendTime * 0.6f; // Quick, responsive transitions
            
        default:
            return BaseBlendTime;
    }
}

void UAnimationAgent::SetupAdaptiveIKForArchetype(ACharacter* Character, ECharacterArchetype Archetype)
{
    if (!Character)
    {
        return;
    }
    
    FArchetypeAnimationData* Config = ArchetypeConfigurations.Find(Archetype);
    if (!Config || !Config->IKRigDefinition.IsValid())
    {
        return;
    }
    
    // Setup IK rig for this character
    // This would involve creating and configuring IK components
    // Implementation would depend on UE5's IK system
    
    UE_LOG(LogTemp, Log, TEXT("Animation Agent: Setup Adaptive IK for archetype %d"), static_cast<int32>(Archetype));
}

void UAnimationAgent::SetupBodyLanguageForArchetype(FBodyLanguageConfig& BodyLanguage, ECharacterArchetype Archetype)
{
    // Configure body language based on archetype characteristics
    switch (Archetype)
    {
        case ECharacterArchetype::Protagonist_Paleontologist:
            BodyLanguage.Style = EBodyLanguageStyle::ScientificCuriosity;
            BodyLanguage.MovementPersonality = EMovementPersonality::Scholarly;
            BodyLanguage.PostureWeight = 0.8f;
            BodyLanguage.GestureFrequency = 0.7f;
            BodyLanguage.EyeContactLevel = 0.9f;
            BodyLanguage.PersonalSpaceRadius = 120.0f;
            BodyLanguage.MovementSpeed = 0.9f;
            BodyLanguage.TensionLevel = 0.3f;
            break;
            
        case ECharacterArchetype::Survivor_Cautious:
            BodyLanguage.Style = EBodyLanguageStyle::ConstantCaution;
            BodyLanguage.MovementPersonality = EMovementPersonality::Cautious;
            BodyLanguage.PostureWeight = 0.6f;
            BodyLanguage.GestureFrequency = 0.3f;
            BodyLanguage.EyeContactLevel = 0.4f;
            BodyLanguage.PersonalSpaceRadius = 200.0f;
            BodyLanguage.MovementSpeed = 0.8f;
            BodyLanguage.TensionLevel = 0.7f;
            break;
            
        case ECharacterArchetype::Explorer_Bold:
            BodyLanguage.Style = EBodyLanguageStyle::TerritorialConfidence;
            BodyLanguage.MovementPersonality = EMovementPersonality::Confident;
            BodyLanguage.PostureWeight = 1.0f;
            BodyLanguage.GestureFrequency = 0.8f;
            BodyLanguage.EyeContactLevel = 0.8f;
            BodyLanguage.PersonalSpaceRadius = 100.0f;
            BodyLanguage.MovementSpeed = 1.2f;
            BodyLanguage.TensionLevel = 0.2f;
            break;
            
        case ECharacterArchetype::Hunter_Predator:
            BodyLanguage.Style = EBodyLanguageStyle::ControlledPredation;
            BodyLanguage.MovementPersonality = EMovementPersonality::Alert;
            BodyLanguage.PostureWeight = 0.9f;
            BodyLanguage.GestureFrequency = 0.4f;
            BodyLanguage.EyeContactLevel = 0.7f;
            BodyLanguage.PersonalSpaceRadius = 150.0f;
            BodyLanguage.MovementSpeed = 1.1f;
            BodyLanguage.TensionLevel = 0.5f;
            break;
            
        default:
            // Default configuration
            BodyLanguage.Style = EBodyLanguageStyle::ScientificCuriosity;
            BodyLanguage.MovementPersonality = EMovementPersonality::Cautious;
            BodyLanguage.PostureWeight = 0.7f;
            BodyLanguage.GestureFrequency = 0.5f;
            BodyLanguage.EyeContactLevel = 0.6f;
            BodyLanguage.PersonalSpaceRadius = 150.0f;
            BodyLanguage.MovementSpeed = 1.0f;
            BodyLanguage.TensionLevel = 0.4f;
            break;
    }
}

void UAnimationAgent::SetupMotionMatchingForCharacter(ACharacter* Character, const FArchetypeAnimationData& Config)
{
    if (!Character || !Character->GetMesh())
    {
        return;
    }
    
    // Setup motion matching databases for this character
    // This would involve configuring the motion matching component with appropriate databases
    
    UE_LOG(LogTemp, Log, TEXT("Animation Agent: Setup Motion Matching for character"));
}

void UAnimationAgent::UpdateAnimationStateFromCharacter(ACharacter* Character, FCharacterAnimationState& AnimState, float DeltaTime)
{
    if (!Character)
    {
        return;
    }
    
    // Update animation state based on character's current condition
    // This would involve reading from character components like health, stamina, etc.
    
    AnimState.LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    // Example: Update fatigue based on movement
    if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
    {
        if (MovementComp->Velocity.Size() > 100.0f)
        {
            AnimState.TimeSinceLastMovement = 0.0f;
        }
        else
        {
            AnimState.TimeSinceLastMovement += DeltaTime;
        }
    }
}

void UAnimationAgent::ApplyBodyLanguageToCharacter(ACharacter* Character, const FCharacterAnimationState& AnimState, float DeltaTime)
{
    if (!Character || !Character->GetMesh())
    {
        return;
    }
    
    UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
    if (!AnimInstance)
    {
        return;
    }
    
    // Apply body language modifications to the character's animation
    // This would involve setting animation blueprint parameters
    
    FArchetypeAnimationData* Config = ArchetypeConfigurations.Find(AnimState.CurrentArchetype);
    if (Config)
    {
        const FBodyLanguageConfig& BodyLanguage = Config->BodyLanguage;
        
        // Example: Set animation parameters based on body language
        // These would be consumed by the animation blueprint
        // AnimInstance->SetFloatValue("PostureWeight", BodyLanguage.PostureWeight);
        // AnimInstance->SetFloatValue("TensionLevel", BodyLanguage.TensionLevel);
        // AnimInstance->SetFloatValue("MovementSpeed", BodyLanguage.MovementSpeed);
    }
}

void UAnimationAgent::UpdateProceduralGestures(ACharacter* Character, FCharacterAnimationState& AnimState, const FBodyLanguageConfig& BodyLanguage, float DeltaTime)
{
    if (!Character)
    {
        return;
    }
    
    // Update procedural gesture system
    AnimState.LastGestureTime += DeltaTime;
    
    // Check if it's time for a new gesture based on frequency
    float GestureInterval = 1.0f / FMath::Max(BodyLanguage.GestureFrequency, 0.1f);
    
    if (AnimState.LastGestureTime >= GestureInterval)
    {
        // Trigger a procedural gesture based on current state and personality
        TriggerProceduralGesture(Character, AnimState, BodyLanguage);
        AnimState.LastGestureTime = 0.0f;
    }
}

void UAnimationAgent::TriggerProceduralGesture(ACharacter* Character, const FCharacterAnimationState& AnimState, const FBodyLanguageConfig& BodyLanguage)
{
    if (!Character)
    {
        return;
    }
    
    // Select and trigger appropriate gesture based on current state
    // This would involve playing gesture animations or montages
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Animation Agent: Triggered procedural gesture for character"));
}