#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/TriggerVolume.h"
#include "GameFramework/Pawn.h"
#include "../SharedTypes.h"
#include "Quest_MissionManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_MissionType : uint8
{
    Gathering       UMETA(DisplayName = "Gathering"),
    Escort          UMETA(DisplayName = "Escort"),
    Evacuation      UMETA(DisplayName = "Evacuation"),
    Hunt            UMETA(DisplayName = "Hunt"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Defense         UMETA(DisplayName = "Defense")
};

UENUM(BlueprintType)
enum class EQuest_MissionStatus : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Available       UMETA(DisplayName = "Available"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 TargetCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    FQuest_MissionObjective()
    {
        ObjectiveText = TEXT("");
        TargetCount = 1;
        CurrentCount = 0;
        bIsCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_MissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_MissionStatus MissionStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_MissionObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    ATriggerVolume* TriggerZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    APawn* QuestGiver;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperienceReward;

    FQuest_MissionData()
    {
        MissionID = TEXT("");
        MissionName = TEXT("");
        MissionDescription = TEXT("");
        MissionType = EQuest_MissionType::Gathering;
        MissionStatus = EQuest_MissionStatus::Inactive;
        TriggerZone = nullptr;
        QuestGiver = nullptr;
        TimeLimit = 0.0f;
        ElapsedTime = 0.0f;
        ExperienceReward = 100;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_MissionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_MissionManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Mission management
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CreateMission(const FString& MissionID, const FString& Name, const FString& Description, EQuest_MissionType Type);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_MissionData GetMissionData(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_MissionData> GetActiveMissions();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_MissionData> GetAvailableMissions();

    // Objective management
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AddObjective(const FString& MissionID, const FString& ObjectiveText, int32 TargetCount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool UpdateObjectiveProgress(const FString& MissionID, int32 ObjectiveIndex, int32 Progress = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteObjective(const FString& MissionID, int32 ObjectiveIndex);

    // Crowd integration
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnCrowdEventTriggered(const FString& EventType, const FVector& Location, int32 CrowdSize);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterCrowdMission(const FString& MissionID, const FString& CrowdEventType);

    // Timer and updates
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateMissionTimers(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CheckMissionCompletion();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TMap<FString, FQuest_MissionData> ActiveMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TMap<FString, FQuest_MissionData> AvailableMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TMap<FString, FString> CrowdMissionRegistry;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 MaxActiveMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    bool bIsInitialized;

private:
    void InitializeDefaultMissions();
    void SetupCrowdIntegration();
    bool ValidateMissionData(const FQuest_MissionData& MissionData);
    void BroadcastMissionEvent(const FString& MissionID, const FString& EventType);
};