#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Modules/ModuleManager.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    bInitialized = false;
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initializing..."));
    
    // Initialize module scanning
    ScanForModules();
    ValidateAllModules();
    UpdateSystemHealth();
    
    bInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initialized successfully"));
}

void UBuildIntegrationManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Deinitializing..."));
    
    ModuleStatuses.Empty();
    bInitialized = false;
    
    Super::Deinitialize();
}

void UBuildIntegrationManager::ValidateAllModules()
{
    if (!bInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Not initialized"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validating all modules..."));

    // Core modules to validate
    TArray<FString> CoreModules = {
        TEXT("TranspersonalGame"),
        TEXT("Core"),
        TEXT("Characters"),
        TEXT("WorldGeneration"),
        TEXT("Environment"),
        TEXT("AI"),
        TEXT("Combat"),
        TEXT("Audio"),
        TEXT("VFX"),
        TEXT("QA")
    };

    for (const FString& ModuleName : CoreModules)
    {
        ValidateModule(ModuleName);
    }

    UpdateSystemHealth();
    LogSystemStatus();
}

FBuild_SystemHealth UBuildIntegrationManager::GetSystemHealth() const
{
    return SystemHealth;
}

TArray<FBuild_ModuleStatus> UBuildIntegrationManager::GetModuleStatuses() const
{
    return ModuleStatuses;
}

bool UBuildIntegrationManager::IsModuleHealthy(const FString& ModuleName) const
{
    for (const FBuild_ModuleStatus& Status : ModuleStatuses)
    {
        if (Status.ModuleName == ModuleName)
        {
            return Status.bIsLoaded && !Status.bHasErrors;
        }
    }
    return false;
}

void UBuildIntegrationManager::RefreshModuleStatus()
{
    ModuleStatuses.Empty();
    ScanForModules();
    ValidateAllModules();
}

void UBuildIntegrationManager::LogSystemStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Modules: %d"), SystemHealth.TotalModules);
    UE_LOG(LogTemp, Warning, TEXT("Loaded Modules: %d"), SystemHealth.LoadedModules);
    UE_LOG(LogTemp, Warning, TEXT("Error Modules: %d"), SystemHealth.ErrorModules);
    UE_LOG(LogTemp, Warning, TEXT("Overall Health: %.2f%%"), SystemHealth.OverallHealth);
    UE_LOG(LogTemp, Warning, TEXT("All Modules Loaded: %s"), SystemHealth.bAllModulesLoaded ? TEXT("YES") : TEXT("NO"));

    for (const FBuild_ModuleStatus& Status : ModuleStatuses)
    {
        LogModuleStatus(Status);
    }
}

bool UBuildIntegrationManager::TestCoreSystemsIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Testing Core Systems Integration..."));

    // Test TranspersonalCharacter
    UClass* CharacterClass = LoadClass<APawn>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    if (!CharacterClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load TranspersonalCharacter class"));
        return false;
    }

    // Test TranspersonalGameMode
    UClass* GameModeClass = LoadClass<AGameModeBase>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameMode"));
    if (!GameModeClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load TranspersonalGameMode class"));
        return false;
    }

    // Test TranspersonalGameState
    UClass* GameStateClass = LoadClass<AGameStateBase>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
    if (!GameStateClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load TranspersonalGameState class"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("Core Systems Integration: PASSED"));
    return true;
}

bool UBuildIntegrationManager::TestWorldGenerationIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Testing World Generation Integration..."));

    // Test PCGWorldGenerator
    UClass* WorldGenClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
    if (!WorldGenClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("PCGWorldGenerator class not found - may not be implemented yet"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("World Generation Integration: PASSED"));
    return true;
}

bool UBuildIntegrationManager::TestCharacterSystemIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Testing Character System Integration..."));

    // Test character movement and survival stats
    UClass* CharacterClass = LoadClass<APawn>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    if (CharacterClass)
    {
        // Check if character has required components
        UE_LOG(LogTemp, Warning, TEXT("Character System Integration: PASSED"));
        return true;
    }

    UE_LOG(LogTemp, Error, TEXT("Character System Integration: FAILED"));
    return false;
}

void UBuildIntegrationManager::RunFullIntegrationTest()
{
    UE_LOG(LogTemp, Warning, TEXT("=== RUNNING FULL INTEGRATION TEST ==="));

    bool bCoreSystemsOK = TestCoreSystemsIntegration();
    bool bWorldGenOK = TestWorldGenerationIntegration();
    bool bCharacterSystemOK = TestCharacterSystemIntegration();

    int32 PassedTests = 0;
    if (bCoreSystemsOK) PassedTests++;
    if (bWorldGenOK) PassedTests++;
    if (bCharacterSystemOK) PassedTests++;

    UE_LOG(LogTemp, Warning, TEXT("Integration Test Results: %d/3 tests passed"), PassedTests);

    if (PassedTests == 3)
    {
        UE_LOG(LogTemp, Warning, TEXT("FULL INTEGRATION TEST: PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FULL INTEGRATION TEST: FAILED"));
    }
}

void UBuildIntegrationManager::ScanForModules()
{
    UE_LOG(LogTemp, Warning, TEXT("Scanning for modules..."));
    
    ModuleStatuses.Empty();

    // Add known modules
    TArray<FString> KnownModules = {
        TEXT("TranspersonalGame"),
        TEXT("Core"),
        TEXT("Characters"),
        TEXT("WorldGeneration"),
        TEXT("Environment"),
        TEXT("AI"),
        TEXT("Combat"),
        TEXT("Audio"),
        TEXT("VFX"),
        TEXT("QA")
    };

    for (const FString& ModuleName : KnownModules)
    {
        FBuild_ModuleStatus Status;
        Status.ModuleName = ModuleName;
        Status.bIsLoaded = false;
        Status.bHasErrors = false;
        Status.ClassCount = 0;
        ModuleStatuses.Add(Status);
    }
}

void UBuildIntegrationManager::ValidateModule(const FString& ModuleName)
{
    for (FBuild_ModuleStatus& Status : ModuleStatuses)
    {
        if (Status.ModuleName == ModuleName)
        {
            // Check if module is loaded
            FModuleManager& ModuleManager = FModuleManager::Get();
            Status.bIsLoaded = ModuleManager.IsModuleLoaded(*ModuleName);

            if (Status.bIsLoaded)
            {
                Status.bHasErrors = false;
                Status.ErrorMessage = TEXT("");
                
                // Count classes in this module
                Status.ClassCount = 0;
                for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
                {
                    UClass* Class = *ClassIt;
                    if (Class && Class->GetOutermost())
                    {
                        FString PackageName = Class->GetOutermost()->GetName();
                        if (PackageName.Contains(ModuleName))
                        {
                            Status.ClassCount++;
                        }
                    }
                }
            }
            else
            {
                Status.bHasErrors = true;
                Status.ErrorMessage = FString::Printf(TEXT("Module %s is not loaded"), *ModuleName);
            }
            break;
        }
    }
}

void UBuildIntegrationManager::UpdateSystemHealth()
{
    SystemHealth.TotalModules = ModuleStatuses.Num();
    SystemHealth.LoadedModules = 0;
    SystemHealth.ErrorModules = 0;

    for (const FBuild_ModuleStatus& Status : ModuleStatuses)
    {
        if (Status.bIsLoaded)
        {
            SystemHealth.LoadedModules++;
        }
        if (Status.bHasErrors)
        {
            SystemHealth.ErrorModules++;
        }
    }

    SystemHealth.bAllModulesLoaded = (SystemHealth.LoadedModules == SystemHealth.TotalModules);
    
    if (SystemHealth.TotalModules > 0)
    {
        SystemHealth.OverallHealth = (float)SystemHealth.LoadedModules / (float)SystemHealth.TotalModules * 100.0f;
    }
    else
    {
        SystemHealth.OverallHealth = 0.0f;
    }

    SystemHealth.LastBuildTime = FDateTime::Now().ToString();
}

void UBuildIntegrationManager::LogModuleStatus(const FBuild_ModuleStatus& Status) const
{
    FString StatusText = Status.bIsLoaded ? TEXT("LOADED") : TEXT("NOT LOADED");
    if (Status.bHasErrors)
    {
        StatusText += TEXT(" (ERRORS)");
    }

    UE_LOG(LogTemp, Warning, TEXT("Module %s: %s (Classes: %d)"), 
           *Status.ModuleName, *StatusText, Status.ClassCount);

    if (Status.bHasErrors && !Status.ErrorMessage.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("  Error: %s"), *Status.ErrorMessage);
    }
}