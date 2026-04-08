#include "ArchetypeAnimationSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/PoseSearchDatabase.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "IKRigDefinition.h"
#include "ControlRig.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Characters/CharacterArchetypes.h"
#include "JurassicAnimationAgent.h"

DEFINE_LOG_CATEGORY_STATIC(LogArchetypeAnimation, Log, All);

UArchetypeAnimationSystem::UArchetypeAnimationSystem()
{
    // Initialize default values
    DefaultBlendTime = 0.2f;
    GestureUpdateFrequency = 0.1f;
    EmotionalUpdateFrequency = 0.5f;
    bEnableDebugLogging = false;
    
    // Initialize archetype configurations
    InitializeArchetypeConfigurations();
}

void UArchetypeAnimationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("ArchetypeAnimationSystem initialized"));
    
    // Setup update timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            GestureUpdateTimerHandle,
            this,
            &UArchetypeAnimationSystem::UpdateGestures,
            GestureUpdateFrequency,
            true
        );
        
        World->GetTimerManager().SetTimer(
            EmotionalUpdateTimerHandle,
            this,
            &UArchetypeAnimationSystem::UpdateEmotionalStates,
            EmotionalUpdateFrequency,
            true
        );
    }
}

void UArchetypeAnimationSystem::Deinitialize()
{
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(GestureUpdateTimerHandle);
        World->GetTimerManager().ClearTimer(EmotionalUpdateTimerHandle);
    }
    
    // Clear all registered characters
    RegisteredCharacters.Empty();
    ArchetypeConfigurations.Empty();
    
    Super::Deinitialize();
}

void UArchetypeAnimationSystem::RegisterCharacter(ACharacter* Character, ECharacterArchetype Archetype)
{
    if (!Character)
    {
        UE_LOG(LogArchetypeAnimation, Warning, TEXT("Attempted to register null character"));
        return;
    }
    
    // Remove existing registration if present
    UnregisterCharacter(Character);
    
    // Create new animation state
    FCharacterAnimationState NewState;
    NewState.CurrentEmotionalState = EEmotionalState::Neutral;
    NewState.CurrentSurvivalState = ESurvivalState::Fresh;
    NewState.CurrentStressLevel = EStressLevel::Calm;
    
    RegisteredCharacters.Add(Character, NewState);
    
    // Apply archetype configuration
    ApplyArchetypeConfiguration(Character, Archetype);
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Registered character %s with archetype %d"), 
           *Character->GetName(), (int32)Archetype);
}

void UArchetypeAnimationSystem::UnregisterCharacter(ACharacter* Character)
{
    if (RegisteredCharacters.Contains(Character))
    {
        RegisteredCharacters.Remove(Character);
        UE_LOG(LogArchetypeAnimation, Log, TEXT("Unregistered character %s"), *Character->GetName());
    }
}

void UArchetypeAnimationSystem::UpdateCharacterEmotionalState(ACharacter* Character, EEmotionalState NewState)
{
    if (!Character || !RegisteredCharacters.Contains(Character))
    {
        return;
    }
    
    FCharacterAnimationState& AnimState = RegisteredCharacters[Character];
    EEmotionalState PreviousState = AnimState.CurrentEmotionalState;
    AnimState.CurrentEmotionalState = NewState;
    
    // Trigger emotional transition
    OnEmotionalStateChanged(Character, PreviousState, NewState);
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Character %s emotional state changed from %d to %d"), 
           *Character->GetName(), (int32)PreviousState, (int32)NewState);
}

void UArchetypeAnimationSystem::UpdateCharacterSurvivalState(ACharacter* Character, ESurvivalState NewState)
{
    if (!Character || !RegisteredCharacters.Contains(Character))
    {
        return;
    }
    
    FCharacterAnimationState& AnimState = RegisteredCharacters[Character];
    ESurvivalState PreviousState = AnimState.CurrentSurvivalState;
    AnimState.CurrentSurvivalState = NewState;
    
    // Trigger survival state transition
    OnSurvivalStateChanged(Character, PreviousState, NewState);
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Character %s survival state changed from %d to %d"), 
           *Character->GetName(), (int32)PreviousState, (int32)NewState);
}

void UArchetypeAnimationSystem::TriggerGesture(ACharacter* Character, EGestureType GestureType)
{
    if (!Character || !RegisteredCharacters.Contains(Character))
    {
        return;
    }
    
    // Find archetype configuration
    ECharacterArchetype Archetype = GetCharacterArchetype(Character);
    if (!ArchetypeConfigurations.Contains(Archetype))
    {
        return;
    }
    
    const FArchetypeAnimationConfig& Config = ArchetypeConfigurations[Archetype];
    
    // Check if gesture animation exists
    if (Config.GestureAnimations.Contains(GestureType))
    {
        TSoftObjectPtr<UAnimMontage> GestureMontage = Config.GestureAnimations[GestureType];
        if (UAnimMontage* Montage = GestureMontage.LoadSynchronous())
        {
            // Play gesture montage
            if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
            {
                if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
                {
                    AnimInstance->Montage_Play(Montage);
                    
                    // Update last gesture time
                    FCharacterAnimationState& AnimState = RegisteredCharacters[Character];
                    AnimState.LastGestureTime = GetWorld()->GetTimeSeconds();
                    
                    UE_LOG(LogArchetypeAnimation, Log, TEXT("Triggered gesture %d for character %s"), 
                           (int32)GestureType, *Character->GetName());
                }
            }
        }
    }
}

void UArchetypeAnimationSystem::SetCharacterFatigueLevel(ACharacter* Character, float FatigueLevel)
{
    if (!Character || !RegisteredCharacters.Contains(Character))
    {
        return;
    }
    
    FCharacterAnimationState& AnimState = RegisteredCharacters[Character];
    AnimState.FatigueLevel = FMath::Clamp(FatigueLevel, 0.0f, 1.0f);
    
    // Update animation blending based on fatigue
    UpdateFatigueAnimations(Character, AnimState.FatigueLevel);
}

void UArchetypeAnimationSystem::SetCharacterInjuryLevel(ACharacter* Character, float InjuryLevel)
{
    if (!Character || !RegisteredCharacters.Contains(Character))
    {
        return;
    }
    
    FCharacterAnimationState& AnimState = RegisteredCharacters[Character];
    AnimState.InjuryLevel = FMath::Clamp(InjuryLevel, 0.0f, 1.0f);
    
    // Update animation blending based on injury
    UpdateInjuryAnimations(Character, AnimState.InjuryLevel);
}

void UArchetypeAnimationSystem::SetCharacterFearLevel(ACharacter* Character, float FearLevel)
{
    if (!Character || !RegisteredCharacters.Contains(Character))
    {
        return;
    }
    
    FCharacterAnimationState& AnimState = RegisteredCharacters[Character];
    AnimState.FearLevel = FMath::Clamp(FearLevel, 0.0f, 1.0f);
    
    // Update emotional state based on fear level
    if (FearLevel > 0.7f)
    {
        UpdateCharacterEmotionalState(Character, EEmotionalState::Fearful);
    }
    else if (FearLevel > 0.4f)
    {
        UpdateCharacterEmotionalState(Character, EEmotionalState::Cautious);
    }
}

FCharacterAnimationState UArchetypeAnimationSystem::GetCharacterAnimationState(ACharacter* Character) const
{
    if (RegisteredCharacters.Contains(Character))
    {
        return RegisteredCharacters[Character];
    }
    
    return FCharacterAnimationState();
}

UPoseSearchDatabase* UArchetypeAnimationSystem::GetMotionMatchingDatabase(ACharacter* Character) const
{
    if (!Character || !RegisteredCharacters.Contains(Character))
    {
        return nullptr;
    }
    
    ECharacterArchetype Archetype = GetCharacterArchetype(Character);
    if (!ArchetypeConfigurations.Contains(Archetype))
    {
        return nullptr;
    }
    
    const FArchetypeAnimationConfig& Config = ArchetypeConfigurations[Archetype];
    const FCharacterAnimationState& AnimState = RegisteredCharacters[Character];
    
    // Select database based on current emotional state
    if (Config.EmotionalDatabases.Contains(AnimState.CurrentEmotionalState))
    {
        TSoftObjectPtr<UPoseSearchDatabase> DatabasePtr = Config.EmotionalDatabases[AnimState.CurrentEmotionalState];
        return DatabasePtr.LoadSynchronous();
    }
    
    // Fallback to survival state database
    if (Config.SurvivalDatabases.Contains(AnimState.CurrentSurvivalState))
    {
        TSoftObjectPtr<UPoseSearchDatabase> DatabasePtr = Config.SurvivalDatabases[AnimState.CurrentSurvivalState];
        return DatabasePtr.LoadSynchronous();
    }
    
    return nullptr;
}

void UArchetypeAnimationSystem::InitializeArchetypeConfigurations()
{
    // Initialize Paleontologist configuration
    FArchetypeAnimationConfig PaleontologistConfig;
    PaleontologistConfig.Archetype = ECharacterArchetype::Protagonist_Paleontologist;
    PaleontologistConfig.bUseMotionMatching = true;
    PaleontologistConfig.bUseAdaptiveIK = true;
    PaleontologistConfig.bUseProceduralGestures = true;
    PaleontologistConfig.bUseEmotionalOverrides = true;
    PaleontologistConfig.BlendTime = 0.2f;
    PaleontologistConfig.GestureFrequency = 0.5f;
    PaleontologistConfig.EmotionalIntensity = 0.8f;
    
    ArchetypeConfigurations.Add(ECharacterArchetype::Protagonist_Paleontologist, PaleontologistConfig);
    
    // Add other archetype configurations as needed
    // TODO: Implement configurations for other character archetypes
}

void UArchetypeAnimationSystem::ApplyArchetypeConfiguration(ACharacter* Character, ECharacterArchetype Archetype)
{
    if (!ArchetypeConfigurations.Contains(Archetype))
    {
        UE_LOG(LogArchetypeAnimation, Warning, TEXT("No configuration found for archetype %d"), (int32)Archetype);
        return;
    }
    
    const FArchetypeAnimationConfig& Config = ArchetypeConfigurations[Archetype];
    
    // Apply configuration to character's animation blueprint
    if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
    {
        if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
        {
            // Set motion matching parameters
            if (Config.bUseMotionMatching)
            {
                // Configure motion matching component
                // This would be implemented based on the specific animation blueprint setup
            }
            
            // Set IK parameters
            if (Config.bUseAdaptiveIK)
            {
                // Configure adaptive IK component
                // This would be implemented based on the IK system setup
            }
        }
    }
}

void UArchetypeAnimationSystem::UpdateGestures()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (auto& CharacterPair : RegisteredCharacters)
    {
        ACharacter* Character = CharacterPair.Key;
        FCharacterAnimationState& AnimState = CharacterPair.Value;
        
        if (!Character)
        {
            continue;
        }
        
        // Check if it's time for a new gesture
        float TimeSinceLastGesture = CurrentTime - AnimState.LastGestureTime;
        ECharacterArchetype Archetype = GetCharacterArchetype(Character);
        
        if (ArchetypeConfigurations.Contains(Archetype))
        {
            const FArchetypeAnimationConfig& Config = ArchetypeConfigurations[Archetype];
            float GestureInterval = 1.0f / Config.GestureFrequency;
            
            if (TimeSinceLastGesture >= GestureInterval && Config.bUseProceduralGestures)
            {
                // Trigger appropriate gesture based on current state
                EGestureType GestureToTrigger = SelectAppropriateGesture(Character, AnimState);
                if (GestureToTrigger != EGestureType::MAX)
                {
                    TriggerGesture(Character, GestureToTrigger);
                }
            }
        }
    }
}

void UArchetypeAnimationSystem::UpdateEmotionalStates()
{
    for (auto& CharacterPair : RegisteredCharacters)
    {
        ACharacter* Character = CharacterPair.Key;
        FCharacterAnimationState& AnimState = CharacterPair.Value;
        
        if (!Character)
        {
            continue;
        }
        
        // Update emotional state based on environmental factors
        UpdateEmotionalStateBasedOnEnvironment(Character, AnimState);
    }
}

void UArchetypeAnimationSystem::OnEmotionalStateChanged(ACharacter* Character, EEmotionalState PreviousState, EEmotionalState NewState)
{
    // Handle emotional state transition
    // This could involve changing motion matching databases, triggering specific animations, etc.
    
    if (bEnableDebugLogging)
    {
        UE_LOG(LogArchetypeAnimation, Log, TEXT("Emotional state transition for %s: %d -> %d"), 
               *Character->GetName(), (int32)PreviousState, (int32)NewState);
    }
}

void UArchetypeAnimationSystem::OnSurvivalStateChanged(ACharacter* Character, ESurvivalState PreviousState, ESurvivalState NewState)
{
    // Handle survival state transition
    // This could involve changing animation sets, adjusting movement parameters, etc.
    
    if (bEnableDebugLogging)
    {
        UE_LOG(LogArchetypeAnimation, Log, TEXT("Survival state transition for %s: %d -> %d"), 
               *Character->GetName(), (int32)PreviousState, (int32)NewState);
    }
}

ECharacterArchetype UArchetypeAnimationSystem::GetCharacterArchetype(ACharacter* Character) const
{
    // This would typically be determined by the character's class or a component
    // For now, return the default archetype
    return ECharacterArchetype::Protagonist_Paleontologist;
}

EGestureType UArchetypeAnimationSystem::SelectAppropriateGesture(ACharacter* Character, const FCharacterAnimationState& AnimState) const
{
    // Select gesture based on current emotional and survival state
    switch (AnimState.CurrentEmotionalState)
    {
        case EEmotionalState::Curious:
            return EGestureType::ObservationPoint;
            
        case EEmotionalState::Fearful:
            return EGestureType::CheckSurroundings;
            
        case EEmotionalState::Cautious:
            return EGestureType::ListenCarefully;
            
        case EEmotionalState::Amazed:
            return EGestureType::ExpressAwe;
            
        case EEmotionalState::Exhausted:
            return EGestureType::DisplayExhaustion;
            
        default:
            return EGestureType::MAX; // No gesture
    }
}

void UArchetypeAnimationSystem::UpdateEmotionalStateBasedOnEnvironment(ACharacter* Character, FCharacterAnimationState& AnimState)
{
    // This would analyze environmental factors to determine emotional state
    // For now, implement basic logic
    
    if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
    {
        float CurrentSpeed = MovementComp->Velocity.Size();
        
        // Update fatigue based on movement
        if (CurrentSpeed > 400.0f) // Running
        {
            AnimState.FatigueLevel = FMath::Min(AnimState.FatigueLevel + 0.001f, 1.0f);
        }
        else if (CurrentSpeed < 50.0f) // Resting
        {
            AnimState.FatigueLevel = FMath::Max(AnimState.FatigueLevel - 0.0005f, 0.0f);
        }
        
        // Update emotional state based on fatigue
        if (AnimState.FatigueLevel > 0.8f)
        {
            AnimState.CurrentEmotionalState = EEmotionalState::Exhausted;
        }
        else if (AnimState.FatigueLevel < 0.2f && AnimState.CurrentEmotionalState == EEmotionalState::Exhausted)
        {
            AnimState.CurrentEmotionalState = EEmotionalState::Neutral;
        }
    }
}

void UArchetypeAnimationSystem::UpdateFatigueAnimations(ACharacter* Character, float FatigueLevel)
{
    // Apply fatigue effects to animations
    // This would modify animation parameters, blend weights, etc.
    
    if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
    {
        if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
        {
            // Set fatigue parameter in animation blueprint
            AnimInstance->SetCurveValue(FName("Fatigue"), FatigueLevel);
        }
    }
}

void UArchetypeAnimationSystem::UpdateInjuryAnimations(ACharacter* Character, float InjuryLevel)
{
    // Apply injury effects to animations
    // This would modify locomotion, add limping, etc.
    
    if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
    {
        if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
        {
            // Set injury parameter in animation blueprint
            AnimInstance->SetCurveValue(FName("Injury"), InjuryLevel);
        }
    }
}