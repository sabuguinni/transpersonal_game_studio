#include "Quest_ObjectiveSystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "../Character/TranspersonalCharacter.h"

UQuest_ObjectiveSystem::UQuest_ObjectiveSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = LocationCheckInterval;
    
    PlayerCharacter = nullptr;
    LocationCheckTimer = 0.0f;
}

void UQuest_ObjectiveSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get reference to player character
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerCharacter = Cast<ATranspersonalCharacter>(PC->GetPawn());
        }
    }
}

void UQuest_ObjectiveSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LocationCheckTimer += DeltaTime;
    if (LocationCheckTimer >= LocationCheckInterval)
    {
        LocationCheckTimer = 0.0f;
        
        if (PlayerCharacter)
        {
            CheckLocationObjectives(PlayerCharacter->GetActorLocation());
        }
    }
}

void UQuest_ObjectiveSystem::CreateObjective(const FQuest_ObjectiveData& ObjectiveData)
{
    if (ObjectiveData.ObjectiveID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot create objective with empty ID"));
        return;
    }
    
    FQuest_ObjectiveData NewObjective = ObjectiveData;
    NewObjective.Status = EQuest_ObjectiveStatus::Active;
    NewObjective.CurrentCount = 0;
    
    ActiveObjectives.Add(ObjectiveData.ObjectiveID, NewObjective);
    
    UE_LOG(LogTemp, Log, TEXT("Created objective: %s - %s"), *ObjectiveData.ObjectiveID, *ObjectiveData.Title);
}

void UQuest_ObjectiveSystem::UpdateObjectiveProgress(const FString& ObjectiveID, int32 ProgressAmount)
{
    if (FQuest_ObjectiveData* Objective = ActiveObjectives.Find(ObjectiveID))
    {
        if (Objective->Status == EQuest_ObjectiveStatus::Active)
        {
            Objective->CurrentCount = FMath::Min(Objective->CurrentCount + ProgressAmount, Objective->TargetCount);
            
            // Check if objective is now complete
            if (Objective->CurrentCount >= Objective->TargetCount)
            {
                CompleteObjective(ObjectiveID);
            }
            
            UE_LOG(LogTemp, Log, TEXT("Updated objective %s progress: %d/%d"), 
                   *ObjectiveID, Objective->CurrentCount, Objective->TargetCount);
        }
    }
}

void UQuest_ObjectiveSystem::CompleteObjective(const FString& ObjectiveID)
{
    if (FQuest_ObjectiveData* Objective = ActiveObjectives.Find(ObjectiveID))
    {
        Objective->Status = EQuest_ObjectiveStatus::Completed;
        CompletedObjectives.Add(ObjectiveID);
        
        NotifyObjectiveComplete(ObjectiveID);
        
        UE_LOG(LogTemp, Log, TEXT("Completed objective: %s"), *ObjectiveID);
    }
}

void UQuest_ObjectiveSystem::FailObjective(const FString& ObjectiveID)
{
    if (FQuest_ObjectiveData* Objective = ActiveObjectives.Find(ObjectiveID))
    {
        Objective->Status = EQuest_ObjectiveStatus::Failed;
        FailedObjectives.Add(ObjectiveID);
        
        NotifyObjectiveFailed(ObjectiveID);
        
        UE_LOG(LogTemp, Log, TEXT("Failed objective: %s"), *ObjectiveID);
    }
}

bool UQuest_ObjectiveSystem::IsObjectiveComplete(const FString& ObjectiveID) const
{
    if (const FQuest_ObjectiveData* Objective = ActiveObjectives.Find(ObjectiveID))
    {
        return Objective->Status == EQuest_ObjectiveStatus::Completed;
    }
    return false;
}

FQuest_ObjectiveData UQuest_ObjectiveSystem::GetObjectiveData(const FString& ObjectiveID) const
{
    if (const FQuest_ObjectiveData* Objective = ActiveObjectives.Find(ObjectiveID))
    {
        return *Objective;
    }
    return FQuest_ObjectiveData();
}

TArray<FQuest_ObjectiveData> UQuest_ObjectiveSystem::GetActiveObjectives() const
{
    TArray<FQuest_ObjectiveData> ActiveList;
    
    for (const auto& ObjectivePair : ActiveObjectives)
    {
        if (ObjectivePair.Value.Status == EQuest_ObjectiveStatus::Active)
        {
            ActiveList.Add(ObjectivePair.Value);
        }
    }
    
    return ActiveList;
}

void UQuest_ObjectiveSystem::OnDinosaurKilled(const FString& DinosaurType, const FVector& Location)
{
    for (auto& ObjectivePair : ActiveObjectives)
    {
        FQuest_ObjectiveData& Objective = ObjectivePair.Value;
        
        if (Objective.Status == EQuest_ObjectiveStatus::Active && 
            Objective.Type == EQuest_ObjectiveType::Hunt_Dinosaur)
        {
            // Check if this dinosaur type matches the objective target
            if (Objective.TargetTag.IsEmpty() || Objective.TargetTag == DinosaurType)
            {
                UpdateObjectiveProgress(ObjectivePair.Key, 1);
            }
        }
    }
}

void UQuest_ObjectiveSystem::OnResourceGathered(const FString& ResourceType, int32 Amount)
{
    for (auto& ObjectivePair : ActiveObjectives)
    {
        FQuest_ObjectiveData& Objective = ObjectivePair.Value;
        
        if (Objective.Status == EQuest_ObjectiveStatus::Active && 
            Objective.Type == EQuest_ObjectiveType::Gather_Resource)
        {
            // Check if this resource type matches the objective target
            if (Objective.TargetTag.IsEmpty() || Objective.TargetTag == ResourceType)
            {
                UpdateObjectiveProgress(ObjectivePair.Key, Amount);
            }
        }
    }
}

void UQuest_ObjectiveSystem::OnLocationReached(const FVector& Location, float Radius)
{
    for (auto& ObjectivePair : ActiveObjectives)
    {
        FQuest_ObjectiveData& Objective = ObjectivePair.Value;
        
        if (Objective.Status == EQuest_ObjectiveStatus::Active && 
            (Objective.Type == EQuest_ObjectiveType::Explore_Location || 
             Objective.Type == EQuest_ObjectiveType::Reach_Location))
        {
            float Distance = FVector::Dist(Location, Objective.TargetLocation);
            if (Distance <= FMath::Max(Radius, Objective.TargetRadius))
            {
                CompleteObjective(ObjectivePair.Key);
            }
        }
    }
}

void UQuest_ObjectiveSystem::OnItemCrafted(const FString& ItemType)
{
    for (auto& ObjectivePair : ActiveObjectives)
    {
        FQuest_ObjectiveData& Objective = ObjectivePair.Value;
        
        if (Objective.Status == EQuest_ObjectiveStatus::Active && 
            Objective.Type == EQuest_ObjectiveType::Craft_Item)
        {
            // Check if this item type matches the objective target
            if (Objective.TargetTag.IsEmpty() || Objective.TargetTag == ItemType)
            {
                UpdateObjectiveProgress(ObjectivePair.Key, 1);
            }
        }
    }
}

void UQuest_ObjectiveSystem::OnNPCInteraction(const FString& NPCID)
{
    for (auto& ObjectivePair : ActiveObjectives)
    {
        FQuest_ObjectiveData& Objective = ObjectivePair.Value;
        
        if (Objective.Status == EQuest_ObjectiveStatus::Active && 
            Objective.Type == EQuest_ObjectiveType::Interact_NPC)
        {
            // Check if this NPC matches the objective target
            if (Objective.TargetTag.IsEmpty() || Objective.TargetTag == NPCID)
            {
                CompleteObjective(ObjectivePair.Key);
            }
        }
    }
}

float UQuest_ObjectiveSystem::GetObjectiveProgress(const FString& ObjectiveID) const
{
    if (const FQuest_ObjectiveData* Objective = ActiveObjectives.Find(ObjectiveID))
    {
        if (Objective->TargetCount > 0)
        {
            return static_cast<float>(Objective->CurrentCount) / static_cast<float>(Objective->TargetCount);
        }
    }
    return 0.0f;
}

FString UQuest_ObjectiveSystem::GetObjectiveProgressText(const FString& ObjectiveID) const
{
    if (const FQuest_ObjectiveData* Objective = ActiveObjectives.Find(ObjectiveID))
    {
        return FString::Printf(TEXT("%d/%d"), Objective->CurrentCount, Objective->TargetCount);
    }
    return TEXT("0/0");
}

void UQuest_ObjectiveSystem::SetObjectiveTrackingEnabled(const FString& ObjectiveID, bool bEnabled)
{
    if (FQuest_ObjectiveData* Objective = ActiveObjectives.Find(ObjectiveID))
    {
        Objective->bTrackProgress = bEnabled;
    }
}

void UQuest_ObjectiveSystem::CheckLocationObjectives(const FVector& PlayerLocation)
{
    for (auto& ObjectivePair : ActiveObjectives)
    {
        FQuest_ObjectiveData& Objective = ObjectivePair.Value;
        
        if (Objective.Status == EQuest_ObjectiveStatus::Active && 
            (Objective.Type == EQuest_ObjectiveType::Explore_Location || 
             Objective.Type == EQuest_ObjectiveType::Reach_Location))
        {
            if (IsPlayerInRange(Objective.TargetLocation, Objective.TargetRadius))
            {
                CompleteObjective(ObjectivePair.Key);
            }
        }
    }
}

void UQuest_ObjectiveSystem::NotifyObjectiveComplete(const FString& ObjectiveID)
{
    // Broadcast objective completion event
    UE_LOG(LogTemp, Log, TEXT("Broadcasting objective completion: %s"), *ObjectiveID);
    
    // Here we would broadcast to UI and other systems
    // For now, just log the completion
}

void UQuest_ObjectiveSystem::NotifyObjectiveFailed(const FString& ObjectiveID)
{
    // Broadcast objective failure event
    UE_LOG(LogTemp, Warning, TEXT("Broadcasting objective failure: %s"), *ObjectiveID);
    
    // Here we would broadcast to UI and other systems
    // For now, just log the failure
}

bool UQuest_ObjectiveSystem::IsPlayerInRange(const FVector& TargetLocation, float Radius) const
{
    if (PlayerCharacter)
    {
        float Distance = FVector::Dist(PlayerCharacter->GetActorLocation(), TargetLocation);
        return Distance <= Radius;
    }
    return false;
}