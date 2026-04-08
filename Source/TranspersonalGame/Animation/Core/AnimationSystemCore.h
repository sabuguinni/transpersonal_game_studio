#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearch.h"
#include "Animation/AnimNode_PoseSearchHistoryCollector.h"
#include "Animation/AnimNode_MotionMatching.h"
#include "IKRig.h"
#include "AnimationSystemCore.generated.h"

/**
 * Core Animation System for Transpersonal Game Studio's Jurassic Survival Game
 * 
 * Architecture Philosophy:
 * - Every movement tells a story about the character's internal state
 * - Motion Matching for fluid, contextual animation selection
 * - IK systems for environmental adaptation
 * - Individual variation system for unique creature personalities
 */

UENUM(BlueprintType)
enum class ECreatureArchetype : uint8
{
    // Human Character
    Paleontologist      UMETA(DisplayName = "Paleontologist"),
    
    // Herbivore Archetypes
    SmallHerbivore      UMETA(DisplayName = "Small Herbivore"),     // Compsognathus-like
    MediumHerbivore     UMETA(DisplayName = "Medium Herbivore"),    // Parasaurolophus-like
    LargeHerbivore      UMETA(DisplayName = "Large Herbivore"),     // Triceratops-like
    
    // Carnivore Archetypes
    SmallCarnivore      UMETA(DisplayName = "Small Carnivore"),     // Velociraptor-like
    MediumCarnivore     UMETA(DisplayName = "Medium Carnivore"),    // Allosaurus-like
    ApexPredator        UMETA(DisplayName = "Apex Predator"),       // T-Rex-like
    
    // Flying Creatures
    SmallFlyer          UMETA(DisplayName = "Small Flyer"),         // Pteranodon-like
    LargeFlyer          UMETA(DisplayName = "Large Flyer")          // Quetzalcoatlus-like
};

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm                UMETA(DisplayName = "Calm"),
    Alert               UMETA(DisplayName = "Alert"),
    Fearful             UMETA(DisplayName = "Fearful"),
    Aggressive          UMETA(DisplayName = "Aggressive"),
    Hunting             UMETA(DisplayName = "Hunting"),
    Feeding             UMETA(DisplayName = "Feeding"),
    Resting             UMETA(DisplayName = "Resting"),
    Curious             UMETA(DisplayName = "Curious"),
    Territorial         UMETA(DisplayName = "Territorial"),
    Domesticated        UMETA(DisplayName = "Domesticated")
};

USTRUCT(BlueprintType)
struct FCreaturePersonality
{
    GENERATED_BODY()

    // Physical Variations (affect animation subtly)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float MovementSpeedMultiplier = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.9", ClampMax = "1.1"))
    float PostureVariation = 1.0f;  // Affects spine curve, head position
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float StepLengthVariation = 1.0f;
    
    // Behavioral Tendencies
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Nervousness = 0.5f;  // How easily startled
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression = 0.5f;   // Tendency toward aggressive postures
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.5f;    // Head movements, investigation behaviors
    
    // Domestication Progress
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DomesticationLevel = 0.0f;  // 0 = wild, 1 = fully domesticated
    
    FCreaturePersonality()
    {
        MovementSpeedMultiplier = FMath::RandRange(0.9f, 1.1f);
        PostureVariation = FMath::RandRange(0.95f, 1.05f);
        StepLengthVariation = FMath::RandRange(0.9f, 1.1f);
        Nervousness = FMath::RandRange(0.3f, 0.8f);
        Aggression = FMath::RandRange(0.2f, 0.7f);
        Curiosity = FMath::RandRange(0.3f, 0.8f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnimationSystemCore : public UObject
{
    GENERATED_BODY()

public:
    UAnimationSystemCore();

    // Motion Matching Database Management
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    class UPoseSearchDatabase* GetMotionMatchingDatabase(ECreatureArchetype Archetype, EEmotionalState State);
    
    // Individual Variation System
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    FCreaturePersonality GenerateUniquePersonality(ECreatureArchetype Archetype);
    
    // Animation State Queries
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    bool ShouldUseStealthMovement(const AActor* Character, const AActor* NearbyThreat);
    
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    float CalculateFearIntensity(const AActor* Character, TArray<AActor*> NearbyThreats);

protected:
    // Motion Matching Databases per Archetype and State
    UPROPERTY(EditDefaultsOnly, Category = "Motion Matching")
    TMap<ECreatureArchetype, TObjectPtr<UPoseSearchDatabase>> BaseLocomotionDatabases;
    
    UPROPERTY(EditDefaultsOnly, Category = "Motion Matching")
    TMap<ECreatureArchetype, TObjectPtr<UPoseSearchDatabase>> CombatDatabases;
    
    UPROPERTY(EditDefaultsOnly, Category = "Motion Matching")
    TMap<ECreatureArchetype, TObjectPtr<UPoseSearchDatabase>> FeedingDatabases;
    
    UPROPERTY(EditDefaultsOnly, Category = "Motion Matching")
    TMap<ECreatureArchetype, TObjectPtr<UPoseSearchDatabase>> RestingDatabases;

private:
    // Personality variation seeds for consistent individual differences
    TMap<FString, FCreaturePersonality> PersonalityCache;
};