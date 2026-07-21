#include "Quest_SurvivalChallenge.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

AQuest_SurvivalChallenge::AQuest_SurvivalChallenge()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create quest marker mesh
    QuestMarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("QuestMarkerMesh"));
    QuestMarkerMesh->SetupAttachment(RootComponent);

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(500.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize quest properties
    QuestName = TEXT("Survival Challenge");
    QuestDescription = TEXT("Survive in the prehistoric wilderness by gathering resources, crafting tools, and avoiding dangers");
    QuestTimeLimit = 600.0f; // 10 minutes
    bIsQuestActive = false;
    bIsQuestCompleted = false;
    RemainingTime = QuestTimeLimit;
    TargetBiome = EQuest_BiomeType::Savana;

    // Initialize survival tracking
    StonesGathered = 0;
    SticksGathered = 0;
    BerriesGathered = 0;
    WaterCollected = 0;
    ToolsCrafted = 0;
    SheltersBuilt = 0;
    FiresLit = 0;
    PredatorsAvoided = 0;
    DangersEscaped = 0;
    HealthMaintained = 100.0f;
    StaminaMaintained = 100.0f;

    // Setup basic survival quest by default
    SetupBasicSurvivalQuest();
}

void AQuest_SurvivalChallenge::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (InteractionSphere)
    {
        InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_SurvivalChallenge::OnInteractionSphereBeginOverlap);
    }

    // Set quest marker color based on quest state
    if (QuestMarkerMesh && QuestMarkerMesh->GetStaticMesh())
    {
        // Quest marker will be yellow for available, blue for active, green for completed
        // This would typically be done with materials in a real implementation
    }
}

void AQuest_SurvivalChallenge::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsQuestActive && !bIsQuestCompleted)
    {
        UpdateQuestTimer();
        CheckTaskCompletion();
    }
}

void AQuest_SurvivalChallenge::StartQuest()
{
    if (bIsQuestActive || bIsQuestCompleted)
    {
        return;
    }

    bIsQuestActive = true;
    RemainingTime = QuestTimeLimit;

    // Start quest timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(QuestTimerHandle, this, &AQuest_SurvivalChallenge::OnQuestTimerExpired, QuestTimeLimit, false);
        GetWorld()->GetTimerManager().SetTimer(UpdateTimerHandle, this, &AQuest_SurvivalChallenge::UpdateQuestTimer, 1.0f, true);
    }

    // Log quest start
    UE_LOG(LogTemp, Warning, TEXT("Quest Started: %s"), *QuestName);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
            FString::Printf(TEXT("Quest Started: %s - Time Limit: %.0f seconds"), *QuestName, QuestTimeLimit));
    }
}

void AQuest_SurvivalChallenge::CompleteQuest()
{
    if (!bIsQuestActive || bIsQuestCompleted)
    {
        return;
    }

    bIsQuestCompleted = true;
    bIsQuestActive = false;

    // Clear timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(QuestTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }

    // Spawn rewards
    SpawnQuestRewards();

    // Log quest completion
    UE_LOG(LogTemp, Warning, TEXT("Quest Completed: %s"), *QuestName);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Quest Completed: %s!"), *QuestName));
    }
}

void AQuest_SurvivalChallenge::FailQuest()
{
    if (!bIsQuestActive || bIsQuestCompleted)
    {
        return;
    }

    bIsQuestActive = false;

    // Clear timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(QuestTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }

    // Log quest failure
    UE_LOG(LogTemp, Warning, TEXT("Quest Failed: %s"), *QuestName);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
            FString::Printf(TEXT("Quest Failed: %s"), *QuestName));
    }
}

void AQuest_SurvivalChallenge::UpdateTaskProgress(EQuest_TaskType TaskType, int32 Amount)
{
    if (!bIsQuestActive)
    {
        return;
    }

    for (FQuest_SurvivalTask& Task : SurvivalTasks)
    {
        if (Task.TaskType == TaskType && !Task.bIsCompleted)
        {
            Task.CurrentCount = FMath::Min(Task.CurrentCount + Amount, Task.RequiredCount);
            
            if (Task.CurrentCount >= Task.RequiredCount)
            {
                Task.bIsCompleted = true;
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                        FString::Printf(TEXT("Task Completed: %s"), *Task.TaskName));
                }
            }
            break;
        }
    }

    CheckQuestCompletion();
}

bool AQuest_SurvivalChallenge::CheckQuestCompletion()
{
    if (!bIsQuestActive)
    {
        return false;
    }

    bool bAllTasksCompleted = true;
    for (const FQuest_SurvivalTask& Task : SurvivalTasks)
    {
        if (!Task.bIsCompleted)
        {
            bAllTasksCompleted = false;
            break;
        }
    }

    if (bAllTasksCompleted)
    {
        CompleteQuest();
        return true;
    }

    return false;
}

void AQuest_SurvivalChallenge::RegisterResourceGathering(EQuest_ResourceType ResourceType, int32 Amount)
{
    if (!bIsQuestActive)
    {
        return;
    }

    switch (ResourceType)
    {
        case EQuest_ResourceType::Stone:
            StonesGathered += Amount;
            UpdateTaskProgress(EQuest_TaskType::Gather, Amount);
            break;
        case EQuest_ResourceType::Wood:
            SticksGathered += Amount;
            UpdateTaskProgress(EQuest_TaskType::Gather, Amount);
            break;
        case EQuest_ResourceType::Food:
            BerriesGathered += Amount;
            UpdateTaskProgress(EQuest_TaskType::Gather, Amount);
            break;
        case EQuest_ResourceType::Water:
            WaterCollected += Amount;
            UpdateTaskProgress(EQuest_TaskType::Gather, Amount);
            break;
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, 
            FString::Printf(TEXT("Resource Gathered: %d"), (int32)ResourceType));
    }
}

void AQuest_SurvivalChallenge::RegisterCraftingAction(EQuest_CraftingType CraftType)
{
    if (!bIsQuestActive)
    {
        return;
    }

    switch (CraftType)
    {
        case EQuest_CraftingType::Tool:
            ToolsCrafted++;
            UpdateTaskProgress(EQuest_TaskType::Craft, 1);
            break;
        case EQuest_CraftingType::Shelter:
            SheltersBuilt++;
            UpdateTaskProgress(EQuest_TaskType::Build, 1);
            break;
        case EQuest_CraftingType::Fire:
            FiresLit++;
            UpdateTaskProgress(EQuest_TaskType::Build, 1);
            break;
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, 
            FString::Printf(TEXT("Item Crafted: %d"), (int32)CraftType));
    }
}

void AQuest_SurvivalChallenge::RegisterSurvivalAction(EQuest_SurvivalAction ActionType)
{
    if (!bIsQuestActive)
    {
        return;
    }

    switch (ActionType)
    {
        case EQuest_SurvivalAction::AvoidPredator:
            PredatorsAvoided++;
            UpdateTaskProgress(EQuest_TaskType::Survive, 1);
            break;
        case EQuest_SurvivalAction::EscapeDanger:
            DangersEscaped++;
            UpdateTaskProgress(EQuest_TaskType::Survive, 1);
            break;
        case EQuest_SurvivalAction::MaintainHealth:
            UpdateTaskProgress(EQuest_TaskType::Survive, 1);
            break;
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Purple, 
            FString::Printf(TEXT("Survival Action: %d"), (int32)ActionType));
    }
}

FString AQuest_SurvivalChallenge::GetQuestProgressText() const
{
    FString ProgressText = FString::Printf(TEXT("Quest: %s\nTime Remaining: %.0f seconds\n\nTasks:\n"), *QuestName, RemainingTime);

    for (const FQuest_SurvivalTask& Task : SurvivalTasks)
    {
        FString StatusText = Task.bIsCompleted ? TEXT("[COMPLETE]") : TEXT("[ACTIVE]");
        ProgressText += FString::Printf(TEXT("%s %s (%d/%d)\n"), *StatusText, *Task.TaskName, Task.CurrentCount, Task.RequiredCount);
    }

    return ProgressText;
}

float AQuest_SurvivalChallenge::GetQuestCompletionPercentage() const
{
    if (SurvivalTasks.Num() == 0)
    {
        return 0.0f;
    }

    int32 CompletedTasks = 0;
    for (const FQuest_SurvivalTask& Task : SurvivalTasks)
    {
        if (Task.bIsCompleted)
        {
            CompletedTasks++;
        }
    }

    return (float)CompletedTasks / (float)SurvivalTasks.Num() * 100.0f;
}

void AQuest_SurvivalChallenge::SetupBasicSurvivalQuest()
{
    SurvivalTasks.Empty();

    // Task 1: Gather basic resources
    FQuest_SurvivalTask GatherTask;
    GatherTask.TaskName = TEXT("Gather Resources");
    GatherTask.TaskType = EQuest_TaskType::Gather;
    GatherTask.RequiredCount = 5;
    GatherTask.TimeLimit = 300.0f;
    SurvivalTasks.Add(GatherTask);

    // Task 2: Craft a tool
    FQuest_SurvivalTask CraftTask;
    CraftTask.TaskName = TEXT("Craft Stone Tool");
    CraftTask.TaskType = EQuest_TaskType::Craft;
    CraftTask.RequiredCount = 1;
    CraftTask.TimeLimit = 200.0f;
    SurvivalTasks.Add(CraftTask);

    // Task 3: Survive dangers
    FQuest_SurvivalTask SurviveTask;
    SurviveTask.TaskName = TEXT("Avoid Predators");
    SurviveTask.TaskType = EQuest_TaskType::Survive;
    SurviveTask.RequiredCount = 2;
    SurviveTask.TimeLimit = 400.0f;
    SurvivalTasks.Add(SurviveTask);

    QuestTimeLimit = 600.0f; // 10 minutes total
    QuestDescription = TEXT("Learn basic survival by gathering resources, crafting tools, and avoiding dangers");
}

void AQuest_SurvivalChallenge::SetupAdvancedSurvivalQuest()
{
    SurvivalTasks.Empty();

    // Task 1: Gather diverse resources
    FQuest_SurvivalTask GatherTask;
    GatherTask.TaskName = TEXT("Gather Diverse Resources");
    GatherTask.TaskType = EQuest_TaskType::Gather;
    GatherTask.RequiredCount = 10;
    GatherTask.TimeLimit = 400.0f;
    SurvivalTasks.Add(GatherTask);

    // Task 2: Build shelter
    FQuest_SurvivalTask BuildTask;
    BuildTask.TaskName = TEXT("Build Shelter");
    BuildTask.TaskType = EQuest_TaskType::Build;
    BuildTask.RequiredCount = 1;
    BuildTask.TimeLimit = 300.0f;
    SurvivalTasks.Add(BuildTask);

    // Task 3: Craft multiple tools
    FQuest_SurvivalTask CraftTask;
    CraftTask.TaskName = TEXT("Craft Advanced Tools");
    CraftTask.TaskType = EQuest_TaskType::Craft;
    CraftTask.RequiredCount = 3;
    CraftTask.TimeLimit = 350.0f;
    SurvivalTasks.Add(CraftTask);

    // Task 4: Extended survival
    FQuest_SurvivalTask SurviveTask;
    SurviveTask.TaskName = TEXT("Extended Survival");
    SurviveTask.TaskType = EQuest_TaskType::Survive;
    SurviveTask.RequiredCount = 5;
    SurviveTask.TimeLimit = 600.0f;
    SurvivalTasks.Add(SurviveTask);

    QuestTimeLimit = 900.0f; // 15 minutes
    QuestDescription = TEXT("Master advanced survival techniques in the harsh prehistoric world");
}

void AQuest_SurvivalChallenge::SetupEmergencySurvivalQuest()
{
    SurvivalTasks.Empty();

    // Task 1: Quick resource gathering
    FQuest_SurvivalTask QuickGatherTask;
    QuickGatherTask.TaskName = TEXT("Emergency Resource Gathering");
    QuickGatherTask.TaskType = EQuest_TaskType::Gather;
    QuickGatherTask.RequiredCount = 3;
    QuickGatherTask.TimeLimit = 120.0f;
    SurvivalTasks.Add(QuickGatherTask);

    // Task 2: Immediate shelter
    FQuest_SurvivalTask ShelterTask;
    ShelterTask.TaskName = TEXT("Emergency Shelter");
    ShelterTask.TaskType = EQuest_TaskType::Build;
    ShelterTask.RequiredCount = 1;
    ShelterTask.TimeLimit = 180.0f;
    SurvivalTasks.Add(ShelterTask);

    // Task 3: Escape predators
    FQuest_SurvivalTask EscapeTask;
    EscapeTask.TaskName = TEXT("Escape Immediate Danger");
    EscapeTask.TaskType = EQuest_TaskType::Survive;
    EscapeTask.RequiredCount = 1;
    EscapeTask.TimeLimit = 300.0f;
    SurvivalTasks.Add(EscapeTask);

    QuestTimeLimit = 300.0f; // 5 minutes - emergency!
    QuestDescription = TEXT("Emergency survival situation - act fast to survive immediate threats!");
}

void AQuest_SurvivalChallenge::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor))
    {
        if (!bIsQuestActive && !bIsQuestCompleted)
        {
            // Auto-start quest when player approaches
            StartQuest();
        }
        else if (bIsQuestActive)
        {
            // Show quest progress
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, GetQuestProgressText());
            }
        }
    }
}

void AQuest_SurvivalChallenge::OnQuestTimerExpired()
{
    FailQuest();
}

void AQuest_SurvivalChallenge::UpdateQuestTimer()
{
    if (bIsQuestActive && !bIsQuestCompleted)
    {
        RemainingTime = FMath::Max(0.0f, RemainingTime - 1.0f);
        
        if (RemainingTime <= 0.0f)
        {
            OnQuestTimerExpired();
        }
    }
}

void AQuest_SurvivalChallenge::CheckTaskCompletion()
{
    // This method is called every tick to check for automatic task completion
    // based on player stats and world state
    
    // Example: Check if player health is maintained above threshold
    // This would integrate with the character's health system
    
    // Example: Check if player is near predators (for avoidance tasks)
    // This would integrate with the AI system
}

void AQuest_SurvivalChallenge::SpawnQuestRewards()
{
    // Spawn reward items at quest location
    // This could include crafted tools, food, or other survival items
    
    if (GetWorld())
    {
        FVector SpawnLocation = GetActorLocation() + FVector(0, 0, 100);
        
        // In a full implementation, this would spawn actual reward actors
        UE_LOG(LogTemp, Warning, TEXT("Quest rewards spawned at location: %s"), *SpawnLocation.ToString());
    }
}