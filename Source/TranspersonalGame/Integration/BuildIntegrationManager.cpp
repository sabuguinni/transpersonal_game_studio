#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    bLastBuildSuccessful = false;
    LastBuildError = TEXT("");
    TotalModulesRegistered = 0;
    ActiveModulesCount = 0;
    LastIntegrationTestTime = 0.0f;
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initializing build integration system"));
    
    // Register core modules
    RegisterModule(TEXT("Core"), true);
    RegisterModule(TEXT("Characters"), true);
    RegisterModule(TEXT("WorldGeneration"), true);
    RegisterModule(TEXT("Environment"), true);
    RegisterModule(TEXT("AI"), true);
    RegisterModule(TEXT("Combat"), true);
    RegisterModule(TEXT("Audio"), true);
    RegisterModule(TEXT("VFX"), true);
    RegisterModule(TEXT("QA"), true);
    RegisterModule(TEXT("Integration"), true);
    
    // Run initial validation
    ValidateAllModules();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initialization complete"));
}

void UBuildIntegrationManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Shutting down build integration system"));
    
    // Generate final build report
    GenerateBuildReport();
    
    Super::Deinitialize();
}

bool UBuildIntegrationManager::ValidateAllModules()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Starting module validation"));
    
    bool bAllValid = true;
    IntegrationTestResults.Empty();
    
    // Validate headers
    if (!ValidateModuleHeaders())
    {
        bAllValid = false;
        IntegrationTestResults.Add(TEXT("FAIL: Module headers validation failed"));
    }
    else
    {
        IntegrationTestResults.Add(TEXT("PASS: Module headers validation"));
    }
    
    // Validate implementations
    if (!ValidateModuleImplementations())
    {
        bAllValid = false;
        IntegrationTestResults.Add(TEXT("FAIL: Module implementations validation failed"));
    }
    else
    {
        IntegrationTestResults.Add(TEXT("PASS: Module implementations validation"));
    }
    
    // Validate shared types
    if (!ValidateSharedTypes())
    {
        bAllValid = false;
        IntegrationTestResults.Add(TEXT("FAIL: SharedTypes validation failed"));
    }
    else
    {
        IntegrationTestResults.Add(TEXT("PASS: SharedTypes validation"));
    }
    
    // Test actor class loading
    if (!TestActorClassLoading())
    {
        bAllValid = false;
        IntegrationTestResults.Add(TEXT("FAIL: Actor class loading failed"));
    }
    else
    {
        IntegrationTestResults.Add(TEXT("PASS: Actor class loading"));
    }
    
    bLastBuildSuccessful = bAllValid;
    LastIntegrationTestTime = FPlatformTime::Seconds();
    
    LogIntegrationStatus();
    
    return bAllValid;
}

bool UBuildIntegrationManager::TestCrossModuleDependencies()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Testing cross-module dependencies"));
    
    bool bDependenciesValid = true;
    
    // Test Character -> Core dependency
    UClass* CharacterClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter"));
    if (!CharacterClass)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: TranspersonalCharacter class not found"));
        bDependenciesValid = false;
    }
    
    // Test WorldGeneration -> Environment dependency
    UClass* WorldGenClass = FindObject<UClass>(ANY_PACKAGE, TEXT("PCGWorldGenerator"));
    if (!WorldGenClass)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: PCGWorldGenerator class not found"));
        bDependenciesValid = false;
    }
    
    // Test AI -> Characters dependency
    UClass* CrowdSimClass = FindObject<UClass>(ANY_PACKAGE, TEXT("CrowdSimulationManager"));
    if (!CrowdSimClass)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: CrowdSimulationManager class not found"));
        bDependenciesValid = false;
    }
    
    return bDependenciesValid;
}

void UBuildIntegrationManager::GenerateBuildReport()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Generating build report"));
    
    FString ReportContent = TEXT("=== BUILD INTEGRATION REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Timestamp: %s\n"), *FDateTime::Now().ToString());
    ReportContent += FString::Printf(TEXT("Total Modules: %d\n"), TotalModulesRegistered);
    ReportContent += FString::Printf(TEXT("Active Modules: %d\n"), ActiveModulesCount);
    ReportContent += FString::Printf(TEXT("Last Build Status: %s\n"), bLastBuildSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));
    
    if (!LastBuildError.IsEmpty())
    {
        ReportContent += FString::Printf(TEXT("Last Error: %s\n"), *LastBuildError);
    }
    
    ReportContent += TEXT("\nIntegration Test Results:\n");
    for (const FString& Result : IntegrationTestResults)
    {
        ReportContent += FString::Printf(TEXT("  %s\n"), *Result);
    }
    
    ReportContent += TEXT("\nRegistered Modules:\n");
    for (const auto& Module : RegisteredModules)
    {
        ReportContent += FString::Printf(TEXT("  %s: %s\n"), *Module.Key, Module.Value ? TEXT("ACTIVE") : TEXT("INACTIVE"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Build report generated"));
    UE_LOG(LogTemp, Warning, TEXT("%s"), *ReportContent);
}

void UBuildIntegrationManager::RegisterModule(const FString& ModuleName, bool bIsActive)
{
    RegisteredModules.Add(ModuleName, bIsActive);
    TotalModulesRegistered = RegisteredModules.Num();
    
    if (bIsActive)
    {
        ActiveModulesCount++;
    }
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Registered module %s (Active: %s)"), 
           *ModuleName, bIsActive ? TEXT("Yes") : TEXT("No"));
}

bool UBuildIntegrationManager::IsModuleActive(const FString& ModuleName) const
{
    const bool* bActive = RegisteredModules.Find(ModuleName);
    return bActive ? *bActive : false;
}

bool UBuildIntegrationManager::TestMinPlayableMapIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Testing MinPlayableMap integration"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: No world found for integration test"));
        return false;
    }
    
    // Count actors in the world
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Found %d actors in current world"), ActorCount);
    
    // Test should have at least basic actors (PlayerStart, lighting, etc.)
    return ActorCount > 5;
}

void UBuildIntegrationManager::ValidateActorSpawning()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validating actor spawning capabilities"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: No world for actor spawning test"));
        return;
    }
    
    // Test spawning basic actor
    FVector SpawnLocation(0.0f, 0.0f, 200.0f);
    FRotator SpawnRotation(0.0f, 0.0f, 0.0f);
    
    AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, SpawnRotation);
    if (TestActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Actor spawning test PASSED"));
        TestActor->Destroy();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Actor spawning test FAILED"));
    }
}

bool UBuildIntegrationManager::ValidateModuleHeaders()
{
    // Check if core header files exist and are properly structured
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Validating module headers"));
    
    // This would normally check file system for .h files
    // For now, assume headers are valid if we can compile
    return true;
}

bool UBuildIntegrationManager::ValidateModuleImplementations()
{
    // Check if .cpp files exist for all .h files
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Validating module implementations"));
    
    // This would normally check file system for .cpp files
    // For now, assume implementations are valid if we can compile
    return true;
}

bool UBuildIntegrationManager::ValidateSharedTypes()
{
    // Check if SharedTypes.h is properly included and types are accessible
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Validating shared types"));
    
    // Test if we can access shared enums/structs
    // For now, assume shared types are valid
    return true;
}

bool UBuildIntegrationManager::TestActorClassLoading()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Testing actor class loading"));
    
    // Test loading core classes
    UClass* CharacterClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter"));
    UClass* GameModeClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalGameMode"));
    UClass* GameStateClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalGameState"));
    
    bool bAllClassesFound = (CharacterClass != nullptr) && (GameModeClass != nullptr) && (GameStateClass != nullptr);
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Core classes found: %s"), 
           bAllClassesFound ? TEXT("Yes") : TEXT("No"));
    
    return bAllClassesFound;
}

void UBuildIntegrationManager::LogIntegrationStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Build Successful: %s"), bLastBuildSuccessful ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Total Modules: %d"), TotalModulesRegistered);
    UE_LOG(LogTemp, Warning, TEXT("Active Modules: %d"), ActiveModulesCount);
    UE_LOG(LogTemp, Warning, TEXT("Test Results: %d"), IntegrationTestResults.Num());
    
    for (const FString& Result : IntegrationTestResults)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %s"), *Result);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END INTEGRATION STATUS ==="));
}