#include "Quest_ExplorationQuestManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/TriggerBox.h"

AQuest_ExplorationQuestManager::AQuest_ExplorationQuestManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    ObjectiveCheckInterval = 2.0f;
    bDebugMode = false;
    
    // Set up root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AQuest_ExplorationQuestManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default exploration quests
    InitializeDefaultQuests();
    
    // Start objective checking timer
    GetWorld()->GetTimerManager().SetTimer(
        ObjectiveCheckTimer,
        this,
        &AQuest_ExplorationQuestManager::CheckObjectiveProgress,
        ObjectiveCheckInterval,
        true
    );
    
    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("ExplorationQuestManager: BeginPlay - Quest system initialized"));
    }
}

void AQuest_ExplorationQuestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Additional per-frame quest logic if needed
    if (bDebugMode && ActiveQuests.Num() > 0)
    {
        // Debug display active quest count
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1, 0.0f, FColor::Green,
                FString::Printf(TEXT("Active Quests: %d"), ActiveQuests.Num())
            );
        }
    }
}

void AQuest_ExplorationQuestManager::StartExplorationQuest(const FString& QuestName)
{
    // Find quest by name and activate it
    for (FQuest_ExplorationQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestName == QuestName && !Quest.bIsActive)
        {
            Quest.bIsActive = true;
            
            if (bDebugMode)
            {
                UE_LOG(LogTemp, Warning, TEXT("ExplorationQuestManager: Started quest '%s'"), *QuestName);
            }
            
            OnQuestStarted(QuestName);
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ExplorationQuestManager: Quest '%s' not found or already active"), *QuestName);
}

void AQuest_ExplorationQuestManager::CompleteObjective(const FString& QuestName, const FString& ObjectiveName)
{
    for (FQuest_ExplorationQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestName == QuestName && Quest.bIsActive)
        {
            for (FQuest_ExplorationObjective& Objective : Quest.Objectives)
            {
                if (Objective.ObjectiveName == ObjectiveName && !Objective.bIsCompleted)
                {
                    MarkObjectiveComplete(Quest, Objective);
                    return;
                }
            }
        }
    }
}

bool AQuest_ExplorationQuestManager::IsQuestActive(const FString& QuestName) const
{
    for (const FQuest_ExplorationQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestName == QuestName)
        {
            return Quest.bIsActive && !Quest.bIsCompleted;
        }
    }
    return false;
}

bool AQuest_ExplorationQuestManager::IsObjectiveCompleted(const FString& QuestName, const FString& ObjectiveName) const
{
    for (const FQuest_ExplorationQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestName == QuestName)
        {
            for (const FQuest_ExplorationObjective& Objective : Quest.Objectives)
            {
                if (Objective.ObjectiveName == ObjectiveName)
                {
                    return Objective.bIsCompleted;
                }
            }
        }
    }
    return false;
}

void AQuest_ExplorationQuestManager::RegisterDiscoveryItem(AActor* DiscoveryActor)
{
    if (DiscoveryActor && !DiscoveryItems.Contains(DiscoveryActor))
    {
        DiscoveryItems.Add(DiscoveryActor);
        
        if (bDebugMode)
        {
            UE_LOG(LogTemp, Warning, TEXT("ExplorationQuestManager: Registered discovery item '%s'"), 
                *DiscoveryActor->GetName());
        }
    }
}

void AQuest_ExplorationQuestManager::RegisterExplorationTrigger(ATriggerBox* TriggerBox)
{
    if (TriggerBox && !ExplorationTriggers.Contains(TriggerBox))
    {
        ExplorationTriggers.Add(TriggerBox);
        
        if (bDebugMode)
        {
            UE_LOG(LogTemp, Warning, TEXT("ExplorationQuestManager: Registered exploration trigger '%s'"), 
                *TriggerBox->GetName());
        }
    }
}

void AQuest_ExplorationQuestManager::CheckPlayerProximityToObjectives(APawn* PlayerPawn)
{
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (FQuest_ExplorationQuest& Quest : ActiveQuests)
    {
        if (!Quest.bIsActive || Quest.bIsCompleted)
        {
            continue;
        }
        
        for (FQuest_ExplorationObjective& Objective : Quest.Objectives)
        {
            if (!Objective.bIsCompleted)
            {
                if (CheckLocationObjective(Objective, PlayerPawn))
                {
                    MarkObjectiveComplete(Quest, Objective);
                }
            }
        }
    }
}

float AQuest_ExplorationQuestManager::GetQuestCompletionPercentage(const FString& QuestName) const
{
    for (const FQuest_ExplorationQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestName == QuestName)
        {
            if (Quest.Objectives.Num() == 0)
            {
                return 0.0f;
            }
            
            int32 CompletedCount = 0;
            for (const FQuest_ExplorationObjective& Objective : Quest.Objectives)
            {
                if (Objective.bIsCompleted)
                {
                    CompletedCount++;
                }
            }
            
            return (float)CompletedCount / (float)Quest.Objectives.Num();
        }
    }
    return 0.0f;
}

TArray<FString> AQuest_ExplorationQuestManager::GetActiveQuestNames() const
{
    TArray<FString> QuestNames;
    
    for (const FQuest_ExplorationQuest& Quest : ActiveQuests)
    {
        if (Quest.bIsActive && !Quest.bIsCompleted)
        {
            QuestNames.Add(Quest.QuestName);
        }
    }
    
    return QuestNames;
}

int32 AQuest_ExplorationQuestManager::CalculateQuestReward(const FString& QuestName) const
{
    for (const FQuest_ExplorationQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestName == QuestName)
        {
            int32 TotalReward = Quest.TotalExperienceReward;
            
            for (const FQuest_ExplorationObjective& Objective : Quest.Objectives)
            {
                TotalReward += Objective.ExperienceReward;
            }
            
            return TotalReward;
        }
    }
    return 0;
}

void AQuest_ExplorationQuestManager::CheckObjectiveProgress()
{
    // Get player pawn for proximity checks
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        CheckPlayerProximityToObjectives(PlayerPawn);
    }
}

void AQuest_ExplorationQuestManager::InitializeDefaultQuests()
{
    // Create "First Steps" exploration quest
    FQuest_ExplorationQuest FirstStepsQuest;
    FirstStepsQuest.QuestName = TEXT("First Steps");
    FirstStepsQuest.QuestDescription = TEXT("Begin your journey by exploring the immediate area and discovering key landmarks.");
    FirstStepsQuest.bIsActive = true;
    FirstStepsQuest.TotalExperienceReward = 200;
    
    // Add objectives to First Steps quest
    FQuest_ExplorationObjective CaveObjective;
    CaveObjective.ObjectiveName = TEXT("Find Cave Entrance");
    CaveObjective.Description = TEXT("Locate the mysterious cave entrance to the north");
    CaveObjective.ObjectiveType = EQuest_ExplorationObjectiveType::DiscoverLocation;
    CaveObjective.TargetLocation = FVector(2000, 1000, 200);
    CaveObjective.CompletionRadius = 300.0f;
    CaveObjective.ExperienceReward = 50;
    FirstStepsQuest.Objectives.Add(CaveObjective);
    
    FQuest_ExplorationObjective CliffObjective;
    CliffObjective.ObjectiveName = TEXT("Reach High Ground");
    CliffObjective.Description = TEXT("Climb to the high cliff for a better view of the area");
    CliffObjective.ObjectiveType = EQuest_ExplorationObjectiveType::ReachElevation;
    CliffObjective.TargetLocation = FVector(-1500, 2000, 800);
    CliffObjective.CompletionRadius = 400.0f;
    CliffObjective.ExperienceReward = 75;
    FirstStepsQuest.Objectives.Add(CliffObjective);
    
    ActiveQuests.Add(FirstStepsQuest);
    
    // Create "Resource Discovery" quest
    FQuest_ExplorationQuest ResourceQuest;
    ResourceQuest.QuestName = TEXT("Resource Discovery");
    ResourceQuest.QuestDescription = TEXT("Find essential resources scattered throughout the prehistoric landscape.");
    ResourceQuest.bIsActive = false;
    ResourceQuest.TotalExperienceReward = 300;
    
    FQuest_ExplorationObjective WaterObjective;
    WaterObjective.ObjectiveName = TEXT("Find Water Source");
    WaterObjective.Description = TEXT("Locate a reliable source of fresh water");
    WaterObjective.ObjectiveType = EQuest_ExplorationObjectiveType::FindResource;
    WaterObjective.TargetLocation = FVector(-200, 1200, 90);
    WaterObjective.CompletionRadius = 250.0f;
    WaterObjective.ExperienceReward = 100;
    ResourceQuest.Objectives.Add(WaterObjective);
    
    ActiveQuests.Add(ResourceQuest);
    
    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("ExplorationQuestManager: Initialized %d default quests"), ActiveQuests.Num());
    }
}

bool AQuest_ExplorationQuestManager::CheckLocationObjective(const FQuest_ExplorationObjective& Objective, APawn* PlayerPawn)
{
    if (!PlayerPawn)
    {
        return false;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, Objective.TargetLocation);
    
    return Distance <= Objective.CompletionRadius;
}

void AQuest_ExplorationQuestManager::MarkObjectiveComplete(FQuest_ExplorationQuest& Quest, FQuest_ExplorationObjective& Objective)
{
    Objective.bIsCompleted = true;
    
    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("ExplorationQuestManager: Completed objective '%s' in quest '%s'"), 
            *Objective.ObjectiveName, *Quest.QuestName);
    }
    
    OnObjectiveCompleted(Quest.QuestName, Objective.ObjectiveName);
    
    // Check if all objectives in quest are complete
    bool bAllObjectivesComplete = true;
    for (const FQuest_ExplorationObjective& QuestObjective : Quest.Objectives)
    {
        if (!QuestObjective.bIsCompleted)
        {
            bAllObjectivesComplete = false;
            break;
        }
    }
    
    if (bAllObjectivesComplete && !Quest.bIsCompleted)
    {
        Quest.bIsCompleted = true;
        int32 TotalReward = CalculateQuestReward(Quest.QuestName);
        
        // Move to completed quests
        CompletedQuests.Add(Quest);
        
        OnQuestCompleted(Quest.QuestName, TotalReward);
        
        if (bDebugMode)
        {
            UE_LOG(LogTemp, Warning, TEXT("ExplorationQuestManager: Completed quest '%s' with reward %d"), 
                *Quest.QuestName, TotalReward);
        }
    }
}