#include "Quest_ProgressionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"

AQuest_ProgressionManager::AQuest_ProgressionManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    StageCompletionRadius = 500.0f;
    bAutoProgressEnabled = true;
    
    // Initialize default progression stages
    InitializeProgressionStages();
}

void AQuest_ProgressionManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Create visual markers for progression stages
    CreateProgressionMarkers();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Progression Manager initialized with %d stages"), ProgressionStages.Num());
}

void AQuest_ProgressionManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update player play time
    PlayerProgress.TotalPlayTime += DeltaTime;
    
    // Check if player is near progression stages
    if (bAutoProgressEnabled)
    {
        CheckPlayerProximityToStages();
    }
    
    // Update marker visibility based on progression
    UpdateMarkerVisibility();
}

void AQuest_ProgressionManager::InitializeProgressionStages()
{
    ProgressionStages.Empty();
    
    // Stage 1: Tutorial - Basic Survival
    FQuest_ProgressionStage TutorialStage;
    TutorialStage.StageName = TEXT("Tutorial - Basic Survival");
    TutorialStage.RequiredLevel = 1;
    TutorialStage.StageLocation = FVector(500, 0, 100);
    TutorialStage.UnlockedQuests.Add(TEXT("Gather Wood"));
    TutorialStage.UnlockedQuests.Add(TEXT("Find Water"));
    TutorialStage.UnlockedQuests.Add(TEXT("Build Shelter"));
    ProgressionStages.Add(TutorialStage);
    
    // Stage 2: Hunting Grounds
    FQuest_ProgressionStage HuntingStage;
    HuntingStage.StageName = TEXT("Hunting Grounds");
    HuntingStage.RequiredLevel = 3;
    HuntingStage.StageLocation = FVector(1500, 500, 100);
    HuntingStage.PrerequisiteQuests.Add(TEXT("Build Shelter"));
    HuntingStage.UnlockedQuests.Add(TEXT("Hunt Small Prey"));
    HuntingStage.UnlockedQuests.Add(TEXT("Craft Weapons"));
    HuntingStage.UnlockedQuests.Add(TEXT("Track Dinosaurs"));
    ProgressionStages.Add(HuntingStage);
    
    // Stage 3: Exploration Zone
    FQuest_ProgressionStage ExplorationStage;
    ExplorationStage.StageName = TEXT("Exploration Zone");
    ExplorationStage.RequiredLevel = 5;
    ExplorationStage.StageLocation = FVector(-1000, 1000, 150);
    ExplorationStage.PrerequisiteQuests.Add(TEXT("Hunt Small Prey"));
    ExplorationStage.UnlockedQuests.Add(TEXT("Explore Cave System"));
    ExplorationStage.UnlockedQuests.Add(TEXT("Discover Ancient Ruins"));
    ExplorationStage.UnlockedQuests.Add(TEXT("Map Territory"));
    ProgressionStages.Add(ExplorationStage);
    
    // Stage 4: Advanced Survival
    FQuest_ProgressionStage AdvancedStage;
    AdvancedStage.StageName = TEXT("Advanced Survival");
    AdvancedStage.RequiredLevel = 8;
    AdvancedStage.StageLocation = FVector(2000, -1000, 120);
    AdvancedStage.PrerequisiteQuests.Add(TEXT("Explore Cave System"));
    AdvancedStage.UnlockedQuests.Add(TEXT("Hunt Large Predators"));
    AdvancedStage.UnlockedQuests.Add(TEXT("Establish Outpost"));
    AdvancedStage.UnlockedQuests.Add(TEXT("Master Fire"));
    ProgressionStages.Add(AdvancedStage);
    
    // Stage 5: Final Challenge
    FQuest_ProgressionStage FinalStage;
    FinalStage.StageName = TEXT("Final Challenge");
    FinalStage.RequiredLevel = 12;
    FinalStage.StageLocation = FVector(-2000, -2000, 180);
    FinalStage.PrerequisiteQuests.Add(TEXT("Hunt Large Predators"));
    FinalStage.PrerequisiteQuests.Add(TEXT("Establish Outpost"));
    FinalStage.UnlockedQuests.Add(TEXT("Face the Alpha"));
    FinalStage.UnlockedQuests.Add(TEXT("Survive the Storm"));
    FinalStage.UnlockedQuests.Add(TEXT("Become the Apex"));
    ProgressionStages.Add(FinalStage);
}

bool AQuest_ProgressionManager::CanProgressToStage(int32 StageIndex)
{
    if (StageIndex < 0 || StageIndex >= ProgressionStages.Num())
    {
        return false;
    }
    
    const FQuest_ProgressionStage& Stage = ProgressionStages[StageIndex];
    
    // Check level requirement
    if (PlayerProgress.CurrentLevel < Stage.RequiredLevel)
    {
        return false;
    }
    
    // Check prerequisite quests
    for (const FString& PrereqQuest : Stage.PrerequisiteQuests)
    {
        if (!PlayerProgress.CompletedQuests.Contains(PrereqQuest))
        {
            return false;
        }
    }
    
    return true;
}

void AQuest_ProgressionManager::CompleteCurrentStage()
{
    if (PlayerProgress.CurrentStageIndex < ProgressionStages.Num())
    {
        ProgressionStages[PlayerProgress.CurrentStageIndex].bIsCompleted = true;
        
        UE_LOG(LogTemp, Warning, TEXT("Completed stage: %s"), 
               *ProgressionStages[PlayerProgress.CurrentStageIndex].StageName);
        
        // Auto-advance if possible
        if (bAutoProgressEnabled)
        {
            AdvanceToNextStage();
        }
    }
}

void AQuest_ProgressionManager::AdvanceToNextStage()
{
    int32 NextStageIndex = PlayerProgress.CurrentStageIndex + 1;
    
    if (NextStageIndex < ProgressionStages.Num() && CanProgressToStage(NextStageIndex))
    {
        HandleStageTransition(NextStageIndex);
    }
}

FQuest_ProgressionStage AQuest_ProgressionManager::GetCurrentStage()
{
    if (PlayerProgress.CurrentStageIndex < ProgressionStages.Num())
    {
        return ProgressionStages[PlayerProgress.CurrentStageIndex];
    }
    
    return FQuest_ProgressionStage();
}

TArray<FString> AQuest_ProgressionManager::GetAvailableQuests()
{
    TArray<FString> AvailableQuests;
    
    // Get quests from current stage
    if (PlayerProgress.CurrentStageIndex < ProgressionStages.Num())
    {
        const FQuest_ProgressionStage& CurrentStage = ProgressionStages[PlayerProgress.CurrentStageIndex];
        
        for (const FString& QuestName : CurrentStage.UnlockedQuests)
        {
            if (!PlayerProgress.CompletedQuests.Contains(QuestName) && 
                !PlayerProgress.ActiveQuests.Contains(QuestName))
            {
                AvailableQuests.Add(QuestName);
            }
        }
    }
    
    return AvailableQuests;
}

void AQuest_ProgressionManager::UpdatePlayerProgress(const FString& CompletedQuest, int32 ExperienceGained)
{
    // Add to completed quests
    if (!PlayerProgress.CompletedQuests.Contains(CompletedQuest))
    {
        PlayerProgress.CompletedQuests.Add(CompletedQuest);
    }
    
    // Remove from active quests
    PlayerProgress.ActiveQuests.Remove(CompletedQuest);
    
    // Calculate level progression (simple XP system)
    int32 NewLevel = 1 + (PlayerProgress.CompletedQuests.Num() / 3);
    if (NewLevel > PlayerProgress.CurrentLevel)
    {
        PlayerProgress.CurrentLevel = NewLevel;
        UE_LOG(LogTemp, Warning, TEXT("Player leveled up to %d!"), NewLevel);
    }
    
    // Check if current stage can be completed
    const FQuest_ProgressionStage& CurrentStage = GetCurrentStage();
    bool bCanCompleteStage = true;
    
    for (const FString& RequiredQuest : CurrentStage.UnlockedQuests)
    {
        if (!PlayerProgress.CompletedQuests.Contains(RequiredQuest))
        {
            bCanCompleteStage = false;
            break;
        }
    }
    
    if (bCanCompleteStage && !CurrentStage.bIsCompleted)
    {
        CompleteCurrentStage();
    }
}

bool AQuest_ProgressionManager::IsQuestUnlocked(const FString& QuestName)
{
    // Check current stage quests
    if (PlayerProgress.CurrentStageIndex < ProgressionStages.Num())
    {
        const FQuest_ProgressionStage& CurrentStage = ProgressionStages[PlayerProgress.CurrentStageIndex];
        return CurrentStage.UnlockedQuests.Contains(QuestName);
    }
    
    return false;
}

void AQuest_ProgressionManager::CreateProgressionMarkers()
{
    QuestMarkers.Empty();
    
    for (int32 i = 0; i < ProgressionStages.Num(); i++)
    {
        const FQuest_ProgressionStage& Stage = ProgressionStages[i];
        FString MarkerName = FString::Printf(TEXT("QuestMarker_%d_%s"), i + 1, *Stage.StageName);
        
        SpawnQuestMarker(Stage.StageLocation + FVector(0, 0, 300), MarkerName);
    }
}

void AQuest_ProgressionManager::UpdateMarkerVisibility()
{
    for (int32 i = 0; i < QuestMarkers.Num() && i < ProgressionStages.Num(); i++)
    {
        if (QuestMarkers[i])
        {
            // Show marker if stage is available or current
            bool bShouldShow = (i <= PlayerProgress.CurrentStageIndex + 1) && CanProgressToStage(i);
            QuestMarkers[i]->SetActorHiddenInGame(!bShouldShow);
        }
    }
}

float AQuest_ProgressionManager::GetProgressionPercentage()
{
    if (ProgressionStages.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 CompletedStages = 0;
    for (const FQuest_ProgressionStage& Stage : ProgressionStages)
    {
        if (Stage.bIsCompleted)
        {
            CompletedStages++;
        }
    }
    
    return (float)CompletedStages / (float)ProgressionStages.Num() * 100.0f;
}

void AQuest_ProgressionManager::CheckPlayerProximityToStages()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    
    // Check if player is near next available stage
    for (int32 i = PlayerProgress.CurrentStageIndex; i < ProgressionStages.Num(); i++)
    {
        if (CanProgressToStage(i))
        {
            float Distance = FVector::Dist(PlayerLocation, ProgressionStages[i].StageLocation);
            if (Distance <= StageCompletionRadius)
            {
                if (i > PlayerProgress.CurrentStageIndex)
                {
                    HandleStageTransition(i);
                }
                break;
            }
        }
    }
}

void AQuest_ProgressionManager::HandleStageTransition(int32 NewStageIndex)
{
    PlayerProgress.CurrentStageIndex = NewStageIndex;
    
    UE_LOG(LogTemp, Warning, TEXT("Advanced to stage %d: %s"), 
           NewStageIndex + 1, *ProgressionStages[NewStageIndex].StageName);
    
    // Unlock new quests
    const FQuest_ProgressionStage& NewStage = ProgressionStages[NewStageIndex];
    for (const FString& QuestName : NewStage.UnlockedQuests)
    {
        UE_LOG(LogTemp, Log, TEXT("Unlocked quest: %s"), *QuestName);
    }
}

void AQuest_ProgressionManager::SpawnQuestMarker(const FVector& Location, const FString& MarkerName)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Spawn a simple static mesh actor as marker
    AStaticMeshActor* MarkerActor = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    if (MarkerActor)
    {
        MarkerActor->SetActorLabel(MarkerName);
        QuestMarkers.Add(MarkerActor);
        
        UE_LOG(LogTemp, Log, TEXT("Created quest marker: %s at %s"), *MarkerName, *Location.ToString());
    }
}