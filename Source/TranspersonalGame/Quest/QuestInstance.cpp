// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "QuestInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UQuestInstance::UQuestInstance()
{
    StartTime = 0.0f;
}

void UQuestInstance::InitializeQuest(const FQuestData& InQuestData)
{
    QuestData = InQuestData;
    StartTime = UGameplayStatics::GetTimeSeconds(GetWorld());
    
    InitializeObjectiveProgress();
    
    // Set initial emotional tone
    if (EmotionalProgression.Num() == 0)
    {
        EmotionalProgression.Add(QuestData.EmotionalTone);
    }
    
    UE_LOG(LogTemp, Log, TEXT("QuestInstance: Initialized quest %s with %d objectives"), 
           *QuestData.QuestID, QuestData.Objectives.Num());
}

bool UQuestInstance::UpdateObjectiveProgress(const FString& ObjectiveID, int32 Progress)
{
    if (!ValidateObjectiveID(ObjectiveID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestInstance: Invalid objective ID %s"), *ObjectiveID);
        return false;
    }

    // Find the objective in quest data
    FQuestObjective* Objective = nullptr;
    for (FQuestObjective& Obj : QuestData.Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID)
        {
            Objective = &Obj;
            break;
        }
    }

    if (!Objective)
    {
        return false;
    }

    // Update progress
    int32 OldProgress = Objective->CurrentCount;
    Objective->CurrentCount = FMath::Min(Objective->CurrentCount + Progress, Objective->TargetCount);
    
    // Cache the progress
    ObjectiveProgressCache.Add(ObjectiveID, Objective->CurrentCount);

    bool bWasCompleted = (OldProgress >= Objective->TargetCount);
    bool bIsCompleted = (Objective->CurrentCount >= Objective->TargetCount);
    bool bJustCompleted = !bWasCompleted && bIsCompleted;

    // Trigger events
    OnObjectiveProgressUpdated.Broadcast(ObjectiveID, Objective->CurrentCount);
    
    if (bJustCompleted)
    {
        OnObjectiveCompleted.Broadcast(ObjectiveID);
        
        // Add emotional moment for objective completion
        EEmotionalTone CompletionTone = EEmotionalTone::Hope;
        
        // Determine emotional tone based on objective type
        switch (Objective->Type)
        {
            case EObjectiveType::Survive:
                CompletionTone = EEmotionalTone::Triumph;
                break;
            case EObjectiveType::Observe:
                CompletionTone = EEmotionalTone::Wonder;
                break;
            case EObjectiveType::Escape:
                CompletionTone = EEmotionalTone::Hope;
                break;
            case EObjectiveType::Discover:
                CompletionTone = EEmotionalTone::Curiosity;
                break;
            default:
                CompletionTone = EEmotionalTone::Hope;
                break;
        }
        
        AddEmotionalMoment(CompletionTone, FString::Printf(TEXT("Completed objective: %s"), *ObjectiveID));
    }

    UE_LOG(LogTemp, Log, TEXT("QuestInstance: Updated objective %s progress to %d/%d (Quest: %s)"), 
           *ObjectiveID, Objective->CurrentCount, Objective->TargetCount, *QuestData.QuestID);

    return bJustCompleted;
}

bool UQuestInstance::IsObjectiveCompleted(const FString& ObjectiveID) const
{
    for (const FQuestObjective& Objective : QuestData.Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return Objective.IsCompleted();
        }
    }
    return false;
}

bool UQuestInstance::AreAllRequiredObjectivesCompleted() const
{
    for (const FQuestObjective& Objective : QuestData.Objectives)
    {
        if (!Objective.bIsOptional && !Objective.IsCompleted())
        {
            return false;
        }
    }
    return true;
}

float UQuestInstance::GetCompletionPercentage() const
{
    if (QuestData.Objectives.Num() == 0)
    {
        return 0.0f;
    }

    int32 CompletedObjectives = 0;
    int32 TotalObjectives = 0;

    for (const FQuestObjective& Objective : QuestData.Objectives)
    {
        if (!Objective.bIsOptional)
        {
            TotalObjectives++;
            if (Objective.IsCompleted())
            {
                CompletedObjectives++;
            }
        }
    }

    return TotalObjectives > 0 ? (float)CompletedObjectives / (float)TotalObjectives : 0.0f;
}

FQuestObjective UQuestInstance::GetObjective(const FString& ObjectiveID) const
{
    for (const FQuestObjective& Objective : QuestData.Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return Objective;
        }
    }
    return FQuestObjective();
}

float UQuestInstance::GetRemainingTime() const
{
    if (!HasTimeLimit())
    {
        return -1.0f; // No time limit
    }

    float CurrentTime = UGameplayStatics::GetTimeSeconds(GetWorld());
    float ElapsedTime = CurrentTime - StartTime;
    float RemainingTime = QuestData.TimeLimit - ElapsedTime;
    
    return FMath::Max(0.0f, RemainingTime);
}

void UQuestInstance::AddEmotionalMoment(EEmotionalTone Tone, const FString& Context)
{
    EmotionalProgression.Add(Tone);
    
    // Broadcast emotional moment
    OnEmotionalMoment.Broadcast(Tone, Context);
    
    UE_LOG(LogTemp, Log, TEXT("QuestInstance: Emotional moment in quest %s - %s: %s"), 
           *QuestData.QuestID, *UEnum::GetValueAsString(Tone), *Context);
}

EEmotionalTone UQuestInstance::GetCurrentEmotionalState() const
{
    if (EmotionalProgression.Num() > 0)
    {
        return EmotionalProgression.Last();
    }
    return QuestData.EmotionalTone;
}

bool UQuestInstance::IsPlayerInQuestArea(const FVector& PlayerLocation) const
{
    if (QuestData.QuestLocation == FVector::ZeroVector)
    {
        return true; // No specific location requirement
    }

    // Check if any objective has a location requirement
    for (const FQuestObjective& Objective : QuestData.Objectives)
    {
        if (Objective.TargetLocation != FVector::ZeroVector)
        {
            float Distance = FVector::Dist(PlayerLocation, Objective.TargetLocation);
            if (Distance <= Objective.TargetRadius)
            {
                return true;
            }
        }
    }

    // Check main quest location
    float Distance = FVector::Dist(PlayerLocation, QuestData.QuestLocation);
    return Distance <= 2000.0f; // Default quest area radius
}

float UQuestInstance::GetDistanceToQuestLocation(const FVector& PlayerLocation) const
{
    if (QuestData.QuestLocation == FVector::ZeroVector)
    {
        return 0.0f;
    }

    return FVector::Dist(PlayerLocation, QuestData.QuestLocation);
}

void UQuestInstance::AddDynamicObjective(const FQuestObjective& NewObjective)
{
    // Validate that the objective ID is unique
    for (const FQuestObjective& ExistingObjective : QuestData.Objectives)
    {
        if (ExistingObjective.ObjectiveID == NewObjective.ObjectiveID)
        {
            UE_LOG(LogTemp, Warning, TEXT("QuestInstance: Objective ID %s already exists"), *NewObjective.ObjectiveID);
            return;
        }
    }

    QuestData.Objectives.Add(NewObjective);
    ObjectiveProgressCache.Add(NewObjective.ObjectiveID, NewObjective.CurrentCount);
    
    UE_LOG(LogTemp, Log, TEXT("QuestInstance: Added dynamic objective %s to quest %s"), 
           *NewObjective.ObjectiveID, *QuestData.QuestID);
}

void UQuestInstance::RemoveObjective(const FString& ObjectiveID)
{
    QuestData.Objectives.RemoveAll([ObjectiveID](const FQuestObjective& Objective)
    {
        return Objective.ObjectiveID == ObjectiveID;
    });
    
    ObjectiveProgressCache.Remove(ObjectiveID);
    
    UE_LOG(LogTemp, Log, TEXT("QuestInstance: Removed objective %s from quest %s"), 
           *ObjectiveID, *QuestData.QuestID);
}

void UQuestInstance::UpdateObjectiveDescription(const FString& ObjectiveID, const FText& NewDescription)
{
    for (FQuestObjective& Objective : QuestData.Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            Objective.Description = NewDescription;
            UE_LOG(LogTemp, Log, TEXT("QuestInstance: Updated description for objective %s"), *ObjectiveID);
            return;
        }
    }
}

void UQuestInstance::InitializeObjectiveProgress()
{
    ObjectiveProgressCache.Empty();
    
    for (const FQuestObjective& Objective : QuestData.Objectives)
    {
        ObjectiveProgressCache.Add(Objective.ObjectiveID, Objective.CurrentCount);
    }
}

bool UQuestInstance::ValidateObjectiveID(const FString& ObjectiveID) const
{
    for (const FQuestObjective& Objective : QuestData.Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return true;
        }
    }
    return false;
}