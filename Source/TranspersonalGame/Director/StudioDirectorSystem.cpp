#include "StudioDirectorSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Landscape/Landscape.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"

UStudioDirectorComponent::UStudioDirectorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bAutoUpdateMetrics = true;
    MetricsUpdateInterval = 5.0f;
    LastMetricsUpdate = 0.0f;
}

void UStudioDirectorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAgentTasks();
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Component initialized"));
}

void UStudioDirectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoUpdateMetrics)
    {
        LastMetricsUpdate += DeltaTime;
        if (LastMetricsUpdate >= MetricsUpdateInterval)
        {
            UpdateProductionMetrics();
            LastMetricsUpdate = 0.0f;
        }
    }
}

void UStudioDirectorComponent::UpdateProductionMetrics()
{
    if (!GetWorld())
    {
        return;
    }

    // Reset metrics
    CurrentMetrics = FDir_ProductionMetrics();
    
    // Count all actors
    CountActorsByType();
    CountActorsByBiome();
    
    // Calculate overall progress
    CurrentMetrics.OverallProgress = CalculateOverallProgress();
    
    UE_LOG(LogTemp, Log, TEXT("Studio Director: Updated production metrics - Total actors: %d"), CurrentMetrics.TotalActorsInWorld);
}

void UStudioDirectorComponent::CountActorsByType()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    CurrentMetrics.TotalActorsInWorld = 0;
    CurrentMetrics.CharacterActors = 0;
    CurrentMetrics.DinosaurActors = 0;
    CurrentMetrics.TerrainActors = 0;

    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor || Actor->IsPendingKill())
        {
            continue;
        }

        CurrentMetrics.TotalActorsInWorld++;

        // Count character actors
        if (Cast<ACharacter>(Actor))
        {
            CurrentMetrics.CharacterActors++;
        }

        // Count dinosaur actors (by label)
        FString ActorLabel = Actor->GetActorLabel();
        if (ActorLabel.Contains(TEXT("Dinosaur")) || 
            ActorLabel.Contains(TEXT("TRex")) || 
            ActorLabel.Contains(TEXT("Raptor")) ||
            ActorLabel.Contains(TEXT("Brachiosaurus")))
        {
            CurrentMetrics.DinosaurActors++;
        }

        // Count terrain actors
        if (Cast<ALandscape>(Actor) || ActorLabel.Contains(TEXT("Terrain")))
        {
            CurrentMetrics.TerrainActors++;
        }
    }
}

void UStudioDirectorComponent::CountActorsByBiome()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Biome coordinates from memory ID 709
    TMap<FString, FVector> BiomeLocations;
    BiomeLocations.Add(TEXT("Savanna"), FVector(0, 0, 0));
    BiomeLocations.Add(TEXT("Swamp"), FVector(-50000, -45000, 0));
    BiomeLocations.Add(TEXT("Forest"), FVector(-45000, 40000, 0));
    BiomeLocations.Add(TEXT("Desert"), FVector(55000, 0, 0));
    BiomeLocations.Add(TEXT("Mountains"), FVector(40000, 50000, 0));

    CurrentMetrics.BiomeActorCounts.Empty();

    for (auto& BiomePair : BiomeLocations)
    {
        FString BiomeName = BiomePair.Key;
        FVector BiomeCenter = BiomePair.Value;
        int32 BiomeActorCount = 0;

        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (!Actor || Actor->IsPendingKill())
            {
                continue;
            }

            FVector ActorLocation = Actor->GetActorLocation();
            float Distance = FVector::Dist2D(ActorLocation, BiomeCenter);
            
            // Count actors within 10km of biome center
            if (Distance < 1000000.0f) // 10km in UE units
            {
                BiomeActorCount++;
            }
        }

        CurrentMetrics.BiomeActorCounts.Add(BiomeName, BiomeActorCount);
    }
}

void UStudioDirectorComponent::InitializeAgentTasks()
{
    AgentTasks.Empty();

    // Initialize all 19 agents
    TArray<FString> AgentNames = {
        TEXT("Engine_Architect"),
        TEXT("Core_Systems_Programmer"),
        TEXT("Performance_Optimizer"),
        TEXT("Procedural_World_Generator"),
        TEXT("Environment_Artist"),
        TEXT("Architecture_Interior_Agent"),
        TEXT("Lighting_Atmosphere_Agent"),
        TEXT("Character_Artist_Agent"),
        TEXT("Animation_Agent"),
        TEXT("NPC_Behavior_Agent"),
        TEXT("Combat_Enemy_AI_Agent"),
        TEXT("Crowd_Traffic_Simulation"),
        TEXT("Quest_Mission_Designer"),
        TEXT("Narrative_Dialogue_Agent"),
        TEXT("Audio_Agent"),
        TEXT("VFX_Agent"),
        TEXT("QA_Testing_Agent"),
        TEXT("Integration_Build_Agent")
    };

    // Biome task locations
    TMap<FString, FVector> TaskLocations;
    TaskLocations.Add(TEXT("Engine_Architect"), FVector(0, 0, 500));
    TaskLocations.Add(TEXT("Core_Systems_Programmer"), FVector(5000, 0, 200));
    TaskLocations.Add(TEXT("Procedural_World_Generator"), FVector(-50000, -45000, 100));
    TaskLocations.Add(TEXT("Environment_Artist"), FVector(-45000, 40000, 100));
    TaskLocations.Add(TEXT("Architecture_Interior_Agent"), FVector(55000, 0, 100));
    TaskLocations.Add(TEXT("Lighting_Atmosphere_Agent"), FVector(40000, 50000, 100));
    TaskLocations.Add(TEXT("Character_Artist_Agent"), FVector(10000, 0, 200));
    TaskLocations.Add(TEXT("Combat_Enemy_AI_Agent"), FVector(15000, 0, 200));

    for (const FString& AgentName : AgentNames)
    {
        FDir_AgentTaskInfo TaskInfo;
        TaskInfo.AgentName = AgentName;
        TaskInfo.Status = EDir_AgentStatus::Idle;
        TaskInfo.CurrentTask = TEXT("Awaiting assignment");
        TaskInfo.ProgressPercentage = 0.0f;
        
        if (TaskLocations.Contains(AgentName))
        {
            TaskInfo.TaskLocation = TaskLocations[AgentName];
        }
        else
        {
            TaskInfo.TaskLocation = FVector::ZeroVector;
        }
        
        AgentTasks.Add(TaskInfo);
    }

    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Initialized %d agent tasks"), AgentTasks.Num());
}

void UStudioDirectorComponent::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus, const FString& TaskDescription)
{
    for (FDir_AgentTaskInfo& TaskInfo : AgentTasks)
    {
        if (TaskInfo.AgentName == AgentName)
        {
            TaskInfo.Status = NewStatus;
            TaskInfo.CurrentTask = TaskDescription;
            
            // Update progress based on status
            switch (NewStatus)
            {
                case EDir_AgentStatus::Idle:
                    TaskInfo.ProgressPercentage = 0.0f;
                    break;
                case EDir_AgentStatus::Working:
                    TaskInfo.ProgressPercentage = FMath::Clamp(TaskInfo.ProgressPercentage + 25.0f, 0.0f, 75.0f);
                    break;
                case EDir_AgentStatus::Completed:
                    TaskInfo.ProgressPercentage = 100.0f;
                    break;
                case EDir_AgentStatus::Blocked:
                case EDir_AgentStatus::Error:
                    // Progress remains unchanged for blocked/error states
                    break;
            }
            
            UE_LOG(LogTemp, Log, TEXT("Studio Director: Updated %s status to %d - %s"), 
                   *AgentName, (int32)NewStatus, *TaskDescription);
            break;
        }
    }
}

FDir_AgentTaskInfo UStudioDirectorComponent::GetAgentTaskInfo(const FString& AgentName)
{
    for (const FDir_AgentTaskInfo& TaskInfo : AgentTasks)
    {
        if (TaskInfo.AgentName == AgentName)
        {
            return TaskInfo;
        }
    }
    
    return FDir_AgentTaskInfo();
}

TArray<FString> UStudioDirectorComponent::GetBlockedAgents()
{
    TArray<FString> BlockedAgents;
    
    for (const FDir_AgentTaskInfo& TaskInfo : AgentTasks)
    {
        if (TaskInfo.Status == EDir_AgentStatus::Blocked || TaskInfo.Status == EDir_AgentStatus::Error)
        {
            BlockedAgents.Add(TaskInfo.AgentName);
        }
    }
    
    return BlockedAgents;
}

float UStudioDirectorComponent::CalculateOverallProgress()
{
    if (AgentTasks.Num() == 0)
    {
        return 0.0f;
    }

    float TotalProgress = 0.0f;
    for (const FDir_AgentTaskInfo& TaskInfo : AgentTasks)
    {
        TotalProgress += TaskInfo.ProgressPercentage;
    }
    
    return TotalProgress / AgentTasks.Num();
}

void UStudioDirectorComponent::CreateTaskMarkers()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (const FDir_AgentTaskInfo& TaskInfo : AgentTasks)
    {
        if (TaskInfo.TaskLocation != FVector::ZeroVector)
        {
            APointLight* TaskMarker = World->SpawnActor<APointLight>(TaskInfo.TaskLocation, FRotator::ZeroRotator);
            if (TaskMarker)
            {
                TaskMarker->SetActorLabel(FString::Printf(TEXT("TASK_%s"), *TaskInfo.AgentName));
                
                UPointLightComponent* LightComponent = TaskMarker->GetPointLightComponent();
                if (LightComponent)
                {
                    // Color code by agent type
                    if (TaskInfo.AgentName.Contains(TEXT("Environment")) || TaskInfo.AgentName.Contains(TEXT("World")))
                    {
                        LightComponent->SetLightColor(FLinearColor::Green);
                    }
                    else if (TaskInfo.AgentName.Contains(TEXT("Character")) || TaskInfo.AgentName.Contains(TEXT("Combat")))
                    {
                        LightComponent->SetLightColor(FLinearColor::Red);
                    }
                    else
                    {
                        LightComponent->SetLightColor(FLinearColor::Blue);
                    }
                    
                    LightComponent->SetIntensity(1000.0f);
                    LightComponent->SetAttenuationRadius(2000.0f);
                }
            }
        }
    }
}

void UStudioDirectorComponent::ValidateBiomePopulation()
{
    // Check if each biome meets the 500+ actor requirement from memory
    for (auto& BiomePair : CurrentMetrics.BiomeActorCounts)
    {
        FString BiomeName = BiomePair.Key;
        int32 ActorCount = BiomePair.Value;
        
        if (ActorCount < 500)
        {
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: %s biome has only %d actors (needs 500+)"), 
                   *BiomeName, ActorCount);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Studio Director: %s biome populated with %d actors"), 
                   *BiomeName, ActorCount);
        }
    }
}

// AStudioDirectorActor Implementation
AStudioDirectorActor::AStudioDirectorActor()
{
    PrimaryActorTick.bCanEverTick = true;
    
    DirectorComponent = CreateDefaultSubobject<UStudioDirectorComponent>(TEXT("DirectorComponent"));
    bShowDebugInfo = true;
}

void AStudioDirectorActor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Actor initialized in world"));
}

void AStudioDirectorActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bShowDebugInfo && DirectorComponent)
    {
        // Display debug info on screen
        if (GEngine)
        {
            FString DebugText = FString::Printf(
                TEXT("Studio Director Status:\nTotal Actors: %d\nCharacters: %d\nDinosaurs: %d\nOverall Progress: %.1f%%"),
                DirectorComponent->CurrentMetrics.TotalActorsInWorld,
                DirectorComponent->CurrentMetrics.CharacterActors,
                DirectorComponent->CurrentMetrics.DinosaurActors,
                DirectorComponent->CurrentMetrics.OverallProgress
            );
            
            GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, DebugText);
        }
    }
}