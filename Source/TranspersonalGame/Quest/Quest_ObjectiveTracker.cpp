#include "Quest_ObjectiveTracker.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

UQuest_ObjectiveTracker::UQuest_ObjectiveTracker()
{
    PrimaryComponentTick.bCanEverTick = true;
    LocationCheckInterval = 2.0f;
    bAutoCompleteLocationObjectives = true;
    LastLocationCheckTime = 0.0f;
}

void UQuest_ObjectiveTracker::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Quest_ObjectiveTracker: Component initialized"));
}

void UQuest_ObjectiveTracker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Check location-based objectives periodically
    if (bAutoCompleteLocationObjectives)
    {
        LastLocationCheckTime += DeltaTime;
        if (LastLocationCheckTime >= LocationCheckInterval)
        {
            LastLocationCheckTime = 0.0f;
            
            // Get player location
            if (UWorld* World = GetWorld())
            {
                if (APlayerController* PC = World->GetFirstPlayerController())
                {
                    if (APawn* PlayerPawn = PC->GetPawn())
                    {
                        CheckLocationObjectives(PlayerPawn->GetActorLocation());
                    }
                }
            }
        }
    }
}

void UQuest_ObjectiveTracker::AddObjective(const FQuest_ObjectiveData& NewObjective)
{
    // Check if objective already exists
    for (const FQuest_ObjectiveData& Existing : ActiveObjectives)
    {
        if (Existing.ObjectiveID == NewObjective.ObjectiveID)
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest_ObjectiveTracker: Objective %s already exists"), *NewObjective.ObjectiveID);
            return;
        }
    }

    ActiveObjectives.Add(NewObjective);
    UE_LOG(LogTemp, Log, TEXT("Quest_ObjectiveTracker: Added objective %s"), *NewObjective.ObjectiveID);
    
    OnObjectiveUpdated.Broadcast(NewObjective.ObjectiveID);
}

void UQuest_ObjectiveTracker::RemoveObjective(const FString& ObjectiveID)
{
    for (int32 i = ActiveObjectives.Num() - 1; i >= 0; i--)
    {
        if (ActiveObjectives[i].ObjectiveID == ObjectiveID)
        {
            ActiveObjectives.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Quest_ObjectiveTracker: Removed objective %s"), *ObjectiveID);
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_ObjectiveTracker: Objective %s not found for removal"), *ObjectiveID);
}

void UQuest_ObjectiveTracker::UpdateObjectiveProgress(const FString& ObjectiveID, int32 ProgressAmount)
{
    for (FQuest_ObjectiveData& Objective : ActiveObjectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            Objective.CurrentCount += ProgressAmount;
            Objective.CurrentCount = FMath::Clamp(Objective.CurrentCount, 0, Objective.TargetCount);
            
            UE_LOG(LogTemp, Log, TEXT("Quest_ObjectiveTracker: Updated %s progress to %d/%d"), 
                   *ObjectiveID, Objective.CurrentCount, Objective.TargetCount);
            
            CheckObjectiveCompletion(Objective);
            OnObjectiveUpdated.Broadcast(ObjectiveID);
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_ObjectiveTracker: Objective %s not found for progress update"), *ObjectiveID);
}

bool UQuest_ObjectiveTracker::IsObjectiveCompleted(const FString& ObjectiveID) const
{
    for (const FQuest_ObjectiveData& Objective : ActiveObjectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return Objective.bIsCompleted;
        }
    }
    
    // Check completed objectives
    for (const FQuest_ObjectiveData& Objective : CompletedObjectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return true;
        }
    }
    
    return false;
}

FQuest_ObjectiveData UQuest_ObjectiveTracker::GetObjectiveData(const FString& ObjectiveID) const
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

TArray<FQuest_ObjectiveData> UQuest_ObjectiveTracker::GetAllObjectives() const
{
    TArray<FQuest_ObjectiveData> AllObjectives = ActiveObjectives;
    AllObjectives.Append(CompletedObjectives);
    return AllObjectives;
}

TArray<FQuest_ObjectiveData> UQuest_ObjectiveTracker::GetActiveObjectives() const
{
    TArray<FQuest_ObjectiveData> Active;
    for (const FQuest_ObjectiveData& Objective : ActiveObjectives)
    {
        if (!Objective.bIsCompleted)
        {
            Active.Add(Objective);
        }
    }
    return Active;
}

void UQuest_ObjectiveTracker::CompleteObjective(const FString& ObjectiveID)
{
    for (int32 i = 0; i < ActiveObjectives.Num(); i++)
    {
        if (ActiveObjectives[i].ObjectiveID == ObjectiveID)
        {
            ActiveObjectives[i].bIsCompleted = true;
            ActiveObjectives[i].CurrentCount = ActiveObjectives[i].TargetCount;
            
            CompletedObjectives.Add(ActiveObjectives[i]);
            BroadcastObjectiveEvents(ActiveObjectives[i]);
            
            ActiveObjectives.RemoveAt(i);
            
            UE_LOG(LogTemp, Log, TEXT("Quest_ObjectiveTracker: Completed objective %s"), *ObjectiveID);
            return;
        }
    }
}

void UQuest_ObjectiveTracker::CheckLocationObjectives(const FVector& PlayerLocation)
{
    for (FQuest_ObjectiveData& Objective : ActiveObjectives)
    {
        if (Objective.ObjectiveType == EQuest_ObjectiveType::Reach && !Objective.bIsCompleted)
        {
            float Distance = FVector::Dist(PlayerLocation, Objective.TargetLocation);
            if (Distance <= Objective.CompletionRadius)
            {
                CompleteObjective(Objective.ObjectiveID);
            }
        }
    }
}

void UQuest_ObjectiveTracker::OnDinosaurKilled(const FString& DinosaurType)
{
    for (FQuest_ObjectiveData& Objective : ActiveObjectives)
    {
        if (Objective.ObjectiveType == EQuest_ObjectiveType::Hunt && !Objective.bIsCompleted)
        {
            // Check if this dinosaur type matches the objective
            if (Objective.ObjectiveDescription.Contains(DinosaurType))
            {
                UpdateObjectiveProgress(Objective.ObjectiveID, 1);
            }
        }
    }
}

void UQuest_ObjectiveTracker::OnResourceGathered(const FString& ResourceType, int32 Amount)
{
    for (FQuest_ObjectiveData& Objective : ActiveObjectives)
    {
        if (Objective.ObjectiveType == EQuest_ObjectiveType::Gather && !Objective.bIsCompleted)
        {
            // Check if this resource type matches the objective
            if (Objective.ObjectiveDescription.Contains(ResourceType))
            {
                UpdateObjectiveProgress(Objective.ObjectiveID, Amount);
            }
        }
    }
}

void UQuest_ObjectiveTracker::OnStructureBuilt(const FString& StructureType)
{
    for (FQuest_ObjectiveData& Objective : ActiveObjectives)
    {
        if (Objective.ObjectiveType == EQuest_ObjectiveType::Build && !Objective.bIsCompleted)
        {
            // Check if this structure type matches the objective
            if (Objective.ObjectiveDescription.Contains(StructureType))
            {
                UpdateObjectiveProgress(Objective.ObjectiveID, 1);
            }
        }
    }
}

void UQuest_ObjectiveTracker::CheckObjectiveCompletion(FQuest_ObjectiveData& Objective)
{
    if (!Objective.bIsCompleted && Objective.CurrentCount >= Objective.TargetCount)
    {
        CompleteObjective(Objective.ObjectiveID);
    }
}

void UQuest_ObjectiveTracker::BroadcastObjectiveEvents(const FQuest_ObjectiveData& Objective)
{
    OnObjectiveCompleted.Broadcast(Objective.ObjectiveID);
    OnObjectiveUpdated.Broadcast(Objective.ObjectiveID);
}