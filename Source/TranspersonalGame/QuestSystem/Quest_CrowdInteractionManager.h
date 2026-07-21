#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Quest_CrowdInteractionManager.generated.h"

// Forward declarations
class ACrowd_MassSimulationManager;
class ATranspersonalCharacter;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CrowdInteraction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    FString InteractionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    ECrowdZoneType ZoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    int32 RequiredAgentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    float InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    float CompletionTime;

    FQuest_CrowdInteraction()
    {
        InteractionID = TEXT("");
        ZoneType = ECrowdZoneType::TribalCamp;
        RequiredAgentCount = 1;
        InteractionRadius = 500.0f;
        bIsCompleted = false;
        CompletionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_TribalMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Mission")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Mission")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Mission")
    ECrowdZoneType TargetZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Mission")
    TArray<FQuest_CrowdInteraction> RequiredInteractions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Mission")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Mission")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Mission")
    float MissionStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Mission")
    float MissionTimeLimit;

    FQuest_TribalMission()
    {
        MissionID = TEXT("");
        MissionName = TEXT("");
        MissionDescription = TEXT("");
        TargetZone = ECrowdZoneType::TribalCamp;
        bIsActive = false;
        bIsCompleted = false;
        MissionStartTime = 0.0f;
        MissionTimeLimit = 300.0f; // 5 minutes default
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_CrowdInteractionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_CrowdInteractionManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool StartTribalMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteTribalMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateMissionProgress(const FString& MissionID, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_TribalMission> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_TribalMission GetMissionByID(const FString& MissionID) const;

    // Crowd Interaction
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CheckCrowdInteraction(ATranspersonalCharacter* Player, ECrowdZoneType ZoneType, float InteractionRadius);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterCrowdManager(ACrowd_MassSimulationManager* CrowdManager);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    ACrowd_MassSimulationManager* GetCrowdManagerForZone(ECrowdZoneType ZoneType) const;

    // Mission Creation
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_TribalMission CreateTribalCampMission();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_TribalMission CreateHuntingPartyMission();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_TribalMission CreateGatheringMission();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_TribalMission CreateWatchtowerMission();

    // Debug and Testing
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest System")
    void InitializeDefaultMissions();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest System")
    void DebugPrintActiveMissions();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_TribalMission> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_TribalMission> CompletedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TMap<ECrowdZoneType, ACrowd_MassSimulationManager*> CrowdManagers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float MissionUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 MaxActiveMissions;

private:
    void CleanupExpiredMissions();
    bool ValidateMissionRequirements(const FQuest_TribalMission& Mission) const;
    void NotifyMissionCompletion(const FQuest_TribalMission& Mission);
};

#include "Quest_CrowdInteractionManager.generated.h"