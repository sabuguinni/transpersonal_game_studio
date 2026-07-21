#include "Quest_CrowdInteractionSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AQuest_CrowdInteractionSystem::AQuest_CrowdInteractionSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    MaxSimultaneousObjectives = 3;
    ObjectiveCheckInterval = 2.0f;
    bSystemEnabled = true;
    PlayerLastKnownPosition = FVector::ZeroVector;
}

void AQuest_CrowdInteractionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    if (bSystemEnabled)
    {
        GetWorldTimerManager().SetTimer(ObjectiveUpdateTimer, this, 
            &AQuest_CrowdInteractionSystem::UpdateObjectiveProgress, 
            ObjectiveCheckInterval, true);
        
        UE_LOG(LogTemp, Warning, TEXT("Quest Crowd Interaction System initialized"));
        
        // Auto-start a sample mission for testing
        FTimerHandle DelayedStartTimer;
        GetWorldTimerManager().SetTimerForNextTick([this]()
        {
            CreateHerdProtectionMission();
        });
    }
}

void AQuest_CrowdInteractionSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update player position for proximity-based objectives
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC && PC->GetPawn())
    {
        PlayerLastKnownPosition = PC->GetPawn()->GetActorLocation();
    }
}

void AQuest_CrowdInteractionSystem::StartCrowdMission(EQuest_CrowdMissionType MissionType, FVector TargetLocation, int32 CrowdSize)
{
    if (ActiveObjectives.Num() >= MaxSimultaneousObjectives)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start new mission - maximum objectives reached"));
        return;
    }
    
    FQuest_CrowdObjective NewObjective;
    NewObjective.MissionType = MissionType;
    NewObjective.TargetLocation = TargetLocation;
    NewObjective.TargetCrowdSize = CrowdSize;
    NewObjective.CompletionRadius = 3000.0f;
    NewObjective.TimeLimit = 600.0f;
    NewObjective.bIsCompleted = false;
    
    switch (MissionType)
    {
        case EQuest_CrowdMissionType::HerdProtection:
            NewObjective.ObjectiveName = TEXT("Protect the Herd");
            break;
        case EQuest_CrowdMissionType::MigrationGuide:
            NewObjective.ObjectiveName = TEXT("Guide Migration");
            break;
        case EQuest_CrowdMissionType::PredatorDistraction:
            NewObjective.ObjectiveName = TEXT("Distract Predators");
            break;
        case EQuest_CrowdMissionType::ResourceGathering:
            NewObjective.ObjectiveName = TEXT("Gather Resources");
            break;
        case EQuest_CrowdMissionType::TerritoryDefense:
            NewObjective.ObjectiveName = TEXT("Defend Territory");
            break;
    }
    
    ActiveObjectives.Add(NewObjective);
    UE_LOG(LogTemp, Warning, TEXT("Started mission: %s at location (%f, %f, %f)"), 
        *NewObjective.ObjectiveName, TargetLocation.X, TargetLocation.Y, TargetLocation.Z);
}

void AQuest_CrowdInteractionSystem::CompleteObjective(int32 ObjectiveIndex)
{
    if (ActiveObjectives.IsValidIndex(ObjectiveIndex))
    {
        ActiveObjectives[ObjectiveIndex].bIsCompleted = true;
        UE_LOG(LogTemp, Warning, TEXT("Objective completed: %s"), 
            *ActiveObjectives[ObjectiveIndex].ObjectiveName);
        
        // Remove completed objective after a delay
        FTimerHandle RemovalTimer;
        GetWorldTimerManager().SetTimer(RemovalTimer, [this, ObjectiveIndex]()
        {
            if (ActiveObjectives.IsValidIndex(ObjectiveIndex))
            {
                ActiveObjectives.RemoveAt(ObjectiveIndex);
            }
        }, 3.0f, false);
    }
}

bool AQuest_CrowdInteractionSystem::CheckObjectiveCompletion(const FQuest_CrowdObjective& Objective)
{
    if (Objective.bIsCompleted)
    {
        return true;
    }
    
    // Check if player is in target area
    float DistanceToTarget = FVector::Dist(PlayerLastKnownPosition, Objective.TargetLocation);
    if (DistanceToTarget > Objective.CompletionRadius)
    {
        return false;
    }
    
    // Check crowd size in area
    TArray<AActor*> CrowdActors = GetCrowdActorsInRadius(Objective.TargetLocation, Objective.CompletionRadius);
    return CrowdActors.Num() >= Objective.TargetCrowdSize;
}

void AQuest_CrowdInteractionSystem::UpdateObjectiveProgress()
{
    for (int32 i = ActiveObjectives.Num() - 1; i >= 0; i--)
    {
        if (CheckObjectiveCompletion(ActiveObjectives[i]))
        {
            CompleteObjective(i);
        }
    }
}

TArray<AActor*> AQuest_CrowdInteractionSystem::GetCrowdActorsInRadius(FVector Center, float Radius)
{
    TArray<AActor*> FoundActors;
    TArray<AActor*> AllActors;
    
    UGameplayStatics::GetAllActorsOfClass(this, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Dinosaur")) || 
            Actor->GetName().Contains(TEXT("Crowd")) ||
            Actor->GetName().Contains(TEXT("Herd")))
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), Center);
            if (Distance <= Radius)
            {
                FoundActors.Add(Actor);
            }
        }
    }
    
    return FoundActors;
}

void AQuest_CrowdInteractionSystem::CreateHerdProtectionMission()
{
    // Create mission in Savanna biome where herds are located
    FVector MissionLocation = FVector(2000.0f, 2000.0f, 200.0f);
    StartCrowdMission(EQuest_CrowdMissionType::HerdProtection, MissionLocation, 15);
    
    UE_LOG(LogTemp, Warning, TEXT("Herd Protection Mission created in Savanna"));
}

void AQuest_CrowdInteractionSystem::CreateMigrationGuideMission()
{
    // Create migration route from Savanna to Forest
    FVector MigrationStart = FVector(0.0f, 0.0f, 200.0f);
    FVector MigrationEnd = FVector(-45000.0f, 40000.0f, 200.0f);
    FVector MidPoint = (MigrationStart + MigrationEnd) * 0.5f;
    
    StartCrowdMission(EQuest_CrowdMissionType::MigrationGuide, MidPoint, 25);
    
    UE_LOG(LogTemp, Warning, TEXT("Migration Guide Mission created"));
}

void AQuest_CrowdInteractionSystem::CreatePredatorDistractionMission()
{
    // Create distraction mission near predator territory
    FVector PredatorZone = FVector(5000.0f, -3000.0f, 200.0f);
    StartCrowdMission(EQuest_CrowdMissionType::PredatorDistraction, PredatorZone, 8);
    
    UE_LOG(LogTemp, Warning, TEXT("Predator Distraction Mission created"));
}

int32 AQuest_CrowdInteractionSystem::GetActiveMissionCount() const
{
    return ActiveObjectives.Num();
}

void AQuest_CrowdInteractionSystem::ClearAllObjectives()
{
    ActiveObjectives.Empty();
    UE_LOG(LogTemp, Warning, TEXT("All objectives cleared"));
}

FString AQuest_CrowdInteractionSystem::GetMissionStatusReport()
{
    FString Report = FString::Printf(TEXT("Active Missions: %d/%d\n"), 
        ActiveObjectives.Num(), MaxSimultaneousObjectives);
    
    for (int32 i = 0; i < ActiveObjectives.Num(); i++)
    {
        const FQuest_CrowdObjective& Obj = ActiveObjectives[i];
        FString Status = Obj.bIsCompleted ? TEXT("COMPLETED") : TEXT("ACTIVE");
        Report += FString::Printf(TEXT("%d. %s [%s]\n"), 
            i + 1, *Obj.ObjectiveName, *Status);
    }
    
    return Report;
}