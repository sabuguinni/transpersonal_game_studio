#include "Quest_ExplorationQuestManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UQuest_ExplorationQuestManager::UQuest_ExplorationQuestManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PlayerDetectionRadius = 300.0f;
    CompletedExplorationCount = 0;
    bAutoGenerateQuests = true;
}

void UQuest_ExplorationQuestManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoGenerateQuests)
    {
        // Generate initial exploration quests
        GenerateRandomExplorationQuest();
        GenerateRandomExplorationQuest();
    }
}

void UQuest_ExplorationQuestManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Get player location
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Check exploration progress
    CheckExplorationProgress(PlayerLocation);
}

void UQuest_ExplorationQuestManager::CreateExplorationQuest(const FString& LocationName, const FVector& TargetLocation, float Radius)
{
    FQuest_ExplorationObjective NewQuest;
    NewQuest.LocationName = LocationName;
    NewQuest.TargetLocation = TargetLocation;
    NewQuest.ExplorationRadius = Radius;
    NewQuest.bIsCompleted = false;
    NewQuest.DiscoveryReward = TEXT("Territory Knowledge");

    ActiveExplorationQuests.Add(NewQuest);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("New exploration quest: Discover %s"), *LocationName));
    }
}

bool UQuest_ExplorationQuestManager::CheckExplorationProgress(const FVector& PlayerLocation)
{
    bool bProgressMade = false;

    for (int32 i = 0; i < ActiveExplorationQuests.Num(); i++)
    {
        FQuest_ExplorationObjective& Quest = ActiveExplorationQuests[i];
        
        if (!Quest.bIsCompleted)
        {
            float DistanceToTarget = FVector::Dist(PlayerLocation, Quest.TargetLocation);
            
            if (DistanceToTarget <= Quest.ExplorationRadius)
            {
                CompleteExplorationQuest(i);
                bProgressMade = true;
            }
        }
    }

    return bProgressMade;
}

void UQuest_ExplorationQuestManager::CompleteExplorationQuest(int32 QuestIndex)
{
    if (QuestIndex >= 0 && QuestIndex < ActiveExplorationQuests.Num())
    {
        FQuest_ExplorationObjective& Quest = ActiveExplorationQuests[QuestIndex];
        
        if (!Quest.bIsCompleted)
        {
            Quest.bIsCompleted = true;
            CompletedExplorationCount++;

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Yellow, 
                    FString::Printf(TEXT("Exploration Complete! Discovered: %s - Reward: %s"), 
                    *Quest.LocationName, *Quest.DiscoveryReward));
            }

            // Generate new quest after completion
            if (bAutoGenerateQuests && CompletedExplorationCount % 2 == 0)
            {
                GenerateRandomExplorationQuest();
            }
        }
    }
}

TArray<FQuest_ExplorationObjective> UQuest_ExplorationQuestManager::GetActiveQuests() const
{
    return ActiveExplorationQuests;
}

void UQuest_ExplorationQuestManager::GenerateRandomExplorationQuest()
{
    // Generate random exploration locations around the map
    TArray<FString> LocationNames = {
        TEXT("Ancient Cave System"),
        TEXT("Dinosaur Nesting Grounds"),
        TEXT("Crystal Formation"),
        TEXT("Volcanic Hot Springs"),
        TEXT("Dense Jungle Grove"),
        TEXT("Rocky Cliff Overlook"),
        TEXT("Riverbank Settlement"),
        TEXT("Bone Graveyard")
    };

    TArray<FString> Rewards = {
        TEXT("Stone Tools"),
        TEXT("Rare Minerals"),
        TEXT("Fresh Water Source"),
        TEXT("Safe Shelter Location"),
        TEXT("Hunting Ground Access"),
        TEXT("Medicinal Plants"),
        TEXT("Crafting Materials"),
        TEXT("Territory Map")
    };

    // Random location within reasonable map bounds
    FVector RandomLocation = FVector(
        FMath::RandRange(-5000.0f, 5000.0f),
        FMath::RandRange(-5000.0f, 5000.0f),
        FMath::RandRange(0.0f, 1000.0f)
    );

    FString RandomLocationName = LocationNames[FMath::RandRange(0, LocationNames.Num() - 1)];
    FString RandomReward = Rewards[FMath::RandRange(0, Rewards.Num() - 1)];

    FQuest_ExplorationObjective NewQuest;
    NewQuest.LocationName = RandomLocationName;
    NewQuest.TargetLocation = RandomLocation;
    NewQuest.ExplorationRadius = FMath::RandRange(400.0f, 800.0f);
    NewQuest.bIsCompleted = false;
    NewQuest.DiscoveryReward = RandomReward;

    ActiveExplorationQuests.Add(NewQuest);
}

FString UQuest_ExplorationQuestManager::GetQuestProgressText() const
{
    int32 ActiveCount = 0;
    for (const FQuest_ExplorationObjective& Quest : ActiveExplorationQuests)
    {
        if (!Quest.bIsCompleted)
        {
            ActiveCount++;
        }
    }

    return FString::Printf(TEXT("Exploration Progress: %d Active Quests | %d Completed"), 
        ActiveCount, CompletedExplorationCount);
}