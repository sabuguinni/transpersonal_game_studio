#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Quest_ExplorationManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_BiomeType : uint8
{
    None = 0,
    Savanna,
    Swamp,
    Forest,
    Desert,
    Mountain
};

UENUM(BlueprintType)
enum class EQuest_DiscoveryType : uint8
{
    None = 0,
    Cave,
    WaterSource,
    ResourceDeposit,
    AncientRuins,
    DinosaurNest,
    VantagePoint,
    SafeZone,
    DangerZone
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DiscoveryData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Discovery")
    EQuest_DiscoveryType Type = EQuest_DiscoveryType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Discovery")
    FString Name;

    UPROPERTY(BlueprintReadOnly, Category = "Discovery")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Discovery")
    EQuest_BiomeType Biome = EQuest_BiomeType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Discovery")
    bool bIsDiscovered = false;

    UPROPERTY(BlueprintReadOnly, Category = "Discovery")
    float DiscoveryRadius = 500.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Discovery")
    int32 ExperienceReward = 50;

    UPROPERTY(BlueprintReadOnly, Category = "Discovery")
    FString Description;

    FQuest_DiscoveryData()
    {
        Type = EQuest_DiscoveryType::None;
        Name = TEXT("");
        Location = FVector::ZeroVector;
        Biome = EQuest_BiomeType::None;
        bIsDiscovered = false;
        DiscoveryRadius = 500.0f;
        ExperienceReward = 50;
        Description = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ExplorationObjective
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Exploration")
    FString ObjectiveName;

    UPROPERTY(BlueprintReadOnly, Category = "Exploration")
    EQuest_BiomeType TargetBiome = EQuest_BiomeType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Exploration")
    EQuest_DiscoveryType TargetDiscovery = EQuest_DiscoveryType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Exploration")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Exploration")
    float TargetDistance = 1000.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Exploration")
    bool bIsCompleted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Exploration")
    int32 ExperienceReward = 100;

    FQuest_ExplorationObjective()
    {
        ObjectiveName = TEXT("");
        TargetBiome = EQuest_BiomeType::None;
        TargetDiscovery = EQuest_DiscoveryType::None;
        TargetLocation = FVector::ZeroVector;
        TargetDistance = 1000.0f;
        bIsCompleted = false;
        ExperienceReward = 100;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UQuest_ExplorationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_ExplorationManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Discovery system
    UFUNCTION(BlueprintCallable, Category = "Quest|Exploration")
    void CheckForDiscoveries(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest|Exploration")
    bool DiscoverLocation(const FQuest_DiscoveryData& Discovery);

    UFUNCTION(BlueprintPure, Category = "Quest|Exploration")
    TArray<FQuest_DiscoveryData> GetDiscoveredLocations() const;

    UFUNCTION(BlueprintPure, Category = "Quest|Exploration")
    TArray<FQuest_DiscoveryData> GetUndiscoveredLocations() const;

    // Biome system
    UFUNCTION(BlueprintPure, Category = "Quest|Exploration")
    EQuest_BiomeType GetCurrentBiome(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Exploration")
    void OnPlayerEnteredBiome(EQuest_BiomeType NewBiome);

    UFUNCTION(BlueprintPure, Category = "Quest|Exploration")
    bool HasVisitedBiome(EQuest_BiomeType Biome) const;

    // Exploration objectives
    UFUNCTION(BlueprintCallable, Category = "Quest|Exploration")
    void GenerateExplorationObjective();

    UFUNCTION(BlueprintCallable, Category = "Quest|Exploration")
    bool CompleteExplorationObjective(const FString& ObjectiveName);

    UFUNCTION(BlueprintPure, Category = "Quest|Exploration")
    TArray<FQuest_ExplorationObjective> GetActiveObjectives() const;

    // Map and navigation
    UFUNCTION(BlueprintCallable, Category = "Quest|Exploration")
    void RevealMapArea(const FVector& Center, float Radius);

    UFUNCTION(BlueprintPure, Category = "Quest|Exploration")
    float GetExplorationProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Exploration")
    void SpawnExplorationMarkers();

    // Quest integration
    UFUNCTION(BlueprintPure, Category = "Quest|Exploration")
    bool HasCompletedExplorationRequirement(const FString& RequirementType) const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Exploration")
    void TrackPlayerMovement(const FVector& NewLocation);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Quest|Exploration")
    TArray<FQuest_DiscoveryData> AllDiscoveries;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Exploration")
    TArray<FQuest_ExplorationObjective> ExplorationObjectives;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Exploration")
    TSet<EQuest_BiomeType> VisitedBiomes;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Exploration")
    EQuest_BiomeType CurrentBiome = EQuest_BiomeType::Savanna;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Exploration")
    FVector LastPlayerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Exploration")
    float TotalDistanceTraveled = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Exploration")
    TSet<FVector> RevealedMapAreas;

    // Initialize discovery locations
    void InitializeDiscoveryLocations();
    void InitializeBiomeLocations();

    // Helper functions
    FQuest_DiscoveryData CreateDiscovery(EQuest_DiscoveryType Type, const FString& Name, const FVector& Location, EQuest_BiomeType Biome) const;
    FQuest_ExplorationObjective CreateExplorationObjective(const FString& Name, EQuest_BiomeType TargetBiome, const FVector& Target) const;
    float CalculateDistanceToDiscovery(const FVector& PlayerLocation, const FQuest_DiscoveryData& Discovery) const;
};