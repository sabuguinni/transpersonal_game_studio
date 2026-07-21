// QuestObjectiveSystem.cpp — Agent #14 Quest & Mission Designer
// Cycle: AUTO_20260702_005
// Full implementation of AQuest_ObjectiveManager

#include "QuestObjectiveSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─── Constructor ──────────────────────────────────────────────────────────────

AQuest_ObjectiveManager::AQuest_ObjectiveManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Check timed objectives every second
    CompletedMissionCount = 0;
}

// ─── BeginPlay ────────────────────────────────────────────────────────────────

void AQuest_ObjectiveManager::BeginPlay()
{
    Super::BeginPlay();

    // Register the 3 core survival missions at game start
    // ── Mission 1: First Hunt ─────────────────────────────────────────────────
    {
        FQuest_MissionData Hunt;
        Hunt.MissionID = FName("MISSION_FIRST_HUNT");
        Hunt.MissionTitle = FText::FromString("First Blood");
        Hunt.MissionDescription = FText::FromString(
            "The tribe is starving. Hunt a small dinosaur to bring back meat and bones.");
        Hunt.QuestGiverTag = FName("NPC_Tracker");

        FQuest_ObjectiveData KillObj;
        KillObj.ObjectiveID = FName("OBJ_KILL_RAPTOR");
        KillObj.ObjectiveDescription = FText::FromString("Kill 1 Raptor");
        KillObj.ObjectiveType = EQuest_ObjectiveType::Hunt;
        KillObj.Priority = EQuest_Priority::Primary;
        KillObj.RequiredCount = 1;
        KillObj.TargetActorTag = FName("Dinosaur_Raptor");
        Hunt.Objectives.Add(KillObj);

        FQuest_ObjectiveData GatherObj;
        GatherObj.ObjectiveID = FName("OBJ_GATHER_BONES");
        GatherObj.ObjectiveDescription = FText::FromString("Collect 2 Raptor Bones");
        GatherObj.ObjectiveType = EQuest_ObjectiveType::Gather;
        GatherObj.Priority = EQuest_Priority::Primary;
        GatherObj.RequiredCount = 2;
        GatherObj.TargetActorTag = FName("Resource_Bone");
        Hunt.Objectives.Add(GatherObj);

        Hunt.Reward.BoneMaterialCount = 3;
        Hunt.Reward.HideCount = 1;
        Hunt.Reward.RewardNarration = FText::FromString(
            "You return with meat and bone. The tribe eats tonight.");
        RegisterMission(Hunt);
    }

    // ── Mission 2: Rescue Kara ────────────────────────────────────────────────
    {
        FQuest_MissionData Rescue;
        Rescue.MissionID = FName("MISSION_RESCUE_KARA");
        Rescue.MissionTitle = FText::FromString("Into the Canyon");
        Rescue.MissionDescription = FText::FromString(
            "Kara was taken by raptors into the canyon. Reach her before nightfall.");
        Rescue.QuestGiverTag = FName("NPC_Elder");
        Rescue.PrerequisiteMissionIDs.Add(FName("MISSION_FIRST_HUNT"));

        FQuest_ObjectiveData ExploreObj;
        ExploreObj.ObjectiveID = FName("OBJ_REACH_CANYON");
        ExploreObj.ObjectiveDescription = FText::FromString("Reach the Raptor Canyon");
        ExploreObj.ObjectiveType = EQuest_ObjectiveType::Explore;
        ExploreObj.Priority = EQuest_Priority::Primary;
        ExploreObj.RequiredCount = 1;
        ExploreObj.TargetLocation = FVector(3500.f, -2000.f, 200.f);
        ExploreObj.TargetRadius = 800.f;
        Rescue.Objectives.Add(ExploreObj);

        FQuest_ObjectiveData DefendObj;
        DefendObj.ObjectiveID = FName("OBJ_DEFEND_KARA");
        DefendObj.ObjectiveDescription = FText::FromString("Protect Kara from 3 Raptors");
        DefendObj.ObjectiveType = EQuest_ObjectiveType::Defend;
        DefendObj.Priority = EQuest_Priority::Critical;
        DefendObj.RequiredCount = 3;
        DefendObj.TargetActorTag = FName("Dinosaur_Raptor");
        DefendObj.TimeLimitSeconds = 120.f; // 2 minutes to defend
        Rescue.Objectives.Add(DefendObj);

        FQuest_ObjectiveData EscapeObj;
        EscapeObj.ObjectiveID = FName("OBJ_ESCAPE_CANYON");
        EscapeObj.ObjectiveDescription = FText::FromString("Escape the canyon with Kara");
        EscapeObj.ObjectiveType = EQuest_ObjectiveType::Escape;
        EscapeObj.Priority = EQuest_Priority::Critical;
        EscapeObj.RequiredCount = 1;
        EscapeObj.TargetLocation = FVector(0.f, 0.f, 0.f); // Camp location
        EscapeObj.TargetRadius = 1000.f;
        Rescue.Objectives.Add(EscapeObj);

        Rescue.Reward.HideCount = 2;
        Rescue.Reward.FlintCount = 3;
        Rescue.Reward.bUnlocksNewArea = true;
        Rescue.Reward.UnlockedAreaTag = FName("Area_RiverDelta");
        Rescue.Reward.RewardNarration = FText::FromString(
            "Kara is safe. She knows a path to the river delta — new hunting grounds.");
        RegisterMission(Rescue);
    }

    // ── Mission 3: Follow the Herd ────────────────────────────────────────────
    {
        FQuest_MissionData Herd;
        Herd.MissionID = FName("MISSION_FOLLOW_HERD");
        Herd.MissionTitle = FText::FromString("The Migration");
        Herd.MissionDescription = FText::FromString(
            "A hadrosaur herd is moving north. Follow them to find new territory.");
        Herd.QuestGiverTag = FName("NPC_Tracker");

        FQuest_ObjectiveData TrackObj;
        TrackObj.ObjectiveID = FName("OBJ_TRACK_HERD");
        TrackObj.ObjectiveDescription = FText::FromString("Track the hadrosaur herd (follow for 300m)");
        TrackObj.ObjectiveType = EQuest_ObjectiveType::Track;
        TrackObj.Priority = EQuest_Priority::Primary;
        TrackObj.RequiredCount = 1;
        TrackObj.TargetActorTag = FName("Dinosaur_Hadrosaur");
        Herd.Objectives.Add(TrackObj);

        FQuest_ObjectiveData SurviveObj;
        SurviveObj.ObjectiveID = FName("OBJ_SURVIVE_NIGHT");
        SurviveObj.ObjectiveDescription = FText::FromString("Survive one full night in the open");
        SurviveObj.ObjectiveType = EQuest_ObjectiveType::Survive;
        SurviveObj.Priority = EQuest_Priority::Secondary;
        SurviveObj.RequiredCount = 1;
        SurviveObj.TimeLimitSeconds = 300.f; // 5 min real = 1 night in-game
        Herd.Objectives.Add(SurviveObj);

        FQuest_ObjectiveData CraftObj;
        CraftObj.ObjectiveID = FName("OBJ_CRAFT_CAMPFIRE");
        CraftObj.ObjectiveDescription = FText::FromString("Craft a campfire before nightfall");
        CraftObj.ObjectiveType = EQuest_ObjectiveType::Craft;
        CraftObj.Priority = EQuest_Priority::Bonus;
        CraftObj.RequiredCount = 1;
        CraftObj.TargetActorTag = FName("Item_Campfire");
        Herd.Objectives.Add(CraftObj);

        Herd.Reward.BoneMaterialCount = 5;
        Herd.Reward.bUnlocksNewArea = true;
        Herd.Reward.UnlockedAreaTag = FName("Area_NorthernPlains");
        Herd.Reward.RewardNarration = FText::FromString(
            "The northern plains. Rich with prey. The tribe can survive here.");
        RegisterMission(Herd);
    }

    UE_LOG(LogTemp, Log, TEXT("QuestObjectiveManager: %d missions registered"), AllMissions.Num());
}

// ─── Tick ─────────────────────────────────────────────────────────────────────

void AQuest_ObjectiveManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    for (FQuest_MissionData& Mission : AllMissions)
    {
        if (Mission.MissionState == EQuest_ObjectiveState::Active)
        {
            TickTimedObjectives(Mission, DeltaTime);
        }
    }
}

// ─── Mission Registration ─────────────────────────────────────────────────────

void AQuest_ObjectiveManager::RegisterMission(const FQuest_MissionData& MissionData)
{
    // Prevent duplicate registration
    for (const FQuest_MissionData& Existing : AllMissions)
    {
        if (Existing.MissionID == MissionData.MissionID)
        {
            UE_LOG(LogTemp, Warning, TEXT("Mission %s already registered — skipping"),
                   *MissionData.MissionID.ToString());
            return;
        }
    }
    AllMissions.Add(MissionData);
    UE_LOG(LogTemp, Log, TEXT("Mission registered: %s"), *MissionData.MissionID.ToString());
}

bool AQuest_ObjectiveManager::ActivateMission(FName MissionID)
{
    FQuest_MissionData* Mission = FindMission(MissionID);
    if (!Mission) return false;

    // Check prerequisites
    for (const FName& PrereqID : Mission->PrerequisiteMissionIDs)
    {
        if (!IsMissionComplete(PrereqID))
        {
            UE_LOG(LogTemp, Warning, TEXT("Mission %s blocked — prerequisite %s not complete"),
                   *MissionID.ToString(), *PrereqID.ToString());
            return false;
        }
    }

    Mission->MissionState = EQuest_ObjectiveState::Active;
    for (FQuest_ObjectiveData& Obj : Mission->Objectives)
    {
        Obj.State = EQuest_ObjectiveState::Active;
    }

    OnMissionActivated.Broadcast(MissionID, Mission->MissionTitle);
    UE_LOG(LogTemp, Log, TEXT("Mission ACTIVATED: %s"), *MissionID.ToString());
    return true;
}

bool AQuest_ObjectiveManager::CompleteMission(FName MissionID)
{
    FQuest_MissionData* Mission = FindMission(MissionID);
    if (!Mission) return false;

    Mission->MissionState = EQuest_ObjectiveState::Completed;
    CompletedMissionCount++;
    OnMissionCompleted.Broadcast(MissionID, Mission->Reward);
    UE_LOG(LogTemp, Log, TEXT("Mission COMPLETED: %s"), *MissionID.ToString());
    return true;
}

bool AQuest_ObjectiveManager::FailMission(FName MissionID)
{
    FQuest_MissionData* Mission = FindMission(MissionID);
    if (!Mission) return false;

    Mission->MissionState = EQuest_ObjectiveState::Failed;
    OnMissionFailed.Broadcast(MissionID);
    UE_LOG(LogTemp, Log, TEXT("Mission FAILED: %s"), *MissionID.ToString());
    return true;
}

// ─── Progress Reporting ───────────────────────────────────────────────────────

void AQuest_ObjectiveManager::ReportProgress(FName MissionID, FName ObjectiveID, int32 ProgressAmount)
{
    FQuest_MissionData* Mission = FindMission(MissionID);
    if (!Mission || Mission->MissionState != EQuest_ObjectiveState::Active) return;

    for (FQuest_ObjectiveData& Obj : Mission->Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID && Obj.State == EQuest_ObjectiveState::Active)
        {
            Obj.CurrentCount = FMath::Min(Obj.CurrentCount + ProgressAmount, Obj.RequiredCount);
            if (Obj.CurrentCount >= Obj.RequiredCount)
            {
                Obj.State = EQuest_ObjectiveState::Completed;
            }
            OnObjectiveUpdated.Broadcast(MissionID, Obj);
            EvaluateObjectiveCompletion(*Mission);
            return;
        }
    }
}

void AQuest_ObjectiveManager::ReportKill(FName ActorTag)
{
    for (FQuest_MissionData& Mission : AllMissions)
    {
        if (Mission.MissionState != EQuest_ObjectiveState::Active) continue;
        for (FQuest_ObjectiveData& Obj : Mission.Objectives)
        {
            if (Obj.State != EQuest_ObjectiveState::Active) continue;
            if ((Obj.ObjectiveType == EQuest_ObjectiveType::Hunt ||
                 Obj.ObjectiveType == EQuest_ObjectiveType::Defend) &&
                Obj.TargetActorTag == ActorTag)
            {
                Obj.CurrentCount = FMath::Min(Obj.CurrentCount + 1, Obj.RequiredCount);
                if (Obj.CurrentCount >= Obj.RequiredCount)
                    Obj.State = EQuest_ObjectiveState::Completed;
                OnObjectiveUpdated.Broadcast(Mission.MissionID, Obj);
                EvaluateObjectiveCompletion(Mission);
            }
        }
    }
}

void AQuest_ObjectiveManager::ReportGather(FName ResourceTag, int32 Amount)
{
    for (FQuest_MissionData& Mission : AllMissions)
    {
        if (Mission.MissionState != EQuest_ObjectiveState::Active) continue;
        for (FQuest_ObjectiveData& Obj : Mission.Objectives)
        {
            if (Obj.State != EQuest_ObjectiveState::Active) continue;
            if (Obj.ObjectiveType == EQuest_ObjectiveType::Gather &&
                Obj.TargetActorTag == ResourceTag)
            {
                Obj.CurrentCount = FMath::Min(Obj.CurrentCount + Amount, Obj.RequiredCount);
                if (Obj.CurrentCount >= Obj.RequiredCount)
                    Obj.State = EQuest_ObjectiveState::Completed;
                OnObjectiveUpdated.Broadcast(Mission.MissionID, Obj);
                EvaluateObjectiveCompletion(Mission);
            }
        }
    }
}

void AQuest_ObjectiveManager::ReportLocationReached(FVector PlayerLocation)
{
    for (FQuest_MissionData& Mission : AllMissions)
    {
        if (Mission.MissionState != EQuest_ObjectiveState::Active) continue;
        for (FQuest_ObjectiveData& Obj : Mission.Objectives)
        {
            if (Obj.State != EQuest_ObjectiveState::Active) continue;
            if (Obj.ObjectiveType == EQuest_ObjectiveType::Explore ||
                Obj.ObjectiveType == EQuest_ObjectiveType::Escape)
            {
                float Dist = FVector::Dist(PlayerLocation, Obj.TargetLocation);
                if (Dist <= Obj.TargetRadius)
                {
                    Obj.CurrentCount = Obj.RequiredCount;
                    Obj.State = EQuest_ObjectiveState::Completed;
                    OnObjectiveUpdated.Broadcast(Mission.MissionID, Obj);
                    EvaluateObjectiveCompletion(Mission);
                }
            }
        }
    }
}

void AQuest_ObjectiveManager::ReportCraft(FName ItemTag)
{
    for (FQuest_MissionData& Mission : AllMissions)
    {
        if (Mission.MissionState != EQuest_ObjectiveState::Active) continue;
        for (FQuest_ObjectiveData& Obj : Mission.Objectives)
        {
            if (Obj.State != EQuest_ObjectiveState::Active) continue;
            if (Obj.ObjectiveType == EQuest_ObjectiveType::Craft &&
                Obj.TargetActorTag == ItemTag)
            {
                Obj.CurrentCount = Obj.RequiredCount;
                Obj.State = EQuest_ObjectiveState::Completed;
                OnObjectiveUpdated.Broadcast(Mission.MissionID, Obj);
                EvaluateObjectiveCompletion(Mission);
            }
        }
    }
}

// ─── Query ────────────────────────────────────────────────────────────────────

bool AQuest_ObjectiveManager::IsMissionActive(FName MissionID) const
{
    for (const FQuest_MissionData& M : AllMissions)
        if (M.MissionID == MissionID)
            return M.MissionState == EQuest_ObjectiveState::Active;
    return false;
}

bool AQuest_ObjectiveManager::IsMissionComplete(FName MissionID) const
{
    for (const FQuest_MissionData& M : AllMissions)
        if (M.MissionID == MissionID)
            return M.MissionState == EQuest_ObjectiveState::Completed;
    return false;
}

TArray<FQuest_MissionData> AQuest_ObjectiveManager::GetActiveMissions() const
{
    TArray<FQuest_MissionData> Active;
    for (const FQuest_MissionData& M : AllMissions)
        if (M.MissionState == EQuest_ObjectiveState::Active)
            Active.Add(M);
    return Active;
}

FQuest_ObjectiveData AQuest_ObjectiveManager::GetObjectiveStatus(FName MissionID, FName ObjectiveID) const
{
    for (const FQuest_MissionData& M : AllMissions)
    {
        if (M.MissionID == MissionID)
        {
            for (const FQuest_ObjectiveData& Obj : M.Objectives)
                if (Obj.ObjectiveID == ObjectiveID)
                    return Obj;
        }
    }
    return FQuest_ObjectiveData();
}

int32 AQuest_ObjectiveManager::GetTotalCompletedMissions() const
{
    return CompletedMissionCount;
}

// ─── Private Helpers ──────────────────────────────────────────────────────────

void AQuest_ObjectiveManager::EvaluateObjectiveCompletion(FQuest_MissionData& Mission)
{
    // Mission completes when ALL Critical and Primary objectives are done
    bool bAllCriticalDone = true;
    bool bAllPrimaryDone = true;

    for (const FQuest_ObjectiveData& Obj : Mission.Objectives)
    {
        if (Obj.Priority == EQuest_Priority::Critical &&
            Obj.State != EQuest_ObjectiveState::Completed)
        {
            bAllCriticalDone = false;
        }
        if (Obj.Priority == EQuest_Priority::Primary &&
            Obj.State != EQuest_ObjectiveState::Completed)
        {
            bAllPrimaryDone = false;
        }
    }

    if (bAllCriticalDone && bAllPrimaryDone)
    {
        CompleteMission(Mission.MissionID);
    }
}

void AQuest_ObjectiveManager::TickTimedObjectives(FQuest_MissionData& Mission, float DeltaTime)
{
    for (FQuest_ObjectiveData& Obj : Mission.Objectives)
    {
        if (Obj.State != EQuest_ObjectiveState::Active) continue;
        if (Obj.TimeLimitSeconds <= 0.f) continue;

        if (Obj.ObjectiveType == EQuest_ObjectiveType::Survive)
        {
            // Survive objectives complete when time runs out (player survived)
            Obj.ElapsedSeconds += DeltaTime;
            if (Obj.ElapsedSeconds >= Obj.TimeLimitSeconds)
            {
                Obj.CurrentCount = Obj.RequiredCount;
                Obj.State = EQuest_ObjectiveState::Completed;
                OnObjectiveUpdated.Broadcast(Mission.MissionID, Obj);
                EvaluateObjectiveCompletion(Mission);
            }
        }
        else
        {
            // Other timed objectives FAIL when time runs out
            Obj.ElapsedSeconds += DeltaTime;
            if (Obj.ElapsedSeconds >= Obj.TimeLimitSeconds &&
                Obj.State != EQuest_ObjectiveState::Completed)
            {
                Obj.State = EQuest_ObjectiveState::Failed;
                OnObjectiveUpdated.Broadcast(Mission.MissionID, Obj);
                // If critical objective fails, fail the whole mission
                if (Obj.Priority == EQuest_Priority::Critical)
                {
                    FailMission(Mission.MissionID);
                }
            }
        }
    }
}

FQuest_MissionData* AQuest_ObjectiveManager::FindMission(FName MissionID)
{
    for (FQuest_MissionData& M : AllMissions)
        if (M.MissionID == MissionID)
            return &M;
    return nullptr;
}
