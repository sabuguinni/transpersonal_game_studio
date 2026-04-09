#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/PoseAsset.h"
#include "Engine/DataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MotionMatchingSystem.generated.h"

class UPoseSearchDatabase;
class UPoseSearchSchema;
class UCharacterMovementComponent;

/**
 * Motion Matching configuration for different character archetypes
 * Each archetype has unique movement patterns and personality
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArchetypeMotionConfig
{
    GENERATED_BODY()

    // Base locomotion database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TSoftObjectPtr<UPoseSearchDatabase> LocomotionDatabase;

    // Idle variations database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TSoftObjectPtr<UPoseSearchDatabase> IdleDatabase;

    // Interaction database (picking up objects, etc)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TSoftObjectPtr<UPoseSearchDatabase> InteractionDatabase;

    // Emotional state databases
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TSoftObjectPtr<UPoseSearchDatabase> FearDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TSoftObjectPtr<UPoseSearchDatabase> CalmDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TSoftObjectPtr<UPoseSearchDatabase> AlertDatabase;

    // Movement style modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Style", meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float MovementSpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Style", meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float StepLengthMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Style", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float TurnSpeedMultiplier = 1.0f;

    // Personality-based animation weights
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ConfidenceLevel = 0.5f; // Affects posture and stride

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CautiousLevel = 0.5f; // Affects head movement and reaction speed

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EnergyLevel = 0.5f; // Affects animation playback speed and intensity

    FArchetypeMotionConfig()
    {
        MovementSpeedMultiplier = 1.0f;
        StepLengthMultiplier = 1.0f;
        TurnSpeedMultiplier = 1.0f;
        ConfidenceLevel = 0.5f;
        CautiousLevel = 0.5f;
        EnergyLevel = 0.5f;
    }
};

/**
 * Current motion state for Motion Matching queries
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMotionState
{
    GENERATED_BODY()

    // Movement data
    UPROPERTY(BlueprintReadWrite, Category = "Motion State")
    FVector Velocity;

    UPROPERTY(BlueprintReadWrite, Category = "Motion State")
    FVector Acceleration;

    UPROPERTY(BlueprintReadWrite, Category = "Motion State")
    float Speed;

    UPROPERTY(BlueprintReadWrite, Category = "Motion State")
    float Direction;

    // Emotional state
    UPROPERTY(BlueprintReadWrite, Category = "Motion State")
    float FearLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Motion State")
    float AlertnessLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Motion State")
    float StressLevel;

    // Environmental factors
    UPROPERTY(BlueprintReadWrite, Category = "Motion State")
    bool bIsOnUneven Terrain;

    UPROPERTY(BlueprintReadWrite, Category = "Motion State")
    bool bIsNearDanger;

    UPROPERTY(BlueprintReadWrite, Category = "Motion State")
    bool bIsCarryingObject;

    // Interaction state
    UPROPERTY(BlueprintReadWrite, Category = "Motion State")
    bool bIsInteracting;

    UPROPERTY(BlueprintReadWrite, Category = "Motion State")
    FString CurrentInteractionType;

    FMotionState()
    {
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        FearLevel = 0.0f;
        AlertnessLevel = 0.0f;
        StressLevel = 0.0f;
        bIsOnUnevenTerrain = false;
        bIsNearDanger = false;
        bIsCarryingObject = false;
        bIsInteracting = false;
        CurrentInteractionType = "";
    }
};

/**
 * Motion Matching System Component
 * Handles character-specific motion matching with archetype-based personality
 * Integrates with UE5's Pose Search system for fluid, responsive animation
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UMotionMatchingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UMotionMatchingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetCharacterArchetype(const FString& ArchetypeName);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void LoadArchetypeConfiguration(const FArchetypeMotionConfig& Config);

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    FString GetCurrentArchetype() const { return CurrentArchetype; }

    // Motion state management
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionState(const FMotionState& NewState);

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    FMotionState GetCurrentMotionState() const { return CurrentMotionState; }

    // Database selection
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UPoseSearchDatabase* GetActiveDatabase() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetEmotionalState(float Fear, float Alertness, float Stress);

    // Environmental awareness
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetEnvironmentalFactors(bool bUneven, bool bDanger, bool bCarrying);

    // Interaction system
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void StartInteraction(const FString& InteractionType);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void EndInteraction();

    // Animation blending weights
    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetPersonalityWeight(const FString& PersonalityTrait) const;

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetEmotionalWeight(const FString& EmotionType) const;

    // Debugging
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void DebugDrawMotionState() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FString GetDebugString() const;

protected:
    // Core configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FString CurrentArchetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FArchetypeMotionConfig ArchetypeConfig;

    // Motion state
    UPROPERTY(BlueprintReadOnly, Category = "Motion State")
    FMotionState CurrentMotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Motion State")
    FMotionState PreviousMotionState;

    // Component references
    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    // Database management
    UPROPERTY()
    UPoseSearchDatabase* CurrentDatabase;

    // Timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 30.0f; // Updates per second

    float LastUpdateTime;

private:
    // Internal state management
    void UpdateMovementData();
    void SelectOptimalDatabase();
    void ApplyPersonalityModifiers();
    
    // Database selection logic
    UPoseSearchDatabase* SelectDatabaseForCurrentState() const;
    float CalculateEmotionalInfluence() const;
    float CalculateEnvironmentalInfluence() const;

    // Performance optimization
    bool ShouldUpdateThisFrame() const;
    void CacheComponentReferences();
};

/**
 * Data Asset for storing archetype motion configurations
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UArchetypeMotionDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    // Archetype configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetypes")
    TMap<FString, FArchetypeMotionConfig> ArchetypeConfigurations;

    // Default fallback configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
    FArchetypeMotionConfig DefaultConfiguration;

    UFUNCTION(BlueprintPure, Category = "Archetypes")
    FArchetypeMotionConfig GetConfigurationForArchetype(const FString& ArchetypeName) const;

    UFUNCTION(BlueprintCallable, Category = "Archetypes")
    void AddArchetypeConfiguration(const FString& ArchetypeName, const FArchetypeMotionConfig& Config);

    UFUNCTION(BlueprintPure, Category = "Archetypes")
    TArray<FString> GetAvailableArchetypes() const;
};