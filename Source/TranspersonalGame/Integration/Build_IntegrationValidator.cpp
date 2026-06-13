#include "Build_IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

ABuild_IntegrationValidator::ABuild_IntegrationValidator()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default metrics
    CurrentMetrics.TotalActors = 0;
    CurrentMetrics.DinosaurCount = 0;
    CurrentMetrics.EnvironmentCount = 0;
    CurrentMetrics.CharacterCount = 0;
    CurrentMetrics.LightingCount = 0;
    CurrentMetrics.AudioCount = 0;
    CurrentMetrics.VFXCount = 0;
    CurrentMetrics.QACount = 0;
    CurrentMetrics.OverallHealth = EBuild_SystemHealth::Healthy;
    CurrentMetrics.PerformanceScore = 100.0f;
}

void ABuild_IntegrationValidator::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoValidateOnBeginPlay)
    {
        ValidateAllSystems();
    }

    // Set up periodic validation
    if (ValidationInterval > 0.0f)
    {
        GetWorldTimerManager().SetTimer(
            ValidationTimerHandle,
            this,
            &ABuild_IntegrationValidator::PerformPeriodicValidation,
            ValidationInterval,
            true
        );
    }
}

void ABuild_IntegrationValidator::ValidateAllSystems()
{
    ClearValidationResults();

    UE_LOG(LogTemp, Warning, TEXT("Integration Validator: Starting system validation"));

    // Validate module compilation
    if (!ValidateModuleCompilation())
    {
        ValidationErrors.Add(TEXT("Module compilation validation failed"));
    }

    // Validate actor counts and system health
    ValidateSystemActorCounts();
    ValidateEssentialSystems();
    ValidatePerformanceMetrics();

    // Validate cross-system dependencies
    if (!ValidateCrossSystemDependencies())
    {
        ValidationWarnings.Add(TEXT("Cross-system dependency issues detected"));
    }

    // Calculate performance score
    CurrentMetrics.PerformanceScore = CalculatePerformanceScore();

    // Update overall health status
    UpdateHealthStatus();

    UE_LOG(LogTemp, Warning, TEXT("Integration Validator: Validation complete - Health: %d, Score: %.1f"), 
           (int32)CurrentMetrics.OverallHealth, CurrentMetrics.PerformanceScore);
}

bool ABuild_IntegrationValidator::ValidateModuleCompilation()
{
    // Test loading of core module classes
    TArray<FString> CoreClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager"),
        TEXT("/Script/TranspersonalGame.CrowdSimulationManager")
    };

    int32 LoadedClasses = 0;
    for (const FString& ClassName : CoreClasses)
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
        if (LoadedClass)
        {
            LoadedClasses++;
            UE_LOG(LogTemp, Log, TEXT("Integration Validator: Successfully loaded class %s"), *ClassName);
        }
        else
        {
            ValidationErrors.Add(FString::Printf(TEXT("Failed to load core class: %s"), *ClassName));
        }
    }

    return LoadedClasses == CoreClasses.Num();
}

bool ABuild_IntegrationValidator::ValidateCrossSystemDependencies()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        ValidationErrors.Add(TEXT("World reference is null"));
        return false;
    }

    // Check for essential actor dependencies
    bool bHasPlayerStart = false;
    bool bHasLighting = false;
    bool bHasCharacter = false;

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;

        FString ActorLabel = Actor->GetActorLabel().ToLower();
        
        if (ActorLabel.Contains(TEXT("playerstart")))
        {
            bHasPlayerStart = true;
        }
        else if (ActorLabel.Contains(TEXT("light")) || ActorLabel.Contains(TEXT("sun")))
        {
            bHasLighting = true;
        }
        else if (ActorLabel.Contains(TEXT("character")) || ActorLabel.Contains(TEXT("player")))
        {
            bHasCharacter = true;
        }
    }

    if (!bHasPlayerStart)
    {
        ValidationErrors.Add(TEXT("No PlayerStart found in level"));
    }
    if (!bHasLighting)
    {
        ValidationWarnings.Add(TEXT("No lighting actors found"));
    }
    if (!bHasCharacter)
    {
        ValidationWarnings.Add(TEXT("No character actors found"));
    }

    return bHasPlayerStart && bHasLighting;
}

float ABuild_IntegrationValidator::CalculatePerformanceScore()
{
    float Score = 100.0f;

    // Penalize for too many actors
    if (CurrentMetrics.TotalActors > 8000)
    {
        Score -= 30.0f;
    }
    else if (CurrentMetrics.TotalActors > 5000)
    {
        Score -= 15.0f;
    }

    // Penalize for too few essential actors
    if (CurrentMetrics.DinosaurCount < 5)
    {
        Score -= 20.0f;
    }
    if (CurrentMetrics.EnvironmentCount < 50)
    {
        Score -= 10.0f;
    }
    if (CurrentMetrics.LightingCount < 2)
    {
        Score -= 15.0f;
    }

    // Bonus for good system balance
    if (CurrentMetrics.DinosaurCount >= 10 && CurrentMetrics.DinosaurCount <= 150)
    {
        Score += 5.0f;
    }
    if (CurrentMetrics.EnvironmentCount >= 100 && CurrentMetrics.EnvironmentCount <= 1000)
    {
        Score += 5.0f;
    }

    return FMath::Clamp(Score, 0.0f, 100.0f);
}

void ABuild_IntegrationValidator::ValidateSystemActorCounts()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        ValidationErrors.Add(TEXT("World reference is null during actor count validation"));
        return;
    }

    // Reset counters
    CurrentMetrics.TotalActors = 0;
    CurrentMetrics.DinosaurCount = 0;
    CurrentMetrics.EnvironmentCount = 0;
    CurrentMetrics.CharacterCount = 0;
    CurrentMetrics.LightingCount = 0;
    CurrentMetrics.AudioCount = 0;
    CurrentMetrics.VFXCount = 0;
    CurrentMetrics.QACount = 0;

    TArray<FString> DinoLabels = {TEXT("trex"), TEXT("veloci"), TEXT("tricera"), TEXT("brachi"), TEXT("ankylo"), TEXT("parasauro")};
    TArray<FString> EnvLabels = {TEXT("tree"), TEXT("rock"), TEXT("grass"), TEXT("bush"), TEXT("flower"), TEXT("plant")};
    TArray<FString> LightLabels = {TEXT("light"), TEXT("sun"), TEXT("sky"), TEXT("fog")};

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;

        CurrentMetrics.TotalActors++;
        FString ActorLabel = Actor->GetActorLabel().ToLower();

        // Categorize actors
        bool bCategorized = false;
        for (const FString& DinoLabel : DinoLabels)
        {
            if (ActorLabel.Contains(DinoLabel))
            {
                CurrentMetrics.DinosaurCount++;
                bCategorized = true;
                break;
            }
        }

        if (!bCategorized)
        {
            for (const FString& EnvLabel : EnvLabels)
            {
                if (ActorLabel.Contains(EnvLabel))
                {
                    CurrentMetrics.EnvironmentCount++;
                    bCategorized = true;
                    break;
                }
            }
        }

        if (!bCategorized)
        {
            for (const FString& LightLabel : LightLabels)
            {
                if (ActorLabel.Contains(LightLabel))
                {
                    CurrentMetrics.LightingCount++;
                    bCategorized = true;
                    break;
                }
            }
        }

        if (!bCategorized)
        {
            if (ActorLabel.Contains(TEXT("character")) || ActorLabel.Contains(TEXT("player")))
            {
                CurrentMetrics.CharacterCount++;
            }
            else if (ActorLabel.Contains(TEXT("audio")) || ActorLabel.Contains(TEXT("sound")))
            {
                CurrentMetrics.AudioCount++;
            }
            else if (ActorLabel.Contains(TEXT("vfx")) || ActorLabel.Contains(TEXT("particle")))
            {
                CurrentMetrics.VFXCount++;
            }
            else if (ActorLabel.Contains(TEXT("qa_")) || ActorLabel.Contains(TEXT("test")))
            {
                CurrentMetrics.QACount++;
            }
        }
    }
}

void ABuild_IntegrationValidator::ValidateEssentialSystems()
{
    // Check for minimum required actors in each system
    if (CurrentMetrics.DinosaurCount == 0)
    {
        ValidationErrors.Add(TEXT("No dinosaur actors found - critical for gameplay"));
    }
    else if (CurrentMetrics.DinosaurCount < 5)
    {
        ValidationWarnings.Add(TEXT("Low dinosaur count - may impact gameplay variety"));
    }

    if (CurrentMetrics.EnvironmentCount < 20)
    {
        ValidationWarnings.Add(TEXT("Low environment actor count - world may feel empty"));
    }

    if (CurrentMetrics.LightingCount == 0)
    {
        ValidationErrors.Add(TEXT("No lighting actors found - level will be dark"));
    }

    if (CurrentMetrics.CharacterCount == 0)
    {
        ValidationWarnings.Add(TEXT("No character actors found - may indicate missing player setup"));
    }
}

void ABuild_IntegrationValidator::ValidatePerformanceMetrics()
{
    if (CurrentMetrics.TotalActors > 8000)
    {
        ValidationWarnings.Add(FString::Printf(TEXT("High actor count (%d) may impact performance"), CurrentMetrics.TotalActors));
    }

    if (CurrentMetrics.DinosaurCount > 150)
    {
        ValidationWarnings.Add(FString::Printf(TEXT("High dinosaur count (%d) may impact AI performance"), CurrentMetrics.DinosaurCount));
    }
}

void ABuild_IntegrationValidator::UpdateHealthStatus()
{
    if (ValidationErrors.Num() > 0)
    {
        CurrentMetrics.OverallHealth = EBuild_SystemHealth::Critical;
    }
    else if (ValidationWarnings.Num() > 3)
    {
        CurrentMetrics.OverallHealth = EBuild_SystemHealth::Degraded;
    }
    else if (ValidationWarnings.Num() > 0)
    {
        CurrentMetrics.OverallHealth = EBuild_SystemHealth::Healthy;
    }
    else
    {
        CurrentMetrics.OverallHealth = EBuild_SystemHealth::Healthy;
    }

    // Additional health checks based on performance score
    if (CurrentMetrics.PerformanceScore < 50.0f)
    {
        CurrentMetrics.OverallHealth = EBuild_SystemHealth::Critical;
    }
    else if (CurrentMetrics.PerformanceScore < 70.0f && CurrentMetrics.OverallHealth == EBuild_SystemHealth::Healthy)
    {
        CurrentMetrics.OverallHealth = EBuild_SystemHealth::Degraded;
    }
}

void ABuild_IntegrationValidator::ClearValidationResults()
{
    ValidationErrors.Empty();
    ValidationWarnings.Empty();
}

void ABuild_IntegrationValidator::PerformPeriodicValidation()
{
    ValidateAllSystems();
}