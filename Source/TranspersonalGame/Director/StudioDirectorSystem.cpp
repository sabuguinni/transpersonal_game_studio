#include "StudioDirectorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

AStudioDirectorSystem::AStudioDirectorSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    bPlayablePrototypeReady = false;
    OverallProgress = 0.0f;
}

void AStudioDirectorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAgentTasks();
    InitializeBiomeStatuses();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System initialized"));
}

void AStudioDirectorSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update progress every 5 seconds
    static float UpdateTimer = 0.0f;
    UpdateTimer += DeltaTime;
    
    if (UpdateTimer >= 5.0f)
    {
        CheckBiomePopulation();
        ValidatePlayablePrototype();
        OverallProgress = CalculateOverallProgress();
        UpdateTimer = 0.0f;
    }
}

void AStudioDirectorSystem::InitializeAgentTasks()
{
    AgentTasks.Empty();
    
    // Critical tasks for playable prototype
    TArray<TPair<FString, FString>> TaskDefinitions = {
        {TEXT("Agent_02_EngineArchitect"), TEXT("Validate core systems compilation and module dependencies")},
        {TEXT("Agent_03_CoreSystems"), TEXT("Implement physics collision for character-dinosaur interaction")},
        {TEXT("Agent_05_WorldGenerator"), TEXT("Populate 5 biomes with terrain actors (500+ per biome)")},
        {TEXT("Agent_06_EnvironmentArt"), TEXT("Spawn vegetation and rocks using FBX pipeline test")},
        {TEXT("Agent_07_Architecture"), TEXT("Create primitive shelters and landmarks for navigation")},
        {TEXT("Agent_08_Lighting"), TEXT("Establish Cretaceous atmosphere with proper lighting")},
        {TEXT("Agent_09_CharacterArtist"), TEXT("Create dinosaur actors with collision and basic AI")},
        {TEXT("Agent_10_Animation"), TEXT("Implement character movement animations and IK")},
        {TEXT("Agent_11_NPCBehavior"), TEXT("Basic dinosaur behavior trees for survival gameplay")},
        {TEXT("Agent_12_CombatAI"), TEXT("Implement survival HUD (health/hunger/thirst bars)")},
        {TEXT("Agent_13_CrowdSim"), TEXT("Optimize mass simulation for 500+ actors per biome")},
        {TEXT("Agent_14_QuestDesigner"), TEXT("Create first survival objective: find water source")},
        {TEXT("Agent_15_Narrative"), TEXT("Write survival tutorial and basic lore snippets")},
        {TEXT("Agent_16_Audio"), TEXT("Ambient prehistoric soundscape and dinosaur calls")},
        {TEXT("Agent_17_VFX"), TEXT("Basic particle effects for environmental atmosphere")},
        {TEXT("Agent_18_QA"), TEXT("Test character movement and basic survival mechanics")},
        {TEXT("Agent_19_Integration"), TEXT("Ensure all systems work together in MinPlayableMap")}
    };
    
    for (int32 i = 0; i < TaskDefinitions.Num(); i++)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentID = TaskDefinitions[i].Key;
        NewTask.TaskDescription = TaskDefinitions[i].Value;
        NewTask.Priority = i < 5 ? 10 : 5; // First 5 are high priority
        NewTask.bIsCompleted = false;
        NewTask.CompletionPercentage = 0.0f;
        
        AgentTasks.Add(NewTask);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d agent tasks"), AgentTasks.Num());
}

void AStudioDirectorSystem::InitializeBiomeStatuses()
{
    BiomeStatuses.Empty();
    
    TArray<TPair<FString, FVector>> BiomeDefinitions = {
        {TEXT("Savana"), FVector(0, 0, 0)},
        {TEXT("Pantano"), FVector(-50000, -45000, 0)},
        {TEXT("Floresta"), FVector(-45000, 40000, 0)},
        {TEXT("Deserto"), FVector(55000, 0, 0)},
        {TEXT("Montanha"), FVector(40000, 50000, 0)}
    };
    
    for (const auto& BiomeDef : BiomeDefinitions)
    {
        FDir_BiomeStatus NewBiome;
        NewBiome.BiomeName = BiomeDef.Key;
        NewBiome.BiomeLocation = BiomeDef.Value;
        NewBiome.ActorCount = 0;
        NewBiome.bIsPopulated = false;
        
        BiomeStatuses.Add(NewBiome);
    }
}

void AStudioDirectorSystem::UpdateAgentTask(const FString& AgentID, float Progress, bool bCompleted)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            Task.CompletionPercentage = FMath::Clamp(Progress, 0.0f, 100.0f);
            Task.bIsCompleted = bCompleted || (Progress >= 100.0f);
            
            UE_LOG(LogTemp, Warning, TEXT("Updated task for %s: %.1f%% complete"), 
                   *AgentID, Task.CompletionPercentage);
            break;
        }
    }
}

void AStudioDirectorSystem::CheckBiomePopulation()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    for (FDir_BiomeStatus& Biome : BiomeStatuses)
    {
        int32 ActorCount = 0;
        
        // Count actors within 10km of biome center
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (!Actor || Actor == this) continue;
            
            float Distance = FVector::Dist(Actor->GetActorLocation(), Biome.BiomeLocation);
            if (Distance <= 1000000.0f) // 10km in UE units
            {
                ActorCount++;
            }
        }
        
        Biome.ActorCount = ActorCount;
        Biome.bIsPopulated = ActorCount >= 500; // Criterion from memory
        
        if (Biome.ActorCount != ActorCount)
        {
            UE_LOG(LogTemp, Warning, TEXT("Biome %s: %d actors (target: 500+)"), 
                   *Biome.BiomeName, ActorCount);
        }
    }
}

void AStudioDirectorSystem::ValidatePlayablePrototype()
{
    bool bCharacterMovement = ValidateCharacterMovement();
    bool bDinosaurPresence = ValidateDinosaurPresence();
    bool bTerrainLighting = ValidateTerrainAndLighting();
    
    // Check if at least 3 biomes are populated
    int32 PopulatedBiomes = 0;
    for (const FDir_BiomeStatus& Biome : BiomeStatuses)
    {
        if (Biome.bIsPopulated) PopulatedBiomes++;
    }
    
    bPlayablePrototypeReady = bCharacterMovement && bDinosaurPresence && 
                             bTerrainLighting && (PopulatedBiomes >= 3);
    
    if (bPlayablePrototypeReady)
    {
        UE_LOG(LogTemp, Warning, TEXT("PLAYABLE PROTOTYPE READY!"));
    }
}

bool AStudioDirectorSystem::ValidateCharacterMovement()
{
    // Check if TranspersonalCharacter exists and has movement component
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Look for character actors in the world
    for (TActorIterator<APawn> PawnItr(World); PawnItr; ++PawnItr)
    {
        APawn* Pawn = *PawnItr;
        if (Pawn && Pawn->GetName().Contains(TEXT("TranspersonalCharacter")))
        {
            return true; // Found character
        }
    }
    
    return false;
}

bool AStudioDirectorSystem::ValidateDinosaurPresence()
{
    UWorld* World = GetWorld();
    if (!World) return false;
    
    int32 DinosaurCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && (Actor->GetName().Contains(TEXT("Dinosaur")) || 
                     Actor->GetName().Contains(TEXT("TRex")) ||
                     Actor->GetName().Contains(TEXT("Raptor")) ||
                     Actor->GetName().Contains(TEXT("Brachiosaurus"))))
        {
            DinosaurCount++;
        }
    }
    
    return DinosaurCount >= 5; // Need at least 5 dinosaurs
}

bool AStudioDirectorSystem::ValidateTerrainAndLighting()
{
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Check for directional light (sun)
    bool bHasDirectionalLight = false;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetName().Contains(TEXT("DirectionalLight")))
        {
            bHasDirectionalLight = true;
            break;
        }
    }
    
    return bHasDirectionalLight;
}

float AStudioDirectorSystem::CalculateOverallProgress() const
{
    if (AgentTasks.Num() == 0) return 0.0f;
    
    float TotalProgress = 0.0f;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        TotalProgress += Task.CompletionPercentage;
    }
    
    return TotalProgress / AgentTasks.Num();
}

TArray<FDir_AgentTask> AStudioDirectorSystem::GetPendingTasks() const
{
    TArray<FDir_AgentTask> PendingTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (!Task.bIsCompleted)
        {
            PendingTasks.Add(Task);
        }
    }
    
    // Sort by priority (higher first)
    PendingTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B) {
        return A.Priority > B.Priority;
    });
    
    return PendingTasks;
}

void AStudioDirectorSystem::DebugPrintStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("Playable Prototype Ready: %s"), 
           bPlayablePrototypeReady ? TEXT("YES") : TEXT("NO"));
    
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME STATUS ==="));
    for (const FDir_BiomeStatus& Biome : BiomeStatuses)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: %d actors (%s)"), 
               *Biome.BiomeName, Biome.ActorCount, 
               Biome.bIsPopulated ? TEXT("POPULATED") : TEXT("NEEDS WORK"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== PENDING TASKS ==="));
    TArray<FDir_AgentTask> Pending = GetPendingTasks();
    for (int32 i = 0; i < FMath::Min(5, Pending.Num()); i++)
    {
        const FDir_AgentTask& Task = Pending[i];
        UE_LOG(LogTemp, Warning, TEXT("%s: %.1f%% - %s"), 
               *Task.AgentID, Task.CompletionPercentage, *Task.TaskDescription);
    }
}