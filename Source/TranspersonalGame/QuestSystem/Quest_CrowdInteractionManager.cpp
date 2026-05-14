#include "Quest_CrowdInteractionManager.h"
#include "../CrowdSimulation/Crowd_PathfindingManager.h"
#include "../Character/TranspersonalCharacter.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/PlayerController.h"

UQuest_CrowdInteractionManager::UQuest_CrowdInteractionManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    ObjectiveCheckInterval = 1.0f;
    MaxInfluenceDistance = 2000.0f;
    MaxSimultaneousMissions = 5;
    bEnableDebugVisualization = false;
    
    LastObjectiveCheck = 0.0f;
    LastInfluenceUpdate = 0.0f;
    
    CrowdPathfindingManager = nullptr;
    PlayerCharacter = nullptr;
}

void UQuest_CrowdInteractionManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find crowd pathfinding manager
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (UCrowd_PathfindingManager* FoundManager = ActorItr->FindComponentByClass<UCrowd_PathfindingManager>())
            {
                CrowdPathfindingManager = FoundManager;
                break;
            }
        }
        
        // Find player character
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerCharacter = Cast<ATranspersonalCharacter>(PC->GetPawn());
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdInteractionManager initialized - CrowdManager: %s, Player: %s"), 
           CrowdPathfindingManager ? TEXT("Found") : TEXT("Not Found"),
           PlayerCharacter ? TEXT("Found") : TEXT("Not Found"));
}

void UQuest_CrowdInteractionManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update crowd objectives
    if (CurrentTime - LastObjectiveCheck >= ObjectiveCheckInterval)
    {
        UpdateCrowdObjectives(DeltaTime);
        LastObjectiveCheck = CurrentTime;
    }
    
    // Update player influence
    if (CurrentPlayerInfluence.bIsActive)
    {
        UpdatePlayerInfluence(DeltaTime);
    }
    
    // Debug visualization
    if (bEnableDebugVisualization)
    {
        DrawDebugInfo();
    }
}

void UQuest_CrowdInteractionManager::StartCrowdMission(const FString& MissionID, const TArray<FQuest_CrowdMissionObjective>& Objectives)
{
    if (ActiveCrowdMissions.Num() >= MaxSimultaneousMissions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start mission %s - maximum concurrent missions reached"), *MissionID);
        return;
    }
    
    if (ActiveCrowdMissions.Contains(MissionID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Mission %s is already active"), *MissionID);
        return;
    }
    
    ActiveCrowdMissions.Add(MissionID, Objectives);
    MissionStartTimes.Add(MissionID, GetWorld()->GetTimeSeconds());
    
    UE_LOG(LogTemp, Log, TEXT("Started crowd mission: %s with %d objectives"), *MissionID, Objectives.Num());
    
    // Initialize crowd pathfinding for mission objectives
    if (CrowdPathfindingManager)
    {
        for (const FQuest_CrowdMissionObjective& Objective : Objectives)
        {
            if (Objective.ObjectiveType == EQuestObjectiveType::Escort || 
                Objective.ObjectiveType == EQuestObjectiveType::Gather)
            {
                CrowdPathfindingManager->AddPathfindingTarget(Objective.TargetLocation, Objective.CompletionRadius);
            }
        }
    }
}

void UQuest_CrowdInteractionManager::EndCrowdMission(const FString& MissionID, bool bSuccess)
{
    if (!ActiveCrowdMissions.Contains(MissionID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot end mission %s - mission not found"), *MissionID);
        return;
    }
    
    ActiveCrowdMissions.Remove(MissionID);
    MissionStartTimes.Remove(MissionID);
    
    UE_LOG(LogTemp, Log, TEXT("Ended crowd mission: %s (Success: %s)"), *MissionID, bSuccess ? TEXT("Yes") : TEXT("No"));
    
    // Clean up pathfinding targets
    if (CrowdPathfindingManager)
    {
        CrowdPathfindingManager->ClearAllTargets();
    }
}

bool UQuest_CrowdInteractionManager::IsCrowdMissionActive(const FString& MissionID) const
{
    return ActiveCrowdMissions.Contains(MissionID);
}

float UQuest_CrowdInteractionManager::GetCrowdMissionProgress(const FString& MissionID) const
{
    if (!ActiveCrowdMissions.Contains(MissionID))
    {
        return 0.0f;
    }
    
    const TArray<FQuest_CrowdMissionObjective>& Objectives = ActiveCrowdMissions[MissionID];
    if (Objectives.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 CompletedObjectives = 0;
    for (const FQuest_CrowdMissionObjective& Objective : Objectives)
    {
        if (Objective.bIsCompleted)
        {
            CompletedObjectives++;
        }
    }
    
    return static_cast<float>(CompletedObjectives) / static_cast<float>(Objectives.Num());
}

void UQuest_CrowdInteractionManager::SetPlayerCrowdInfluence(const FQuest_CrowdInfluenceData& InfluenceData)
{
    CurrentPlayerInfluence = InfluenceData;
    CurrentPlayerInfluence.bIsActive = true;
    
    if (PlayerCharacter)
    {
        CurrentPlayerInfluence.InfluenceDirection = PlayerCharacter->GetActorLocation();
    }
    
    OnCrowdInfluenceChanged.Broadcast(CurrentPlayerInfluence.InfluenceDirection, 
                                     CurrentPlayerInfluence.InfluenceRadius, 
                                     CurrentPlayerInfluence.InfluenceStrength);
    
    UE_LOG(LogTemp, Log, TEXT("Player crowd influence activated - Radius: %.1f, Strength: %.2f"), 
           InfluenceData.InfluenceRadius, InfluenceData.InfluenceStrength);
}

void UQuest_CrowdInteractionManager::UpdatePlayerInfluenceLocation(FVector NewLocation)
{
    if (CurrentPlayerInfluence.bIsActive)
    {
        CurrentPlayerInfluence.InfluenceDirection = NewLocation;
        OnCrowdInfluenceChanged.Broadcast(NewLocation, CurrentPlayerInfluence.InfluenceRadius, CurrentPlayerInfluence.InfluenceStrength);
    }
}

void UQuest_CrowdInteractionManager::DisablePlayerCrowdInfluence()
{
    CurrentPlayerInfluence.bIsActive = false;
    OnCrowdInfluenceChanged.Broadcast(FVector::ZeroVector, 0.0f, 0.0f);
    UE_LOG(LogTemp, Log, TEXT("Player crowd influence disabled"));
}

void UQuest_CrowdInteractionManager::AddCrowdObjective(const FString& MissionID, const FQuest_CrowdMissionObjective& Objective)
{
    if (TArray<FQuest_CrowdMissionObjective>* Objectives = ActiveCrowdMissions.Find(MissionID))
    {
        Objectives->Add(Objective);
        UE_LOG(LogTemp, Log, TEXT("Added objective %s to mission %s"), *Objective.ObjectiveID, *MissionID);
    }
}

void UQuest_CrowdInteractionManager::RemoveCrowdObjective(const FString& MissionID, const FString& ObjectiveID)
{
    if (TArray<FQuest_CrowdMissionObjective>* Objectives = ActiveCrowdMissions.Find(MissionID))
    {
        Objectives->RemoveAll([&ObjectiveID](const FQuest_CrowdMissionObjective& Obj) {
            return Obj.ObjectiveID == ObjectiveID;
        });
        UE_LOG(LogTemp, Log, TEXT("Removed objective %s from mission %s"), *ObjectiveID, *MissionID);
    }
}

bool UQuest_CrowdInteractionManager::IsObjectiveCompleted(const FString& MissionID, const FString& ObjectiveID) const
{
    if (const TArray<FQuest_CrowdMissionObjective>* Objectives = ActiveCrowdMissions.Find(MissionID))
    {
        for (const FQuest_CrowdMissionObjective& Objective : *Objectives)
        {
            if (Objective.ObjectiveID == ObjectiveID)
            {
                return Objective.bIsCompleted;
            }
        }
    }
    return false;
}

int32 UQuest_CrowdInteractionManager::GetCrowdCountInArea(FVector Center, float Radius) const
{
    if (!CrowdPathfindingManager)
    {
        return 0;
    }
    
    // Use crowd pathfinding manager to get agent count in area
    return CrowdPathfindingManager->GetAgentCountInRadius(Center, Radius);
}

TArray<FVector> UQuest_CrowdInteractionManager::GetCrowdPositionsInArea(FVector Center, float Radius) const
{
    TArray<FVector> Positions;
    
    if (CrowdPathfindingManager)
    {
        Positions = CrowdPathfindingManager->GetAgentPositionsInRadius(Center, Radius);
    }
    
    return Positions;
}

void UQuest_CrowdInteractionManager::GuideCrowdToLocation(FVector TargetLocation, float InfluenceRadius)
{
    if (CrowdPathfindingManager)
    {
        CrowdPathfindingManager->AddPathfindingTarget(TargetLocation, InfluenceRadius);
        CrowdPathfindingManager->UpdateFlowField(TargetLocation);
        
        UE_LOG(LogTemp, Log, TEXT("Guiding crowd to location: %s (Radius: %.1f)"), 
               *TargetLocation.ToString(), InfluenceRadius);
    }
}

void UQuest_CrowdInteractionManager::TriggerCrowdEvacuation(FVector DangerZone, float DangerRadius, FVector SafeZone)
{
    if (CrowdPathfindingManager)
    {
        // Add danger zone as obstacle
        CrowdPathfindingManager->RegisterDynamicObstacle(DangerZone, DangerRadius);
        
        // Set safe zone as primary target
        CrowdPathfindingManager->AddPathfindingTarget(SafeZone, 1000.0f);
        CrowdPathfindingManager->UpdateFlowField(SafeZone);
        
        UE_LOG(LogTemp, Warning, TEXT("EVACUATION TRIGGERED - Danger: %s (R:%.1f), Safe: %s"), 
               *DangerZone.ToString(), DangerRadius, *SafeZone.ToString());
    }
}

void UQuest_CrowdInteractionManager::CreateCrowdGatheringPoint(FVector Location, float Radius, int32 TargetCount)
{
    if (CrowdPathfindingManager)
    {
        CrowdPathfindingManager->AddPathfindingTarget(Location, Radius);
        CrowdPathfindingManager->UpdateFlowField(Location);
        
        UE_LOG(LogTemp, Log, TEXT("Created gathering point at %s (R:%.1f, Target:%d)"), 
               *Location.ToString(), Radius, TargetCount);
    }
}

void UQuest_CrowdInteractionManager::DisperseCrowdFromArea(FVector Center, float Radius)
{
    if (CrowdPathfindingManager)
    {
        CrowdPathfindingManager->RegisterDynamicObstacle(Center, Radius);
        UE_LOG(LogTemp, Log, TEXT("Dispersing crowd from area: %s (R:%.1f)"), *Center.ToString(), Radius);
    }
}

void UQuest_CrowdInteractionManager::UpdateCrowdObjectives(float DeltaTime)
{
    for (auto& MissionPair : ActiveCrowdMissions)
    {
        const FString& MissionID = MissionPair.Key;
        TArray<FQuest_CrowdMissionObjective>& Objectives = MissionPair.Value;
        
        for (FQuest_CrowdMissionObjective& Objective : Objectives)
        {
            if (!Objective.bIsCompleted)
            {
                CheckObjectiveCompletion(MissionID, Objective);
            }
        }
    }
}

void UQuest_CrowdInteractionManager::CheckObjectiveCompletion(const FString& MissionID, FQuest_CrowdMissionObjective& Objective)
{
    bool bWasCompleted = false;
    
    switch (Objective.ObjectiveType)
    {
        case EQuestObjectiveType::Gather:
        {
            int32 CrowdCount = GetCrowdCountInArea(Objective.TargetLocation, Objective.CompletionRadius);
            if (CrowdCount >= Objective.RequiredCrowdCount)
            {
                Objective.bIsCompleted = true;
                bWasCompleted = true;
            }
            
            float Progress = FMath::Clamp(static_cast<float>(CrowdCount) / static_cast<float>(Objective.RequiredCrowdCount), 0.0f, 1.0f);
            OnCrowdObjectiveProgress.Broadcast(Objective.ObjectiveID, Progress);
            break;
        }
        
        case EQuestObjectiveType::Escort:
        {
            // Check if enough NPCs have reached the target location
            int32 CrowdCount = GetCrowdCountInArea(Objective.TargetLocation, Objective.CompletionRadius);
            if (CrowdCount >= Objective.RequiredCrowdCount)
            {
                Objective.bIsCompleted = true;
                bWasCompleted = true;
            }
            break;
        }
        
        case EQuestObjectiveType::Defend:
        {
            // Check if crowd is still present in the area (not dispersed)
            int32 CrowdCount = GetCrowdCountInArea(Objective.TargetLocation, Objective.CompletionRadius);
            if (CrowdCount >= Objective.RequiredCrowdCount)
            {
                Objective.bIsCompleted = true;
                bWasCompleted = true;
            }
            break;
        }
    }
    
    // Check time limit
    if (!Objective.bIsCompleted && Objective.TimeLimit > 0.0f)
    {
        float* StartTime = MissionStartTimes.Find(MissionID);
        if (StartTime && (GetWorld()->GetTimeSeconds() - *StartTime) >= Objective.TimeLimit)
        {
            UE_LOG(LogTemp, Warning, TEXT("Objective %s timed out"), *Objective.ObjectiveID);
        }
    }
    
    if (bWasCompleted)
    {
        OnCrowdObjectiveCompleted.Broadcast(Objective.ObjectiveID);
        UE_LOG(LogTemp, Log, TEXT("Objective completed: %s"), *Objective.ObjectiveID);
    }
}

void UQuest_CrowdInteractionManager::UpdatePlayerInfluence(float DeltaTime)
{
    if (PlayerCharacter && CurrentPlayerInfluence.bIsActive)
    {
        FVector PlayerLocation = PlayerCharacter->GetActorLocation();
        UpdatePlayerInfluenceLocation(PlayerLocation);
        
        // Apply influence to nearby crowd agents through pathfinding manager
        if (CrowdPathfindingManager)
        {
            CrowdPathfindingManager->ApplyPlayerInfluence(PlayerLocation, 
                                                         CurrentPlayerInfluence.InfluenceRadius, 
                                                         CurrentPlayerInfluence.InfluenceStrength);
        }
    }
}

void UQuest_CrowdInteractionManager::DrawDebugInfo() const
{
    if (!GetWorld())
    {
        return;
    }
    
    // Draw active objectives
    for (const auto& MissionPair : ActiveCrowdMissions)
    {
        const TArray<FQuest_CrowdMissionObjective>& Objectives = MissionPair.Value;
        
        for (const FQuest_CrowdMissionObjective& Objective : Objectives)
        {
            FColor DebugColor = Objective.bIsCompleted ? FColor::Green : FColor::Yellow;
            
            DrawDebugSphere(GetWorld(), Objective.TargetLocation, Objective.CompletionRadius, 
                           16, DebugColor, false, -1.0f, 0, 5.0f);
            
            DrawDebugString(GetWorld(), Objective.TargetLocation + FVector(0, 0, 200), 
                           FString::Printf(TEXT("%s\n%s\nCount: %d/%d"), 
                                         *Objective.ObjectiveID, 
                                         *Objective.Description,
                                         GetCrowdCountInArea(Objective.TargetLocation, Objective.CompletionRadius),
                                         Objective.RequiredCrowdCount), 
                           nullptr, DebugColor, 0.0f);
        }
    }
    
    // Draw player influence
    if (CurrentPlayerInfluence.bIsActive && PlayerCharacter)
    {
        FVector PlayerLocation = PlayerCharacter->GetActorLocation();
        DrawDebugSphere(GetWorld(), PlayerLocation, CurrentPlayerInfluence.InfluenceRadius, 
                       32, FColor::Blue, false, -1.0f, 0, 3.0f);
    }
}