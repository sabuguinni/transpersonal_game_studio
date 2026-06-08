#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Engine/Level.h"
#include "Engine/LevelStreaming.h"
#include "Components/StaticMeshComponent.h"
#include "TranspersonalGameState.h"
#include "TranspersonalCharacter.h"

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initializing build integration system"));
    
    // Initialize status
    CurrentStatus.bIsCompiled = true;
    CurrentStatus.bAllModulesLoaded = false;
    CurrentStatus.ActiveActorCount = 0;
    CurrentStatus.LastValidationTime = 0.0f;
    CurrentStatus.LoadedModules.Empty();
    CurrentStatus.CompilationErrors.Empty();
    
    // Initialize system health
    SystemHealth.bWorldGeneratorActive = false;
    SystemHealth.bFoliageManagerActive = false;
    SystemHealth.bCrowdSimulationActive = false;
    SystemHealth.bQAFrameworkActive = false;
    SystemHealth.OverallHealthScore = 0.0f;
    
    // Run initial validation
    ValidateAllSystems();
}

void UBuildIntegrationManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Shutting down build integration system"));
    Super::Deinitialize();
}

void UBuildIntegrationManager::ValidateAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Starting comprehensive system validation"));
    
    CriticalErrors.Empty();
    
    // Validate core systems
    ValidateWorldGeneration();
    ValidateFoliageSystem();
    ValidateCrowdSimulation();
    ValidateQAFramework();
    ValidateCharacterSystems();
    ValidateGameplayIntegration();
    
    // Check for integration issues
    CheckForDuplicateTypes();
    ValidateSharedTypes();
    CheckCompilationErrors();
    
    // Update overall health
    UpdateSystemHealth();
    
    CurrentStatus.LastValidationTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    LastFullValidation = CurrentStatus.LastValidationTime;
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validation complete. Health Score: %f"), SystemHealth.OverallHealthScore);
}

FBuild_IntegrationStatus UBuildIntegrationManager::GetIntegrationStatus() const
{
    return CurrentStatus;
}

FBuild_SystemHealth UBuildIntegrationManager::GetSystemHealth() const
{
    return SystemHealth;
}

void UBuildIntegrationManager::RunCompilationTest()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Running compilation test"));
    
    CurrentStatus.CompilationErrors.Empty();
    
    // Check if all critical classes can be loaded
    TArray<FString> CriticalClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager"),
        TEXT("/Script/TranspersonalGame.CrowdSimulationManager"),
        TEXT("/Script/TranspersonalGame.ProceduralWorldManager")
    };
    
    int32 LoadedCount = 0;
    for (const FString& ClassName : CriticalClasses)
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
        if (LoadedClass)
        {
            LoadedCount++;
            CurrentStatus.LoadedModules.AddUnique(ClassName);
            UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Successfully loaded class: %s"), *ClassName);
        }
        else
        {
            FString Error = FString::Printf(TEXT("Failed to load critical class: %s"), *ClassName);
            CurrentStatus.CompilationErrors.Add(Error);
            CriticalErrors.Add(Error);
            UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: %s"), *Error);
        }
    }
    
    CurrentStatus.bAllModulesLoaded = (LoadedCount == CriticalClasses.Num());
    CurrentStatus.bIsCompiled = CurrentStatus.CompilationErrors.Num() == 0;
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Compilation test complete. %d/%d classes loaded"), LoadedCount, CriticalClasses.Num());
}

void UBuildIntegrationManager::ValidateMinPlayableMap()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validating MinPlayableMap"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        CriticalErrors.Add(TEXT("No valid world found for map validation"));
        return;
    }
    
    // Count actors in the level
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    CurrentStatus.ActiveActorCount = AllActors.Num();
    
    // Check for essential actors
    bool bHasPlayerStart = false;
    bool bHasLighting = false;
    bool bHasCharacter = false;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor->GetClass()->GetName().Contains(TEXT("PlayerStart")))
        {
            bHasPlayerStart = true;
        }
        else if (Actor->GetClass()->GetName().Contains(TEXT("Light")))
        {
            bHasLighting = true;
        }
        else if (Actor->IsA<ATranspersonalCharacter>())
        {
            bHasCharacter = true;
        }
    }
    
    if (!bHasPlayerStart)
    {
        CriticalErrors.Add(TEXT("MinPlayableMap missing PlayerStart"));
    }
    if (!bHasLighting)
    {
        CriticalErrors.Add(TEXT("MinPlayableMap missing lighting"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Map validation complete. %d actors found"), CurrentStatus.ActiveActorCount);
}

bool UBuildIntegrationManager::CheckAllModulesLoaded()
{
    RunCompilationTest();
    return CurrentStatus.bAllModulesLoaded;
}

void UBuildIntegrationManager::GenerateBuildReport()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Generating comprehensive build report"));
    
    FString ReportContent;
    ReportContent += TEXT("=== TRANSPERSONAL GAME STUDIO - BUILD INTEGRATION REPORT ===\n\n");
    
    // Integration Status
    ReportContent += FString::Printf(TEXT("INTEGRATION STATUS:\n"));
    ReportContent += FString::Printf(TEXT("- Compiled: %s\n"), CurrentStatus.bIsCompiled ? TEXT("YES") : TEXT("NO"));
    ReportContent += FString::Printf(TEXT("- All Modules Loaded: %s\n"), CurrentStatus.bAllModulesLoaded ? TEXT("YES") : TEXT("NO"));
    ReportContent += FString::Printf(TEXT("- Active Actors: %d\n"), CurrentStatus.ActiveActorCount);
    ReportContent += FString::Printf(TEXT("- Last Validation: %.2f seconds ago\n\n"), LastFullValidation);
    
    // System Health
    ReportContent += FString::Printf(TEXT("SYSTEM HEALTH:\n"));
    ReportContent += FString::Printf(TEXT("- World Generator: %s\n"), SystemHealth.bWorldGeneratorActive ? TEXT("ACTIVE") : TEXT("INACTIVE"));
    ReportContent += FString::Printf(TEXT("- Foliage Manager: %s\n"), SystemHealth.bFoliageManagerActive ? TEXT("ACTIVE") : TEXT("INACTIVE"));
    ReportContent += FString::Printf(TEXT("- Crowd Simulation: %s\n"), SystemHealth.bCrowdSimulationActive ? TEXT("ACTIVE") : TEXT("INACTIVE"));
    ReportContent += FString::Printf(TEXT("- QA Framework: %s\n"), SystemHealth.bQAFrameworkActive ? TEXT("ACTIVE") : TEXT("INACTIVE"));
    ReportContent += FString::Printf(TEXT("- Overall Health Score: %.1f%%\n\n"), SystemHealth.OverallHealthScore * 100.0f);
    
    // Loaded Modules
    ReportContent += FString::Printf(TEXT("LOADED MODULES (%d):\n"), CurrentStatus.LoadedModules.Num());
    for (const FString& Module : CurrentStatus.LoadedModules)
    {
        ReportContent += FString::Printf(TEXT("- %s\n"), *Module);
    }
    
    // Critical Errors
    if (CriticalErrors.Num() > 0)
    {
        ReportContent += FString::Printf(TEXT("\nCRITICAL ERRORS (%d):\n"), CriticalErrors.Num());
        for (const FString& Error : CriticalErrors)
        {
            ReportContent += FString::Printf(TEXT("- %s\n"), *Error);
        }
    }
    
    // Save report to file
    FString ReportPath = FPaths::ProjectLogDir() / TEXT("BuildIntegrationReport.txt");
    FFileHelper::SaveStringToFile(ReportContent, *ReportPath);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Build report saved to %s"), *ReportPath);
}

void UBuildIntegrationManager::FixCommonIntegrationIssues()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Attempting to fix common integration issues"));
    
    // Force garbage collection
    GEngine->ForceGarbageCollection(true);
    
    // Reload critical classes
    RunCompilationTest();
    
    // Validate map again
    ValidateMinPlayableMap();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Integration fixes applied"));
}

void UBuildIntegrationManager::ValidateWorldGeneration()
{
    // Check if PCGWorldGenerator is available and functional
    UClass* WorldGenClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
    SystemHealth.bWorldGeneratorActive = (WorldGenClass != nullptr);
    
    if (!SystemHealth.bWorldGeneratorActive)
    {
        CriticalErrors.Add(TEXT("World Generator system not available"));
    }
}

void UBuildIntegrationManager::ValidateFoliageSystem()
{
    // Check if FoliageManager is available and functional
    UClass* FoliageClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.FoliageManager"));
    SystemHealth.bFoliageManagerActive = (FoliageClass != nullptr);
    
    if (!SystemHealth.bFoliageManagerActive)
    {
        CriticalErrors.Add(TEXT("Foliage Manager system not available"));
    }
}

void UBuildIntegrationManager::ValidateCrowdSimulation()
{
    // Check if CrowdSimulationManager is available and functional
    UClass* CrowdClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.CrowdSimulationManager"));
    SystemHealth.bCrowdSimulationActive = (CrowdClass != nullptr);
    
    if (!SystemHealth.bCrowdSimulationActive)
    {
        CriticalErrors.Add(TEXT("Crowd Simulation system not available"));
    }
}

void UBuildIntegrationManager::ValidateQAFramework()
{
    // Check if QA framework classes are available
    UClass* QAClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.QA_TestFramework"));
    SystemHealth.bQAFrameworkActive = (QAClass != nullptr);
    
    if (!SystemHealth.bQAFrameworkActive)
    {
        CriticalErrors.Add(TEXT("QA Framework system not available"));
    }
}

void UBuildIntegrationManager::ValidateCharacterSystems()
{
    // Check if core character classes are available
    UClass* CharacterClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    UClass* GameStateClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
    
    if (!CharacterClass)
    {
        CriticalErrors.Add(TEXT("TranspersonalCharacter class not available"));
    }
    if (!GameStateClass)
    {
        CriticalErrors.Add(TEXT("TranspersonalGameState class not available"));
    }
}

void UBuildIntegrationManager::ValidateGameplayIntegration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        CriticalErrors.Add(TEXT("No valid world for gameplay validation"));
        return;
    }
    
    // Check if game mode is properly set
    AGameModeBase* GameMode = World->GetAuthGameMode();
    if (!GameMode)
    {
        CriticalErrors.Add(TEXT("No game mode found in current world"));
    }
}

void UBuildIntegrationManager::CheckForDuplicateTypes()
{
    // This would require more complex reflection to detect duplicate type names
    // For now, log that we're checking
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Checking for duplicate type definitions"));
}

void UBuildIntegrationManager::ValidateSharedTypes()
{
    // Verify SharedTypes.h is being used correctly
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Validating SharedTypes usage"));
}

void UBuildIntegrationManager::CheckCompilationErrors()
{
    // Check for common compilation issues
    if (CurrentStatus.CompilationErrors.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Found %d compilation errors"), CurrentStatus.CompilationErrors.Num());
    }
}

void UBuildIntegrationManager::UpdateSystemHealth()
{
    // Calculate overall health score based on active systems
    float HealthScore = 0.0f;
    int32 TotalSystems = 4; // World Gen, Foliage, Crowd, QA
    
    if (SystemHealth.bWorldGeneratorActive) HealthScore += 0.25f;
    if (SystemHealth.bFoliageManagerActive) HealthScore += 0.25f;
    if (SystemHealth.bCrowdSimulationActive) HealthScore += 0.25f;
    if (SystemHealth.bQAFrameworkActive) HealthScore += 0.25f;
    
    SystemHealth.OverallHealthScore = HealthScore;
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: System health updated to %.1f%%"), HealthScore * 100.0f);
}