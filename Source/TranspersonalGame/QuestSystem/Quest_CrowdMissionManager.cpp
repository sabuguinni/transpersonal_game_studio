#include "Quest_CrowdMissionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"
#include "Components/PrimitiveComponent.h"

UQuest_CrowdMissionManager::UQuest_CrowdMissionManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    NextMissionID = 1;
    UpdateTimer = 0.0f;
    UpdateFrequency = 0.5f;
    PlayerInfluenceStrength = 1000.0f;
    bEnableDebugVisualization = false;
}

void UQuest_CrowdMissionManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Quest_CrowdMissionManager: Component initialized"));
}

void UQuest_CrowdMissionManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateTimer += DeltaTime;
    if (UpdateTimer >= UpdateFrequency)
    {
        UpdateMissionProgress(DeltaTime);
        UpdateTimer = 0.0f;
    }
}

int32 UQuest_CrowdMissionManager::CreateCrowdMission(const FQuest_CrowdMissionData& MissionData)
{
    int32 MissionID = NextMissionID++;
    
    ActiveMissions.Add(MissionID, MissionData);
    MissionStatuses.Add(MissionID, EQuest_CrowdMissionStatus::NotStarted);
    MissionCrowdActors.Add(MissionID, TArray<AActor*>());
    MissionTimers.Add(MissionID, 0.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Quest_CrowdMissionManager: Created mission %d - %s"), MissionID, *MissionData.MissionName);
    
    return MissionID;
}

bool UQuest_CrowdMissionManager::StartCrowdMission(int32 MissionID)
{
    if (!ActiveMissions.Contains(MissionID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: Cannot start mission %d - does not exist"), MissionID);
        return false;
    }
    
    if (MissionStatuses[MissionID] != EQuest_CrowdMissionStatus::NotStarted)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: Mission %d already started or completed"), MissionID);
        return false;
    }
    
    MissionStatuses[MissionID] = EQuest_CrowdMissionStatus::InProgress;
    MissionTimers[MissionID] = 0.0f;
    
    OnMissionStatusChanged.Broadcast(MissionID, EQuest_CrowdMissionStatus::InProgress);
    
    UE_LOG(LogTemp, Log, TEXT("Quest_CrowdMissionManager: Started mission %d"), MissionID);
    return true;
}

bool UQuest_CrowdMissionManager::CompleteCrowdMission(int32 MissionID)
{
    if (!ActiveMissions.Contains(MissionID))
    {
        return false;
    }
    
    MissionStatuses[MissionID] = EQuest_CrowdMissionStatus::Completed;
    OnMissionStatusChanged.Broadcast(MissionID, EQuest_CrowdMissionStatus::Completed);
    
    UE_LOG(LogTemp, Log, TEXT("Quest_CrowdMissionManager: Completed mission %d"), MissionID);
    
    CleanupMission(MissionID);
    return true;
}

bool UQuest_CrowdMissionManager::FailCrowdMission(int32 MissionID)
{
    if (!ActiveMissions.Contains(MissionID))
    {
        return false;
    }
    
    MissionStatuses[MissionID] = EQuest_CrowdMissionStatus::Failed;
    OnMissionStatusChanged.Broadcast(MissionID, EQuest_CrowdMissionStatus::Failed);
    
    UE_LOG(LogTemp, Log, TEXT("Quest_CrowdMissionManager: Failed mission %d"), MissionID);
    
    CleanupMission(MissionID);
    return true;
}

void UQuest_CrowdMissionManager::AbandonCrowdMission(int32 MissionID)
{
    if (!ActiveMissions.Contains(MissionID))
    {
        return;
    }
    
    MissionStatuses[MissionID] = EQuest_CrowdMissionStatus::Abandoned;
    OnMissionStatusChanged.Broadcast(MissionID, EQuest_CrowdMissionStatus::Abandoned);
    
    UE_LOG(LogTemp, Log, TEXT("Quest_CrowdMissionManager: Abandoned mission %d"), MissionID);
    
    CleanupMission(MissionID);
}

void UQuest_CrowdMissionManager::RegisterCrowdActor(AActor* CrowdActor, int32 MissionID)
{
    if (!CrowdActor || !ActiveMissions.Contains(MissionID))
    {
        return;
    }
    
    TArray<AActor*>& CrowdActors = MissionCrowdActors[MissionID];
    if (!CrowdActors.Contains(CrowdActor))
    {
        CrowdActors.Add(CrowdActor);
        UE_LOG(LogTemp, Log, TEXT("Quest_CrowdMissionManager: Registered crowd actor for mission %d"), MissionID);
    }
}

void UQuest_CrowdMissionManager::UnregisterCrowdActor(AActor* CrowdActor, int32 MissionID)
{
    if (!CrowdActor || !ActiveMissions.Contains(MissionID))
    {
        return;
    }
    
    TArray<AActor*>& CrowdActors = MissionCrowdActors[MissionID];
    CrowdActors.Remove(CrowdActor);
    
    UE_LOG(LogTemp, Log, TEXT("Quest_CrowdMissionManager: Unregistered crowd actor for mission %d"), MissionID);
}

int32 UQuest_CrowdMissionManager::GetActiveCrowdCount(int32 MissionID) const
{
    if (!MissionCrowdActors.Contains(MissionID))
    {
        return 0;
    }
    
    const TArray<AActor*>& CrowdActors = MissionCrowdActors[MissionID];
    int32 ActiveCount = 0;
    
    for (AActor* Actor : CrowdActors)
    {
        if (IsValid(Actor))
        {
            ActiveCount++;
        }
    }
    
    return ActiveCount;
}

float UQuest_CrowdMissionManager::GetMissionProgress(int32 MissionID) const
{
    if (!ActiveMissions.Contains(MissionID))
    {
        return 0.0f;
    }
    
    const FQuest_CrowdMissionData& MissionData = ActiveMissions[MissionID];
    int32 CurrentCrowdSize = GetActiveCrowdCount(MissionID);
    
    if (MissionData.bIsEvacuationMission)
    {
        // For evacuation missions, progress is based on how many NPCs reached the target
        int32 NPCsAtTarget = 0;
        const TArray<AActor*>& CrowdActors = MissionCrowdActors[MissionID];
        
        for (AActor* Actor : CrowdActors)
        {
            if (IsValid(Actor) && IsLocationInRadius(Actor->GetActorLocation(), MissionData.TargetLocation, MissionData.CompletionRadius))
            {
                NPCsAtTarget++;
            }
        }
        
        return MissionData.RequiredCrowdSize > 0 ? (float)NPCsAtTarget / (float)MissionData.RequiredCrowdSize : 0.0f;
    }
    else
    {
        // For gathering missions, progress is based on crowd size
        return MissionData.RequiredCrowdSize > 0 ? (float)CurrentCrowdSize / (float)MissionData.RequiredCrowdSize : 0.0f;
    }
}

void UQuest_CrowdMissionManager::SetPlayerGuidanceTarget(int32 MissionID, const FVector& TargetLocation)
{
    if (!ActiveMissions.Contains(MissionID))
    {
        return;
    }
    
    ActiveMissions[MissionID].TargetLocation = TargetLocation;
    UE_LOG(LogTemp, Log, TEXT("Quest_CrowdMissionManager: Set guidance target for mission %d"), MissionID);
}

void UQuest_CrowdMissionManager::EnablePlayerInfluence(int32 MissionID, float InfluenceRadius)
{
    if (!ActiveMissions.Contains(MissionID))
    {
        return;
    }
    
    ActiveMissions[MissionID].bRequiresPlayerGuidance = true;
    ActiveMissions[MissionID].CompletionRadius = InfluenceRadius;
    
    UE_LOG(LogTemp, Log, TEXT("Quest_CrowdMissionManager: Enabled player influence for mission %d"), MissionID);
}

void UQuest_CrowdMissionManager::DisablePlayerInfluence(int32 MissionID)
{
    if (!ActiveMissions.Contains(MissionID))
    {
        return;
    }
    
    ActiveMissions[MissionID].bRequiresPlayerGuidance = false;
    UE_LOG(LogTemp, Log, TEXT("Quest_CrowdMissionManager: Disabled player influence for mission %d"), MissionID);
}

EQuest_CrowdMissionStatus UQuest_CrowdMissionManager::GetMissionStatus(int32 MissionID) const
{
    if (!MissionStatuses.Contains(MissionID))
    {
        return EQuest_CrowdMissionStatus::NotStarted;
    }
    
    return MissionStatuses[MissionID];
}

bool UQuest_CrowdMissionManager::IsMissionActive(int32 MissionID) const
{
    EQuest_CrowdMissionStatus Status = GetMissionStatus(MissionID);
    return Status == EQuest_CrowdMissionStatus::InProgress;
}

FQuest_CrowdMissionData UQuest_CrowdMissionManager::GetMissionData(int32 MissionID) const
{
    if (!ActiveMissions.Contains(MissionID))
    {
        return FQuest_CrowdMissionData();
    }
    
    return ActiveMissions[MissionID];
}

TArray<int32> UQuest_CrowdMissionManager::GetActiveMissionIDs() const
{
    TArray<int32> ActiveIDs;
    
    for (const auto& Pair : MissionStatuses)
    {
        if (Pair.Value == EQuest_CrowdMissionStatus::InProgress)
        {
            ActiveIDs.Add(Pair.Key);
        }
    }
    
    return ActiveIDs;
}

void UQuest_CrowdMissionManager::UpdateMissionProgress(float DeltaTime)
{
    TArray<int32> MissionsToUpdate = GetActiveMissionIDs();
    
    for (int32 MissionID : MissionsToUpdate)
    {
        // Update mission timer
        MissionTimers[MissionID] += UpdateFrequency;
        
        // Check for completion
        CheckMissionCompletion(MissionID);
        
        // Check for failure
        CheckMissionFailure(MissionID);
        
        // Update player influence if enabled
        UpdatePlayerInfluence(MissionID);
        
        // Draw debug info if enabled
        if (bEnableDebugVisualization)
        {
            DrawDebugInfo(MissionID);
        }
        
        // Broadcast progress update
        float Progress = GetMissionProgress(MissionID);
        int32 CrowdSize = GetActiveCrowdCount(MissionID);
        OnCrowdProgress.Broadcast(MissionID, CrowdSize, Progress);
    }
}

void UQuest_CrowdMissionManager::CheckMissionCompletion(int32 MissionID)
{
    if (!ActiveMissions.Contains(MissionID) || !IsMissionActive(MissionID))
    {
        return;
    }
    
    float Progress = GetMissionProgress(MissionID);
    if (Progress >= 1.0f)
    {
        CompleteCrowdMission(MissionID);
    }
}

void UQuest_CrowdMissionManager::CheckMissionFailure(int32 MissionID)
{
    if (!ActiveMissions.Contains(MissionID) || !IsMissionActive(MissionID))
    {
        return;
    }
    
    const FQuest_CrowdMissionData& MissionData = ActiveMissions[MissionID];
    
    // Check time limit
    if (MissionData.TimeLimit > 0.0f && MissionTimers[MissionID] >= MissionData.TimeLimit)
    {
        FailCrowdMission(MissionID);
        return;
    }
    
    // Check if all crowd actors are destroyed/invalid
    int32 ActiveCrowdCount = GetActiveCrowdCount(MissionID);
    if (ActiveCrowdCount == 0 && MissionCrowdActors[MissionID].Num() > 0)
    {
        FailCrowdMission(MissionID);
        return;
    }
}

void UQuest_CrowdMissionManager::UpdatePlayerInfluence(int32 MissionID)
{
    if (!ActiveMissions.Contains(MissionID))
    {
        return;
    }
    
    const FQuest_CrowdMissionData& MissionData = ActiveMissions[MissionID];
    if (!MissionData.bRequiresPlayerGuidance)
    {
        return;
    }
    
    // Get player location
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    
    // Apply influence to crowd actors
    const TArray<AActor*>& CrowdActors = MissionCrowdActors[MissionID];
    for (AActor* Actor : CrowdActors)
    {
        if (!IsValid(Actor))
        {
            continue;
        }
        
        // Find flocking behavior component
        UCrowd_FlockingBehavior* FlockingComponent = Actor->FindComponentByClass<UCrowd_FlockingBehavior>();
        if (FlockingComponent)
        {
            float DistanceToPlayer = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
            if (DistanceToPlayer <= MissionData.CompletionRadius)
            {
                // Apply player influence to guide crowd toward target
                FVector DirectionToTarget = (MissionData.TargetLocation - Actor->GetActorLocation()).GetSafeNormal();
                FVector InfluenceForce = DirectionToTarget * PlayerInfluenceStrength;
                
                // This would require extending the flocking component to accept external forces
                // For now, we log the influence application
                UE_LOG(LogTemp, VeryVerbose, TEXT("Quest_CrowdMissionManager: Applying player influence to crowd actor"));
            }
        }
    }
}

void UQuest_CrowdMissionManager::DrawDebugInfo(int32 MissionID)
{
    if (!ActiveMissions.Contains(MissionID))
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    const FQuest_CrowdMissionData& MissionData = ActiveMissions[MissionID];
    
    // Draw target location
    DrawDebugSphere(World, MissionData.TargetLocation, MissionData.CompletionRadius, 12, FColor::Green, false, UpdateFrequency + 0.1f);
    
    // Draw mission progress text
    FString ProgressText = FString::Printf(TEXT("Mission %d: %.1f%%"), MissionID, GetMissionProgress(MissionID) * 100.0f);
    DrawDebugString(World, MissionData.TargetLocation + FVector(0, 0, 200), ProgressText, nullptr, FColor::White, UpdateFrequency + 0.1f);
    
    // Draw lines from crowd actors to target
    const TArray<AActor*>& CrowdActors = MissionCrowdActors[MissionID];
    for (AActor* Actor : CrowdActors)
    {
        if (IsValid(Actor))
        {
            FColor LineColor = IsLocationInRadius(Actor->GetActorLocation(), MissionData.TargetLocation, MissionData.CompletionRadius) ? FColor::Green : FColor::Red;
            DrawDebugLine(World, Actor->GetActorLocation(), MissionData.TargetLocation, LineColor, false, UpdateFrequency + 0.1f);
        }
    }
}

bool UQuest_CrowdMissionManager::IsLocationInRadius(const FVector& Location, const FVector& Target, float Radius) const
{
    return FVector::Dist(Location, Target) <= Radius;
}

void UQuest_CrowdMissionManager::CleanupMission(int32 MissionID)
{
    // Remove from active tracking but keep data for historical purposes
    if (MissionCrowdActors.Contains(MissionID))
    {
        MissionCrowdActors[MissionID].Empty();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Quest_CrowdMissionManager: Cleaned up mission %d"), MissionID);
}