#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "../SharedTypes.h"
#include "Quest_DinosaurHuntManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_HuntTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    FString DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    float DifficultyLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    int32 RequiredKills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    int32 CurrentKills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    bool bIsCompleted;

    FQuest_HuntTarget()
    {
        DinosaurSpecies = TEXT("Unknown");
        TargetLocation = FVector::ZeroVector;
        DifficultyLevel = 1.0f;
        RequiredKills = 1;
        CurrentKills = 0;
        bIsCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_HuntReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Reward")
    FString RewardType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Reward")
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Reward")
    TArray<FString> CraftingMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Reward")
    float SurvivalBonus;

    FQuest_HuntReward()
    {
        RewardType = TEXT("Basic");
        ExperiencePoints = 100;
        SurvivalBonus = 0.1f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_DinosaurHuntManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_DinosaurHuntManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core hunt management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Management")
    TArray<FQuest_HuntTarget> ActiveHuntTargets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Management")
    TArray<FQuest_HuntTarget> CompletedHuntTargets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Management")
    int32 MaxActiveHunts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Management")
    float HuntDetectionRadius;

    // Biome-specific hunting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Hunting")
    TMap<FString, TArray<FString>> BiomeDinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Hunting")
    TMap<FString, FVector> BiomeHuntingZones;

    // Player tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Tracking")
    ACharacter* PlayerCharacter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Tracking")
    FVector LastPlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Tracking")
    float PlayerTrackingUpdateInterval;

    // Hunt progression
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Progression")
    int32 TotalHuntsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Progression")
    float HuntDifficultyMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Progression")
    TArray<FQuest_HuntReward> HuntRewards;

    // Hunt functions
    UFUNCTION(BlueprintCallable, Category = "Hunt Management")
    void StartNewHunt(const FString& DinosaurSpecies, const FVector& TargetLocation, int32 RequiredKills = 1);

    UFUNCTION(BlueprintCallable, Category = "Hunt Management")
    void RegisterDinosaurKill(const FString& DinosaurSpecies, const FVector& KillLocation);

    UFUNCTION(BlueprintCallable, Category = "Hunt Management")
    bool IsHuntCompleted(const FString& DinosaurSpecies);

    UFUNCTION(BlueprintCallable, Category = "Hunt Management")
    void CompleteHunt(const FString& DinosaurSpecies);

    // Biome hunting
    UFUNCTION(BlueprintCallable, Category = "Biome Hunting")
    void InitializeBiomeHunts();

    UFUNCTION(BlueprintCallable, Category = "Biome Hunting")
    TArray<FString> GetAvailableHuntsInBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Biome Hunting")
    void StartBiomeHunt(const FString& BiomeName, const FString& DinosaurSpecies);

    // Player interaction
    UFUNCTION(BlueprintCallable, Category = "Player Interaction")
    void UpdatePlayerLocation();

    UFUNCTION(BlueprintCallable, Category = "Player Interaction")
    FString GetNearestBiome(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Player Interaction")
    TArray<FQuest_HuntTarget> GetActiveHuntsNearPlayer(float SearchRadius = 10000.0f);

    // Reward system
    UFUNCTION(BlueprintCallable, Category = "Reward System")
    void GrantHuntReward(const FQuest_HuntTarget& CompletedHunt);

    UFUNCTION(BlueprintCallable, Category = "Reward System")
    void CalculateHuntDifficulty(FQuest_HuntTarget& HuntTarget);

    // Debug and testing
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugSpawnHuntTargets();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugPrintActiveHunts();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugClearAllHunts();

private:
    float PlayerTrackingTimer;
    
    void UpdateHuntProgress(float DeltaTime);
    void CheckHuntCompletion();
    void SpawnHuntMarkers();
    void UpdateBiomeHuntAvailability();
};