#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalGame/SharedTypes.h"

ABuildIntegrationManager::ABuildIntegrationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Initialize build status
    BuildStatus = EBuild_BuildStatus::Unknown;
    LastValidationTime = 0.0f;
    ValidationInterval = 30.0f; // Validate every 30 seconds
    
    // Initialize counters
    LoadedClassCount = 0;
    TotalClassCount = 0;
    ActiveActorCount = 0;
    ErrorCount = 0;
    
    // Initialize flags
    bProjectLoaded = false;
    bMapFunctional = false;
    bCompilationArtifactsFound = false;
    bCriticalErrorsDetected = false;
    
    BuildHealthScore = 0.0f;
}

void ABuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Perform initial validation
    PerformBuildValidation();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Started with health score %.1f"), BuildHealthScore);
}

void ABuildIntegrationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Periodic validation
    LastValidationTime += DeltaTime;
    if (LastValidationTime >= ValidationInterval)
    {
        PerformBuildValidation();
        LastValidationTime = 0.0f;
    }
}

void ABuildIntegrationManager::PerformBuildValidation()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Starting build validation"));
    
    // Reset counters
    LoadedClassCount = 0;
    TotalClassCount = 0;
    ActiveActorCount = 0;
    ErrorCount = 0;
    CriticalErrors.Empty();
    
    // Validate project loading
    ValidateProjectLoading();
    
    // Validate core classes
    ValidateCoreClasses();
    
    // Validate map functionality
    ValidateMapFunctionality();
    
    // Validate compilation artifacts
    ValidateCompilationArtifacts();
    
    // Calculate health score
    CalculateBuildHealthScore();
    
    // Update build status
    UpdateBuildStatus();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validation complete - Health: %.1f, Status: %s"), 
           BuildHealthScore, *UEnum::GetValueAsString(BuildStatus));
}

void ABuildIntegrationManager::ValidateProjectLoading()
{
    UWorld* World = GetWorld();
    if (World && World->GetGameInstance())
    {
        bProjectLoaded = true;
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Project loaded successfully"));
    }
    else
    {
        bProjectLoaded = false;
        CriticalErrors.Add(TEXT("Project not properly loaded"));
        ErrorCount++;
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Project loading failed"));
    }
}

void ABuildIntegrationManager::ValidateCoreClasses()
{
    // List of critical classes to validate
    TArray<FString> CriticalClasses = {
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameState"),
        TEXT("TranspersonalGameMode"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager"),
        TEXT("ProceduralWorldManager")
    };
    
    TotalClassCount = CriticalClasses.Num();
    LoadedClassCount = 0;
    
    for (const FString& ClassName : CriticalClasses)
    {
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        
        if (LoadedClass)
        {
            LoadedClassCount++;
            UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Class loaded - %s"), *ClassName);
        }
        else
        {
            CriticalErrors.Add(FString::Printf(TEXT("Missing class: %s"), *ClassName));
            ErrorCount++;
            UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Failed to load class - %s"), *ClassName);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Core classes - %d/%d loaded"), LoadedClassCount, TotalClassCount);
}

void ABuildIntegrationManager::ValidateMapFunctionality()
{
    UWorld* World = GetWorld();
    if (World)
    {
        // Count active actors
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && !Actor->IsPendingKill())
            {
                ActiveActorCount++;
            }
        }
        
        if (ActiveActorCount > 0)
        {
            bMapFunctional = true;
            UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Map functional with %d actors"), ActiveActorCount);
        }
        else
        {
            bMapFunctional = false;
            CriticalErrors.Add(TEXT("No active actors in scene"));
            ErrorCount++;
        }
    }
    else
    {
        bMapFunctional = false;
        CriticalErrors.Add(TEXT("No world/map loaded"));
        ErrorCount++;
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: No world available"));
    }
}

void ABuildIntegrationManager::ValidateCompilationArtifacts()
{
    // For now, assume compilation artifacts exist if we can load classes
    bCompilationArtifactsFound = (LoadedClassCount > 0);
    
    if (!bCompilationArtifactsFound)
    {
        CriticalErrors.Add(TEXT("No compilation artifacts found"));
        ErrorCount++;
    }
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Compilation artifacts - %s"), 
           bCompilationArtifactsFound ? TEXT("Found") : TEXT("Missing"));
}

void ABuildIntegrationManager::CalculateBuildHealthScore()
{
    BuildHealthScore = 0.0f;
    
    // Project loading (20 points)
    if (bProjectLoaded)
    {
        BuildHealthScore += 20.0f;
    }
    
    // Core classes (40 points)
    if (TotalClassCount > 0)
    {
        BuildHealthScore += (float(LoadedClassCount) / float(TotalClassCount)) * 40.0f;
    }
    
    // Map functionality (20 points)
    if (bMapFunctional)
    {
        BuildHealthScore += 20.0f;
    }
    
    // Compilation artifacts (20 points)
    if (bCompilationArtifactsFound)
    {
        BuildHealthScore += 20.0f;
    }
    
    // Cap at 100
    BuildHealthScore = FMath::Clamp(BuildHealthScore, 0.0f, 100.0f);
}

void ABuildIntegrationManager::UpdateBuildStatus()
{
    bCriticalErrorsDetected = (ErrorCount > 0);
    
    if (BuildHealthScore >= 80.0f)
    {
        BuildStatus = EBuild_BuildStatus::Healthy;
    }
    else if (BuildHealthScore >= 60.0f)
    {
        BuildStatus = EBuild_BuildStatus::Functional;
    }
    else
    {
        BuildStatus = EBuild_BuildStatus::Critical;
    }
}

FString ABuildIntegrationManager::GetBuildStatusString() const
{
    switch (BuildStatus)
    {
        case EBuild_BuildStatus::Healthy:
            return TEXT("BUILD HEALTHY ✓");
        case EBuild_BuildStatus::Functional:
            return TEXT("BUILD FUNCTIONAL ⚠");
        case EBuild_BuildStatus::Critical:
            return TEXT("BUILD CRITICAL ✗");
        default:
            return TEXT("BUILD UNKNOWN");
    }
}

TArray<FString> ABuildIntegrationManager::GetCriticalErrors() const
{
    return CriticalErrors;
}

void ABuildIntegrationManager::LogBuildReport() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Project Status: %s"), bProjectLoaded ? TEXT("LOADED") : TEXT("FAILED"));
    UE_LOG(LogTemp, Warning, TEXT("Core Classes: %d/%d loaded"), LoadedClassCount, TotalClassCount);
    UE_LOG(LogTemp, Warning, TEXT("Map Status: %s"), bMapFunctional ? TEXT("FUNCTIONAL") : TEXT("BROKEN"));
    UE_LOG(LogTemp, Warning, TEXT("Scene Actors: %d"), ActiveActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Compilation: %s"), bCompilationArtifactsFound ? TEXT("OK") : TEXT("MISSING"));
    UE_LOG(LogTemp, Warning, TEXT("Overall Health: %.1f/100"), BuildHealthScore);
    UE_LOG(LogTemp, Warning, TEXT("Status: %s"), *GetBuildStatusString());
    
    if (bCriticalErrorsDetected)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL ERRORS (%d):"), ErrorCount);
        for (const FString& Error : CriticalErrors)
        {
            UE_LOG(LogTemp, Error, TEXT("  - %s"), *Error);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("✓ NO CRITICAL ERRORS DETECTED"));
    }
}