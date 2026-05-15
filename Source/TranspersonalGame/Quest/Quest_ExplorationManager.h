#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Quest_ExplorationManager.generated.h"

// Forward declarations
class ATranspersonalCharacter;

USTRUCT(BlueprintType)
struct FQuest_ExplorationZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    FVector ZoneLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    bool bIsDiscovered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    float DiscoveryTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    int32 ExplorationPoints;

    FQuest_ExplorationZone()
    {
        ZoneName = TEXT("Unknown Zone");
        ZoneLocation = FVector::ZeroVector;
        ZoneRadius = 1000.0f;
        BiomeType = EBiomeType::Grassland;
        bIsDiscovered = false;
        DiscoveryTime = 0.0f;
        ExplorationPoints = 10;
    }
};

USTRUCT(BlueprintType)
struct FQuest_ExplorationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    TArray<FQuest_ExplorationZone> TargetZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    int32 RequiredZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    int32 DiscoveredZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    float ElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    int32 TotalExplorationPoints;

    FQuest_ExplorationData()
    {
        QuestName = TEXT("Exploration Quest");
        RequiredZones = 1;
        DiscoveredZones = 0;
        TimeLimit = 1200.0f; // 20 minutes
        ElapsedTime = 0.0f;
        bIsActive = false;
        TotalExplorationPoints = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_ExplorationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_ExplorationManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest management
    UFUNCTION(BlueprintCallable, Category = "Quest Exploration")
    void StartExplorationQuest(const TArray<FQuest_ExplorationZone>& Zones, int32 RequiredCount, float TimeLimit, const FString& QuestName);

    UFUNCTION(BlueprintCallable, Category = "Quest Exploration")
    void CompleteExplorationQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest Exploration")
    void CancelExplorationQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest Exploration")
    void RegisterZoneDiscovery(const FString& ZoneName, ATranspersonalCharacter* Explorer);

    UFUNCTION(BlueprintCallable, Category = "Quest Exploration")
    bool IsExplorationQuestActive() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Exploration")
    FQuest_ExplorationData GetCurrentExplorationQuest() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Exploration")
    float GetExplorationProgress() const;

    // Zone management
    UFUNCTION(BlueprintCallable, Category = "Quest Exploration")
    void AddExplorationZone(const FQuest_ExplorationZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Quest Exploration")
    TArray<FQuest_ExplorationZone> GetNearbyZones(const FVector& Location, float SearchRadius) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Exploration")
    bool IsPlayerInZone(const FVector& PlayerLocation, const FQuest_ExplorationZone& Zone) const;

    // Preset exploration quests
    UFUNCTION(BlueprintCallable, Category = "Quest Exploration")
    void CreateSwampExplorationQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest Exploration")
    void CreateForestExplorationQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest Exploration")
    void CreateDesertExplorationQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest Exploration")
    void CreateMountainExplorationQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest Exploration")
    void CreateFullWorldExplorationQuest();

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExplorationQuestStarted, FString, QuestName, int32, RequiredZones);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExplorationQuestCompleted, FString, QuestName, int32, TotalPoints);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExplorationQuestFailed, FString, Reason);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnZoneDiscovered, FString, ZoneName, EBiomeType, BiomeType, int32, Points);

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnExplorationQuestStarted OnExplorationQuestStarted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnExplorationQuestCompleted OnExplorationQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnExplorationQuestFailed OnExplorationQuestFailed;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnZoneDiscovered OnZoneDiscovered;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest State")
    FQuest_ExplorationData CurrentExplorationQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config")
    TArray<FQuest_ExplorationZone> AllExplorationZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config")
    float BaseTimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config")
    float ZoneDiscoveryRadius;

private:
    void UpdateQuestTimer();
    void CheckQuestCompletion();
    void InitializeExplorationZones();

    FTimerHandle QuestTimerHandle;
};