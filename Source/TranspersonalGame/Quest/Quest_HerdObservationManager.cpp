#include "Quest_HerdObservationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

AQuest_HerdObservationManager::AQuest_HerdObservationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    PlayerDetectionRange = 2000.0f;
    TaskUpdateInterval = 2.0f;
    PlayerCharacter = nullptr;
    CompletedTaskCount = 0;
    TotalObservationTime = 0.0f;
    LastUpdateTime = 0.0f;
    
    SetActorTickEnabled(true);
}

void AQuest_HerdObservationManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeLocations();
    
    // Find player character
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerCharacter = PC->GetPawn();
        }
    }
    
    // Set up timer for task updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(TaskUpdateTimer, this, 
            &AQuest_HerdObservationManager::UpdateTaskProgress, 
            TaskUpdateInterval, true);
    }
    
    // Start with basic observation quest
    StartBasicObservationQuest();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_HerdObservationManager initialized with %d biomes"), BiomeLocations.Num());
}

void AQuest_HerdObservationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    TotalObservationTime += DeltaTime;
    
    // Update task progress for time-based observations
    for (int32 i = 0; i < ActiveTasks.Num(); i++)
    {
        if (!ActiveTasks[i].bIsCompleted && ActiveTasks[i].ObservationType == EQuest_ObservationType::StudyBehavior)
        {
            if (PlayerCharacter)
            {
                FVector BiomeCenter;
                if (IsPlayerInBiome(ActiveTasks[i].TargetBiome, BiomeCenter))
                {
                    TArray<AActor*> NearbyDinosaurs = FindDinosaursInRange(
                        PlayerCharacter->GetActorLocation(), 
                        PlayerDetectionRange, 
                        ActiveTasks[i].TargetSpecies
                    );
                    
                    if (NearbyDinosaurs.Num() > 0)
                    {
                        ActiveTasks[i].ProgressPercent += (DeltaTime / ActiveTasks[i].ObservationTime) * 100.0f;
                        
                        if (ActiveTasks[i].ProgressPercent >= 100.0f)
                        {
                            CompleteTask(i);
                        }
                    }
                }
            }
        }
    }
}

void AQuest_HerdObservationManager::InitializeBiomeLocations()
{
    BiomeLocations.Empty();
    
    // Savana (0,0)
    FQuest_BiomeLocation Savana;
    Savana.BiomeType = EQuest_BiomeType::Savana;
    Savana.CenterLocation = FVector(0.0f, 0.0f, 100.0f);
    Savana.Radius = 15000.0f;
    BiomeLocations.Add(Savana);
    
    // Pantano (-50000,-45000)
    FQuest_BiomeLocation Pantano;
    Pantano.BiomeType = EQuest_BiomeType::Pantano;
    Pantano.CenterLocation = FVector(-50000.0f, -45000.0f, 100.0f);
    Pantano.Radius = 15000.0f;
    BiomeLocations.Add(Pantano);
    
    // Floresta (-45000,40000)
    FQuest_BiomeLocation Floresta;
    Floresta.BiomeType = EQuest_BiomeType::Floresta;
    Floresta.CenterLocation = FVector(-45000.0f, 40000.0f, 100.0f);
    Floresta.Radius = 15000.0f;
    BiomeLocations.Add(Floresta);
    
    // Deserto (55000,0)
    FQuest_BiomeLocation Deserto;
    Deserto.BiomeType = EQuest_BiomeType::Deserto;
    Deserto.CenterLocation = FVector(55000.0f, 0.0f, 100.0f);
    Deserto.Radius = 15000.0f;
    BiomeLocations.Add(Deserto);
    
    // Montanha (40000,50000)
    FQuest_BiomeLocation Montanha;
    Montanha.BiomeType = EQuest_BiomeType::Montanha;
    Montanha.CenterLocation = FVector(40000.0f, 50000.0f, 100.0f);
    Montanha.Radius = 15000.0f;
    BiomeLocations.Add(Montanha);
}

void AQuest_HerdObservationManager::CreateObservationTask(const FString& TaskName, 
    EQuest_ObservationType ObsType, EQuest_BiomeType TargetBiome, 
    const FString& Species, int32 Count, float Duration)
{
    FQuest_ObservationTask NewTask;
    NewTask.TaskName = TaskName;
    NewTask.ObservationType = ObsType;
    NewTask.TargetBiome = TargetBiome;
    NewTask.TargetSpecies = Species;
    NewTask.RequiredCount = Count;
    NewTask.ObservationTime = Duration;
    NewTask.ProgressPercent = 0.0f;
    NewTask.bIsCompleted = false;
    
    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Created observation task: %s in biome %d"), 
        *TaskName, (int32)TargetBiome);
}

void AQuest_HerdObservationManager::UpdateTaskProgress()
{
    if (!PlayerCharacter)
    {
        return;
    }
    
    for (int32 i = 0; i < ActiveTasks.Num(); i++)
    {
        if (ActiveTasks[i].bIsCompleted)
        {
            continue;
        }
        
        FVector BiomeCenter;
        if (IsPlayerInBiome(ActiveTasks[i].TargetBiome, BiomeCenter))
        {
            TArray<AActor*> NearbyDinosaurs = FindDinosaursInRange(
                PlayerCharacter->GetActorLocation(), 
                PlayerDetectionRange, 
                ActiveTasks[i].TargetSpecies
            );
            
            switch (ActiveTasks[i].ObservationType)
            {
                case EQuest_ObservationType::CountAnimals:
                {
                    if (NearbyDinosaurs.Num() >= ActiveTasks[i].RequiredCount)
                    {
                        ActiveTasks[i].ProgressPercent = 100.0f;
                        CompleteTask(i);
                    }
                    else
                    {
                        ActiveTasks[i].ProgressPercent = (float(NearbyDinosaurs.Num()) / float(ActiveTasks[i].RequiredCount)) * 100.0f;
                    }
                    break;
                }
                case EQuest_ObservationType::WatchFeeding:
                {
                    if (NearbyDinosaurs.Num() > 0)
                    {
                        ActiveTasks[i].ProgressPercent += 10.0f;
                        if (ActiveTasks[i].ProgressPercent >= 100.0f)
                        {
                            CompleteTask(i);
                        }
                    }
                    break;
                }
                case EQuest_ObservationType::TrackMovement:
                {
                    if (NearbyDinosaurs.Num() > 0)
                    {
                        ActiveTasks[i].ProgressPercent += 5.0f;
                        if (ActiveTasks[i].ProgressPercent >= 100.0f)
                        {
                            CompleteTask(i);
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
}

bool AQuest_HerdObservationManager::IsPlayerInBiome(EQuest_BiomeType BiomeType, FVector& OutBiomeCenter)
{
    if (!PlayerCharacter)
    {
        return false;
    }
    
    for (const FQuest_BiomeLocation& Biome : BiomeLocations)
    {
        if (Biome.BiomeType == BiomeType)
        {
            OutBiomeCenter = Biome.CenterLocation;
            float Distance = FVector::Dist(PlayerCharacter->GetActorLocation(), Biome.CenterLocation);
            return Distance <= Biome.Radius;
        }
    }
    
    return false;
}

TArray<AActor*> AQuest_HerdObservationManager::FindDinosaursInRange(const FVector& Location, 
    float Range, const FString& SpeciesFilter)
{
    TArray<AActor*> FoundDinosaurs;
    
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        for (AActor* Actor : AllActors)
        {
            if (!Actor)
            {
                continue;
            }
            
            FString ActorName = Actor->GetName();
            
            // Check if this is a dinosaur actor (contains species name)
            if (SpeciesFilter.IsEmpty() || ActorName.Contains(SpeciesFilter) || 
                ActorName.Contains(TEXT("Trex")) || ActorName.Contains(TEXT("Raptor")) ||
                ActorName.Contains(TEXT("Tricera")) || ActorName.Contains(TEXT("Brachio")) ||
                ActorName.Contains(TEXT("Ankylo")))
            {
                float Distance = FVector::Dist(Location, Actor->GetActorLocation());
                if (Distance <= Range)
                {
                    FoundDinosaurs.Add(Actor);
                }
            }
        }
    }
    
    return FoundDinosaurs;
}

void AQuest_HerdObservationManager::CompleteTask(int32 TaskIndex)
{
    if (TaskIndex >= 0 && TaskIndex < ActiveTasks.Num())
    {
        ActiveTasks[TaskIndex].bIsCompleted = true;
        ActiveTasks[TaskIndex].ProgressPercent = 100.0f;
        CompletedTaskCount++;
        
        UE_LOG(LogTemp, Warning, TEXT("Quest task completed: %s"), *ActiveTasks[TaskIndex].TaskName);
        
        // Print completion message
        if (GEngine)
        {
            FString Message = FString::Printf(TEXT("QUEST COMPLETED: %s"), *ActiveTasks[TaskIndex].TaskName);
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, Message);
        }
    }
}

FString AQuest_HerdObservationManager::GetTaskStatusString(int32 TaskIndex)
{
    if (TaskIndex >= 0 && TaskIndex < ActiveTasks.Num())
    {
        const FQuest_ObservationTask& Task = ActiveTasks[TaskIndex];
        if (Task.bIsCompleted)
        {
            return FString::Printf(TEXT("%s - COMPLETED"), *Task.TaskName);
        }
        else
        {
            return FString::Printf(TEXT("%s - %.1f%% Complete"), *Task.TaskName, Task.ProgressPercent);
        }
    }
    
    return TEXT("Invalid Task");
}

void AQuest_HerdObservationManager::StartBasicObservationQuest()
{
    CreateObservationTask(TEXT("Count Triceratops in Savana"), 
        EQuest_ObservationType::CountAnimals, 
        EQuest_BiomeType::Savana, 
        TEXT("Tricera"), 2, 0.0f);
        
    CreateObservationTask(TEXT("Observe Brachiosaurus Feeding"), 
        EQuest_ObservationType::WatchFeeding, 
        EQuest_BiomeType::Floresta, 
        TEXT("Brachio"), 1, 30.0f);
}

void AQuest_HerdObservationManager::StartAdvancedTrackingQuest()
{
    CreateObservationTask(TEXT("Track Velociraptor Pack Movement"), 
        EQuest_ObservationType::TrackMovement, 
        EQuest_BiomeType::Pantano, 
        TEXT("Raptor"), 3, 0.0f);
        
    CreateObservationTask(TEXT("Study Ankylosaurus Behavior"), 
        EQuest_ObservationType::StudyBehavior, 
        EQuest_BiomeType::Deserto, 
        TEXT("Ankylo"), 1, 60.0f);
}

void AQuest_HerdObservationManager::StartSurvivalObservationQuest()
{
    CreateObservationTask(TEXT("Avoid T-Rex While Observing"), 
        EQuest_ObservationType::AvoidPredator, 
        EQuest_BiomeType::Montanha, 
        TEXT("Trex"), 1, 45.0f);
        
    CreateObservationTask(TEXT("Count All Species in Pantano"), 
        EQuest_ObservationType::CountAnimals, 
        EQuest_BiomeType::Pantano, 
        TEXT(""), 5, 0.0f);
}

int32 AQuest_HerdObservationManager::GetActiveTaskCount() const
{
    int32 ActiveCount = 0;
    for (const FQuest_ObservationTask& Task : ActiveTasks)
    {
        if (!Task.bIsCompleted)
        {
            ActiveCount++;
        }
    }
    return ActiveCount;
}

float AQuest_HerdObservationManager::GetQuestCompletionPercent() const
{
    if (ActiveTasks.Num() == 0)
    {
        return 0.0f;
    }
    
    return (float(CompletedTaskCount) / float(ActiveTasks.Num())) * 100.0f;
}