#include "Quest_SurvivalQuestController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AQuest_SurvivalQuestController::AQuest_SurvivalQuestController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create quest marker mesh component
    QuestMarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("QuestMarkerMesh"));
    QuestMarkerMesh->SetupAttachment(RootComponent);

    // Initialize default values
    MaxActiveQuests = 5;
    QuestUpdateInterval = 1.0f;
    MarkerScale = 1.0f;
    bShowQuestMarkers = true;
    LastUpdateTime = 0.0f;

    // Try to load default mesh for quest markers
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMeshAsset.Succeeded())
    {
        DefaultQuestMarkerMesh = SphereMeshAsset.Object;
        QuestMarkerMesh->SetStaticMesh(DefaultQuestMarkerMesh);
    }

    // Set initial scale
    QuestMarkerMesh->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
    QuestMarkerMesh->SetVisibility(false); // Hidden by default
}

void AQuest_SurvivalQuestController::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalQuestController: BeginPlay - Sistema de quests inicializado"));

    // Initialize quest system
    ActiveQuests.Empty();
    CompletedQuests.Empty();

    // Generate some initial survival quests
    FQuest_SurvivalQuest InitialGatherQuest = GenerateGatheringQuest(
        FVector(5000.0f, 5000.0f, 0.0f), 
        TEXT("Sticks"), 
        5
    );
    StartQuest(InitialGatherQuest);

    FQuest_SurvivalQuest InitialCraftQuest = GenerateCraftingQuest(TEXT("Stone Axe"));
    StartQuest(InitialCraftQuest);

    FQuest_SurvivalQuest InitialExploreQuest = GenerateExplorationQuest(
        FVector(10000.0f, 10000.0f, 0.0f), 
        2000.0f
    );
    StartQuest(InitialExploreQuest);

    UpdateQuestMarkers();
}

void AQuest_SurvivalQuestController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= QuestUpdateInterval)
    {
        UpdateActiveQuests(DeltaTime);
        CheckQuestCompletion();
        LastUpdateTime = 0.0f;
    }
}

bool AQuest_SurvivalQuestController::StartQuest(const FQuest_SurvivalQuest& NewQuest)
{
    if (ActiveQuests.Num() >= MaxActiveQuests)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalQuestController: Cannot start quest - max active quests reached"));
        return false;
    }

    // Check if quest is already active
    for (const FQuest_SurvivalQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestName == NewQuest.QuestName)
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalQuestController: Quest already active: %s"), *NewQuest.QuestName);
            return false;
        }
    }

    FQuest_SurvivalQuest QuestToStart = NewQuest;
    QuestToStart.bIsActive = true;
    QuestToStart.bIsCompleted = false;
    QuestToStart.ElapsedTime = 0.0f;

    ActiveQuests.Add(QuestToStart);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalQuestController: Started quest: %s"), *NewQuest.QuestName);
    
    UpdateQuestMarkers();
    return true;
}

bool AQuest_SurvivalQuestController::CompleteQuest(const FString& QuestName)
{
    for (int32 i = 0; i < ActiveQuests.Num(); i++)
    {
        if (ActiveQuests[i].QuestName == QuestName)
        {
            ActiveQuests[i].bIsCompleted = true;
            ActiveQuests[i].bIsActive = false;
            
            CompletedQuests.Add(ActiveQuests[i]);
            ActiveQuests.RemoveAt(i);
            
            UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalQuestController: Completed quest: %s"), *QuestName);
            
            UpdateQuestMarkers();
            return true;
        }
    }
    
    return false;
}

bool AQuest_SurvivalQuestController::UpdateQuestProgress(const FString& QuestName, int32 ObjectiveIndex, int32 ProgressAmount)
{
    for (FQuest_SurvivalQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestName == QuestName)
        {
            if (ObjectiveIndex >= 0 && ObjectiveIndex < Quest.Objectives.Num())
            {
                FQuest_SurvivalObjective& Objective = Quest.Objectives[ObjectiveIndex];
                Objective.CurrentProgress = FMath::Min(Objective.CurrentProgress + ProgressAmount, Objective.TargetQuantity);
                
                if (Objective.CurrentProgress >= Objective.TargetQuantity)
                {
                    Objective.bIsCompleted = true;
                    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalQuestController: Objective completed: %s"), *Objective.ObjectiveDescription);
                }
                
                return true;
            }
        }
    }
    
    return false;
}

FQuest_SurvivalQuest AQuest_SurvivalQuestController::GetActiveQuest(const FString& QuestName)
{
    for (const FQuest_SurvivalQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestName == QuestName)
        {
            return Quest;
        }
    }
    
    return FQuest_SurvivalQuest(); // Return default quest if not found
}

TArray<FQuest_SurvivalQuest> AQuest_SurvivalQuestController::GetActiveQuests()
{
    return ActiveQuests;
}

void AQuest_SurvivalQuestController::ClearCompletedQuests()
{
    CompletedQuests.Empty();
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalQuestController: Cleared completed quests"));
}

FQuest_SurvivalQuest AQuest_SurvivalQuestController::GenerateHuntingQuest(const FVector& HuntLocation, const FString& TargetSpecies)
{
    FQuest_SurvivalQuest HuntQuest;
    HuntQuest.QuestName = FString::Printf(TEXT("Hunt %s"), *TargetSpecies);
    HuntQuest.QuestDescription = FString::Printf(TEXT("Track and hunt a %s for food and materials"), *TargetSpecies);
    HuntQuest.Priority = EQuest_QuestPriority::High;
    HuntQuest.ExperienceReward = 200;

    FQuest_SurvivalObjective HuntObjective;
    HuntObjective.ObjectiveDescription = FString::Printf(TEXT("Hunt 1 %s"), *TargetSpecies);
    HuntObjective.QuestType = EQuest_SurvivalQuestType::Hunt_Herbivore;
    HuntObjective.TargetQuantity = 1;
    HuntObjective.TargetLocation = HuntLocation;
    HuntObjective.CompletionRadius = 1000.0f;

    HuntQuest.Objectives.Add(HuntObjective);
    
    return HuntQuest;
}

FQuest_SurvivalQuest AQuest_SurvivalQuestController::GenerateGatheringQuest(const FVector& GatherLocation, const FString& ResourceType, int32 Quantity)
{
    FQuest_SurvivalQuest GatherQuest;
    GatherQuest.QuestName = FString::Printf(TEXT("Gather %s"), *ResourceType);
    GatherQuest.QuestDescription = FString::Printf(TEXT("Collect %d %s for crafting and survival"), Quantity, *ResourceType);
    GatherQuest.Priority = EQuest_QuestPriority::Medium;
    GatherQuest.ExperienceReward = 50 * Quantity;

    FQuest_SurvivalObjective GatherObjective;
    GatherObjective.ObjectiveDescription = FString::Printf(TEXT("Gather %d %s"), Quantity, *ResourceType);
    GatherObjective.QuestType = EQuest_SurvivalQuestType::Gather_Resources;
    GatherObjective.TargetQuantity = Quantity;
    GatherObjective.TargetLocation = GatherLocation;
    GatherObjective.CompletionRadius = 500.0f;

    GatherQuest.Objectives.Add(GatherObjective);
    
    return GatherQuest;
}

FQuest_SurvivalQuest AQuest_SurvivalQuestController::GenerateCraftingQuest(const FString& ItemToCraft)
{
    FQuest_SurvivalQuest CraftQuest;
    CraftQuest.QuestName = FString::Printf(TEXT("Craft %s"), *ItemToCraft);
    CraftQuest.QuestDescription = FString::Printf(TEXT("Create a %s to improve your survival chances"), *ItemToCraft);
    CraftQuest.Priority = EQuest_QuestPriority::High;
    CraftQuest.ExperienceReward = 150;

    FQuest_SurvivalObjective CraftObjective;
    CraftObjective.ObjectiveDescription = FString::Printf(TEXT("Craft 1 %s"), *ItemToCraft);
    CraftObjective.QuestType = EQuest_SurvivalQuestType::Craft_Tool;
    CraftObjective.TargetQuantity = 1;
    CraftObjective.CompletionRadius = 0.0f; // No location requirement

    CraftQuest.Objectives.Add(CraftObjective);
    
    return CraftQuest;
}

FQuest_SurvivalQuest AQuest_SurvivalQuestController::GenerateExplorationQuest(const FVector& ExploreLocation, float ExploreRadius)
{
    FQuest_SurvivalQuest ExploreQuest;
    ExploreQuest.QuestName = TEXT("Explore New Territory");
    ExploreQuest.QuestDescription = TEXT("Venture into unknown areas to discover resources and landmarks");
    ExploreQuest.Priority = EQuest_QuestPriority::Medium;
    ExploreQuest.ExperienceReward = 100;

    FQuest_SurvivalObjective ExploreObjective;
    ExploreObjective.ObjectiveDescription = TEXT("Explore the marked area");
    ExploreObjective.QuestType = EQuest_SurvivalQuestType::Explore_Territory;
    ExploreObjective.TargetQuantity = 1;
    ExploreObjective.TargetLocation = ExploreLocation;
    ExploreObjective.CompletionRadius = ExploreRadius;

    ExploreQuest.Objectives.Add(ExploreObjective);
    
    return ExploreQuest;
}

FQuest_SurvivalQuest AQuest_SurvivalQuestController::GenerateDefenseQuest(const FVector& DefenseLocation)
{
    FQuest_SurvivalQuest DefenseQuest;
    DefenseQuest.QuestName = TEXT("Defend Camp");
    DefenseQuest.QuestDescription = TEXT("Protect your camp from predator attacks");
    DefenseQuest.Priority = EQuest_QuestPriority::Critical;
    DefenseQuest.ExperienceReward = 300;
    DefenseQuest.TimeLimit = 300.0f; // 5 minutes

    FQuest_SurvivalObjective DefenseObjective;
    DefenseObjective.ObjectiveDescription = TEXT("Survive the predator attack");
    DefenseObjective.QuestType = EQuest_SurvivalQuestType::Defend_Camp;
    DefenseObjective.TargetQuantity = 1;
    DefenseObjective.TargetLocation = DefenseLocation;
    DefenseObjective.CompletionRadius = 1000.0f;

    DefenseQuest.Objectives.Add(DefenseObjective);
    
    return DefenseQuest;
}

bool AQuest_SurvivalQuestController::IsQuestActive(const FString& QuestName)
{
    for (const FQuest_SurvivalQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestName == QuestName && Quest.bIsActive)
        {
            return true;
        }
    }
    return false;
}

float AQuest_SurvivalQuestController::GetQuestCompletionPercentage(const FString& QuestName)
{
    for (const FQuest_SurvivalQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestName == QuestName)
        {
            if (Quest.Objectives.Num() == 0) return 0.0f;
            
            int32 CompletedObjectives = 0;
            for (const FQuest_SurvivalObjective& Objective : Quest.Objectives)
            {
                if (Objective.bIsCompleted)
                {
                    CompletedObjectives++;
                }
            }
            
            return (float)CompletedObjectives / (float)Quest.Objectives.Num() * 100.0f;
        }
    }
    return 0.0f;
}

void AQuest_SurvivalQuestController::UpdateQuestMarkers()
{
    if (!bShowQuestMarkers) return;

    // Simple marker update - in a full implementation, this would manage multiple marker actors
    if (ActiveQuests.Num() > 0 && ActiveQuests[0].Objectives.Num() > 0)
    {
        FVector MarkerLocation = ActiveQuests[0].Objectives[0].TargetLocation;
        if (MarkerLocation != FVector::ZeroVector)
        {
            SetActorLocation(MarkerLocation + FVector(0, 0, 200)); // Elevated marker
            QuestMarkerMesh->SetVisibility(true);
        }
    }
    else
    {
        QuestMarkerMesh->SetVisibility(false);
    }
}

void AQuest_SurvivalQuestController::SetQuestMarkerVisibility(bool bVisible)
{
    bShowQuestMarkers = bVisible;
    QuestMarkerMesh->SetVisibility(bVisible && ActiveQuests.Num() > 0);
}

void AQuest_SurvivalQuestController::UpdateActiveQuests(float DeltaTime)
{
    for (FQuest_SurvivalQuest& Quest : ActiveQuests)
    {
        if (Quest.bIsActive)
        {
            Quest.ElapsedTime += DeltaTime;
            
            // Check time limit
            if (Quest.TimeLimit > 0.0f && Quest.ElapsedTime >= Quest.TimeLimit)
            {
                UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalQuestController: Quest timed out: %s"), *Quest.QuestName);
                Quest.bIsActive = false;
            }
        }
    }
}

void AQuest_SurvivalQuestController::CheckQuestCompletion()
{
    TArray<FString> QuestsToComplete;
    
    for (const FQuest_SurvivalQuest& Quest : ActiveQuests)
    {
        if (Quest.bIsActive && AreAllObjectivesCompleted(Quest))
        {
            QuestsToComplete.Add(Quest.QuestName);
        }
    }
    
    for (const FString& QuestName : QuestsToComplete)
    {
        CompleteQuest(QuestName);
    }
}

bool AQuest_SurvivalQuestController::AreAllObjectivesCompleted(const FQuest_SurvivalQuest& Quest)
{
    for (const FQuest_SurvivalObjective& Objective : Quest.Objectives)
    {
        if (!Objective.bIsOptional && !Objective.bIsCompleted)
        {
            return false;
        }
    }
    return true;
}

void AQuest_SurvivalQuestController::SpawnQuestMarker(const FVector& Location, const FString& QuestName)
{
    // Implementation for spawning individual quest markers
    // This would create separate marker actors for each quest objective
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalQuestController: Spawning marker for quest: %s at location: %s"), *QuestName, *Location.ToString());
}

void AQuest_SurvivalQuestController::RemoveQuestMarker(const FString& QuestName)
{
    // Implementation for removing quest markers
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalQuestController: Removing marker for quest: %s"), *QuestName);
}