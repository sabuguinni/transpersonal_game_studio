#include "ArchetypeAnimationSystem.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogArchetypeAnimation, Log, All);

UArchetypeAnimationSystem::UArchetypeAnimationSystem()
{
    // Initialize default archetype configurations
    InitializeArchetypeConfigurations();
}

void UArchetypeAnimationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Archetype Animation System initialized"));
    
    // Load archetype data assets
    LoadArchetypeDataAssets();
    
    // Initialize motion matching databases
    InitializeMotionMatchingDatabases();
    
    // Setup behavioral patterns
    SetupBehavioralPatterns();
}

void UArchetypeAnimationSystem::Deinitialize()
{
    // Clean up registered characters
    RegisteredCharacters.Empty();
    ArchetypeConfigurations.Empty();
    MotionMatchingDatabases.Empty();
    
    Super::Deinitialize();
}

void UArchetypeAnimationSystem::RegisterCharacter(ACharacter* Character, EBodyLanguageArchetype Archetype)
{
    if (!Character)
    {
        UE_LOG(LogArchetypeAnimation, Warning, TEXT("Attempted to register null character"));
        return;
    }
    
    // Create archetype data for character
    FCharacterArchetypeData ArchetypeData;
    ArchetypeData.Character = Character;
    ArchetypeData.Archetype = Archetype;
    ArchetypeData.BodyLanguage = GetArchetypeBodyLanguage(Archetype);
    ArchetypeData.MotionData = GetArchetypeMotionData(Archetype);
    ArchetypeData.SurvivalState = ESurvivalState::Fresh;
    ArchetypeData.StressLevel = EStressLevel::Calm;
    ArchetypeData.EmotionalState = EEmotionalState::Neutral;
    ArchetypeData.LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    RegisteredCharacters.Add(Character, ArchetypeData);
    
    // Apply initial archetype configuration
    ApplyArchetypeConfiguration(Character, ArchetypeData);
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Registered character %s with archetype %s"), 
           *Character->GetName(), 
           *UEnum::GetValueAsString(Archetype));
}

void UArchetypeAnimationSystem::UnregisterCharacter(ACharacter* Character)
{
    if (RegisteredCharacters.Contains(Character))
    {
        RegisteredCharacters.Remove(Character);
        UE_LOG(LogArchetypeAnimation, Log, TEXT("Unregistered character %s"), *Character->GetName());
    }
}

void UArchetypeAnimationSystem::UpdateCharacterState(ACharacter* Character, ESurvivalState NewSurvivalState, EStressLevel NewStressLevel, EEmotionalState NewEmotionalState)
{
    if (!RegisteredCharacters.Contains(Character))
    {
        return;
    }
    
    FCharacterArchetypeData& ArchetypeData = RegisteredCharacters[Character];
    
    // Update states
    ArchetypeData.SurvivalState = NewSurvivalState;
    ArchetypeData.StressLevel = NewStressLevel;
    ArchetypeData.EmotionalState = NewEmotionalState;
    ArchetypeData.LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    // Apply state-based modifications
    ApplyStateModifications(Character, ArchetypeData);
    
    // Update motion matching database if needed
    UpdateMotionMatchingDatabase(Character, ArchetypeData);
}

FArchetypeBodyLanguage UArchetypeAnimationSystem::GetArchetypeBodyLanguage(EBodyLanguageArchetype Archetype) const
{
    if (const FArchetypeBodyLanguage* Found = ArchetypeConfigurations.Find(Archetype))
    {
        return *Found;
    }
    
    // Return default configuration
    return FArchetypeBodyLanguage();
}

FArchetypeMotionData UArchetypeAnimationSystem::GetArchetypeMotionData(EBodyLanguageArchetype Archetype) const
{
    if (const FArchetypeMotionData* Found = MotionDataConfigurations.Find(Archetype))
    {
        return *Found;
    }
    
    // Return default motion data
    return FArchetypeMotionData();
}

void UArchetypeAnimationSystem::ApplyPersonalityModifiers(ACharacter* Character, const FArchetypeBodyLanguage& BodyLanguage)
{
    if (!Character)
    {
        return;
    }
    
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!MovementComp)
    {
        return;
    }
    
    // Apply movement speed modifiers
    float BaseMaxWalkSpeed = MovementComp->GetClass()->GetDefaultObject<UCharacterMovementComponent>()->MaxWalkSpeed;
    MovementComp->MaxWalkSpeed = BaseMaxWalkSpeed * BodyLanguage.MovementSpeed;
    
    // Apply other movement modifiers based on personality
    switch (BodyLanguage.MovementPersonality)
    {
        case EMovementPersonality::Cautious:
            MovementComp->MaxAcceleration *= 0.7f;
            MovementComp->BrakingDecelerationWalking *= 1.5f;
            break;
            
        case EMovementPersonality::Confident:
            MovementComp->MaxAcceleration *= 1.3f;
            break;
            
        case EMovementPersonality::Nervous:
            MovementComp->MaxAcceleration *= 1.5f;
            MovementComp->BrakingDecelerationWalking *= 2.0f;
            break;
            
        case EMovementPersonality::Aggressive:
            MovementComp->MaxAcceleration *= 1.4f;
            MovementComp->MaxWalkSpeed *= 1.2f;
            break;
            
        case EMovementPersonality::Exhausted:
            MovementComp->MaxWalkSpeed *= 0.6f;
            MovementComp->MaxAcceleration *= 0.5f;
            break;
            
        default:
            break;
    }
}

void UArchetypeAnimationSystem::UpdateBehavioralAnimation(ACharacter* Character, float DeltaTime)
{
    if (!RegisteredCharacters.Contains(Character))
    {
        return;
    }
    
    FCharacterArchetypeData& ArchetypeData = RegisteredCharacters[Character];
    
    // Update behavioral timers
    ArchetypeData.BehaviorTimer += DeltaTime;
    
    // Update based on archetype
    switch (ArchetypeData.Archetype)
    {
        case EBodyLanguageArchetype::ScientificCuriosity:
            UpdateScientificBehavior(Character, ArchetypeData, DeltaTime);
            break;
            
        case EBodyLanguageArchetype::ConstantVigilance:
            UpdateVigilantBehavior(Character, ArchetypeData, DeltaTime);
            break;
            
        case EBodyLanguageArchetype::TerritorialConfidence:
            UpdateTerritorialBehavior(Character, ArchetypeData, DeltaTime);
            break;
            
        default:
            UpdateGenericBehavior(Character, ArchetypeData, DeltaTime);
            break;
    }
    
    // Apply micro-expressions and gestures
    ApplyMicroExpressions(Character, ArchetypeData);
    
    // Update posture based on current state
    UpdatePosture(Character, ArchetypeData);
}

void UArchetypeAnimationSystem::InitializeArchetypeConfigurations()
{
    // Scientific Curiosity (Paleontologist)
    FArchetypeBodyLanguage ScientificConfig;
    ScientificConfig.Archetype = EBodyLanguageArchetype::ScientificCuriosity;
    ScientificConfig.MovementPersonality = EMovementPersonality::Contemplative;
    ScientificConfig.PostureWeight = 0.8f;
    ScientificConfig.GestureFrequency = 0.7f;
    ScientificConfig.EyeContactLevel = 0.9f;
    ScientificConfig.PersonalSpaceRadius = 120.0f;
    ScientificConfig.MovementSpeed = 0.9f;
    ScientificConfig.TensionLevel = 0.2f;
    ScientificConfig.HeadTiltAngle = 5.0f;
    ScientificConfig.ShoulderTension = 0.1f;
    ScientificConfig.HandGestureStyle = 0.8f;
    ScientificConfig.BlinkRate = 0.8f;
    ScientificConfig.BreathingDepth = 1.2f;
    ArchetypeConfigurations.Add(EBodyLanguageArchetype::ScientificCuriosity, ScientificConfig);
    
    // Constant Vigilance (Survivor)
    FArchetypeBodyLanguage VigilantConfig;
    VigilantConfig.Archetype = EBodyLanguageArchetype::ConstantVigilance;
    VigilantConfig.MovementPersonality = EMovementPersonality::Hyperalert;
    VigilantConfig.PostureWeight = 1.0f;
    VigilantConfig.GestureFrequency = 0.3f;
    VigilantConfig.EyeContactLevel = 0.6f;
    VigilantConfig.PersonalSpaceRadius = 200.0f;
    VigilantConfig.MovementSpeed = 1.1f;
    VigilantConfig.TensionLevel = 0.8f;
    VigilantConfig.HeadTiltAngle = 0.0f;
    VigilantConfig.ShoulderTension = 0.7f;
    VigilantConfig.HandGestureStyle = 0.2f;
    VigilantConfig.BlinkRate = 1.5f;
    VigilantConfig.BreathingDepth = 0.8f;
    ArchetypeConfigurations.Add(EBodyLanguageArchetype::ConstantVigilance, VigilantConfig);
    
    // Territorial Confidence (Explorer)
    FArchetypeBodyLanguage TerritorialConfig;
    TerritorialConfig.Archetype = EBodyLanguageArchetype::TerritorialConfidence;
    TerritorialConfig.MovementPersonality = EMovementPersonality::Confident;
    TerritorialConfig.PostureWeight = 1.2f;
    TerritorialConfig.GestureFrequency = 0.6f;
    TerritorialConfig.EyeContactLevel = 0.8f;
    TerritorialConfig.PersonalSpaceRadius = 150.0f;
    TerritorialConfig.MovementSpeed = 1.2f;
    TerritorialConfig.TensionLevel = 0.3f;
    TerritorialConfig.HeadTiltAngle = -2.0f;
    TerritorialConfig.ShoulderTension = 0.2f;
    TerritorialConfig.HandGestureStyle = 0.7f;
    TerritorialConfig.BlinkRate = 0.9f;
    TerritorialConfig.BreathingDepth = 1.1f;
    ArchetypeConfigurations.Add(EBodyLanguageArchetype::TerritorialConfidence, TerritorialConfig);
    
    // Add other archetypes...
    InitializeRemainingArchetypes();
}

void UArchetypeAnimationSystem::InitializeRemainingArchetypes()
{
    // Predatory Focus (Hunter)
    FArchetypeBodyLanguage PredatoryConfig;
    PredatoryConfig.Archetype = EBodyLanguageArchetype::PredatoryFocus;
    PredatoryConfig.MovementPersonality = EMovementPersonality::Aggressive;
    PredatoryConfig.PostureWeight = 1.1f;
    PredatoryConfig.GestureFrequency = 0.4f;
    PredatoryConfig.EyeContactLevel = 1.0f;
    PredatoryConfig.PersonalSpaceRadius = 180.0f;
    PredatoryConfig.MovementSpeed = 1.3f;
    PredatoryConfig.TensionLevel = 0.6f;
    PredatoryConfig.HeadTiltAngle = -1.0f;
    PredatoryConfig.ShoulderTension = 0.5f;
    PredatoryConfig.HandGestureStyle = 0.3f;
    PredatoryConfig.BlinkRate = 0.7f;
    PredatoryConfig.BreathingDepth = 0.9f;
    ArchetypeConfigurations.Add(EBodyLanguageArchetype::PredatoryFocus, PredatoryConfig);
    
    // Spiritual Centering (Shaman)
    FArchetypeBodyLanguage SpiritualConfig;
    SpiritualConfig.Archetype = EBodyLanguageArchetype::SpiritualCentering;
    SpiritualConfig.MovementPersonality = EMovementPersonality::Spiritual;
    SpiritualConfig.PostureWeight = 0.9f;
    SpiritualConfig.GestureFrequency = 0.8f;
    SpiritualConfig.EyeContactLevel = 0.7f;
    SpiritualConfig.PersonalSpaceRadius = 100.0f;
    SpiritualConfig.MovementSpeed = 0.8f;
    SpiritualConfig.TensionLevel = 0.1f;
    SpiritualConfig.HeadTiltAngle = 3.0f;
    SpiritualConfig.ShoulderTension = 0.0f;
    SpiritualConfig.HandGestureStyle = 1.0f;
    SpiritualConfig.BlinkRate = 0.6f;
    SpiritualConfig.BreathingDepth = 1.5f;
    ArchetypeConfigurations.Add(EBodyLanguageArchetype::SpiritualCentering, SpiritualConfig);
}

void UArchetypeAnimationSystem::LoadArchetypeDataAssets()
{
    // Load archetype-specific data assets
    // This would typically load from DataAssets or configuration files
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Loading archetype data assets"));
}

void UArchetypeAnimationSystem::InitializeMotionMatchingDatabases()
{
    // Initialize motion matching databases for each archetype
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Initializing motion matching databases"));
}

void UArchetypeAnimationSystem::SetupBehavioralPatterns()
{
    // Setup behavioral patterns for each archetype
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Setting up behavioral patterns"));
}

void UArchetypeAnimationSystem::ApplyArchetypeConfiguration(ACharacter* Character, const FCharacterArchetypeData& ArchetypeData)
{
    if (!Character)
    {
        return;
    }
    
    // Apply body language configuration
    ApplyPersonalityModifiers(Character, ArchetypeData.BodyLanguage);
    
    // Setup animation blueprint variables if available
    UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
    if (AnimInstance)
    {
        // Set archetype-specific animation variables
        // This would typically involve setting variables on a custom AnimInstance
    }
}

void UArchetypeAnimationSystem::ApplyStateModifications(ACharacter* Character, FCharacterArchetypeData& ArchetypeData)
{
    // Modify behavior based on survival state
    switch (ArchetypeData.SurvivalState)
    {
        case ESurvivalState::Fresh:
            ArchetypeData.BodyLanguage.MovementSpeed *= 1.0f;
            ArchetypeData.BodyLanguage.TensionLevel *= 0.8f;
            break;
            
        case ESurvivalState::Desperate:
            ArchetypeData.BodyLanguage.MovementSpeed *= 1.3f;
            ArchetypeData.BodyLanguage.TensionLevel *= 1.5f;
            break;
            
        case ESurvivalState::Broken:
            ArchetypeData.BodyLanguage.MovementSpeed *= 0.6f;
            ArchetypeData.BodyLanguage.PostureWeight *= 0.7f;
            break;
            
        default:
            break;
    }
    
    // Modify behavior based on stress level
    switch (ArchetypeData.StressLevel)
    {
        case EStressLevel::Panicked:
            ArchetypeData.BodyLanguage.GestureFrequency *= 2.0f;
            ArchetypeData.BodyLanguage.BlinkRate *= 2.5f;
            break;
            
        case EStressLevel::Tense:
            ArchetypeData.BodyLanguage.ShoulderTension *= 1.5f;
            ArchetypeData.BodyLanguage.BreathingDepth *= 0.8f;
            break;
            
        default:
            break;
    }
}

void UArchetypeAnimationSystem::UpdateMotionMatchingDatabase(ACharacter* Character, const FCharacterArchetypeData& ArchetypeData)
{
    // Update motion matching database based on current state
    // This would select appropriate databases for the current archetype and state combination
}

void UArchetypeAnimationSystem::UpdateScientificBehavior(ACharacter* Character, FCharacterArchetypeData& ArchetypeData, float DeltaTime)
{
    // Scientific curiosity behavior: frequent observation pauses, head movements
    if (ArchetypeData.BehaviorTimer > 3.0f) // Every 3 seconds
    {
        // Trigger observation behavior
        ArchetypeData.BehaviorTimer = 0.0f;
    }
}

void UArchetypeAnimationSystem::UpdateVigilantBehavior(ACharacter* Character, FCharacterArchetypeData& ArchetypeData, float DeltaTime)
{
    // Vigilant behavior: constant scanning, quick reactions
    if (ArchetypeData.BehaviorTimer > 1.5f) // Every 1.5 seconds
    {
        // Trigger scanning behavior
        ArchetypeData.BehaviorTimer = 0.0f;
    }
}

void UArchetypeAnimationSystem::UpdateTerritorialBehavior(ACharacter* Character, FCharacterArchetypeData& ArchetypeData, float DeltaTime)
{
    // Territorial behavior: confident posture, territorial gestures
    if (ArchetypeData.BehaviorTimer > 4.0f) // Every 4 seconds
    {
        // Trigger territorial behavior
        ArchetypeData.BehaviorTimer = 0.0f;
    }
}

void UArchetypeAnimationSystem::UpdateGenericBehavior(ACharacter* Character, FCharacterArchetypeData& ArchetypeData, float DeltaTime)
{
    // Generic behavior updates
    if (ArchetypeData.BehaviorTimer > 2.0f)
    {
        ArchetypeData.BehaviorTimer = 0.0f;
    }
}

void UArchetypeAnimationSystem::ApplyMicroExpressions(ACharacter* Character, const FCharacterArchetypeData& ArchetypeData)
{
    // Apply micro-expressions based on archetype and current state
    // This would typically involve facial animation or subtle bone adjustments
}

void UArchetypeAnimationSystem::UpdatePosture(ACharacter* Character, const FCharacterArchetypeData& ArchetypeData)
{
    // Update character posture based on archetype and current state
    // This would involve spine curve adjustments, shoulder positioning, etc.
}