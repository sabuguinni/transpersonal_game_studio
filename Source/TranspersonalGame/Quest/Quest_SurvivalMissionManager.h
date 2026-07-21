#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Quest_SurvivalMissionManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_MissionType : uint8
{
    Hunt_Dinosaur      UMETA(DisplayName = "Hunt Dinosaur"),
    Gather_Resources   UMETA(DisplayName = "Gather Resources"),
    Explore_Territory  UMETA(DisplayName = "Explore Territory"),
    Build_Shelter      UMETA(DisplayName = "Build Shelter"),
    Survive_Night      UMETA(DisplayName = "Survive Night"),
    Escape_Predator    UMETA(DisplayName = "Escape Predator")
};

UENUM(BlueprintType)
enum class EQuest_MissionStatus : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 TargetQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeRemaining;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> RewardItems;

    FQuest_SurvivalMission()
    {
        MissionID = TEXT("");
        MissionName = TEXT("");
        Description = TEXT("");
        MissionType = EQuest_MissionType::Hunt_Dinosaur;
        Status = EQuest_MissionStatus::Inactive;
        TargetQuantity = 1;
        CurrentProgress = 0;
        TargetLocation = FVector::ZeroVector;
        TimeLimit = 300.0f;
        TimeRemaining = 300.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_SurvivalMissionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_SurvivalMissionManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void FailMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateMissionProgress(const FString& MissionID, int32 ProgressIncrement);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_SurvivalMission GetMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_SurvivalMission> GetActiveMissions();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateHuntMission(const FString& DinosaurType, const FVector& HuntLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateGatherMission(const FString& ResourceType, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateExploreMission(const FVector& ExploreLocation, float ExploreRadius);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void ProcessMissionTimers(float DeltaTime);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TArray<FQuest_SurvivalMission> ActiveMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TArray<FQuest_SurvivalMission> CompletedMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    int32 NextMissionID;

private:
    void InitializeDefaultMissions();
    FString GenerateMissionID();
    void BroadcastMissionUpdate(const FQuest_SurvivalMission& Mission);
};