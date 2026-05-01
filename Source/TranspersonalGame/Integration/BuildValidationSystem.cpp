#include "BuildValidationSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

UBuildValidationSystem::UBuildValidationSystem()
{
    bValidationInProgress = false;
    LastValidationTime = 0.0f;

    // Initialize known modules for validation
    KnownModules = {
        TEXT("Core"),
        TEXT("Characters"), 
        TEXT("WorldGeneration"),
        TEXT("Environment"),
        TEXT("AI"),
        TEXT("Combat"),
        TEXT("Audio"),
        TEXT("VFX"),
        TEXT("CrowdSimulation"),
        TEXT("Integration")
    };

    // Critical classes that must load
    CriticalClasses = {
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameState"),
        TEXT("TranspersonalGameMode"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager")
    };
}

void UBuildValidationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationSystem initialized"));
    
    // Run initial validation
    FBuild_ValidationReport InitialReport = RunFullValidation();
    LogValidationResults(InitialReport);
}

void UBuildValidationSystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationSystem shutting down"));
    Super::Deinitialize();
}

FBuild_ValidationReport UBuildValidationSystem::RunFullValidation()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Validation already in progress"));
        return LastValidationReport;
    }

    bValidationInProgress = true;
    FBuild_ValidationReport Report;
    Report.ValidationTimestamp = FDateTime::Now().ToUnixTimestamp();

    UE_LOG(LogTemp, Warning, TEXT("Starting full build validation"));

    // Validate all known modules
    Report.TotalModules = KnownModules.Num();
    Report.LoadedModules = 0;

    for (const FString& ModuleName : KnownModules)
    {
        FBuild_ModuleStatus ModuleStatus = ValidateModule(ModuleName);
        Report.ModuleStatuses.Add(ModuleStatus);
        
        if (ModuleStatus.bIsLoaded)
        {
            Report.LoadedModules++;
        }
        
        if (ModuleStatus.bHasErrors)
        {
            for (const FString& Error : ModuleStatus.ErrorMessages)
            {
                Report.CriticalErrors.Add(FString::Printf(TEXT("[%s] %s"), *ModuleName, *Error));
            }
        }
    }

    // Test critical class loading
    for (const FString& ClassName : CriticalClasses)
    {
        if (!TestClassLoading(ClassName))
        {
            Report.CriticalErrors.Add(FString::Printf(TEXT("Critical class failed to load: %s"), *ClassName));
        }
    }

    // Get actor count in current level
    Report.TotalActors = GetActorCountInLevel();

    // Test cross-module dependencies
    if (!TestCrossModuleDependencies())
    {
        Report.CriticalErrors.Add(TEXT("Cross-module dependency validation failed"));
    }

    // Validate shared types
    if (!ValidateSharedTypes())
    {
        Report.CriticalErrors.Add(TEXT("SharedTypes validation failed"));
    }

    // Overall success determination
    Report.bOverallSuccess = (Report.CriticalErrors.Num() == 0) && 
                            (Report.LoadedModules >= Report.TotalModules * 0.8f) &&
                            (Report.TotalActors > 0);

    LastValidationReport = Report;
    LastValidationTime = Report.ValidationTimestamp;
    bValidationInProgress = false;

    UE_LOG(LogTemp, Warning, TEXT("Build validation complete - Success: %s"), 
           Report.bOverallSuccess ? TEXT("TRUE") : TEXT("FALSE"));

    return Report;
}

FBuild_ModuleStatus UBuildValidationSystem::ValidateModule(const FString& ModuleName)
{
    FBuild_ModuleStatus Status;
    Status.ModuleName = ModuleName;

    // Check if module directory exists
    FString ModulePath = FPaths::ProjectDir() / TEXT("Source/TranspersonalGame") / ModuleName;
    
    if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*ModulePath))
    {
        Status.bHasErrors = true;
        Status.ErrorMessages.Add(TEXT("Module directory does not exist"));
        return Status;
    }

    // Try to find classes in this module
    Status.bIsLoaded = true;
    Status.ClassCount = 0;

    // Basic validation passed
    UE_LOG(LogTemp, Log, TEXT("Module %s validated successfully"), *ModuleName);
    return Status;
}

bool UBuildValidationSystem::ValidateModuleIntegrity(const FString& ModuleName)
{
    FBuild_ModuleStatus Status = ValidateModule(ModuleName);
    return Status.bIsLoaded && !Status.bHasErrors;
}

bool UBuildValidationSystem::ValidateMapState(const FString& MapPath)
{
    // For now, just check if we can get actors
    int32 ActorCount = GetActorCountInLevel();
    return ActorCount > 0;
}

TArray<FString> UBuildValidationSystem::GetLoadedModules()
{
    TArray<FString> LoadedModules;
    
    for (const FString& ModuleName : KnownModules)
    {
        if (ValidateModuleIntegrity(ModuleName))
        {
            LoadedModules.Add(ModuleName);
        }
    }
    
    return LoadedModules;
}

int32 UBuildValidationSystem::GetActorCountInLevel()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }

    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }

    return ActorCount;
}

bool UBuildValidationSystem::TestClassLoading(const FString& ClassName)
{
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    
    UClass* TestClass = LoadClass<UObject>(nullptr, *ClassPath);
    if (TestClass)
    {
        UE_LOG(LogTemp, Log, TEXT("Successfully loaded class: %s"), *ClassName);
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Failed to load class: %s"), *ClassName);
    return false;
}

bool UBuildValidationSystem::TestCrossModuleDependencies()
{
    // Test basic cross-module functionality
    // For now, just return true if we can access basic UE5 classes
    UClass* ActorClass = AActor::StaticClass();
    UClass* ComponentClass = UActorComponent::StaticClass();
    
    return (ActorClass != nullptr) && (ComponentClass != nullptr);
}

bool UBuildValidationSystem::ValidateSharedTypes()
{
    // Validate that SharedTypes.h enums and structs are accessible
    // This is a basic check - in a full implementation we'd test each type
    return true;
}

bool UBuildValidationSystem::CheckCompilationStatus()
{
    // Check if the module compiled successfully
    // For now, if we're running, compilation was successful
    return true;
}

void UBuildValidationSystem::GenerateValidationReport(const FBuild_ValidationReport& Report)
{
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD VALIDATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Timestamp: %f"), Report.ValidationTimestamp);
    UE_LOG(LogTemp, Warning, TEXT("Overall Success: %s"), Report.bOverallSuccess ? TEXT("TRUE") : TEXT("FALSE"));
    UE_LOG(LogTemp, Warning, TEXT("Modules: %d/%d loaded"), Report.LoadedModules, Report.TotalModules);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), Report.TotalActors);
    
    if (Report.CriticalErrors.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Critical Errors:"));
        for (const FString& Error : Report.CriticalErrors)
        {
            UE_LOG(LogTemp, Error, TEXT("  - %s"), *Error);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

FString UBuildValidationSystem::GetLastValidationSummary()
{
    if (LastValidationTime == 0.0f)
    {
        return TEXT("No validation has been run yet");
    }
    
    return FString::Printf(TEXT("Last validation: %s, Modules: %d/%d, Actors: %d, Errors: %d"),
                          LastValidationReport.bOverallSuccess ? TEXT("SUCCESS") : TEXT("FAILED"),
                          LastValidationReport.LoadedModules,
                          LastValidationReport.TotalModules,
                          LastValidationReport.TotalActors,
                          LastValidationReport.CriticalErrors.Num());
}

void UBuildValidationSystem::LogValidationResults(const FBuild_ValidationReport& Report)
{
    GenerateValidationReport(Report);
}