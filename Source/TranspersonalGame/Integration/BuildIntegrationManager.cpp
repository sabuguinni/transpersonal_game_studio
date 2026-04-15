#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogBuildIntegration, Log, All);

UBuildIntegrationManager::UBuildIntegrationManager()
{
    CurrentBuildStatus = EBuild_BuildStatus::NotStarted;
    BuildProgress = 0.0f;

    // Initialize core system classes that must be validated
    CoreSystemClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalGameMode"),
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager"),
        TEXT("/Script/TranspersonalGame.CrowdSimulationManager"),
        TEXT("/Script/TranspersonalGame.ProceduralWorldManager")
    };
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogBuildIntegration, Log, TEXT("BuildIntegrationManager initialized"));
    
    // Start initial validation
    UpdateBuildStatus(EBuild_BuildStatus::Initializing);
    
    // Schedule initial system validation
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            SystemCheckTimer,
            this,
            &UBuildIntegrationManager::ValidateAllSystems,
            2.0f,
            false
        );
    }
}

void UBuildIntegrationManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(BuildValidationTimer);
        World->GetTimerManager().ClearTimer(SystemCheckTimer);
    }
    
    Super::Deinitialize();
}

void UBuildIntegrationManager::StartBuildProcess()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Starting build process"));
    
    UpdateBuildStatus(EBuild_BuildStatus::Building);
    BuildProgress = 0.0f;
    IntegrationErrors.Empty();
    
    // Clear previous validation results
    ValidatedSystems.Empty();
    FailedSystems.Empty();
    
    // Start validation process
    ValidateAllSystems();
}

void UBuildIntegrationManager::ValidateAllSystems()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validating all systems"));
    
    UpdateBuildStatus(EBuild_BuildStatus::Validating);
    BuildProgress = 0.1f;
    
    bool bAllSystemsValid = true;
    
    // Validate core game systems
    if (ValidateCoreGameSystems())
    {
        ValidatedSystems.Add(TEXT("CoreGameSystems"));
        BuildProgress = 0.3f;
    }
    else
    {
        FailedSystems.Add(TEXT("CoreGameSystems"));
        bAllSystemsValid = false;
    }
    
    // Validate world generation
    if (ValidateWorldGeneration())
    {
        ValidatedSystems.Add(TEXT("WorldGeneration"));
        BuildProgress = 0.5f;
    }
    else
    {
        FailedSystems.Add(TEXT("WorldGeneration"));
        bAllSystemsValid = false;
    }
    
    // Validate character systems
    if (ValidateCharacterSystems())
    {
        ValidatedSystems.Add(TEXT("CharacterSystems"));
        BuildProgress = 0.7f;
    }
    else
    {
        FailedSystems.Add(TEXT("CharacterSystems"));
        bAllSystemsValid = false;
    }
    
    // Validate AI systems
    if (ValidateAISystems())
    {
        ValidatedSystems.Add(TEXT("AISystems"));
        BuildProgress = 0.9f;
    }
    else
    {
        FailedSystems.Add(TEXT("AISystems"));
        bAllSystemsValid = false;
    }
    
    // Validate environment systems
    if (ValidateEnvironmentSystems())
    {
        ValidatedSystems.Add(TEXT("EnvironmentSystems"));
        BuildProgress = 1.0f;
    }
    else
    {
        FailedSystems.Add(TEXT("EnvironmentSystems"));
        bAllSystemsValid = false;
    }
    
    // Update final status
    if (bAllSystemsValid)
    {
        UpdateBuildStatus(EBuild_BuildStatus::Success);
        UE_LOG(LogBuildIntegration, Log, TEXT("All systems validated successfully"));
    }
    else
    {
        UpdateBuildStatus(EBuild_BuildStatus::Failed);
        UE_LOG(LogBuildIntegration, Warning, TEXT("System validation failed. Failed systems: %d"), FailedSystems.Num());
    }
}

TArray<FString> UBuildIntegrationManager::GetFailedModules() const
{
    TArray<FString> FailedModules;
    
    for (const auto& ModulePair : ModuleStatuses)
    {
        if (ModulePair.Value == EBuild_ModuleStatus::Failed || ModulePair.Value == EBuild_ModuleStatus::Error)
        {
            FailedModules.Add(ModulePair.Key);
        }
    }
    
    return FailedModules;
}

bool UBuildIntegrationManager::ValidateModule(const FString& ModuleName)
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validating module: %s"), *ModuleName);
    
    UpdateModuleStatus(ModuleName, EBuild_ModuleStatus::Validating);
    
    // Check if module has required classes
    if (ModuleRequiredClasses.Contains(ModuleName))
    {
        const TArray<FString>& RequiredClasses = ModuleRequiredClasses[ModuleName];
        
        for (const FString& ClassName : RequiredClasses)
        {
            if (!ValidateClassLoading(ClassName))
            {
                LogIntegrationError(FString::Printf(TEXT("Module %s: Failed to load required class %s"), *ModuleName, *ClassName));
                UpdateModuleStatus(ModuleName, EBuild_ModuleStatus::Failed);
                return false;
            }
        }
    }
    
    UpdateModuleStatus(ModuleName, EBuild_ModuleStatus::Loaded);
    return true;
}

void UBuildIntegrationManager::RegisterModuleForValidation(const FString& ModuleName, const TArray<FString>& RequiredClasses)
{
    ModuleRequiredClasses.Add(ModuleName, RequiredClasses);
    UpdateModuleStatus(ModuleName, EBuild_ModuleStatus::Registered);
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Registered module %s with %d required classes"), *ModuleName, RequiredClasses.Num());
}

EBuild_ModuleStatus UBuildIntegrationManager::GetModuleStatus(const FString& ModuleName) const
{
    if (const EBuild_ModuleStatus* Status = ModuleStatuses.Find(ModuleName))
    {
        return *Status;
    }
    return EBuild_ModuleStatus::Unknown;
}

bool UBuildIntegrationManager::ValidateSystemIntegration()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validating system integration"));
    
    // Test cross-system dependencies
    bool bIntegrationValid = true;
    
    // Test GameMode -> Character integration
    UClass* GameModeClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameMode"));
    UClass* CharacterClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    
    if (!GameModeClass || !CharacterClass)
    {
        LogIntegrationError(TEXT("Core game classes not available for integration test"));
        bIntegrationValid = false;
    }
    
    // Test WorldGen -> Environment integration
    UClass* WorldGenClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
    UClass* FoliageClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.FoliageManager"));
    
    if (!WorldGenClass || !FoliageClass)
    {
        LogIntegrationError(TEXT("World generation classes not available for integration test"));
        bIntegrationValid = false;
    }
    
    return bIntegrationValid;
}

void UBuildIntegrationManager::RunIntegrationTests()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Running integration tests"));
    
    IntegrationErrors.Empty();
    
    // Test 1: Class loading
    for (const FString& ClassName : CoreSystemClasses)
    {
        if (!ValidateClassLoading(ClassName))
        {
            LogIntegrationError(FString::Printf(TEXT("Integration test failed: Cannot load class %s"), *ClassName));
        }
    }
    
    // Test 2: System integration
    if (!ValidateSystemIntegration())
    {
        LogIntegrationError(TEXT("System integration validation failed"));
    }
    
    // Test 3: Module dependencies
    for (const auto& ModulePair : ModuleStatuses)
    {
        if (!ValidateModule(ModulePair.Key))
        {
            LogIntegrationError(FString::Printf(TEXT("Module validation failed: %s"), *ModulePair.Key));
        }
    }
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Integration tests completed. Errors: %d"), IntegrationErrors.Num());
}

bool UBuildIntegrationManager::ValidateClassLoading(const FString& ClassName)
{
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
    return LoadedClass != nullptr;
}

bool UBuildIntegrationManager::ValidateCoreGameSystems()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validating core game systems"));
    
    TArray<FString> CoreClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalGameMode"),
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState")
    };
    
    for (const FString& ClassName : CoreClasses)
    {
        if (!ValidateClassLoading(ClassName))
        {
            LogIntegrationError(FString::Printf(TEXT("Core system validation failed: %s"), *ClassName));
            return false;
        }
    }
    
    return true;
}

bool UBuildIntegrationManager::ValidateWorldGeneration()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validating world generation systems"));
    
    return ValidateClassLoading(TEXT("/Script/TranspersonalGame.PCGWorldGenerator")) &&
           ValidateClassLoading(TEXT("/Script/TranspersonalGame.ProceduralWorldManager"));
}

bool UBuildIntegrationManager::ValidateCharacterSystems()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validating character systems"));
    
    return ValidateClassLoading(TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
}

bool UBuildIntegrationManager::ValidateAISystems()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validating AI systems"));
    
    return ValidateClassLoading(TEXT("/Script/TranspersonalGame.CrowdSimulationManager"));
}

bool UBuildIntegrationManager::ValidateEnvironmentSystems()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validating environment systems"));
    
    return ValidateClassLoading(TEXT("/Script/TranspersonalGame.FoliageManager"));
}

void UBuildIntegrationManager::UpdateBuildStatus(EBuild_BuildStatus NewStatus)
{
    if (CurrentBuildStatus != NewStatus)
    {
        CurrentBuildStatus = NewStatus;
        OnBuildStatusChanged.Broadcast(NewStatus);
        
        UE_LOG(LogBuildIntegration, Log, TEXT("Build status changed to: %d"), (int32)NewStatus);
    }
}

void UBuildIntegrationManager::UpdateModuleStatus(const FString& ModuleName, EBuild_ModuleStatus NewStatus)
{
    EBuild_ModuleStatus* CurrentStatus = ModuleStatuses.Find(ModuleName);
    if (!CurrentStatus || *CurrentStatus != NewStatus)
    {
        ModuleStatuses.Add(ModuleName, NewStatus);
        OnModuleStatusChanged.Broadcast(ModuleName, NewStatus);
        
        UE_LOG(LogBuildIntegration, Log, TEXT("Module %s status changed to: %d"), *ModuleName, (int32)NewStatus);
    }
}

void UBuildIntegrationManager::LogIntegrationError(const FString& Error)
{
    IntegrationErrors.Add(Error);
    UE_LOG(LogBuildIntegration, Error, TEXT("Integration Error: %s"), *Error);
}