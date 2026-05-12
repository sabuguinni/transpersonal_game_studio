#include "Build_FinalCycleCompletionSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

ABuild_FinalCycleCompletionSystem::ABuild_FinalCycleCompletionSystem()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    CurrentStatus = EBuild_CycleCompletionStatus::Pending;
    CurrentCycleID = TEXT("UNKNOWN");
    bAllSystemsOperational = false;
    OverallBuildHealth = 0.0f;
}

void ABuild_FinalCycleCompletionSystem::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionSystem: System initialized"));
    
    // Auto-initialize with current timestamp if no cycle ID set
    if (CurrentCycleID == TEXT("UNKNOWN"))
    {
        FDateTime Now = FDateTime::Now();
        CurrentCycleID = FString::Printf(TEXT("AUTO_%s"), *Now.ToString(TEXT("%Y%m%d_%H%M%S")));
        InitializeCycleCompletion(CurrentCycleID);
    }
}

void ABuild_FinalCycleCompletionSystem::InitializeCycleCompletion(const FString& CycleID)
{
    CurrentCycleID = CycleID;
    CurrentStatus = EBuild_CycleCompletionStatus::InProgress;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionSystem: Initializing cycle completion for %s"), *CycleID);
    
    // Clear previous results
    SystemValidationResults.Empty();
    
    // Reset metrics
    CurrentCycleMetrics = FBuild_CycleMetrics();
    CurrentCycleMetrics.CompletionTimestamp = FDateTime::Now();
    
    // Trigger Blueprint event
    OnCycleInitialized(CycleID);
    
    // Start validation process
    ValidateAllSystems();
}

void ABuild_FinalCycleCompletionSystem::ValidateAllSystems()
{
    CurrentStatus = EBuild_CycleCompletionStatus::ValidationPhase;
    SystemValidationResults.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionSystem: Starting system validation"));
    
    // Define system validation parameters
    struct FSystemValidationParams
    {
        FString SystemName;
        TArray<FString> Keywords;
    };
    
    TArray<FSystemValidationParams> SystemsToValidate = {
        {TEXT("Core"), {TEXT("GAME"), TEXT("STATE"), TEXT("MODE"), TEXT("CONTROLLER")}},
        {TEXT("VFX"), {TEXT("VFX"), TEXT("IMPACT"), TEXT("PARTICLE"), TEXT("EFFECT")}},
        {TEXT("Dinosaur"), {TEXT("DINOSAUR"), TEXT("TREX"), TEXT("RAPTOR"), TEXT("BRACHIO")}},
        {TEXT("Character"), {TEXT("CHARACTER"), TEXT("PLAYER"), TEXT("PAWN")}},
        {TEXT("QA"), {TEXT("QA"), TEXT("TEST"), TEXT("VALIDATION"), TEXT("FRAMEWORK")}},
        {TEXT("Integration"), {TEXT("INTEGRATION"), TEXT("BUILD"), TEXT("MANAGER"), TEXT("VALIDATOR")}},
        {TEXT("Environment"), {TEXT("TREE"), TEXT("ROCK"), TEXT("TERRAIN"), TEXT("LANDSCAPE")}}
    };
    
    // Validate each system
    for (const auto& SystemParams : SystemsToValidate)
    {
        FBuild_SystemValidationResult ValidationResult;
        ValidationResult.SystemName = SystemParams.SystemName;
        
        ValidateSystemActors(SystemParams.SystemName, SystemParams.Keywords, ValidationResult);
        SystemValidationResults.Add(ValidationResult);
        
        UE_LOG(LogTemp, Log, TEXT("System %s: %d actors, Health: %.2f"), 
               *SystemParams.SystemName, ValidationResult.ActorCount, ValidationResult.HealthScore);
    }
    
    // Calculate overall health and update status
    CalculateOverallHealth();
    UpdateCompletionStatus();
    
    // Trigger Blueprint event
    OnSystemValidationComplete(SystemValidationResults);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionSystem: System validation complete. Overall health: %.2f"), OverallBuildHealth);
}

void ABuild_FinalCycleCompletionSystem::ValidateSystemActors(const FString& SystemName, const TArray<FString>& Keywords, FBuild_SystemValidationResult& Result)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        Result.bIsOperational = false;
        Result.HealthScore = 0.0f;
        Result.ValidationMessages.Add(TEXT("World not available"));
        return;
    }
    
    // Count actors matching system keywords
    int32 MatchingActors = 0;
    TArray<FString> FoundActorNames;
    
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor) continue;
        
        FString ActorName = Actor->GetName().ToUpper();
        
        // Check if actor name contains any of the system keywords
        for (const FString& Keyword : Keywords)
        {
            if (ActorName.Contains(Keyword.ToUpper()))
            {
                MatchingActors++;
                FoundActorNames.Add(Actor->GetName());
                break; // Don't count the same actor multiple times
            }
        }
    }
    
    // Update result
    Result.ActorCount = MatchingActors;
    Result.bIsOperational = MatchingActors > 0;
    
    // Calculate health score based on actor count and system importance
    float BaseScore = FMath::Clamp(static_cast<float>(MatchingActors) / 5.0f, 0.0f, 1.0f); // Normalize to 0-1
    
    // Apply system importance multipliers
    float ImportanceMultiplier = 1.0f;
    if (SystemName == TEXT("Core") || SystemName == TEXT("Character"))
    {
        ImportanceMultiplier = 1.2f; // Critical systems
    }
    else if (SystemName == TEXT("Integration") || SystemName == TEXT("QA"))
    {
        ImportanceMultiplier = 1.1f; // Important systems
    }
    
    Result.HealthScore = FMath::Clamp(BaseScore * ImportanceMultiplier, 0.0f, 1.0f);
    
    // Add validation messages
    if (MatchingActors > 0)
    {
        Result.ValidationMessages.Add(FString::Printf(TEXT("Found %d actors for %s system"), MatchingActors, *SystemName));
        if (FoundActorNames.Num() <= 5)
        {
            for (const FString& ActorName : FoundActorNames)
            {
                Result.ValidationMessages.Add(FString::Printf(TEXT("- %s"), *ActorName));
            }
        }
        else
        {
            Result.ValidationMessages.Add(FString::Printf(TEXT("- %d actors total (showing first 3)"), FoundActorNames.Num()));
            for (int32 i = 0; i < 3 && i < FoundActorNames.Num(); i++)
            {
                Result.ValidationMessages.Add(FString::Printf(TEXT("- %s"), *FoundActorNames[i]));
            }
        }
    }
    else
    {
        Result.ValidationMessages.Add(FString::Printf(TEXT("No actors found for %s system"), *SystemName));
    }
}

void ABuild_FinalCycleCompletionSystem::CalculateOverallHealth()
{
    if (SystemValidationResults.Num() == 0)
    {
        OverallBuildHealth = 0.0f;
        bAllSystemsOperational = false;
        return;
    }
    
    float TotalHealth = 0.0f;
    int32 OperationalSystems = 0;
    
    for (const auto& Result : SystemValidationResults)
    {
        TotalHealth += Result.HealthScore;
        if (Result.bIsOperational)
        {
            OperationalSystems++;
        }
    }
    
    OverallBuildHealth = TotalHealth / static_cast<float>(SystemValidationResults.Num());
    bAllSystemsOperational = (OperationalSystems == SystemValidationResults.Num());
    
    UE_LOG(LogTemp, Log, TEXT("Overall Build Health: %.2f (%d/%d systems operational)"), 
           OverallBuildHealth, OperationalSystems, SystemValidationResults.Num());
}

void ABuild_FinalCycleCompletionSystem::UpdateCompletionStatus()
{
    if (OverallBuildHealth >= 0.8f && bAllSystemsOperational)
    {
        CurrentStatus = EBuild_CycleCompletionStatus::Complete;
    }
    else if (OverallBuildHealth >= 0.5f)
    {
        CurrentStatus = EBuild_CycleCompletionStatus::ValidationPhase;
    }
    else
    {
        CurrentStatus = EBuild_CycleCompletionStatus::Failed;
    }
}

void ABuild_FinalCycleCompletionSystem::GenerateCycleMetrics()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Count all actors and categorize them
    CurrentCycleMetrics = FBuild_CycleMetrics();
    CurrentCycleMetrics.CompletionTimestamp = FDateTime::Now();
    
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor) continue;
        
        CurrentCycleMetrics.TotalActors++;
        
        FString ActorName = Actor->GetName().ToUpper();
        
        // Categorize actors
        if (ActorName.Contains(TEXT("GAME")) || ActorName.Contains(TEXT("STATE")) || ActorName.Contains(TEXT("MODE")))
        {
            CurrentCycleMetrics.CoreSystemActors++;
        }
        else if (ActorName.Contains(TEXT("VFX")) || ActorName.Contains(TEXT("IMPACT")) || ActorName.Contains(TEXT("PARTICLE")))
        {
            CurrentCycleMetrics.VFXSystemActors++;
        }
        else if (ActorName.Contains(TEXT("DINOSAUR")) || ActorName.Contains(TEXT("TREX")) || ActorName.Contains(TEXT("RAPTOR")))
        {
            CurrentCycleMetrics.DinosaurSystemActors++;
        }
        else if (ActorName.Contains(TEXT("CHARACTER")) || ActorName.Contains(TEXT("PLAYER")) || ActorName.Contains(TEXT("PAWN")))
        {
            CurrentCycleMetrics.CharacterSystemActors++;
        }
        else if (ActorName.Contains(TEXT("QA")) || ActorName.Contains(TEXT("TEST")) || ActorName.Contains(TEXT("VALIDATION")))
        {
            CurrentCycleMetrics.QASystemActors++;
        }
        else if (ActorName.Contains(TEXT("INTEGRATION")) || ActorName.Contains(TEXT("BUILD")) || ActorName.Contains(TEXT("MANAGER")))
        {
            CurrentCycleMetrics.IntegrationSystemActors++;
        }
        else if (ActorName.Contains(TEXT("TREE")) || ActorName.Contains(TEXT("ROCK")) || ActorName.Contains(TEXT("TERRAIN")))
        {
            CurrentCycleMetrics.EnvironmentSystemActors++;
        }
    }
    
    // Calculate completion percentage
    CurrentCycleMetrics.CompletionPercentage = OverallBuildHealth * 100.0f;
    
    // Set build status
    if (OverallBuildHealth >= 0.8f)
    {
        CurrentCycleMetrics.BuildStatus = TEXT("GREEN");
    }
    else if (OverallBuildHealth >= 0.5f)
    {
        CurrentCycleMetrics.BuildStatus = TEXT("YELLOW");
    }
    else
    {
        CurrentCycleMetrics.BuildStatus = TEXT("RED");
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Cycle Metrics Generated - Total Actors: %d, Build Status: %s"), 
           CurrentCycleMetrics.TotalActors, *CurrentCycleMetrics.BuildStatus);
}

void ABuild_FinalCycleCompletionSystem::CompleteCycle()
{
    // Generate final metrics
    GenerateCycleMetrics();
    
    // Update final status
    UpdateCompletionStatus();
    
    // Log completion
    LogCycleCompletion();
    
    // Trigger Blueprint event
    OnCycleComplete(CurrentCycleMetrics);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionSystem: Cycle %s completed with status %s"), 
           *CurrentCycleID, *CurrentCycleMetrics.BuildStatus);
}

void ABuild_FinalCycleCompletionSystem::LogCycleCompletion()
{
    UE_LOG(LogTemp, Warning, TEXT("=== CYCLE COMPLETION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle ID: %s"), *CurrentCycleID);
    UE_LOG(LogTemp, Warning, TEXT("Completion Time: %s"), *CurrentCycleMetrics.CompletionTimestamp.ToString());
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentCycleMetrics.TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("Build Status: %s"), *CurrentCycleMetrics.BuildStatus);
    UE_LOG(LogTemp, Warning, TEXT("Overall Health: %.2f%%"), CurrentCycleMetrics.CompletionPercentage);
    UE_LOG(LogTemp, Warning, TEXT("All Systems Operational: %s"), bAllSystemsOperational ? TEXT("YES") : TEXT("NO"));
    
    UE_LOG(LogTemp, Warning, TEXT("System Breakdown:"));
    UE_LOG(LogTemp, Warning, TEXT("- Core: %d actors"), CurrentCycleMetrics.CoreSystemActors);
    UE_LOG(LogTemp, Warning, TEXT("- VFX: %d actors"), CurrentCycleMetrics.VFXSystemActors);
    UE_LOG(LogTemp, Warning, TEXT("- Dinosaur: %d actors"), CurrentCycleMetrics.DinosaurSystemActors);
    UE_LOG(LogTemp, Warning, TEXT("- Character: %d actors"), CurrentCycleMetrics.CharacterSystemActors);
    UE_LOG(LogTemp, Warning, TEXT("- QA: %d actors"), CurrentCycleMetrics.QASystemActors);
    UE_LOG(LogTemp, Warning, TEXT("- Integration: %d actors"), CurrentCycleMetrics.IntegrationSystemActors);
    UE_LOG(LogTemp, Warning, TEXT("- Environment: %d actors"), CurrentCycleMetrics.EnvironmentSystemActors);
    UE_LOG(LogTemp, Warning, TEXT("=== END CYCLE COMPLETION REPORT ==="));
}