#include "Quest_SurvivalQuestManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

AQuest_SurvivalQuestManager::AQuest_SurvivalQuestManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create quest marker mesh component
    QuestMarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("QuestMarkerMesh"));
    RootComponent = QuestMarkerMesh;

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(200.0f);

    // Initialize quest system parameters
    MaxActiveQuests = 3;
    QuestGenerationInterval = 300.0f; // 5 minutes
    bAutoGenerateQuests = true;
    SurvivalDifficultyMultiplier = 1.0f;

    // Initialize dinosaur hunt targets
    DinosaurHuntTargets.Add(TEXT("Raptor"));
    DinosaurHuntTargets.Add(TEXT("TRex"));
    DinosaurHuntTargets.Add(TEXT("Triceratops"));
    DinosaurHuntTargets.Add(TEXT("Brachiosaurus"));
    DinosaurHuntTargets.Add(TEXT("Stegosaurus"));

    // Initialize resource gather targets
    ResourceGatherTargets.Add(TEXT("Stone"));
    ResourceGatherTargets.Add(TEXT("Wood"));
    ResourceGatherTargets.Add(TEXT("Fiber"));
    ResourceGatherTargets.Add(TEXT("Berries"));
    ResourceGatherTargets.Add(TEXT("Meat"));
    ResourceGatherTargets.Add(TEXT("Hide"));
}

void AQuest_SurvivalQuestManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize default quests
    InitializeDefaultQuests();

    // Start auto quest generation if enabled
    if (bAutoGenerateQuests)
    {
        GetWorldTimerManager().SetTimer(
            QuestGenerationTimer,
            this,
            &AQuest_SurvivalQuestManager::GenerateRandomSurvivalQuest,
            QuestGenerationInterval,
            true
        );
    }

    UE_LOG(LogTemp, Log, TEXT("Quest_SurvivalQuestManager: Initialized with %d active quests"), ActiveQuests.Num());
}

void AQuest_SurvivalQuestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Check for completed quests
    for (int32 i = ActiveQuests.Num() - 1; i >= 0; i--)
    {
        if (ActiveQuests[i].CurrentProgress >= ActiveQuests[i].TargetAmount && !ActiveQuests[i].bIsCompleted)
        {
            CompleteQuest(i);
        }
    }
}

void AQuest_SurvivalQuestManager::GenerateRandomSurvivalQuest()
{
    if (ActiveQuests.Num() >= MaxActiveQuests)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalQuestManager: Maximum active quests reached"));
        return;
    }

    // Randomly select quest type
    int32 QuestTypeIndex = FMath::RandRange(0, 5);
    EQuest_SurvivalQuestType QuestType = static_cast<EQuest_SurvivalQuestType>(QuestTypeIndex);

    FQuest_SurvivalObjective NewQuest;
    NewQuest.QuestType = QuestType;

    switch (QuestType)
    {
        case EQuest_SurvivalQuestType::Hunt_Dinosaur:
        {
            FString Target = DinosaurHuntTargets[FMath::RandRange(0, DinosaurHuntTargets.Num() - 1)];
            int32 Count = FMath::RandRange(1, 3);
            NewQuest = CreateHuntQuest(Target, Count);
            break;
        }
        case EQuest_SurvivalQuestType::Gather_Resources:
        {
            FString Resource = ResourceGatherTargets[FMath::RandRange(0, ResourceGatherTargets.Num() - 1)];
            int32 Amount = FMath::RandRange(5, 20);
            NewQuest = CreateGatherQuest(Resource, Amount);
            break;
        }
        case EQuest_SurvivalQuestType::Build_Shelter:
        {
            NewQuest.ObjectiveDescription = TEXT("Build a shelter to protect from the elements");
            NewQuest.TargetAmount = 1;
            NewQuest.RewardExperience = 50.0f;
            break;
        }
        case EQuest_SurvivalQuestType::Survive_Night:
        {
            float Duration = FMath::RandRange(300.0f, 600.0f); // 5-10 minutes
            NewQuest = CreateSurvivalQuest(Duration);
            break;
        }
        case EQuest_SurvivalQuestType::Explore_Territory:
        {
            NewQuest.ObjectiveDescription = TEXT("Explore new territory and discover landmarks");
            NewQuest.TargetAmount = FMath::RandRange(3, 7);
            NewQuest.RewardExperience = 30.0f;
            break;
        }
        case EQuest_SurvivalQuestType::Craft_Tools:
        {
            NewQuest.ObjectiveDescription = TEXT("Craft essential survival tools");
            NewQuest.TargetAmount = FMath::RandRange(2, 5);
            NewQuest.RewardExperience = 25.0f;
            break;
        }
    }

    // Apply difficulty multiplier
    NewQuest.RewardExperience *= SurvivalDifficultyMultiplier;

    ActiveQuests.Add(NewQuest);
    UE_LOG(LogTemp, Log, TEXT("Quest_SurvivalQuestManager: Generated new quest: %s"), *NewQuest.ObjectiveDescription);
}

void AQuest_SurvivalQuestManager::UpdateQuestProgress(EQuest_SurvivalQuestType QuestType, int32 ProgressAmount)
{
    for (FQuest_SurvivalObjective& Quest : ActiveQuests)
    {
        if (Quest.QuestType == QuestType && !Quest.bIsCompleted)
        {
            Quest.CurrentProgress = FMath::Min(Quest.CurrentProgress + ProgressAmount, Quest.TargetAmount);
            OnQuestProgressUpdated(Quest);
            UE_LOG(LogTemp, Log, TEXT("Quest_SurvivalQuestManager: Updated quest progress: %d/%d"), Quest.CurrentProgress, Quest.TargetAmount);
            break;
        }
    }
}

bool AQuest_SurvivalQuestManager::CompleteQuest(int32 QuestIndex)
{
    if (!ActiveQuests.IsValidIndex(QuestIndex))
    {
        return false;
    }

    FQuest_SurvivalObjective CompletedQuest = ActiveQuests[QuestIndex];
    CompletedQuest.bIsCompleted = true;
    
    // Move to completed quests
    CompletedQuests.Add(CompletedQuest);
    ActiveQuests.RemoveAt(QuestIndex);

    // Trigger completion event
    OnQuestCompleted(CompletedQuest);

    UE_LOG(LogTemp, Log, TEXT("Quest_SurvivalQuestManager: Completed quest: %s (Reward: %.1f XP)"), 
           *CompletedQuest.ObjectiveDescription, CompletedQuest.RewardExperience);

    return true;
}

TArray<FQuest_SurvivalObjective> AQuest_SurvivalQuestManager::GetActiveQuests() const
{
    return ActiveQuests;
}

int32 AQuest_SurvivalQuestManager::GetActiveQuestCount() const
{
    return ActiveQuests.Num();
}

void AQuest_SurvivalQuestManager::StartHuntQuest(const FString& DinosaurType, int32 TargetCount)
{
    if (ActiveQuests.Num() >= MaxActiveQuests)
    {
        return;
    }

    FQuest_SurvivalObjective HuntQuest = CreateHuntQuest(DinosaurType, TargetCount);
    ActiveQuests.Add(HuntQuest);
    UE_LOG(LogTemp, Log, TEXT("Quest_SurvivalQuestManager: Started hunt quest for %s (x%d)"), *DinosaurType, TargetCount);
}

void AQuest_SurvivalQuestManager::StartGatherQuest(const FString& ResourceType, int32 TargetAmount)
{
    if (ActiveQuests.Num() >= MaxActiveQuests)
    {
        return;
    }

    FQuest_SurvivalObjective GatherQuest = CreateGatherQuest(ResourceType, TargetAmount);
    ActiveQuests.Add(GatherQuest);
    UE_LOG(LogTemp, Log, TEXT("Quest_SurvivalQuestManager: Started gather quest for %s (x%d)"), *ResourceType, TargetAmount);
}

void AQuest_SurvivalQuestManager::StartSurvivalChallenge(float Duration)
{
    if (ActiveQuests.Num() >= MaxActiveQuests)
    {
        return;
    }

    FQuest_SurvivalObjective SurvivalQuest = CreateSurvivalQuest(Duration);
    ActiveQuests.Add(SurvivalQuest);
    UE_LOG(LogTemp, Log, TEXT("Quest_SurvivalQuestManager: Started survival challenge (%.1f seconds)"), Duration);
}

void AQuest_SurvivalQuestManager::ResetAllQuests()
{
    ActiveQuests.Empty();
    CompletedQuests.Empty();
    InitializeDefaultQuests();
    UE_LOG(LogTemp, Log, TEXT("Quest_SurvivalQuestManager: All quests reset"));
}

void AQuest_SurvivalQuestManager::InitializeDefaultQuests()
{
    // Add starter quests
    FQuest_SurvivalObjective StarterGather;
    StarterGather.ObjectiveDescription = TEXT("Gather basic resources to survive");
    StarterGather.QuestType = EQuest_SurvivalQuestType::Gather_Resources;
    StarterGather.TargetAmount = 10;
    StarterGather.CurrentProgress = 0;
    StarterGather.bIsCompleted = false;
    StarterGather.RewardExperience = 20.0f;
    ActiveQuests.Add(StarterGather);

    FQuest_SurvivalObjective StarterCraft;
    StarterCraft.ObjectiveDescription = TEXT("Craft your first tool");
    StarterCraft.QuestType = EQuest_SurvivalQuestType::Craft_Tools;
    StarterCraft.TargetAmount = 1;
    StarterCraft.CurrentProgress = 0;
    StarterCraft.bIsCompleted = false;
    StarterCraft.RewardExperience = 15.0f;
    ActiveQuests.Add(StarterCraft);
}

FQuest_SurvivalObjective AQuest_SurvivalQuestManager::CreateHuntQuest(const FString& Target, int32 Count)
{
    FQuest_SurvivalObjective HuntQuest;
    HuntQuest.ObjectiveDescription = FString::Printf(TEXT("Hunt %d %s"), Count, *Target);
    HuntQuest.QuestType = EQuest_SurvivalQuestType::Hunt_Dinosaur;
    HuntQuest.TargetAmount = Count;
    HuntQuest.CurrentProgress = 0;
    HuntQuest.bIsCompleted = false;
    HuntQuest.RewardExperience = Count * 40.0f; // 40 XP per kill
    return HuntQuest;
}

FQuest_SurvivalObjective AQuest_SurvivalQuestManager::CreateGatherQuest(const FString& Resource, int32 Amount)
{
    FQuest_SurvivalObjective GatherQuest;
    GatherQuest.ObjectiveDescription = FString::Printf(TEXT("Gather %d %s"), Amount, *Resource);
    GatherQuest.QuestType = EQuest_SurvivalQuestType::Gather_Resources;
    GatherQuest.TargetAmount = Amount;
    GatherQuest.CurrentProgress = 0;
    GatherQuest.bIsCompleted = false;
    GatherQuest.RewardExperience = Amount * 2.0f; // 2 XP per resource
    return GatherQuest;
}

FQuest_SurvivalObjective AQuest_SurvivalQuestManager::CreateSurvivalQuest(float Duration)
{
    FQuest_SurvivalObjective SurvivalQuest;
    SurvivalQuest.ObjectiveDescription = FString::Printf(TEXT("Survive for %.1f minutes"), Duration / 60.0f);
    SurvivalQuest.QuestType = EQuest_SurvivalQuestType::Survive_Night;
    SurvivalQuest.TargetAmount = static_cast<int32>(Duration);
    SurvivalQuest.CurrentProgress = 0;
    SurvivalQuest.bIsCompleted = false;
    SurvivalQuest.RewardExperience = Duration * 0.5f; // 0.5 XP per second survived
    return SurvivalQuest;
}