#include "QuestMissionData.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// Quest & Mission Designer — Agent #14
// QuestMissionData.cpp — Mission zone implementation
// ============================================================

AQuest_MissionZone::AQuest_MissionZone()
{
    PrimaryActorTick.bCanEverTick = true;
    bPlayerInZone = false;
    ElapsedTime = 0.0f;
}

void AQuest_MissionZone::BeginPlay()
{
    Super::BeginPlay();

    // Populate default missions based on linked waypoint type
    // These match the 8 crowd waypoints from Agent #13

    if (MissionDefinition.MissionID.IsEmpty())
    {
        MissionDefinition.MissionID = FString::Printf(TEXT("MISSION_%s"), *GetActorLabel());
    }

    UE_LOG(LogTemp, Log, TEXT("QuestMissionZone BeginPlay: %s | Type: %d | Waypoint: %s"),
        *MissionDefinition.MissionTitle,
        (int32)MissionDefinition.MissionType,
        *MissionDefinition.LinkedWaypointLabel);
}

void AQuest_MissionZone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Time limit enforcement
    if (MissionDefinition.State == EQuest_MissionState::Active &&
        MissionDefinition.TimeLimit > 0.0f)
    {
        ElapsedTime += DeltaTime;
        if (ElapsedTime >= MissionDefinition.TimeLimit)
        {
            FailMission();
        }
    }
}

void AQuest_MissionZone::OnPlayerEnterZone(AActor* PlayerActor)
{
    if (!PlayerActor) return;

    bPlayerInZone = true;

    if (MissionDefinition.State == EQuest_MissionState::Available)
    {
        MissionDefinition.State = EQuest_MissionState::Active;
        ElapsedTime = 0.0f;

        UE_LOG(LogTemp, Log, TEXT("QUEST_ACTIVATED: %s at waypoint %s"),
            *MissionDefinition.MissionTitle,
            *MissionDefinition.LinkedWaypointLabel);
    }
}

void AQuest_MissionZone::AdvanceObjective(const FString& ObjectiveID, int32 Count)
{
    if (MissionDefinition.State != EQuest_MissionState::Active) return;

    for (FQuest_MissionObjective& Obj : MissionDefinition.Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID)
        {
            Obj.CurrentCount = FMath::Min(Obj.CurrentCount + Count, Obj.RequiredCount);
            Obj.bIsCompleted = Obj.IsComplete();

            UE_LOG(LogTemp, Log, TEXT("OBJECTIVE_PROGRESS: %s [%d/%d]"),
                *Obj.Description, Obj.CurrentCount, Obj.RequiredCount);

            if (IsMissionComplete())
            {
                MissionDefinition.State = EQuest_MissionState::Completed;
                UE_LOG(LogTemp, Log, TEXT("MISSION_COMPLETE: %s"), *MissionDefinition.MissionTitle);
            }
            break;
        }
    }
}

bool AQuest_MissionZone::IsMissionComplete() const
{
    return MissionDefinition.AreAllObjectivesComplete();
}

void AQuest_MissionZone::FailMission()
{
    if (MissionDefinition.State == EQuest_MissionState::Active)
    {
        MissionDefinition.State = EQuest_MissionState::Failed;
        UE_LOG(LogTemp, Log, TEXT("MISSION_FAILED: %s (time=%.1fs)"),
            *MissionDefinition.MissionTitle, ElapsedTime);
    }
}
