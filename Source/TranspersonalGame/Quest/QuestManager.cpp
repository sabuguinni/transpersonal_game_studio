// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "QuestManager.h"
#include "QuestInstance.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    DynamicQuestCounter = 0;
    
    // Load quest data table
    QuestDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/QuestDataTable"));
    if (!QuestDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Failed to load QuestDataTable"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Initialized successfully"));
}

void UQuestManager::Deinitialize()
{
    // Clean up active quest instances
    for (auto& QuestPair : ActiveQuestInstances)
    {
        if (QuestPair.Value)
        {
            QuestPair.Value->ConditionalBeginDestroy();
        }
    }
    ActiveQuestInstances.Empty();
    
    Super::Deinitialize();
}

bool UQuestManager::StartQuest(const FString& QuestID)
{
    if (!CanStartQuest(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Cannot start quest %s"), *QuestID);
        return false;
    }

    // Get quest data
    FQuestData* QuestData = nullptr;
    if (QuestDataTable)
    {
        QuestData = QuestDataTable->FindRow<FQuestData>(FName(*QuestID), TEXT("QuestManager::StartQuest"));
    }

    if (!QuestData)
    {
        UE_LOG(LogTemp, Error, TEXT("QuestManager: Quest data not found for %s"), *QuestID);
        return false;
    }

    // Create quest instance
    UQuestInstance* QuestInstance = NewObject<UQuestInstance>(this);
    QuestInstance->InitializeQuest(*QuestData);
    
    ActiveQuestInstances.Add(QuestID, QuestInstance);
    QuestStatusMap.Add(QuestID, EQuestStatus::Active);

    // Trigger emotional moment for quest start
    TriggerEmotionalMoment(QuestID, QuestData->EmotionalTone, TEXT("Quest Started"));

    // Broadcast quest status change
    OnQuestStatusChanged.Broadcast(QuestID, EQuestStatus::Active);

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Started quest %s with emotional tone %s"), 
           *QuestID, *UEnum::GetValueAsString(QuestData->EmotionalTone));

    return true;
}

bool UQuestManager::CompleteQuest(const FString& QuestID)
{
    UQuestInstance* QuestInstance = GetQuestInstance(QuestID);
    if (!QuestInstance)
    {
        return false;
    }

    // Check if all required objectives are completed
    if (!QuestInstance->AreAllRequiredObjectivesCompleted())
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Cannot complete quest %s - objectives not finished"), *QuestID);
        return false;
    }

    // Process completion
    ProcessQuestCompletion(QuestID);
    QuestStatusMap[QuestID] = EQuestStatus::Completed;

    // Trigger emotional climax
    FQuestData QuestData = GetQuestData(QuestID);
    TriggerEmotionalMoment(QuestID, EEmotionalTone::Triumph, TEXT("Quest Completed"));

    // Broadcast events
    OnQuestCompleted.Broadcast(QuestID);
    OnQuestStatusChanged.Broadcast(QuestID, EQuestStatus::Completed);

    // Remove from active quests
    ActiveQuestInstances.Remove(QuestID);

    // Update quest availability for potential new quests
    UpdateQuestAvailability();

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Completed quest %s"), *QuestID);
    return true;
}

bool UQuestManager::FailQuest(const FString& QuestID)
{
    UQuestInstance* QuestInstance = GetQuestInstance(QuestID);
    if (!QuestInstance)
    {
        return false;
    }

    QuestStatusMap[QuestID] = EQuestStatus::Failed;
    
    // Trigger emotional moment for failure
    TriggerEmotionalMoment(QuestID, EEmotionalTone::Desperation, TEXT("Quest Failed"));
    
    OnQuestStatusChanged.Broadcast(QuestID, EQuestStatus::Failed);
    ActiveQuestInstances.Remove(QuestID);

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Failed quest %s"), *QuestID);
    return true;
}

void UQuestManager::UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress)
{
    UQuestInstance* QuestInstance = GetQuestInstance(QuestID);
    if (!QuestInstance)
    {
        return;
    }

    bool bObjectiveCompleted = QuestInstance->UpdateObjectiveProgress(ObjectiveID, Progress);
    
    OnObjectiveUpdated.Broadcast(QuestID, ObjectiveID);

    if (bObjectiveCompleted)
    {
        // Trigger emotional moment for objective completion
        TriggerEmotionalMoment(QuestID, EEmotionalTone::Hope, FString::Printf(TEXT("Objective %s Completed"), *ObjectiveID));
        
        // Check if quest is now completable
        if (QuestInstance->AreAllRequiredObjectivesCompleted())
        {
            TriggerEmotionalMoment(QuestID, EEmotionalTone::Triumph, TEXT("All Objectives Complete"));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Updated objective %s in quest %s (Progress: %d)"), 
           *ObjectiveID, *QuestID, Progress);
}

TArray<FString> UQuestManager::GetActiveQuests() const
{
    TArray<FString> ActiveQuests;
    for (const auto& QuestPair : QuestStatusMap)
    {
        if (QuestPair.Value == EQuestStatus::Active)
        {
            ActiveQuests.Add(QuestPair.Key);
        }
    }
    return ActiveQuests;
}

TArray<FString> UQuestManager::GetAvailableQuests() const
{
    TArray<FString> AvailableQuests;
    for (const auto& QuestPair : QuestStatusMap)
    {
        if (QuestPair.Value == EQuestStatus::Available)
        {
            AvailableQuests.Add(QuestPair.Key);
        }
    }
    return AvailableQuests;
}

EQuestStatus UQuestManager::GetQuestStatus(const FString& QuestID) const
{
    const EQuestStatus* Status = QuestStatusMap.Find(QuestID);
    return Status ? *Status : EQuestStatus::NotStarted;
}

UQuestInstance* UQuestManager::GetQuestInstance(const FString& QuestID) const
{
    UQuestInstance* const* Instance = ActiveQuestInstances.Find(QuestID);
    return Instance ? *Instance : nullptr;
}

FQuestData UQuestManager::GetQuestData(const FString& QuestID) const
{
    if (QuestDataTable)
    {
        FQuestData* QuestData = QuestDataTable->FindRow<FQuestData>(FName(*QuestID), TEXT("QuestManager::GetQuestData"));
        if (QuestData)
        {
            return *QuestData;
        }
    }
    return FQuestData();
}

void UQuestManager::TriggerEmotionalMoment(const FString& QuestID, EEmotionalTone Tone, const FString& Context)
{
    // Track emotional progression
    TArray<EEmotionalTone>& EmotionalHistory = QuestEmotionalHistory.FindOrAdd(QuestID);
    EmotionalHistory.Add(Tone);

    // Log emotional moment for analytics and debugging
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Emotional moment in quest %s - Tone: %s, Context: %s"), 
           *QuestID, *UEnum::GetValueAsString(Tone), *Context);

    // TODO: Trigger audio/visual feedback based on emotional tone
    // TODO: Update player emotional state tracking
    // TODO: Influence dynamic quest generation based on emotional patterns
}

void UQuestManager::CheckLocationBasedQuests(const FVector& PlayerLocation, float Radius)
{
    for (const auto& TriggerPair : QuestTriggerLocations)
    {
        const FString& QuestID = TriggerPair.Key;
        const FVector& TriggerLocation = TriggerPair.Value;
        
        float TriggerRadius = QuestTriggerRadii.FindRef(QuestID);
        float Distance = FVector::Dist(PlayerLocation, TriggerLocation);
        
        if (Distance <= TriggerRadius && GetQuestStatus(QuestID) == EQuestStatus::Available)
        {
            // Auto-start location-based quests
            StartQuest(QuestID);
            UE_LOG(LogTemp, Log, TEXT("QuestManager: Location-triggered quest %s at distance %.2f"), *QuestID, Distance);
        }
    }
}

FString UQuestManager::GenerateSurvivalQuest(EEmotionalTone Tone, const FVector& Location)
{
    FString QuestID = GenerateUniqueQuestID(TEXT("SURVIVAL"));
    
    FQuestData NewQuest;
    NewQuest.QuestID = QuestID;
    NewQuest.QuestType = EQuestType::Survival;
    NewQuest.EmotionalTone = Tone;
    NewQuest.QuestLocation = Location;
    NewQuest.Priority = 3; // High priority for survival
    
    // Generate emotional description based on tone
    NewQuest.Description = GenerateEmotionalDescription(Tone, EQuestType::Survival, TEXT("Dynamic"));
    NewQuest.Title = FText::FromString(FString::Printf(TEXT("Survive the %s"), 
                                                      Tone == EEmotionalTone::Fear ? TEXT("Terror") : TEXT("Challenge")));
    
    // Generate objectives based on emotional tone
    NewQuest.Objectives = GenerateObjectivesForTone(Tone, EQuestType::Survival, Location);
    
    // Create dynamic quest data
    CreateDynamicQuestData(QuestID, NewQuest);
    
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Generated survival quest %s with tone %s"), 
           *QuestID, *UEnum::GetValueAsString(Tone));
    
    return QuestID;
}

FString UQuestManager::GenerateDiscoveryQuest(const FString& DinosaurSpecies, const FVector& Location)
{
    FString QuestID = GenerateUniqueQuestID(TEXT("DISCOVERY"));
    
    FQuestData NewQuest;
    NewQuest.QuestID = QuestID;
    NewQuest.QuestType = EQuestType::Discovery;
    NewQuest.EmotionalTone = EEmotionalTone::Wonder;
    NewQuest.QuestLocation = Location;
    NewQuest.Priority = 2;
    
    NewQuest.Title = FText::FromString(FString::Printf(TEXT("Discover the %s"), *DinosaurSpecies));
    NewQuest.Description = FText::FromString(FString::Printf(
        TEXT("A magnificent %s has been spotted in this area. Observe its behavior and learn about this ancient creature. "
             "Remember - you are the intruder in their world. Approach with caution and respect."), *DinosaurSpecies));
    
    // Create observation objectives
    FQuestObjective ObserveObjective;
    ObserveObjective.ObjectiveID = TEXT("OBSERVE_SPECIES");
    ObserveObjective.Type = EObjectiveType::Observe;
    ObserveObjective.Description = FText::FromString(FString::Printf(TEXT("Observe the %s for 60 seconds"), *DinosaurSpecies));
    ObserveObjective.TargetCount = 60; // 60 seconds
    ObserveObjective.TargetLocation = Location;
    ObserveObjective.TargetRadius = 1000.0f;
    
    NewQuest.Objectives.Add(ObserveObjective);
    
    CreateDynamicQuestData(QuestID, NewQuest);
    
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Generated discovery quest %s for %s"), *QuestID, *DinosaurSpecies);
    
    return QuestID;
}

bool UQuestManager::CanStartQuest(const FString& QuestID) const
{
    // Check if quest is already active or completed
    EQuestStatus CurrentStatus = GetQuestStatus(QuestID);
    if (CurrentStatus == EQuestStatus::Active || CurrentStatus == EQuestStatus::Completed)
    {
        return false;
    }

    // Check prerequisites
    FQuestData QuestData = GetQuestData(QuestID);
    for (const FString& PrereqQuestID : QuestData.PrerequisiteQuests)
    {
        if (GetQuestStatus(PrereqQuestID) != EQuestStatus::Completed)
        {
            return false;
        }
    }

    return true;
}

void UQuestManager::ProcessQuestCompletion(const FString& QuestID)
{
    UQuestInstance* QuestInstance = GetQuestInstance(QuestID);
    if (!QuestInstance)
    {
        return;
    }

    // Award rewards
    FQuestData QuestData = GetQuestData(QuestID);
    
    // TODO: Implement reward system
    // - Give experience points
    // - Add items to inventory
    // - Unlock features/abilities
    // - Update player progression
    
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Processing rewards for quest %s - XP: %d"), 
           *QuestID, QuestData.Reward.ExperiencePoints);
}

FString UQuestManager::GenerateUniqueQuestID(const FString& Prefix)
{
    DynamicQuestCounter++;
    return FString::Printf(TEXT("%s_%d"), *Prefix, DynamicQuestCounter);
}

void UQuestManager::CreateDynamicQuestData(const FString& QuestID, const FQuestData& QuestData)
{
    // For dynamic quests, we store them in memory rather than the data table
    // TODO: Implement dynamic quest storage system
    // This would allow for procedurally generated quests that persist across sessions
    
    QuestStatusMap.Add(QuestID, EQuestStatus::Available);
    
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Created dynamic quest data for %s"), *QuestID);
}

FText UQuestManager::GenerateEmotionalDescription(EEmotionalTone Tone, EQuestType Type, const FString& Context)
{
    FString Description;
    
    switch (Tone)
    {
        case EEmotionalTone::Fear:
            Description = TEXT("Danger lurks in every shadow. Your survival depends on quick thinking and careful movement.");
            break;
        case EEmotionalTone::Wonder:
            Description = TEXT("The ancient world reveals its secrets to those who observe with patience and respect.");
            break;
        case EEmotionalTone::Hope:
            Description = TEXT("Each small victory brings you closer to finding your way home.");
            break;
        case EEmotionalTone::Desperation:
            Description = TEXT("Time is running out. Every decision could be your last.");
            break;
        case EEmotionalTone::Triumph:
            Description = TEXT("Your perseverance has paid off. You've proven that humanity can adapt to any challenge.");
            break;
        default:
            Description = TEXT("The prehistoric world awaits your discovery.");
            break;
    }
    
    return FText::FromString(Description);
}

TArray<FQuestObjective> UQuestManager::GenerateObjectivesForTone(EEmotionalTone Tone, EQuestType Type, const FVector& Location)
{
    TArray<FQuestObjective> Objectives;
    
    // Generate objectives based on emotional tone and quest type
    switch (Tone)
    {
        case EEmotionalTone::Fear:
        {
            FQuestObjective SurviveObjective;
            SurviveObjective.ObjectiveID = TEXT("SURVIVE_ENCOUNTER");
            SurviveObjective.Type = EObjectiveType::Survive;
            SurviveObjective.Description = FText::FromString(TEXT("Survive for 5 minutes without being detected"));
            SurviveObjective.TargetCount = 300; // 5 minutes in seconds
            SurviveObjective.TargetLocation = Location;
            SurviveObjective.TargetRadius = 2000.0f;
            Objectives.Add(SurviveObjective);
            break;
        }
        case EEmotionalTone::Wonder:
        {
            FQuestObjective ObserveObjective;
            ObserveObjective.ObjectiveID = TEXT("OBSERVE_BEHAVIOR");
            ObserveObjective.Type = EObjectiveType::Observe;
            ObserveObjective.Description = FText::FromString(TEXT("Observe dinosaur behavior patterns"));
            ObserveObjective.TargetCount = 3; // Observe 3 different behaviors
            ObserveObjective.TargetLocation = Location;
            ObserveObjective.TargetRadius = 1500.0f;
            Objectives.Add(ObserveObjective);
            break;
        }
        default:
        {
            FQuestObjective DefaultObjective;
            DefaultObjective.ObjectiveID = TEXT("REACH_LOCATION");
            DefaultObjective.Type = EObjectiveType::ReachLocation;
            DefaultObjective.Description = FText::FromString(TEXT("Reach the target location"));
            DefaultObjective.TargetCount = 1;
            DefaultObjective.TargetLocation = Location;
            DefaultObjective.TargetRadius = 500.0f;
            Objectives.Add(DefaultObjective);
            break;
        }
    }
    
    return Objectives;
}