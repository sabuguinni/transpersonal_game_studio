#include "QuestObjective.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UQuestObjective::UQuestObjective()
{
    CurrentProgress = 0.0f;
    CurrentCount = 0;
    bIsCompleted = false;
    bIsTracking = false;
}

void UQuestObjective::Initialize(const FString& ObjectiveID, EQuestType QuestType)
{
    LoadObjectiveData(ObjectiveID);
    bIsTracking = true;
    
    UE_LOG(LogTemp, Log, TEXT("Initialized objective: %s - %s"), 
           *ObjectiveData.ObjectiveID, *ObjectiveData.ObjectiveDescription.ToString());
}

void UQuestObjective::UpdateObjective(float DeltaTime)
{
    if (bIsCompleted || !bIsTracking)
    {
        return;
    }

    switch (ObjectiveData.ObjectiveType)
    {
        case EObjectiveType::Reach:
        case EObjectiveType::Discover:
            UpdateLocationTracking();
            break;
            
        case EObjectiveType::Survive:
            UpdateSurvivalTracking(DeltaTime);
            break;
            
        default:
            // Other objective types are updated through external events
            break;
    }
}

bool UQuestObjective::CompleteObjective()
{
    if (bIsCompleted)
    {
        return false;
    }

    bIsCompleted = true;
    CurrentProgress = 1.0f;
    CurrentCount = ObjectiveData.RequiredCount;
    
    UE_LOG(LogTemp, Log, TEXT("Completed objective: %s"), *ObjectiveData.ObjectiveID);
    
    OnObjectiveCompleted.Broadcast(ObjectiveData.ObjectiveID);
    NotifyProgressChanged();
    
    return true;
}

void UQuestObjective::SetProgress(float NewProgress)
{
    float ClampedProgress = FMath::Clamp(NewProgress, 0.0f, 1.0f);
    
    if (FMath::IsNearlyEqual(CurrentProgress, ClampedProgress))
    {
        return;
    }
    
    CurrentProgress = ClampedProgress;
    CurrentCount = FMath::RoundToInt(CurrentProgress * ObjectiveData.RequiredCount);
    
    // Check if objective is completed
    if (CurrentProgress >= 1.0f && !bIsCompleted)
    {
        CompleteObjective();
    }
    else
    {
        NotifyProgressChanged();
    }
}

void UQuestObjective::IncrementProgress(float Amount)
{
    if (ObjectiveData.RequiredCount <= 0)
    {
        return;
    }
    
    float ProgressIncrement = Amount / ObjectiveData.RequiredCount;
    SetProgress(CurrentProgress + ProgressIncrement);
}

float UQuestObjective::GetProgressPercentage() const
{
    return CurrentProgress * 100.0f;
}

bool UQuestObjective::CheckLocationObjective(const FVector& PlayerLocation)
{
    if (ObjectiveData.ObjectiveType != EObjectiveType::Reach && 
        ObjectiveData.ObjectiveType != EObjectiveType::Discover)
    {
        return false;
    }
    
    float Distance = FVector::Dist(PlayerLocation, ObjectiveData.TargetLocation);
    bool bInRange = Distance <= ObjectiveData.TargetRadius;
    
    if (bInRange && !bIsCompleted)
    {
        CompleteObjective();
        return true;
    }
    
    return bInRange;
}

void UQuestObjective::LoadObjectiveData(const FString& ObjectiveID)
{
    // Initialize with default data - in a real implementation, this would load from a data table
    ObjectiveData.ObjectiveID = ObjectiveID;
    
    // Set default values based on objective ID patterns
    if (ObjectiveID.Contains(TEXT("Kill")))
    {
        ObjectiveData.ObjectiveType = EObjectiveType::Kill;
        ObjectiveData.ObjectiveDescription = FText::FromString(TEXT("Eliminate the target"));
        ObjectiveData.RequiredCount = 1;
    }
    else if (ObjectiveID.Contains(TEXT("Collect")))
    {
        ObjectiveData.ObjectiveType = EObjectiveType::Collect;
        ObjectiveData.ObjectiveDescription = FText::FromString(TEXT("Gather the required items"));
        ObjectiveData.RequiredCount = 5;
    }
    else if (ObjectiveID.Contains(TEXT("Reach")))
    {
        ObjectiveData.ObjectiveType = EObjectiveType::Reach;
        ObjectiveData.ObjectiveDescription = FText::FromString(TEXT("Travel to the designated location"));
        ObjectiveData.RequiredCount = 1;
        ObjectiveData.TargetRadius = 500.0f;
    }
    else if (ObjectiveID.Contains(TEXT("Survive")))
    {
        ObjectiveData.ObjectiveType = EObjectiveType::Survive;
        ObjectiveData.ObjectiveDescription = FText::FromString(TEXT("Survive for the required duration"));
        ObjectiveData.RequiredCount = 300; // 5 minutes in seconds
    }
    else if (ObjectiveID.Contains(TEXT("Tame")))
    {
        ObjectiveData.ObjectiveType = EObjectiveType::Tame;
        ObjectiveData.ObjectiveDescription = FText::FromString(TEXT("Domesticate the target creature"));
        ObjectiveData.RequiredCount = 1;
    }
    else if (ObjectiveID.Contains(TEXT("Craft")))
    {
        ObjectiveData.ObjectiveType = EObjectiveType::Craft;
        ObjectiveData.ObjectiveDescription = FText::FromString(TEXT("Create the required items"));
        ObjectiveData.RequiredCount = 3;
    }
    else if (ObjectiveID.Contains(TEXT("Build")))
    {
        ObjectiveData.ObjectiveType = EObjectiveType::Build;
        ObjectiveData.ObjectiveDescription = FText::FromString(TEXT("Construct the required structure"));
        ObjectiveData.RequiredCount = 1;
    }
    else if (ObjectiveID.Contains(TEXT("Gather")))
    {
        ObjectiveData.ObjectiveType = EObjectiveType::Gather;
        ObjectiveData.ObjectiveDescription = FText::FromString(TEXT("Collect natural resources"));
        ObjectiveData.RequiredCount = 10;
    }
    else
    {
        // Default objective
        ObjectiveData.ObjectiveType = EObjectiveType::Interact;
        ObjectiveData.ObjectiveDescription = FText::FromString(TEXT("Complete the objective"));
        ObjectiveData.RequiredCount = 1;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Loaded objective data for %s: %s (Type: %d, Count: %d)"), 
           *ObjectiveID, 
           *ObjectiveData.ObjectiveDescription.ToString(),
           (int32)ObjectiveData.ObjectiveType,
           ObjectiveData.RequiredCount);
}

void UQuestObjective::UpdateLocationTracking()
{
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
    CheckLocationObjective(PlayerLocation);
}

void UQuestObjective::UpdateSurvivalTracking(float DeltaTime)
{
    if (ObjectiveData.ObjectiveType != EObjectiveType::Survive)
    {
        return;
    }
    
    // Increment survival time
    IncrementProgress(DeltaTime);
}

void UQuestObjective::NotifyProgressChanged()
{
    OnObjectiveProgressChanged.Broadcast(ObjectiveData.ObjectiveID, CurrentProgress);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Objective %s progress: %d/%d (%.1f%%)"), 
           *ObjectiveData.ObjectiveID, 
           CurrentCount, 
           ObjectiveData.RequiredCount, 
           GetProgressPercentage());
}