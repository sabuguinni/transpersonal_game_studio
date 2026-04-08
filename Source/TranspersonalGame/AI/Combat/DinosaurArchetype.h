#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTags/GameplayTags.h"
#include "DinosaurArchetype.generated.h"

UENUM(BlueprintType)
enum class EDinosaurSize : uint8
{
    Small,      // Compsognathus, Microraptor
    Medium,     // Velociraptor, Deinonychus
    Large,      // Allosaurus, Carnotaurus
    Massive     // T-Rex, Spinosaurus
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorType : uint8
{
    Ambush,     // Stealth predators
    Pack,       // Coordinated group hunters
    Territorial,// Defend area aggressively
    Opportunist,// Scavenger, attacks when advantageous
    Apex        // Dominant predator, direct confrontation
};

UENUM(BlueprintType)
enum class EAttackPattern : uint8
{
    DirectCharge,
    CircleAndStrike,
    AmbushFromCover,
    PackCoordination,
    TerritorialDisplay
};

USTRUCT(BlueprintType)
struct FCombatStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FleeHealthThreshold = 0.3f;
};

USTRUCT(BlueprintType)
struct FTacticalBehavior
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CautiousDistance = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OptimalAttackDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUsesAmbushTactics = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanCoordinateWithPack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bTerritorialBehavior = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritoryRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PatrolRadius = 500.0f;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UDinosaurArchetype : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UDinosaurArchetype();

    // Basic Information
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Basic Info")
    FString DinosaurName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Basic Info")
    FString ScientificName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Basic Info")
    EDinosaurSize Size;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Basic Info")
    EDinosaurBehaviorType BehaviorType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Basic Info")
    FGameplayTag DinosaurTag;

    // Combat Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    FCombatStats CombatStats;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    FTacticalBehavior TacticalBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    EAttackPattern PrimaryAttackPattern;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    TArray<EAttackPattern> SecondaryAttackPatterns;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardAsset* BlackboardAsset;

    // Audio Cues for Combat Commentary
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    class USoundCue* SpottedPlayerSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    class USoundCue* AttackingSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    class USoundCue* TakingDamageSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    class USoundCue* LostPlayerSound;

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsApexPredator() const { return BehaviorType == EDinosaurBehaviorType::Apex; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsPackHunter() const { return BehaviorType == EDinosaurBehaviorType::Pack; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsAmbushPredator() const { return BehaviorType == EDinosaurBehaviorType::Ambush; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetThreatLevel() const;

    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};