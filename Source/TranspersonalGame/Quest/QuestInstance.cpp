#include "QuestInstance.h"
#include "QuestObjective.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UQuestInstance::UQuestInstance()
{
    QuestProgress = 0.0f;
    bIsTimedQuest = false;
    TimeLimit = 0.0f;
    ElapsedTime = 0.0f;
}

void UQuestInstance::Initialize(const FQuestData& InQuestData)
{
    QuestData = InQuestData;
    StartTime = FDateTime::Now();
    CreateObjectives();
    
    UE_LOG(LogTemp, Log, TEXT("Initialized quest instance: %s"), *QuestData.QuestTitle.ToString());
}

void UQuestInstance::UpdateQuest(float DeltaTime)
{
    // Update elapsed time
    ElapsedTime += DeltaTime;
    
    // Check time limit if this is a timed quest
    if (bIsTimedQuest)
    {
        CheckTimeLimit(DeltaTime);
    }
    
    // Update all objectives
    for (UQuestObjective* Objective : Objectives)
    {
        if (Objective)
        {
            Objective->UpdateObjective(DeltaTime);
        }
    }
    
    // Update overall quest progress
    UpdateQuestProgress();
}

bool UQuestInstance::AreAllObjectivesCompleted() const
{
    for (const UQuestObjective* Objective : Objectives)
    {
        if (Objective && !Objective->IsCompleted())
        {
            return false;
        }
    }
    return Objectives.Num() > 0; // Must have at least one objective
}

bool UQuestInstance::HasObjective(const FString& ObjectiveID) const
{
    for (const UQuestObjective* Objective : Objectives)
    {
        if (Objective && Objective->GetObjectiveID() == ObjectiveID)
        {
            return true;
        }
    }
    return false;
}

bool UQuestInstance::CompleteObjective(const FString& ObjectiveID)
{
    for (UQuestObjective* Objective : Objectives)
    {
        if (Objective && Objective->GetObjectiveID() == ObjectiveID)
        {
            Objective->CompleteObjective();
            UpdateQuestProgress();
            
            UE_LOG(LogTemp, Log, TEXT("Completed objective %s for quest %s"), *ObjectiveID, *QuestData.QuestID);
            return true;
        }
    }
    return false;
}

void UQuestInstance::GiveRewards()
{
    // Give experience reward
    if (QuestData.ExperienceReward > 0)
    {
        // TODO: Implement experience system integration
        UE_LOG(LogTemp, Log, TEXT("Giving %d experience for quest %s"), QuestData.ExperienceReward, *QuestData.QuestID);
    }
    
    // Give item rewards (if any)
    // TODO: Implement item reward system
    
    // Trigger quest completion effects
    // TODO: Implement visual/audio feedback for quest completion
    
    UE_LOG(LogTemp, Log, TEXT("Gave rewards for quest: %s"), *QuestData.QuestTitle.ToString());
}

float UQuestInstance::GetCompletionPercentage() const
{
    if (Objectives.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalProgress = 0.0f;
    for (const UQuestObjective* Objective : Objectives)
    {
        if (Objective)
        {
            TotalProgress += Objective->GetProgress();
        }
    }
    
    return TotalProgress / Objectives.Num();
}

void UQuestInstance::SetQuestProgress(float Progress)
{
    QuestProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
}

void UQuestInstance::StartQuestTimer(float InTimeLimit)
{
    bIsTimedQuest = true;
    TimeLimit = InTimeLimit;
    ElapsedTime = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Started timer for quest %s: %f seconds"), *QuestData.QuestID, TimeLimit);
}

float UQuestInstance::GetRemainingTime() const
{
    if (!bIsTimedQuest)
    {
        return -1.0f; // No time limit
    }
    
    return FMath::Max(0.0f, TimeLimit - ElapsedTime);
}

void UQuestInstance::CreateObjectives()
{
    // Clear existing objectives
    Objectives.Empty();
    
    // Create objectives based on quest data
    for (const FString& ObjectiveID : QuestData.ObjectiveIDs)
    {
        UQuestObjective* NewObjective = NewObject<UQuestObjective>(this);
        if (NewObjective)
        {
            NewObjective->Initialize(ObjectiveID, QuestData.QuestType);
            Objectives.Add(NewObjective);
            
            UE_LOG(LogTemp, Log, TEXT("Created objective %s for quest %s"), *ObjectiveID, *QuestData.QuestID);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Created %d objectives for quest %s"), Objectives.Num(), *QuestData.QuestID);
}

void UQuestInstance::UpdateQuestProgress()
{
    float NewProgress = GetCompletionPercentage();
    SetQuestProgress(NewProgress);
}

void UQuestInstance::CheckTimeLimit(float DeltaTime)
{
    if (ElapsedTime >= TimeLimit)
    {
        // Quest has timed out - this should trigger quest failure
        UE_LOG(LogTemp, Warning, TEXT("Quest %s has timed out!"), *QuestData.QuestID);
        
        // TODO: Trigger quest failure through quest manager
        // This would typically be handled by the quest manager checking this state
    }
}