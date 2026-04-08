#include "ArchetypeAnimationSystem.h"
#include "BehavioralAnimationSystem.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Animation/PoseSearchDatabase.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Characters/CharacterArchetypes.h"
#include "MotionMatching/MotionMatchingSubsystem.h"
#include "IK/AdaptiveIKComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogArchetypeAnimation, Log, All);

UArchetypeAnimationSystem::UArchetypeAnimationSystem()
{
    // Initialize default archetype configurations
    InitializeArchetypeConfigurations();
}

void UArchetypeAnimationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Archetype Animation System Initialized"));
    
    // Initialize archetype data
    InitializeArchetypeConfigurations();
    
    // Cache motion matching subsystem
    MotionMatchingSubsystem = GetWorld()->GetSubsystem<UMotionMatchingSubsystem>();
}

void UArchetypeAnimationSystem::InitializeArchetypeConfigurations()
{
    // Clear existing configurations
    ArchetypeConfigurations.Empty();
    
    // SCIENTIFIC CURIOSITY - Paleontologist
    FArchetypeBodyLanguage ScientificConfig;
    ScientificConfig.Archetype = EBodyLanguageArchetype::ScientificCuriosity;
    ScientificConfig.MovementPersonality = EMovementPersonality::Contemplative;
    ScientificConfig.PostureWeight = 0.8f; // Slightly hunched, observational
    ScientificConfig.GestureFrequency = 0.6f; // Moderate gesturing
    ScientificConfig.EyeContactLevel = 0.9f; // High attention to details
    ScientificConfig.PersonalSpaceRadius = 120.0f; // Comfortable with close observation
    ScientificConfig.MovementSpeed = 0.85f; // Deliberate, careful movement
    ScientificConfig.TensionLevel = 0.2f; // Low tension, focused
    ScientificConfig.HeadTiltAngle = 5.0f; // Slight inquisitive tilt
    ScientificConfig.ShoulderTension = 0.1f; // Relaxed shoulders
    ScientificConfig.HandGestureStyle = 0.7f; // Precise, pointing gestures
    ScientificConfig.BlinkRate = 0.8f; // Slower blinking, concentration
    ScientificConfig.BreathingDepth = 0.9f; // Deep, controlled breathing
    ArchetypeConfigurations.Add(EBodyLanguageArchetype::ScientificCuriosity, ScientificConfig);
    
    // CONSTANT VIGILANCE - Survivor
    FArchetypeBodyLanguage VigilanceConfig;
    VigilanceConfig.Archetype = EBodyLanguageArchetype::ConstantVigilance;
    VigilanceConfig.MovementPersonality = EMovementPersonality::Hyperalert;
    VigilanceConfig.PostureWeight = 0.9f; // Upright, ready posture
    VigilanceConfig.GestureFrequency = 0.3f; // Minimal gesturing
    VigilanceConfig.EyeContactLevel = 0.95f; // Constantly scanning
    VigilanceConfig.PersonalSpaceRadius = 200.0f; // Larger safety zone
    VigilanceConfig.MovementSpeed = 1.1f; // Quick, efficient movement
    VigilanceConfig.TensionLevel = 0.7f; // High tension, ready to react
    VigilanceConfig.HeadTiltAngle = 0.0f; // Straight, alert head position
    VigilanceConfig.ShoulderTension = 0.8f; // Tense shoulders
    VigilanceConfig.HandGestureStyle = 0.2f; // Minimal hand movement
    VigilanceConfig.BlinkRate = 1.5f; // Rapid blinking, alertness
    VigilanceConfig.BreathingDepth = 0.6f; // Shallow, quick breathing
    ArchetypeConfigurations.Add(EBodyLanguageArchetype::ConstantVigilance, VigilanceConfig);
    
    // TERRITORIAL CONFIDENCE - Explorer
    FArchetypeBodyLanguage ConfidenceConfig;
    ConfidenceConfig.Archetype = EBodyLanguageArchetype::TerritorialConfidence;
    ConfidenceConfig.MovementPersonality = EMovementPersonality::Confident;
    ConfidenceConfig.PostureWeight = 1.0f; // Fully upright, commanding
    ConfidenceConfig.GestureFrequency = 0.8f; // Expressive gesturing
    ConfidenceConfig.EyeContactLevel = 0.85f; // Strong eye contact
    ConfidenceConfig.PersonalSpaceRadius = 180.0f; // Moderate space, not afraid
    ConfidenceConfig.MovementSpeed = 1.2f; // Bold, decisive movement
    ConfidenceConfig.TensionLevel = 0.4f; // Controlled tension
    ConfidenceConfig.HeadTiltAngle = -2.0f; // Slight chin up, confidence
    ConfidenceConfig.ShoulderTension = 0.3f; // Relaxed but ready
    ConfidenceConfig.HandGestureStyle = 0.9f; // Bold, sweeping gestures
    ConfidenceConfig.BlinkRate = 0.9f; // Normal, confident blinking
    ConfidenceConfig.BreathingDepth = 1.0f; // Deep, confident breathing
    ArchetypeConfigurations.Add(EBodyLanguageArchetype::TerritorialConfidence, ConfidenceConfig);
    
    // PREDATORY FOCUS - Hunter
    FArchetypeBodyLanguage PredatoryConfig;
    PredatoryConfig.Archetype = EBodyLanguageArchetype::PredatoryFocus;
    PredatoryConfig.MovementPersonality = EMovementPersonality::Aggressive;
    PredatoryConfig.PostureWeight = 0.95f; // Lean, predatory stance
    PredatoryConfig.GestureFrequency = 0.4f; // Controlled, purposeful gestures
    PredatoryConfig.EyeContactLevel = 1.0f; // Intense, focused gaze
    PredatoryConfig.PersonalSpaceRadius = 160.0f; // Comfortable with proximity
    PredatoryConfig.MovementSpeed = 1.15f; // Fluid, predatory movement
    PredatoryConfig.TensionLevel = 0.6f; // Coiled tension, ready to strike
    PredatoryConfig.HeadTiltAngle = -1.0f; // Slight forward lean
    PredatoryConfig.ShoulderTension = 0.6f; // Controlled tension
    PredatoryConfig.HandGestureStyle = 0.3f; // Minimal, precise movements
    PredatoryConfig.BlinkRate = 0.7f; // Slow, predatory blinking
    PredatoryConfig.BreathingDepth = 0.8f; // Controlled breathing
    ArchetypeConfigurations.Add(EBodyLanguageArchetype::PredatoryFocus, PredatoryConfig);
    
    // SPIRITUAL CENTERING - Shaman
    FArchetypeBodyLanguage SpiritualConfig;
    SpiritualConfig.Archetype = EBodyLanguageArchetype::SpiritualCentering;
    SpiritualConfig.MovementPersonality = EMovementPersonality::Spiritual;
    SpiritualConfig.PostureWeight = 0.9f; // Centered, balanced posture
    SpiritualConfig.GestureFrequency = 0.7f; // Flowing, meaningful gestures
    SpiritualConfig.EyeContactLevel = 0.6f; // Inward focus, occasional contact
    SpiritualConfig.PersonalSpaceRadius = 140.0f; // Comfortable with closeness
    SpiritualConfig.MovementSpeed = 0.8f; // Slow, deliberate movement
    SpiritualConfig.TensionLevel = 0.1f; // Very relaxed, centered
    SpiritualConfig.HeadTiltAngle = 3.0f; // Slight upward gaze
    SpiritualConfig.ShoulderTension = 0.05f; // Very relaxed shoulders
    SpiritualConfig.HandGestureStyle = 0.8f; // Flowing, graceful gestures
    SpiritualConfig.BlinkRate = 0.6f; // Slow, meditative blinking
    SpiritualConfig.BreathingDepth = 1.2f; // Deep, meditative breathing
    ArchetypeConfigurations.Add(EBodyLanguageArchetype::SpiritualCentering, SpiritualConfig);
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Initialized %d archetype configurations"), ArchetypeConfigurations.Num());
}

FArchetypeBodyLanguage UArchetypeAnimationSystem::GetArchetypeConfiguration(EBodyLanguageArchetype Archetype) const
{
    if (const FArchetypeBodyLanguage* Config = ArchetypeConfigurations.Find(Archetype))
    {
        return *Config;
    }
    
    // Return default configuration if not found
    UE_LOG(LogArchetypeAnimation, Warning, TEXT("Archetype configuration not found for %d, returning default"), (int32)Archetype);
    return FArchetypeBodyLanguage();
}

void UArchetypeAnimationSystem::ApplyArchetypeToCharacter(ACharacter* Character, EBodyLanguageArchetype Archetype)
{
    if (!Character)
    {
        UE_LOG(LogArchetypeAnimation, Error, TEXT("Cannot apply archetype to null character"));
        return;
    }
    
    FArchetypeBodyLanguage Config = GetArchetypeConfiguration(Archetype);
    
    // Apply to animation instance if available
    if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
    {
        // Set archetype variables in animation blueprint
        if (AnimInstance->GetClass()->ImplementsInterface(UArchetypeAnimationInterface::StaticClass()))
        {
            IArchetypeAnimationInterface::Execute_SetBodyLanguageArchetype(AnimInstance, Archetype);
            IArchetypeAnimationInterface::Execute_SetMovementPersonality(AnimInstance, Config.MovementPersonality);
            IArchetypeAnimationInterface::Execute_SetPostureWeight(AnimInstance, Config.PostureWeight);
            IArchetypeAnimationInterface::Execute_SetGestureFrequency(AnimInstance, Config.GestureFrequency);
            IArchetypeAnimationInterface::Execute_SetTensionLevel(AnimInstance, Config.TensionLevel);
        }
    }
    
    // Apply to adaptive IK component if available
    if (UAdaptiveIKComponent* IKComponent = Character->FindComponentByClass<UAdaptiveIKComponent>())
    {
        IKComponent->SetArchetypeConfiguration(Config);
    }
    
    // Cache character archetype
    CharacterArchetypes.Add(Character, Archetype);
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Applied archetype %d to character %s"), (int32)Archetype, *Character->GetName());
}

void UArchetypeAnimationSystem::UpdateCharacterEmotionalState(ACharacter* Character, EEmotionalState NewState, float Intensity)
{
    if (!Character)
    {
        return;
    }
    
    // Update emotional state in animation instance
    if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
    {
        if (AnimInstance->GetClass()->ImplementsInterface(UArchetypeAnimationInterface::StaticClass()))
        {
            IArchetypeAnimationInterface::Execute_SetEmotionalState(AnimInstance, NewState);
            IArchetypeAnimationInterface::Execute_SetEmotionalIntensity(AnimInstance, Intensity);
        }
    }
    
    // Cache emotional state
    FCharacterEmotionalData& EmotionalData = CharacterEmotionalStates.FindOrAdd(Character);
    EmotionalData.CurrentState = NewState;
    EmotionalData.Intensity = Intensity;
    EmotionalData.LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogArchetypeAnimation, Verbose, TEXT("Updated emotional state for %s: %d (Intensity: %.2f)"), 
           *Character->GetName(), (int32)NewState, Intensity);
}

void UArchetypeAnimationSystem::UpdateCharacterStressLevel(ACharacter* Character, EStressLevel NewStress)
{
    if (!Character)
    {
        return;
    }
    
    // Update stress level in animation instance
    if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
    {
        if (AnimInstance->GetClass()->ImplementsInterface(UArchetypeAnimationInterface::StaticClass()))
        {
            IArchetypeAnimationInterface::Execute_SetStressLevel(AnimInstance, NewStress);
        }
    }
    
    // Cache stress level
    FCharacterEmotionalData& EmotionalData = CharacterEmotionalStates.FindOrAdd(Character);
    EmotionalData.StressLevel = NewStress;
    EmotionalData.LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogArchetypeAnimation, Verbose, TEXT("Updated stress level for %s: %d"), 
           *Character->GetName(), (int32)NewStress);
}

void UArchetypeAnimationSystem::UpdateCharacterSurvivalState(ACharacter* Character, ESurvivalState NewSurvival)
{
    if (!Character)
    {
        return;
    }
    
    // Update survival state in animation instance
    if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
    {
        if (AnimInstance->GetClass()->ImplementsInterface(UArchetypeAnimationInterface::StaticClass()))
        {
            IArchetypeAnimationInterface::Execute_SetSurvivalState(AnimInstance, NewSurvival);
        }
    }
    
    // Cache survival state
    FCharacterEmotionalData& EmotionalData = CharacterEmotionalStates.FindOrAdd(Character);
    EmotionalData.SurvivalState = NewSurvival;
    EmotionalData.LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogArchetypeAnimation, Verbose, TEXT("Updated survival state for %s: %d"), 
           *Character->GetName(), (int32)NewSurvival);
}

EBodyLanguageArchetype UArchetypeAnimationSystem::GetCharacterArchetype(ACharacter* Character) const
{
    if (const EBodyLanguageArchetype* Archetype = CharacterArchetypes.Find(Character))
    {
        return *Archetype;
    }
    return EBodyLanguageArchetype::ScientificCuriosity; // Default
}

FCharacterEmotionalData UArchetypeAnimationSystem::GetCharacterEmotionalData(ACharacter* Character) const
{
    if (const FCharacterEmotionalData* Data = CharacterEmotionalStates.Find(Character))
    {
        return *Data;
    }
    return FCharacterEmotionalData(); // Default
}

void UArchetypeAnimationSystem::RegisterArchetypeAnimationInterface(UAnimInstance* AnimInstance)
{
    if (AnimInstance && AnimInstance->GetClass()->ImplementsInterface(UArchetypeAnimationInterface::StaticClass()))
    {
        RegisteredAnimInstances.AddUnique(AnimInstance);
        UE_LOG(LogArchetypeAnimation, Log, TEXT("Registered animation instance: %s"), *AnimInstance->GetName());
    }
}

void UArchetypeAnimationSystem::UnregisterArchetypeAnimationInterface(UAnimInstance* AnimInstance)
{
    RegisteredAnimInstances.Remove(AnimInstance);
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Unregistered animation instance: %s"), 
           AnimInstance ? *AnimInstance->GetName() : TEXT("NULL"));
}

void UArchetypeAnimationSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update emotional state decay
    UpdateEmotionalStateDecay(DeltaTime);
    
    // Update procedural animations
    UpdateProceduralAnimations(DeltaTime);
}

void UArchetypeAnimationSystem::UpdateEmotionalStateDecay(float DeltaTime)
{
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    const float DecayRate = 0.1f; // Emotional states decay over time
    
    for (auto& Pair : CharacterEmotionalStates)
    {
        FCharacterEmotionalData& Data = Pair.Value;
        
        // Decay intensity over time
        if (Data.Intensity > 0.0f)
        {
            Data.Intensity = FMath::Max(0.0f, Data.Intensity - (DecayRate * DeltaTime));
            
            // Update animation instance if intensity changed significantly
            if (Pair.Key && Data.Intensity < 0.1f)
            {
                UpdateCharacterEmotionalState(Pair.Key, EEmotionalState::Neutral, 0.0f);
            }
        }
    }
}

void UArchetypeAnimationSystem::UpdateProceduralAnimations(float DeltaTime)
{
    // Update procedural micro-expressions and gestures
    for (UAnimInstance* AnimInstance : RegisteredAnimInstances)
    {
        if (IsValid(AnimInstance) && AnimInstance->GetClass()->ImplementsInterface(UArchetypeAnimationInterface::StaticClass()))
        {
            // Update procedural breathing
            float BreathingCycle = FMath::Sin(GetWorld()->GetTimeSeconds() * 2.0f) * 0.5f + 0.5f;
            IArchetypeAnimationInterface::Execute_UpdateBreathingCycle(AnimInstance, BreathingCycle);
            
            // Update procedural blinking
            float BlinkCycle = FMath::Sin(GetWorld()->GetTimeSeconds() * 3.0f) * 0.5f + 0.5f;
            IArchetypeAnimationInterface::Execute_UpdateBlinkCycle(AnimInstance, BlinkCycle);
        }
    }
}

bool UArchetypeAnimationSystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return Super::ShouldCreateSubsystem(Outer);
}