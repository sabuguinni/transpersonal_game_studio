#include "Quest_CrowdMissionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "../Crowd/Crowd_MassSimulationManager.h"
#include "../TranspersonalCharacter.h"

AQuest_CrowdMissionManager::AQuest_CrowdMissionManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    MaxActiveMissions = 5;
    CrowdManager = nullptr;
    PlayerCharacter = nullptr;
    
    bEnableEvacuationMissions = true;
    bEnableHuntingMissions = true;
    bEnableGatheringMissions = true;
    bEnableDefenseMissions = true;
    
    MissionSpawnRadius = 2000.0f;
    LastMissionSpawnTime = 0.0f;
    MissionSpawnCooldown = 60.0f;
    
    // Initialize default mission spawn locations
    MissionSpawnLocations.Add(FVector(2000.0f, 2000.0f, 100.0f));
    MissionSpawnLocations.Add(FVector(-2000.0f, 2000.0f, 100.0f));
    MissionSpawnLocations.Add(FVector(2000.0f, -2000.0f, 100.0f));
    MissionSpawnLocations.Add(FVector(-2000.0f, -2000.0f, 100.0f));
    MissionSpawnLocations.Add(FVector(0.0f, 3000.0f, 100.0f));
}

void AQuest_CrowdMissionManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find crowd simulation manager
    if (!CrowdManager)
    {
        CrowdManager = Cast<ACrowdSimulationManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ACrowdSimulationManager::StaticClass()));
        if (CrowdManager)
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: Found CrowdSimulationManager"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Quest_CrowdMissionManager: CrowdSimulationManager not found!"));
        }
    }
    
    // Find player character
    if (!PlayerCharacter)
    {
        PlayerCharacter = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
        if (PlayerCharacter)
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: Found PlayerCharacter"));
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: BeginPlay completed"));
}

void AQuest_CrowdMissionManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateMissionProgress(DeltaTime);
    
    // Auto-spawn missions if conditions are met
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastMissionSpawnTime > MissionSpawnCooldown && ActiveMissions.Num() < MaxActiveMissions)
    {
        // Create a random mission type
        int32 MissionTypeRoll = FMath::RandRange(0, 3);
        FQuest_CrowdMission NewMission;
        
        switch (MissionTypeRoll)
        {
            case 0:
                if (bEnableEvacuationMissions)
                {
                    FVector DangerLoc = MissionSpawnLocations[FMath::RandRange(0, MissionSpawnLocations.Num() - 1)];
                    FVector SafeLoc = MissionSpawnLocations[FMath::RandRange(0, MissionSpawnLocations.Num() - 1)];
                    NewMission = CreateEvacuationMission(DangerLoc, SafeLoc);
                }
                break;
            case 1:
                if (bEnableHuntingMissions)
                {
                    FVector HuntLoc = MissionSpawnLocations[FMath::RandRange(0, MissionSpawnLocations.Num() - 1)];
                    NewMission = CreateHuntingMission(HuntLoc, FMath::RandRange(5, 15));
                }
                break;
            case 2:
                if (bEnableGatheringMissions)
                {
                    FVector ResourceLoc = MissionSpawnLocations[FMath::RandRange(0, MissionSpawnLocations.Num() - 1)];
                    NewMission = CreateGatheringMission(ResourceLoc, FMath::RandRange(10, 25));
                }
                break;
            case 3:
                if (bEnableDefenseMissions)
                {
                    FVector DefenseLoc = MissionSpawnLocations[FMath::RandRange(0, MissionSpawnLocations.Num() - 1)];
                    NewMission = CreateDefenseMission(DefenseLoc, 1000.0f);
                }
                break;
        }
        
        if (!NewMission.MissionName.IsEmpty())
        {
            StartMission(NewMission);
            LastMissionSpawnTime = CurrentTime;
        }
    }
}

bool AQuest_CrowdMissionManager::StartMission(const FQuest_CrowdMission& Mission)
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: Cannot start mission - max active missions reached"));
        return false;
    }
    
    FQuest_CrowdMission NewMission = Mission;
    NewMission.MissionStatus = EQuest_MissionStatus::Active;
    NewMission.MissionStartTime = GetWorld()->GetTimeSeconds();
    NewMission.CompletedObjectives = 0;
    
    ActiveMissions.Add(NewMission);
    SpawnMissionMarkers(NewMission);
    
    FString NotificationMessage = FString::Printf(TEXT("New Mission: %s"), *NewMission.MissionName);
    NotifyPlayerMissionUpdate(NotificationMessage);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: Started mission '%s'"), *NewMission.MissionName);
    return true;
}

bool AQuest_CrowdMissionManager::CompleteMission(int32 MissionIndex)
{
    if (!ActiveMissions.IsValidIndex(MissionIndex))
    {
        return false;
    }
    
    FQuest_CrowdMission CompletedMission = ActiveMissions[MissionIndex];
    CompletedMission.MissionStatus = EQuest_MissionStatus::Completed;
    
    CompletedMissions.Add(CompletedMission);
    CleanupMissionMarkers(MissionIndex);
    ActiveMissions.RemoveAt(MissionIndex);
    
    FString NotificationMessage = FString::Printf(TEXT("Mission Completed: %s (+%d XP)"), 
        *CompletedMission.MissionName, CompletedMission.RewardExperience);
    NotifyPlayerMissionUpdate(NotificationMessage);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: Completed mission '%s'"), *CompletedMission.MissionName);
    return true;
}

void AQuest_CrowdMissionManager::UpdateMissionProgress(float DeltaTime)
{
    for (int32 i = ActiveMissions.Num() - 1; i >= 0; i--)
    {
        FQuest_CrowdMission& Mission = ActiveMissions[i];
        bool bMissionComplete = true;
        
        for (FQuest_CrowdMissionObjective& Objective : Mission.Objectives)
        {
            if (!Objective.bIsCompleted)
            {
                Objective.ElapsedTime += DeltaTime;
                CheckObjectiveCompletion(Objective);
                
                if (!Objective.bIsCompleted)
                {
                    bMissionComplete = false;
                    
                    // Check time limit
                    if (Objective.TimeLimit > 0.0f && Objective.ElapsedTime >= Objective.TimeLimit)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: Objective timed out"));
                        Mission.MissionStatus = EQuest_MissionStatus::Failed;
                        CleanupMissionMarkers(i);
                        ActiveMissions.RemoveAt(i);
                        return;
                    }
                }
                else
                {
                    Mission.CompletedObjectives++;
                }
            }
        }
        
        if (bMissionComplete && Mission.MissionStatus == EQuest_MissionStatus::Active)
        {
            CompleteMission(i);
        }
    }
}

FQuest_CrowdMission AQuest_CrowdMissionManager::CreateEvacuationMission(FVector DangerLocation, FVector SafeLocation)
{
    FQuest_CrowdMission Mission;
    Mission.MissionName = TEXT("Emergency Evacuation");
    Mission.MissionDescription = TEXT("Guide the tribe away from danger to safety");
    Mission.MissionType = EQuest_MissionType::Escort;
    Mission.RewardExperience = 200;
    Mission.RewardItems.Add(TEXT("Survival Knowledge"));
    
    FQuest_CrowdMissionObjective Objective;
    Objective.ObjectiveDescription = TEXT("Lead 20 tribe members to safety");
    Objective.ObjectiveType = EQuest_ObjectiveType::Escort;
    Objective.RequiredCrowdCount = 20;
    Objective.TargetLocation = SafeLocation;
    Objective.CompletionRadius = 300.0f;
    Objective.TimeLimit = 180.0f;
    
    Mission.Objectives.Add(Objective);
    return Mission;
}

FQuest_CrowdMission AQuest_CrowdMissionManager::CreateHuntingMission(FVector HuntLocation, int32 RequiredHunters)
{
    FQuest_CrowdMission Mission;
    Mission.MissionName = TEXT("Coordinated Hunt");
    Mission.MissionDescription = TEXT("Organize hunters to take down large prey");
    Mission.MissionType = EQuest_MissionType::Hunt;
    Mission.RewardExperience = 150;
    Mission.RewardItems.Add(TEXT("Fresh Meat"));
    Mission.RewardItems.Add(TEXT("Animal Hide"));
    
    FQuest_CrowdMissionObjective Objective;
    Objective.ObjectiveDescription = FString::Printf(TEXT("Gather %d hunters at the hunting ground"), RequiredHunters);
    Objective.ObjectiveType = EQuest_ObjectiveType::Gather;
    Objective.RequiredCrowdCount = RequiredHunters;
    Objective.TargetLocation = HuntLocation;
    Objective.CompletionRadius = 400.0f;
    Objective.TimeLimit = 240.0f;
    
    Mission.Objectives.Add(Objective);
    return Mission;
}

FQuest_CrowdMission AQuest_CrowdMissionManager::CreateGatheringMission(FVector ResourceLocation, int32 RequiredGatherers)
{
    FQuest_CrowdMission Mission;
    Mission.MissionName = TEXT("Resource Gathering");
    Mission.MissionDescription = TEXT("Organize the tribe to collect essential resources");
    Mission.MissionType = EQuest_MissionType::Gather;
    Mission.RewardExperience = 100;
    Mission.RewardItems.Add(TEXT("Berries"));
    Mission.RewardItems.Add(TEXT("Medicinal Herbs"));
    
    FQuest_CrowdMissionObjective Objective;
    Objective.ObjectiveDescription = FString::Printf(TEXT("Send %d gatherers to collect resources"), RequiredGatherers);
    Objective.ObjectiveType = EQuest_ObjectiveType::Gather;
    Objective.RequiredCrowdCount = RequiredGatherers;
    Objective.TargetLocation = ResourceLocation;
    Objective.CompletionRadius = 250.0f;
    Objective.TimeLimit = 300.0f;
    
    Mission.Objectives.Add(Objective);
    return Mission;
}

FQuest_CrowdMission AQuest_CrowdMissionManager::CreateDefenseMission(FVector DefenseLocation, float DefenseRadius)
{
    FQuest_CrowdMission Mission;
    Mission.MissionName = TEXT("Defend the Camp");
    Mission.MissionDescription = TEXT("Rally defenders to protect the settlement");
    Mission.MissionType = EQuest_MissionType::Defense;
    Mission.RewardExperience = 250;
    Mission.RewardItems.Add(TEXT("Stone Weapons"));
    Mission.RewardItems.Add(TEXT("Defensive Knowledge"));
    
    FQuest_CrowdMissionObjective Objective;
    Objective.ObjectiveDescription = TEXT("Position 15 defenders around the camp");
    Objective.ObjectiveType = EQuest_ObjectiveType::Defend;
    Objective.RequiredCrowdCount = 15;
    Objective.TargetLocation = DefenseLocation;
    Objective.CompletionRadius = DefenseRadius;
    Objective.TimeLimit = 120.0f;
    
    Mission.Objectives.Add(Objective);
    return Mission;
}

int32 AQuest_CrowdMissionManager::GetCrowdCountAtLocation(FVector Location, float Radius)
{
    if (!CrowdManager)
    {
        return 0;
    }
    
    // This would integrate with the crowd simulation system
    // For now, return a simulated count based on distance from spawn points
    int32 CrowdCount = 0;
    for (const FVector& SpawnLoc : MissionSpawnLocations)
    {
        float Distance = FVector::Dist(Location, SpawnLoc);
        if (Distance <= Radius)
        {
            CrowdCount += FMath::RandRange(5, 20);
        }
    }
    
    return FMath::Min(CrowdCount, 50); // Cap at reasonable number
}

bool AQuest_CrowdMissionManager::DirectCrowdToLocation(FVector SourceLocation, FVector TargetLocation, int32 CrowdCount)
{
    if (!CrowdManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: Cannot direct crowd - no CrowdManager"));
        return false;
    }
    
    // This would call methods on the crowd simulation manager
    // For now, just log the action
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: Directing %d crowd members from %s to %s"), 
        CrowdCount, *SourceLocation.ToString(), *TargetLocation.ToString());
    
    return true;
}

TArray<FQuest_CrowdMission> AQuest_CrowdMissionManager::GetActiveMissions() const
{
    return ActiveMissions;
}

int32 AQuest_CrowdMissionManager::GetActiveMissionCount() const
{
    return ActiveMissions.Num();
}

bool AQuest_CrowdMissionManager::HasActiveMissionOfType(EQuest_MissionType MissionType) const
{
    for (const FQuest_CrowdMission& Mission : ActiveMissions)
    {
        if (Mission.MissionType == MissionType)
        {
            return true;
        }
    }
    return false;
}

void AQuest_CrowdMissionManager::OnPlayerNearMissionArea(FVector PlayerLocation)
{
    for (const FQuest_CrowdMission& Mission : ActiveMissions)
    {
        for (const FQuest_CrowdMissionObjective& Objective : Mission.Objectives)
        {
            float Distance = FVector::Dist(PlayerLocation, Objective.TargetLocation);
            if (Distance <= Objective.CompletionRadius + 200.0f)
            {
                FString Message = FString::Printf(TEXT("Mission Area Nearby: %s"), *Mission.MissionName);
                NotifyPlayerMissionUpdate(Message);
                break;
            }
        }
    }
}

void AQuest_CrowdMissionManager::OnPlayerInfluenceCrowd(FVector InfluenceLocation, float InfluenceRadius)
{
    // Check if player influence affects any active mission objectives
    for (FQuest_CrowdMission& Mission : ActiveMissions)
    {
        for (FQuest_CrowdMissionObjective& Objective : Mission.Objectives)
        {
            if (!Objective.bIsCompleted)
            {
                float Distance = FVector::Dist(InfluenceLocation, Objective.TargetLocation);
                if (Distance <= InfluenceRadius + Objective.CompletionRadius)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: Player influence detected near mission objective"));
                    // Could trigger crowd movement or objective progress here
                }
            }
        }
    }
}

void AQuest_CrowdMissionManager::CheckObjectiveCompletion(FQuest_CrowdMissionObjective& Objective)
{
    if (Objective.bIsCompleted)
    {
        return;
    }
    
    // Check crowd count at target location
    int32 CurrentCrowdCount = GetCrowdCountAtLocation(Objective.TargetLocation, Objective.CompletionRadius);
    
    if (CurrentCrowdCount >= Objective.RequiredCrowdCount)
    {
        Objective.bIsCompleted = true;
        UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: Objective completed - %s"), *Objective.ObjectiveDescription);
    }
}

void AQuest_CrowdMissionManager::SpawnMissionMarkers(const FQuest_CrowdMission& Mission)
{
    for (const FQuest_CrowdMissionObjective& Objective : Mission.Objectives)
    {
        // Spawn a visual marker at the objective location
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        AStaticMeshActor* Marker = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            Objective.TargetLocation + FVector(0, 0, 100),
            FRotator::ZeroRotator,
            SpawnParams
        );
        
        if (Marker)
        {
            MissionMarkers.Add(Marker);
            UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdMissionManager: Spawned mission marker at %s"), *Objective.TargetLocation.ToString());
        }
    }
}

void AQuest_CrowdMissionManager::CleanupMissionMarkers(int32 MissionIndex)
{
    // Remove markers associated with this mission
    // For simplicity, remove the last few markers (should be improved with proper tracking)
    if (MissionMarkers.Num() > 0)
    {
        for (int32 i = MissionMarkers.Num() - 1; i >= FMath::Max(0, MissionMarkers.Num() - 3); i--)
        {
            if (MissionMarkers[i] && IsValid(MissionMarkers[i]))
            {
                MissionMarkers[i]->Destroy();
                MissionMarkers.RemoveAt(i);
            }
        }
    }
}

void AQuest_CrowdMissionManager::NotifyPlayerMissionUpdate(const FString& Message)
{
    UE_LOG(LogTemp, Warning, TEXT("MISSION UPDATE: %s"), *Message);
    
    // In a full implementation, this would show UI notifications
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, Message);
    }
}