#include "Quest_ObjectiveManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TriggerBox.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AQuest_ObjectiveManager::AQuest_ObjectiveManager()
{
    PrimaryActorTick.bCanEverTick = true;
    ObjectiveCheckInterval = 1.0f;
    bDebugObjectives = true;
}

void AQuest_ObjectiveManager::BeginPlay()
{
    Super::BeginPlay();
    
    SetupDefaultObjectives();
    
    if (bDebugObjectives)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_ObjectiveManager: Initialized with %d default objectives"), ActiveObjectives.Num());
    }
}

void AQuest_ObjectiveManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    static float CheckTimer = 0.0f;
    CheckTimer += DeltaTime;
    
    if (CheckTimer >= ObjectiveCheckInterval)
    {
        CheckObjectiveCompletion();
        UpdateObjectiveTriggers();
        CheckTimer = 0.0f;
    }
    
    // Debug draw active objectives
    if (bDebugObjectives)
    {
        for (const FQuest_ObjectiveData& Objective : ActiveObjectives)
        {
            if (Objective.Status == EQuest_ObjectiveStatus::Active)
            {
                FColor DebugColor = FColor::Yellow;
                switch (Objective.ObjectiveType)
                {
                    case EQuest_ObjectiveType::Hunt:
                        DebugColor = FColor::Red;
                        break;
                    case EQuest_ObjectiveType::Gather:
                        DebugColor = FColor::Green;
                        break;
                    case EQuest_ObjectiveType::Explore:
                        DebugColor = FColor::Blue;
                        break;
                    case EQuest_ObjectiveType::Defend:
                        DebugColor = FColor::Orange;
                        break;
                    case EQuest_ObjectiveType::Escort:
                        DebugColor = FColor::Purple;
                        break;
                    case EQuest_ObjectiveType::Survive:
                        DebugColor = FColor::Cyan;
                        break;
                }
                
                DrawDebugSphere(GetWorld(), Objective.TargetLocation, Objective.CompletionRadius, 12, DebugColor, false, ObjectiveCheckInterval + 0.1f);
                DrawDebugString(GetWorld(), Objective.TargetLocation + FVector(0, 0, 100), 
                               FString::Printf(TEXT("%s (%d/%d)"), *Objective.ObjectiveName, Objective.CurrentCount, Objective.RequiredCount),
                               nullptr, DebugColor, ObjectiveCheckInterval + 0.1f);
            }
        }
    }
}

void AQuest_ObjectiveManager::CreateObjective(const FString& ObjectiveID, const FString& Name, const FString& Description, 
                                             EQuest_ObjectiveType Type, const FVector& Location, int32 RequiredCount)
{
    FQuest_ObjectiveData NewObjective;
    NewObjective.ObjectiveID = ObjectiveID;
    NewObjective.ObjectiveName = Name;
    NewObjective.Description = Description;
    NewObjective.ObjectiveType = Type;
    NewObjective.Status = EQuest_ObjectiveStatus::Inactive;
    NewObjective.RequiredCount = RequiredCount;
    NewObjective.CurrentCount = 0;
    NewObjective.TargetLocation = Location;
    NewObjective.CompletionRadius = 500.0f;
    
    ActiveObjectives.Add(NewObjective);
    
    if (bDebugObjectives)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_ObjectiveManager: Created objective %s at location %s"), *Name, *Location.ToString());
    }
}

void AQuest_ObjectiveManager::ActivateObjective(const FString& ObjectiveID)
{
    FQuest_ObjectiveData* Objective = FindObjectiveByID(ObjectiveID);
    if (Objective)
    {
        Objective->Status = EQuest_ObjectiveStatus::Active;
        
        if (bDebugObjectives)
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest_ObjectiveManager: Activated objective %s"), *Objective->ObjectiveName);
        }
    }
}

void AQuest_ObjectiveManager::CompleteObjective(const FString& ObjectiveID)
{
    FQuest_ObjectiveData* Objective = FindObjectiveByID(ObjectiveID);
    if (Objective)
    {
        Objective->Status = EQuest_ObjectiveStatus::Completed;
        CompletedObjectives.Add(*Objective);
        ActiveObjectives.RemoveAll([ObjectiveID](const FQuest_ObjectiveData& Obj) { return Obj.ObjectiveID == ObjectiveID; });
        
        if (bDebugObjectives)
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest_ObjectiveManager: Completed objective %s"), *Objective->ObjectiveName);
        }
    }
}

void AQuest_ObjectiveManager::FailObjective(const FString& ObjectiveID)
{
    FQuest_ObjectiveData* Objective = FindObjectiveByID(ObjectiveID);
    if (Objective)
    {
        Objective->Status = EQuest_ObjectiveStatus::Failed;
        
        if (bDebugObjectives)
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest_ObjectiveManager: Failed objective %s"), *Objective->ObjectiveName);
        }
    }
}

void AQuest_ObjectiveManager::UpdateObjectiveProgress(const FString& ObjectiveID, int32 ProgressAmount)
{
    FQuest_ObjectiveData* Objective = FindObjectiveByID(ObjectiveID);
    if (Objective && Objective->Status == EQuest_ObjectiveStatus::Active)
    {
        Objective->CurrentCount = FMath::Clamp(Objective->CurrentCount + ProgressAmount, 0, Objective->RequiredCount);
        
        if (Objective->CurrentCount >= Objective->RequiredCount)
        {
            CompleteObjective(ObjectiveID);
        }
        
        if (bDebugObjectives)
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest_ObjectiveManager: Updated objective %s progress to %d/%d"), 
                   *Objective->ObjectiveName, Objective->CurrentCount, Objective->RequiredCount);
        }
    }
}

bool AQuest_ObjectiveManager::IsObjectiveActive(const FString& ObjectiveID) const
{
    for (const FQuest_ObjectiveData& Objective : ActiveObjectives)
    {
        if (Objective.ObjectiveID == ObjectiveID && Objective.Status == EQuest_ObjectiveStatus::Active)
        {
            return true;
        }
    }
    return false;
}

bool AQuest_ObjectiveManager::IsObjectiveCompleted(const FString& ObjectiveID) const
{
    for (const FQuest_ObjectiveData& Objective : CompletedObjectives)
    {
        if (Objective.ObjectiveID == ObjectiveID && Objective.Status == EQuest_ObjectiveStatus::Completed)
        {
            return true;
        }
    }
    return false;
}

FQuest_ObjectiveData AQuest_ObjectiveManager::GetObjectiveData(const FString& ObjectiveID) const
{
    for (const FQuest_ObjectiveData& Objective : ActiveObjectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return Objective;
        }
    }
    
    for (const FQuest_ObjectiveData& Objective : CompletedObjectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return Objective;
        }
    }
    
    return FQuest_ObjectiveData();
}

TArray<FQuest_ObjectiveData> AQuest_ObjectiveManager::GetActiveObjectives() const
{
    TArray<FQuest_ObjectiveData> Result;
    for (const FQuest_ObjectiveData& Objective : ActiveObjectives)
    {
        if (Objective.Status == EQuest_ObjectiveStatus::Active)
        {
            Result.Add(Objective);
        }
    }
    return Result;
}

void AQuest_ObjectiveManager::CheckLocationObjectives(const FVector& PlayerLocation)
{
    for (FQuest_ObjectiveData& Objective : ActiveObjectives)
    {
        if (Objective.Status == EQuest_ObjectiveStatus::Active && Objective.ObjectiveType == EQuest_ObjectiveType::Explore)
        {
            float Distance = FVector::Dist(PlayerLocation, Objective.TargetLocation);
            if (Distance <= Objective.CompletionRadius)
            {
                UpdateObjectiveProgress(Objective.ObjectiveID, 1);
            }
        }
    }
}

void AQuest_ObjectiveManager::CheckHuntObjectives(const FString& KilledActorLabel)
{
    for (FQuest_ObjectiveData& Objective : ActiveObjectives)
    {
        if (Objective.Status == EQuest_ObjectiveStatus::Active && Objective.ObjectiveType == EQuest_ObjectiveType::Hunt)
        {
            if (KilledActorLabel.Contains(Objective.TargetActorLabel) || Objective.TargetActorLabel.Contains(KilledActorLabel))
            {
                UpdateObjectiveProgress(Objective.ObjectiveID, 1);
            }
        }
    }
}

void AQuest_ObjectiveManager::CheckGatherObjectives(const FString& GatheredItem, int32 Amount)
{
    for (FQuest_ObjectiveData& Objective : ActiveObjectives)
    {
        if (Objective.Status == EQuest_ObjectiveStatus::Active && Objective.ObjectiveType == EQuest_ObjectiveType::Gather)
        {
            if (Objective.RequiredItems.Contains(GatheredItem))
            {
                UpdateObjectiveProgress(Objective.ObjectiveID, Amount);
            }
        }
    }
}

void AQuest_ObjectiveManager::SetupDefaultObjectives()
{
    // Hunt Quest: Kill T-Rex
    CreateObjective("HUNT_TREX", "Hunt the Great Predator", "Track and kill the T-Rex that threatens our tribe", 
                   EQuest_ObjectiveType::Hunt, FVector(2000, 0, 100), 1);
    
    FQuest_ObjectiveData* HuntObjective = FindObjectiveByID("HUNT_TREX");
    if (HuntObjective)
    {
        HuntObjective->TargetActorLabel = "trex";
        HuntObjective->CompletionRadius = 1000.0f;
    }
    
    // Gather Quest: Collect Berries
    CreateObjective("GATHER_BERRIES", "Harvest Food Supplies", "Gather berries to feed the tribe during the harsh season", 
                   EQuest_ObjectiveType::Gather, FVector(-1500, 1500, 100), 10);
    
    FQuest_ObjectiveData* GatherObjective = FindObjectiveByID("GATHER_BERRIES");
    if (GatherObjective)
    {
        GatherObjective->RequiredItems.Add("Berries");
        GatherObjective->RequiredItems.Add("Fruit");
        GatherObjective->CompletionRadius = 800.0f;
    }
    
    // Explore Quest: Discover Cave
    CreateObjective("EXPLORE_CAVE", "Discover the Hidden Cave", "Find the ancient cave system in the eastern mountains", 
                   EQuest_ObjectiveType::Explore, FVector(3000, -2000, 200), 1);
    
    FQuest_ObjectiveData* ExploreObjective = FindObjectiveByID("EXPLORE_CAVE");
    if (ExploreObjective)
    {
        ExploreObjective->CompletionRadius = 600.0f;
    }
    
    // Activate all default objectives
    ActivateObjective("HUNT_TREX");
    ActivateObjective("GATHER_BERRIES");
    ActivateObjective("EXPLORE_CAVE");
}

void AQuest_ObjectiveManager::CheckObjectiveCompletion()
{
    // Check for player location and update exploration objectives
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        CheckLocationObjectives(PlayerPawn->GetActorLocation());
    }
}

void AQuest_ObjectiveManager::UpdateObjectiveTriggers()
{
    // Update trigger boxes for objectives if needed
    for (ATriggerBox* Trigger : ObjectiveTriggers)
    {
        if (IsValid(Trigger))
        {
            // Trigger logic can be expanded here
        }
    }
}

FQuest_ObjectiveData* AQuest_ObjectiveManager::FindObjectiveByID(const FString& ObjectiveID)
{
    for (FQuest_ObjectiveData& Objective : ActiveObjectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return &Objective;
        }
    }
    return nullptr;
}