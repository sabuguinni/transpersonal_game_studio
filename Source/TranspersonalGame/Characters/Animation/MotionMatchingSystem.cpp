#include "MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UMotionMatchingSystem::UMotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    SetComponentTickEnabled(true);
    
    CurrentArchetype = "Default";
    CurrentDatabase = nullptr;
    LastUpdateTime = 0.0f;
    UpdateFrequency = 30.0f;
}

void UMotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    CacheComponentReferences();
    
    // Initialize with default state
    CurrentMotionState = FMotionState();
    PreviousMotionState = FMotionState();
    
    UE_LOG(LogTemp, Log, TEXT("Motion Matching System initialized for archetype: %s"), *CurrentArchetype);
}

void UMotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!ShouldUpdateThisFrame())
    {
        return;
    }
    
    // Store previous state
    PreviousMotionState = CurrentMotionState;
    
    // Update movement data from character
    UpdateMovementData();
    
    // Select optimal database based on current state
    SelectOptimalDatabase();
    
    // Apply personality modifiers
    ApplyPersonalityModifiers();
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UMotionMatchingSystem::SetCharacterArchetype(const FString& ArchetypeName)
{
    if (CurrentArchetype != ArchetypeName)
    {
        CurrentArchetype = ArchetypeName;
        
        // Reset database selection to force re-evaluation
        CurrentDatabase = nullptr;
        
        UE_LOG(LogTemp, Log, TEXT("Character archetype changed to: %s"), *ArchetypeName);
    }
}

void UMotionMatchingSystem::LoadArchetypeConfiguration(const FArchetypeMotionConfig& Config)
{
    ArchetypeConfig = Config;
    
    // Force database re-selection
    CurrentDatabase = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("Archetype configuration loaded for: %s"), *CurrentArchetype);
}

void UMotionMatchingSystem::UpdateMotionState(const FMotionState& NewState)
{
    CurrentMotionState = NewState;
}

UPoseSearchDatabase* UMotionMatchingSystem::GetActiveDatabase() const
{
    return CurrentDatabase;
}

void UMotionMatchingSystem::SetEmotionalState(float Fear, float Alertness, float Stress)
{
    CurrentMotionState.FearLevel = FMath::Clamp(Fear, 0.0f, 1.0f);
    CurrentMotionState.AlertnessLevel = FMath::Clamp(Alertness, 0.0f, 1.0f);
    CurrentMotionState.StressLevel = FMath::Clamp(Stress, 0.0f, 1.0f);
    
    // Emotional state changes should trigger immediate database re-evaluation
    SelectOptimalDatabase();
}

void UMotionMatchingSystem::SetEnvironmentalFactors(bool bUneven, bool bDanger, bool bCarrying)
{
    CurrentMotionState.bIsOnUnevenTerrain = bUneven;
    CurrentMotionState.bIsNearDanger = bDanger;
    CurrentMotionState.bIsCarryingObject = bCarrying;
}

void UMotionMatchingSystem::StartInteraction(const FString& InteractionType)
{
    CurrentMotionState.bIsInteracting = true;
    CurrentMotionState.CurrentInteractionType = InteractionType;
    
    UE_LOG(LogTemp, Log, TEXT("Started interaction: %s"), *InteractionType);
}

void UMotionMatchingSystem::EndInteraction()
{
    CurrentMotionState.bIsInteracting = false;
    CurrentMotionState.CurrentInteractionType = "";
    
    UE_LOG(LogTemp, Log, TEXT("Ended interaction"));
}

float UMotionMatchingSystem::GetPersonalityWeight(const FString& PersonalityTrait) const
{
    if (PersonalityTrait == "Confidence")
    {
        return ArchetypeConfig.ConfidenceLevel;
    }
    else if (PersonalityTrait == "Caution")
    {
        return ArchetypeConfig.CautiousLevel;
    }
    else if (PersonalityTrait == "Energy")
    {
        return ArchetypeConfig.EnergyLevel;
    }
    
    return 0.5f; // Default neutral value
}

float UMotionMatchingSystem::GetEmotionalWeight(const FString& EmotionType) const
{
    if (EmotionType == "Fear")
    {
        return CurrentMotionState.FearLevel;
    }
    else if (EmotionType == "Alertness")
    {
        return CurrentMotionState.AlertnessLevel;
    }
    else if (EmotionType == "Stress")
    {
        return CurrentMotionState.StressLevel;
    }
    
    return 0.0f;
}

void UMotionMatchingSystem::DebugDrawMotionState() const
{
    if (!GetWorld())
        return;
        
    FVector ActorLocation = GetOwner()->GetActorLocation();
    FVector DebugLocation = ActorLocation + FVector(0, 0, 200);
    
    // Draw velocity vector
    if (CurrentMotionState.Speed > 0.1f)
    {
        FVector VelocityEnd = ActorLocation + CurrentMotionState.Velocity.GetSafeNormal() * 100.0f;
        DrawDebugDirectionalArrow(GetWorld(), ActorLocation, VelocityEnd, 50.0f, FColor::Green, false, -1.0f, 0, 3.0f);
    }
    
    // Draw emotional state as colored sphere
    FColor EmotionalColor = FColor::White;
    if (CurrentMotionState.FearLevel > 0.5f)
    {
        EmotionalColor = FColor::Red;
    }
    else if (CurrentMotionState.AlertnessLevel > 0.5f)
    {
        EmotionalColor = FColor::Yellow;
    }
    else if (CurrentMotionState.StressLevel < 0.3f)
    {
        EmotionalColor = FColor::Blue;
    }
    
    DrawDebugSphere(GetWorld(), DebugLocation, 25.0f, 12, EmotionalColor, false, -1.0f, 0, 2.0f);
}

FString UMotionMatchingSystem::GetDebugString() const
{
    FString DebugInfo = FString::Printf(TEXT("Motion Matching Debug - Archetype: %s\n"), *CurrentArchetype);
    DebugInfo += FString::Printf(TEXT("Speed: %.2f | Direction: %.2f\n"), CurrentMotionState.Speed, CurrentMotionState.Direction);
    DebugInfo += FString::Printf(TEXT("Fear: %.2f | Alert: %.2f | Stress: %.2f\n"), 
        CurrentMotionState.FearLevel, CurrentMotionState.AlertnessLevel, CurrentMotionState.StressLevel);
    DebugInfo += FString::Printf(TEXT("Confidence: %.2f | Caution: %.2f | Energy: %.2f\n"),
        ArchetypeConfig.ConfidenceLevel, ArchetypeConfig.CautiousLevel, ArchetypeConfig.EnergyLevel);
    
    if (CurrentDatabase)
    {
        DebugInfo += FString::Printf(TEXT("Active Database: %s"), *CurrentDatabase->GetName());
    }
    else
    {
        DebugInfo += TEXT("No Active Database");
    }
    
    return DebugInfo;
}

void UMotionMatchingSystem::UpdateMovementData()
{
    if (!MovementComponent)
        return;
        
    // Update basic movement data
    CurrentMotionState.Velocity = MovementComponent->Velocity;
    CurrentMotionState.Speed = CurrentMotionState.Velocity.Size();
    
    if (CurrentMotionState.Speed > 0.1f)
    {
        FVector ForwardVector = GetOwner()->GetActorForwardVector();
        FVector VelocityNormal = CurrentMotionState.Velocity.GetSafeNormal();
        CurrentMotionState.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityNormal)));
    }
    else
    {
        CurrentMotionState.Direction = 0.0f;
    }
    
    // Calculate acceleration
    if (GetWorld()->GetDeltaSeconds() > 0.0f)
    {
        CurrentMotionState.Acceleration = (CurrentMotionState.Velocity - PreviousMotionState.Velocity) / GetWorld()->GetDeltaSeconds();
    }
}

void UMotionMatchingSystem::SelectOptimalDatabase()
{
    UPoseSearchDatabase* NewDatabase = SelectDatabaseForCurrentState();
    
    if (NewDatabase != CurrentDatabase)
    {
        CurrentDatabase = NewDatabase;
        
        if (CurrentDatabase)
        {
            UE_LOG(LogTemp, Log, TEXT("Switched to database: %s"), *CurrentDatabase->GetName());
        }
    }
}

void UMotionMatchingSystem::ApplyPersonalityModifiers()
{
    // Apply personality-based modifiers to movement component
    if (!MovementComponent)
        return;
        
    // Confidence affects walk speed and posture
    float ConfidenceModifier = FMath::Lerp(0.8f, 1.2f, ArchetypeConfig.ConfidenceLevel);
    
    // Energy affects overall animation speed
    float EnergyModifier = FMath::Lerp(0.9f, 1.1f, ArchetypeConfig.EnergyLevel);
    
    // Apply modifiers (this would typically be done through animation blueprint variables)
    float FinalSpeedModifier = ConfidenceModifier * EnergyModifier * ArchetypeConfig.MovementSpeedMultiplier;
    
    // Note: In a real implementation, these values would be passed to the Animation Blueprint
    // through variables or curve data
}

UPoseSearchDatabase* UMotionMatchingSystem::SelectDatabaseForCurrentState() const
{
    // Priority system for database selection
    
    // 1. Interaction takes highest priority
    if (CurrentMotionState.bIsInteracting && ArchetypeConfig.InteractionDatabase.IsValid())
    {
        return ArchetypeConfig.InteractionDatabase.LoadSynchronous();
    }
    
    // 2. Emotional state influences selection
    float EmotionalInfluence = CalculateEmotionalInfluence();
    
    if (EmotionalInfluence > 0.7f && CurrentMotionState.FearLevel > 0.5f && ArchetypeConfig.FearDatabase.IsValid())
    {
        return ArchetypeConfig.FearDatabase.LoadSynchronous();
    }
    
    if (EmotionalInfluence > 0.5f && CurrentMotionState.AlertnessLevel > 0.6f && ArchetypeConfig.AlertDatabase.IsValid())
    {
        return ArchetypeConfig.AlertDatabase.LoadSynchronous();
    }
    
    // 3. Movement state
    if (CurrentMotionState.Speed < 0.1f && ArchetypeConfig.IdleDatabase.IsValid())
    {
        return ArchetypeConfig.IdleDatabase.LoadSynchronous();
    }
    
    // 4. Default locomotion
    if (ArchetypeConfig.LocomotionDatabase.IsValid())
    {
        return ArchetypeConfig.LocomotionDatabase.LoadSynchronous();
    }
    
    return nullptr;
}

float UMotionMatchingSystem::CalculateEmotionalInfluence() const
{
    return (CurrentMotionState.FearLevel + CurrentMotionState.AlertnessLevel + CurrentMotionState.StressLevel) / 3.0f;
}

float UMotionMatchingSystem::CalculateEnvironmentalInfluence() const
{
    float Influence = 0.0f;
    
    if (CurrentMotionState.bIsOnUnevenTerrain)
        Influence += 0.3f;
    if (CurrentMotionState.bIsNearDanger)
        Influence += 0.5f;
    if (CurrentMotionState.bIsCarryingObject)
        Influence += 0.2f;
        
    return FMath::Clamp(Influence, 0.0f, 1.0f);
}

bool UMotionMatchingSystem::ShouldUpdateThisFrame() const
{
    if (!GetWorld())
        return false;
        
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceLastUpdate = CurrentTime - LastUpdateTime;
    float UpdateInterval = 1.0f / UpdateFrequency;
    
    return TimeSinceLastUpdate >= UpdateInterval;
}

void UMotionMatchingSystem::CacheComponentReferences()
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        MovementComponent = Character->GetCharacterMovement();
        SkeletalMeshComponent = Character->GetMesh();
    }
}

// UArchetypeMotionDataAsset Implementation
FArchetypeMotionConfig UArchetypeMotionDataAsset::GetConfigurationForArchetype(const FString& ArchetypeName) const
{
    if (const FArchetypeMotionConfig* Config = ArchetypeConfigurations.Find(ArchetypeName))
    {
        return *Config;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Archetype configuration not found for: %s, using default"), *ArchetypeName);
    return DefaultConfiguration;
}

void UArchetypeMotionDataAsset::AddArchetypeConfiguration(const FString& ArchetypeName, const FArchetypeMotionConfig& Config)
{
    ArchetypeConfigurations.Add(ArchetypeName, Config);
}

TArray<FString> UArchetypeMotionDataAsset::GetAvailableArchetypes() const
{
    TArray<FString> Archetypes;
    ArchetypeConfigurations.GetKeys(Archetypes);
    return Archetypes;
}