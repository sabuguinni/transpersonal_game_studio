#include "StudioDirector.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

AStudioDirector::AStudioDirector()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize production state
    bMilestone1_WalkAround = false;
    bCharacterMovementReady = false;
    bLandscapeReady = false;
    bDinosaursPlaced = false;
    bLightingSetup = false;

    // Initialize metrics
    TotalHeaderFiles = 0;
    TotalCppFiles = 0;
    PhantomHeaders = 0;
    CompilationStatus = 0.0f;

    LastProductionCheck = 0.0f;
}

void AStudioDirector::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("Studio Director initialized - Beginning production coordination"));
    
    // Initial production health check
    CheckProductionHealth();
    
    // Start agent coordination
    CoordinateAgentPipeline();
}

void AStudioDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    LastProductionCheck += DeltaTime;

    // Check production health every 30 seconds
    if (LastProductionCheck >= 30.0f)
    {
        CheckProductionHealth();
        ValidateMilestone1();
        LastProductionCheck = 0.0f;
    }
}

void AStudioDirector::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription)
{
    FString TaskEntry = FString::Printf(TEXT("%s: %s"), *AgentName, *TaskDescription);
    ActiveAgents.AddUnique(AgentName);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director - Task assigned to %s: %s"), *AgentName, *TaskDescription);
}

void AStudioDirector::MarkTaskCompleted(const FString& AgentName, const FString& TaskDescription)
{
    FString TaskEntry = FString::Printf(TEXT("%s: %s"), *AgentName, *TaskDescription);
    CompletedTasks.AddUnique(TaskEntry);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director - Task completed by %s: %s"), *AgentName, *TaskDescription);
    
    // Update milestone progress
    UpdateMilestoneProgress();
}

void AStudioDirector::BlockTask(const FString& AgentName, const FString& Reason)
{
    FString BlockEntry = FString::Printf(TEXT("%s: BLOCKED - %s"), *AgentName, *Reason);
    BlockedTasks.AddUnique(BlockEntry);
    
    UE_LOG(LogTemp, Error, TEXT("Studio Director - Task blocked for %s: %s"), *AgentName, *Reason);
    
    // Escalate critical issue
    EscalateCriticalIssue(FString::Printf(TEXT("Agent %s blocked: %s"), *AgentName, *Reason));
}

bool AStudioDirector::ValidateMilestone1()
{
    // Check if Milestone 1 "Walk Around" is complete
    bool bCharacterExists = false;
    bool bLandscapeExists = false;
    bool bDinosaursExist = false;
    bool bLightingExists = false;

    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    // Check for TranspersonalCharacter
    TArray<AActor*> FoundCharacters;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundCharacters);
    for (AActor* Actor : FoundCharacters)
    {
        if (Actor->GetClass()->GetName().Contains(TEXT("TranspersonalCharacter")))
        {
            bCharacterExists = true;
            break;
        }
    }

    // Check for landscape
    TArray<AActor*> FoundLandscapes;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundLandscapes);
    for (AActor* Actor : FoundLandscapes)
    {
        if (Actor->GetClass()->GetName().Contains(TEXT("Landscape")))
        {
            bLandscapeExists = true;
            break;
        }
    }

    // Check for dinosaurs
    TArray<AActor*> FoundDinosaurs;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundDinosaurs);
    for (AActor* Actor : FoundDinosaurs)
    {
        FString ActorName = Actor->GetClass()->GetName();
        if (ActorName.Contains(TEXT("Dinosaur")) || ActorName.Contains(TEXT("TRex")) || ActorName.Contains(TEXT("Raptor")))
        {
            bDinosaursExist = true;
            break;
        }
    }

    // Check for lighting
    TArray<AActor*> FoundLights;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundLights);
    for (AActor* Actor : FoundLights)
    {
        if (Actor->GetClass()->GetName().Contains(TEXT("DirectionalLight")))
        {
            bLightingExists = true;
            break;
        }
    }

    // Update milestone flags
    bCharacterMovementReady = bCharacterExists;
    bLandscapeReady = bLandscapeExists;
    bDinosaursPlaced = bDinosaursExist;
    bLightingSetup = bLightingExists;

    // Overall milestone status
    bMilestone1_WalkAround = bCharacterExists && bLandscapeExists && bDinosaursExist && bLightingExists;

    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 Status - Character: %s, Landscape: %s, Dinosaurs: %s, Lighting: %s, Overall: %s"),
        bCharacterExists ? TEXT("OK") : TEXT("MISSING"),
        bLandscapeExists ? TEXT("OK") : TEXT("MISSING"),
        bDinosaursExist ? TEXT("OK") : TEXT("MISSING"),
        bLightingExists ? TEXT("OK") : TEXT("MISSING"),
        bMilestone1_WalkAround ? TEXT("COMPLETE") : TEXT("INCOMPLETE"));

    return bMilestone1_WalkAround;
}

void AStudioDirector::CheckProductionHealth()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director - Production health check initiated"));

    // Clear previous issues
    CriticalIssues.Empty();

    // Check for phantom headers (from memory directive)
    if (PhantomHeaders > 0)
    {
        CriticalIssues.Add(FString::Printf(TEXT("CRITICAL: %d phantom headers detected"), PhantomHeaders));
    }

    // Check compilation status
    if (CompilationStatus < 1.0f)
    {
        CriticalIssues.Add(TEXT("CRITICAL: Compilation failures detected"));
    }

    // Check milestone progress
    if (!ValidateMilestone1())
    {
        CriticalIssues.Add(TEXT("CRITICAL: Milestone 1 incomplete - no playable prototype"));
    }

    // Check for duplicate actors
    CheckForDuplicateActors();

    // Log all critical issues
    for (const FString& Issue : CriticalIssues)
    {
        UE_LOG(LogTemp, Error, TEXT("Studio Director - %s"), *Issue);
    }

    LogProductionStatus();
}

void AStudioDirector::CoordinateAgentPipeline()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director - Coordinating agent pipeline"));

    // Priority tasks based on memory directives
    TArray<FString> PriorityTasks;
    
    // Critical Path: Compilation first
    PriorityTasks.Add(TEXT("Agent #20: Clean 122 phantom headers and attempt first C++ compilation"));
    PriorityTasks.Add(TEXT("Agent #5: Expand landscape to 200km² with 5 biome distribution"));
    PriorityTasks.Add(TEXT("Agent #9/#10: Create dinosaur actors with collision and basic AI"));
    PriorityTasks.Add(TEXT("Agent #12: Implement survival HUD with health/hunger bars"));

    // Assign priority tasks
    for (const FString& Task : PriorityTasks)
    {
        UE_LOG(LogTemp, Warning, TEXT("Studio Director - Priority: %s"), *Task);
    }

    // Enforce gameplay-first directive
    EnforceGameplayFirstDirective();
}

FString AStudioDirector::GetNextPriorityTask()
{
    // Based on memory directives and current production state
    if (PhantomHeaders > 0)
    {
        return TEXT("URGENT: Clean phantom headers before any new code");
    }

    if (CompilationStatus < 1.0f)
    {
        return TEXT("URGENT: Fix compilation errors");
    }

    if (!bCharacterMovementReady)
    {
        return TEXT("HIGH: Implement TranspersonalCharacter movement");
    }

    if (!bLandscapeReady)
    {
        return TEXT("HIGH: Create 200km² landscape with 5 biomes");
    }

    if (!bDinosaursPlaced)
    {
        return TEXT("HIGH: Place dinosaur actors with proper biome distribution");
    }

    return TEXT("NORMAL: Continue with assigned agent tasks");
}

void AStudioDirector::EnforceGameplayFirstDirective()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director - Enforcing GAMEPLAY-FIRST directive"));
    
    // Rule: Every cycle must produce visible, playable content
    // Rule: No more abstract systems without concrete implementation
    // Rule: Use existing UE5 classes, not custom movement systems
    
    UE_LOG(LogTemp, Warning, TEXT("DIRECTIVE: 80%% action, 20%% assessment maximum"));
    UE_LOG(LogTemp, Warning, TEXT("DIRECTIVE: Every .h must have matching .cpp"));
    UE_LOG(LogTemp, Warning, TEXT("DIRECTIVE: Use existing UE5 classes (ACharacter, APlayerController)"));
}

void AStudioDirector::ValidateAgentOutputs()
{
    // Validate that agents are producing real, compilable code
    for (const FString& Agent : ActiveAgents)
    {
        if (!IsAgentOutputValid(Agent))
        {
            BlockTask(Agent, TEXT("Output validation failed - no concrete deliverables"));
        }
    }
}

void AStudioDirector::CheckForDuplicateActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Count lighting actors (from memory directive)
    TArray<AActor*> DirectionalLights;
    TArray<AActor*> SkyAtmospheres;
    TArray<AActor*> SkyLights;
    TArray<AActor*> FogActors;

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        FString ClassName = Actor->GetClass()->GetName();
        
        if (ClassName.Contains(TEXT("DirectionalLight")))
        {
            DirectionalLights.Add(Actor);
        }
        else if (ClassName.Contains(TEXT("SkyAtmosphere")))
        {
            SkyAtmospheres.Add(Actor);
        }
        else if (ClassName.Contains(TEXT("SkyLight")))
        {
            SkyLights.Add(Actor);
        }
        else if (ClassName.Contains(TEXT("ExponentialHeightFog")))
        {
            FogActors.Add(Actor);
        }
    }

    // Report duplicates
    if (DirectionalLights.Num() > 1)
    {
        CriticalIssues.Add(FString::Printf(TEXT("DUPLICATE: %d DirectionalLights found"), DirectionalLights.Num()));
    }
    if (SkyAtmospheres.Num() > 1)
    {
        CriticalIssues.Add(FString::Printf(TEXT("DUPLICATE: %d SkyAtmospheres found"), SkyAtmospheres.Num()));
    }
    if (SkyLights.Num() > 1)
    {
        CriticalIssues.Add(FString::Printf(TEXT("DUPLICATE: %d SkyLights found"), SkyLights.Num()));
    }
    if (FogActors.Num() > 1)
    {
        CriticalIssues.Add(FString::Printf(TEXT("DUPLICATE: %d ExponentialHeightFog found"), FogActors.Num()));
    }
}

void AStudioDirector::ValidateBiomeDistribution()
{
    // Validate that actors are distributed across the 5 biomes as per memory directive
    UE_LOG(LogTemp, Warning, TEXT("Studio Director - Validating biome distribution"));
    
    // Biome centers from memory:
    // PANTANO: (-50000, -45000, 0)
    // FLORESTA: (-45000, 40000, 0)
    // SAVANA: (0, 0, 0)
    // DESERTO: (55000, 0, 0)
    // MONTANHA NEVADA: (40000, 50000, 500)
    
    UE_LOG(LogTemp, Warning, TEXT("RULE: NEVER spawn actors at Vector(0,0,0) - distribute across biomes"));
}

void AStudioDirector::AssessCompilationStatus()
{
    // This would normally check UBT output, for now we simulate
    UE_LOG(LogTemp, Warning, TEXT("Studio Director - Assessing compilation status"));
    UE_LOG(LogTemp, Warning, TEXT("CRITICAL: Agent #20 must run UBT compilation check"));
}

void AStudioDirector::LogProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR PRODUCTION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Active Agents: %d"), ActiveAgents.Num());
    UE_LOG(LogTemp, Warning, TEXT("Completed Tasks: %d"), CompletedTasks.Num());
    UE_LOG(LogTemp, Warning, TEXT("Blocked Tasks: %d"), BlockedTasks.Num());
    UE_LOG(LogTemp, Warning, TEXT("Critical Issues: %d"), CriticalIssues.Num());
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 Status: %s"), bMilestone1_WalkAround ? TEXT("COMPLETE") : TEXT("INCOMPLETE"));
}

void AStudioDirector::UpdateMilestoneProgress()
{
    // Update based on completed tasks
    ValidateMilestone1();
}

bool AStudioDirector::IsAgentOutputValid(const FString& AgentName)
{
    // Check if agent produced concrete deliverables
    // This would normally check file system and compilation results
    return true; // Simplified for now
}

void AStudioDirector::EscalateCriticalIssue(const FString& Issue)
{
    UE_LOG(LogTemp, Error, TEXT("STUDIO DIRECTOR ESCALATION: %s"), *Issue);
    
    // In a real system, this would notify Miguel via Telegram
    // For now, we log as critical error
}