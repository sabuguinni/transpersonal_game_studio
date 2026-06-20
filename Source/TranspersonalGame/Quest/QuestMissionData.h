#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestMissionData.generated.h"

// ============================================================
// Quest & Mission Designer — Agent #14
// QuestMissionData.h — Mission definitions linked to crowd waypoints
// ============================================================

UENUM(BlueprintType)
enum class EQuest_MissionType : uint8
{
    Hunt        UMETA(DisplayName = "Hunt"),
    Gather      UMETA(DisplayName = "Gather"),
    Defend      UMETA(DisplayName = "Defend"),
    Explore     UMETA(DisplayName = "Explore"),
    Survive     UMETA(DisplayName = "Survive"),
    Rescue      UMETA(DisplayName = "Rescue"),
};

UENUM(BlueprintType)
enum class EQuest_MissionState : uint8
{
    Locked      UMETA(DisplayName = "Locked"),
    Available   UMETA(DisplayName = "Available"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
};

USTRUCT(BlueprintType)
struct FQuest_MissionObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted = false;

    bool IsComplete() const { return CurrentCount >= RequiredCount; }
};

USTRUCT(BlueprintType)
struct FQuest_MissionDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString MissionTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_MissionType MissionType = EQuest_MissionType::Hunt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_MissionState State = EQuest_MissionState::Available;

    // Linked crowd waypoint label (from Agent #13 CrowdWaypointSystem)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString LinkedWaypointLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_MissionObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit = 0.0f; // 0 = no time limit

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString VoiceBriefAudioURL; // ElevenLabs TTS URL

    bool AreAllObjectivesComplete() const
    {
        for (const FQuest_MissionObjective& Obj : Objectives)
        {
            if (!Obj.IsComplete()) return false;
        }
        return Objectives.Num() > 0;
    }
};

/**
 * AQuest_MissionZone
 * Placed in the world at crowd waypoint locations.
 * Activates/completes missions when the player enters the zone.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_MissionZone : public AActor
{
    GENERATED_BODY()

public:
    AQuest_MissionZone();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_MissionDefinition MissionDefinition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TriggerRadius = 300.0f;

    // Called when player enters the zone
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerEnterZone(AActor* PlayerActor);

    // Advance a specific objective by count
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void AdvanceObjective(const FString& ObjectiveID, int32 Count = 1);

    // Check if mission is complete
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsMissionComplete() const;

    // Fail the mission
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailMission();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    bool bPlayerInZone = false;
    float ElapsedTime = 0.0f;
};
