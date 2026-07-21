#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Components/ActorComponent.h"
#include "Quest_ResourceGatheringSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    Stone,
    Wood,
    Plant,
    Bone,
    Hide,
    Water,
    Fire
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ResourceRequirement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EQuest_ResourceType ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 RequiredAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FString ResourceName;

    FQuest_ResourceRequirement()
    {
        ResourceType = EQuest_ResourceType::Stone;
        RequiredAmount = 1;
        ResourceName = TEXT("Unknown Resource");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_GatheringMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FQuest_ResourceRequirement> RequiredResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsActive;

    FQuest_GatheringMission()
    {
        MissionName = TEXT("Gather Resources");
        Description = TEXT("Collect the required materials to survive");
        ExperienceReward = 100;
        TimeLimit = 300.0f;
        bIsActive = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_ResourceGatheringSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_ResourceGatheringSystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartGatheringMission(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void AddResource(EQuest_ResourceType ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CheckMissionCompletion(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_GatheringMission> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreatePresetMissions();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TArray<FQuest_GatheringMission> AvailableMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TArray<FQuest_GatheringMission> ActiveMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TMap<EQuest_ResourceType, int32> PlayerInventory;

private:
    void InitializePresetMissions();
    void CompleteMission(const FString& MissionName);
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_ResourcePickupComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_ResourcePickupComponent();

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void PickupResource(AActor* Player);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EQuest_ResourceType ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 ResourceAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bCanBePickedUp;

protected:
    virtual void BeginPlay() override;
};