#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Quest_ResourceGatheringManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    Stone,
    Wood,
    Plant,
    Water,
    Meat,
    Hide,
    Bone
};

USTRUCT(BlueprintType)
struct FQuest_ResourceTarget
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Resource")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::Stone;

    UPROPERTY(BlueprintReadWrite, Category = "Resource")
    int32 TargetAmount = 1;

    UPROPERTY(BlueprintReadWrite, Category = "Resource")
    int32 CurrentAmount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Resource")
    FVector SearchLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Resource")
    float SearchRadius = 5000.0f;

    FQuest_ResourceTarget()
    {
        ResourceType = EQuest_ResourceType::Stone;
        TargetAmount = 1;
        CurrentAmount = 0;
        SearchLocation = FVector::ZeroVector;
        SearchRadius = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct FQuest_GatheringMission
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Mission")
    FString MissionID;

    UPROPERTY(BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(BlueprintReadWrite, Category = "Mission")
    FString Description;

    UPROPERTY(BlueprintReadWrite, Category = "Mission")
    TArray<FQuest_ResourceTarget> ResourceTargets;

    UPROPERTY(BlueprintReadWrite, Category = "Mission")
    EMissionPriority Priority = EMissionPriority::Medium;

    UPROPERTY(BlueprintReadWrite, Category = "Mission")
    EMissionStatus Status = EMissionStatus::Available;

    UPROPERTY(BlueprintReadWrite, Category = "Mission")
    float TimeLimit = 600.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Mission")
    float ElapsedTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Mission")
    FVector MissionLocation = FVector::ZeroVector;

    FQuest_GatheringMission()
    {
        MissionID = TEXT("");
        MissionName = TEXT("");
        Description = TEXT("");
        Priority = EMissionPriority::Medium;
        Status = EMissionStatus::Available;
        TimeLimit = 600.0f;
        ElapsedTime = 0.0f;
        MissionLocation = FVector::ZeroVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_ResourceGatheringManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_ResourceGatheringManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Quest Resource")
    void StartGatheringMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest Resource")
    void CompleteGatheringMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest Resource")
    void UpdateResourceProgress(const FString& MissionID, EQuest_ResourceType ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest Resource")
    FQuest_GatheringMission GetGatheringMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest Resource")
    TArray<FQuest_GatheringMission> GetActiveGatheringMissions();

    UFUNCTION(BlueprintCallable, Category = "Quest Resource")
    void CreateStoneGatheringMission(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Quest Resource")
    void CreateWoodGatheringMission(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Quest Resource")
    void CreatePlantGatheringMission(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Quest Resource")
    void CreateWaterGatheringMission(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Quest Resource")
    void CreateHuntingMission(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Quest Resource")
    bool IsResourceInRange(EQuest_ResourceType ResourceType, FVector PlayerLocation, float Range);

    UFUNCTION(BlueprintCallable, Category = "Quest Resource")
    FVector FindNearestResourceLocation(EQuest_ResourceType ResourceType, FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest Resource")
    void SpawnResourcePickups(EQuest_ResourceType ResourceType, FVector Location, int32 Count);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Quest Resource")
    TArray<FQuest_GatheringMission> ActiveMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Resource")
    TArray<FQuest_GatheringMission> CompletedMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Resource")
    int32 NextMissionID = 1;

    void GenerateRandomGatheringMission();
    void UpdateMissionTimers(float DeltaTime);
    FString GenerateMissionID();
    FString GetResourceTypeName(EQuest_ResourceType ResourceType);
};