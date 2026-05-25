#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Quest_ResourceGatheringSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    Stone       UMETA(DisplayName = "Stone"),
    Wood        UMETA(DisplayName = "Wood"),
    Plant       UMETA(DisplayName = "Plant"),
    Bone        UMETA(DisplayName = "Bone"),
    Hide        UMETA(DisplayName = "Hide"),
    Water       UMETA(DisplayName = "Water")
};

USTRUCT(BlueprintType)
struct FQuest_ResourceRequirement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EQuest_ResourceType ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentAmount;

    FQuest_ResourceRequirement()
    {
        ResourceType = EQuest_ResourceType::Stone;
        RequiredAmount = 1;
        CurrentAmount = 0;
    }
};

USTRUCT(BlueprintType)
struct FQuest_GatheringMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FQuest_ResourceRequirement> Requirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCompleted;

    FQuest_GatheringMission()
    {
        MissionName = TEXT("Gather Resources");
        Description = TEXT("Collect the required resources to survive");
        TimeLimit = 300.0f;
        bIsActive = false;
        bIsCompleted = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_ResourceGatheringSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_ResourceGatheringSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_GatheringMission> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_GatheringMission> CompletedMissions;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartGatheringMission(const FString& MissionName, const TArray<FQuest_ResourceRequirement>& Requirements, float TimeLimit);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void AddResource(EQuest_ResourceType ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CheckMissionCompletion(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteMission(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_GatheringMission GetActiveMission(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_GatheringMission> GetAllActiveMissions();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateBasicSurvivalMissions();

private:
    void UpdateMissionTimers(float DeltaTime);
    void CheckAllMissionsCompletion();
};