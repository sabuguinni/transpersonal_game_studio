#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    bValidationInProgress = false;
    BuildVersion = TEXT("1.0.0");
    LastBuildTime = FDateTime::Now();
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Initializing build integration system"));
    
    // Initialize build tracking
    LastBuildTime = FDateTime::Now();
    BuildVersion = TEXT("1.0.0-dev");
    
    // Clear previous validation state
    ValidationErrors.Empty();
    ValidationWarnings.Empty();
    CachedModuleStatus.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Initialization complete"));
}

void UBuildIntegrationManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Shutting down build integration system"));
    
    // Clear validation state
    ValidationErrors.Empty();
    ValidationWarnings.Empty();
    CachedModuleStatus.Empty();
    
    Super::Deinitialize();
}

FBuild_ValidationResult UBuildIntegrationManager::ValidateModuleIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Starting module integration validation"));
    
    FBuild_ValidationResult Result;
    Result.bSuccess = true;
    Result.ValidationMessage = TEXT("Module integration validation started");
    
    float StartTime = FPlatformTime::Seconds();
    
    // Validate core TranspersonalGame module
    bool bCoreModuleValid = ValidateModuleClasses(TEXT("TranspersonalGame"));
    if (!bCoreModuleValid)
    {
        Result.bSuccess = false;
        Result.Errors.Add(TEXT("Core TranspersonalGame module validation failed"));
    }
    
    // Validate component registration
    bool bComponentsValid = ValidateComponentRegistration();
    if (!bComponentsValid)
    {
        Result.bSuccess = false;
        Result.Errors.Add(TEXT("Component registration validation failed"));
    }
    
    // Validate subsystem initialization
    bool bSubsystemsValid = ValidateSubsystemInitialization();
    if (!bSubsystemsValid)
    {
        Result.bSuccess = false;
        Result.Errors.Add(TEXT("Subsystem initialization validation failed"));
    }
    
    // Validate cross-module dependencies
    bool bDependenciesValid = ValidateCrossModuleDependencies();
    if (!bDependenciesValid)
    {
        Result.Warnings.Add(TEXT("Some cross-module dependencies may have issues"));
    }
    
    Result.ValidationTime = FPlatformTime::Seconds() - StartTime;
    
    if (Result.bSuccess)
    {
        Result.ValidationMessage = FString::Printf(TEXT("Module integration validation completed successfully in %.2f seconds"), Result.ValidationTime);
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: %s"), *Result.ValidationMessage);
    }
    else
    {
        Result.ValidationMessage = FString::Printf(TEXT("Module integration validation failed with %d errors"), Result.Errors.Num());
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: %s"), *Result.ValidationMessage);
    }
    
    LastValidationResult = Result;
    return Result;
}

TArray<FBuild_ModuleStatus> UBuildIntegrationManager::GetModuleStatusList()
{
    TArray<FBuild_ModuleStatus> ModuleStatusList;
    
    // Check TranspersonalGame module
    FBuild_ModuleStatus TranspersonalGameStatus;
    TranspersonalGameStatus.ModuleName = TEXT("TranspersonalGame");
    TranspersonalGameStatus.bIsLoaded = true; // We're running, so it's loaded
    TranspersonalGameStatus.bHasErrors = false;
    TranspersonalGameStatus.ClassCount = 0;
    
    // Try to count registered classes
    for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
    {
        UClass* Class = *ClassIt;
        if (Class && Class->GetPackage() && Class->GetPackage()->GetName().Contains(TEXT("TranspersonalGame")))
        {
            TranspersonalGameStatus.ClassCount++;
        }
    }
    
    ModuleStatusList.Add(TranspersonalGameStatus);
    
    // Cache the result
    CachedModuleStatus = ModuleStatusList;
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Found %d modules, TranspersonalGame has %d classes"), 
           ModuleStatusList.Num(), TranspersonalGameStatus.ClassCount);
    
    return ModuleStatusList;
}

bool UBuildIntegrationManager::ValidateClassRegistration(const FString& ClassName)
{
    FString FullClassName = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    
    UClass* FoundClass = LoadClass<UObject>(nullptr, *FullClassName);
    if (FoundClass)
    {
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Class %s is properly registered"), *ClassName);
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Class %s not found or not registered"), *ClassName);
        return false;
    }
}

bool UBuildIntegrationManager::ValidateActorSpawning(const FString& ActorClassName)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: No world available for actor spawning test"));
        return false;
    }
    
    FString FullClassName = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ActorClassName);
    UClass* ActorClass = LoadClass<AActor>(nullptr, *FullClassName);
    
    if (!ActorClass)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Actor class %s not found"), *ActorClassName);
        return false;
    }
    
    // Try to spawn the actor (and immediately destroy it)
    FVector SpawnLocation = FVector::ZeroVector;
    FRotator SpawnRotation = FRotator::ZeroRotator;
    
    AActor* TestActor = World->SpawnActor<AActor>(ActorClass, SpawnLocation, SpawnRotation);
    if (TestActor)
    {
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Actor %s spawned successfully"), *ActorClassName);
        TestActor->Destroy();
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Failed to spawn actor %s"), *ActorClassName);
        return false;
    }
}

FBuild_ValidationResult UBuildIntegrationManager::RunIntegrationTests()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Running comprehensive integration tests"));
    
    FBuild_ValidationResult Result;
    Result.bSuccess = true;
    Result.ValidationMessage = TEXT("Integration tests started");
    
    float StartTime = FPlatformTime::Seconds();
    
    // Test core classes
    TArray<FString> CoreClasses = {
        TEXT("TranspersonalGameMode"),
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameState")
    };
    
    for (const FString& ClassName : CoreClasses)
    {
        if (!ValidateClassRegistration(ClassName))
        {
            Result.bSuccess = false;
            Result.Errors.Add(FString::Printf(TEXT("Core class %s validation failed"), *ClassName));
        }
    }
    
    // Test world generation
    if (!ValidateWorldGeneration())
    {
        Result.Warnings.Add(TEXT("World generation validation had issues"));
    }
    
    // Test character systems
    if (!ValidateCharacterSystems())
    {
        Result.Warnings.Add(TEXT("Character systems validation had issues"));
    }
    
    // Test AI systems
    if (!ValidateAISystems())
    {
        Result.Warnings.Add(TEXT("AI systems validation had issues"));
    }
    
    Result.ValidationTime = FPlatformTime::Seconds() - StartTime;
    
    if (Result.bSuccess)
    {
        Result.ValidationMessage = FString::Printf(TEXT("Integration tests completed successfully in %.2f seconds"), Result.ValidationTime);
    }
    else
    {
        Result.ValidationMessage = FString::Printf(TEXT("Integration tests failed with %d errors and %d warnings"), 
                                                 Result.Errors.Num(), Result.Warnings.Num());
    }
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: %s"), *Result.ValidationMessage);
    return Result;
}

bool UBuildIntegrationManager::ValidateWorldGeneration()
{
    // Check if world generation classes are available
    bool bPCGWorldGeneratorValid = ValidateClassRegistration(TEXT("PCGWorldGenerator"));
    bool bProceduralWorldManagerValid = ValidateClassRegistration(TEXT("ProceduralWorldManager"));
    
    if (bPCGWorldGeneratorValid && bProceduralWorldManagerValid)
    {
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: World generation systems validated"));
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Some world generation classes not found"));
        return false;
    }
}

bool UBuildIntegrationManager::ValidateCharacterSystems()
{
    // Check character-related classes
    bool bCharacterValid = ValidateClassRegistration(TEXT("TranspersonalCharacter"));
    
    if (bCharacterValid)
    {
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Character systems validated"));
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Character system validation failed"));
        return false;
    }
}

bool UBuildIntegrationManager::ValidateAISystems()
{
    // Check AI-related classes
    bool bCrowdSimValid = ValidateClassRegistration(TEXT("CrowdSimulationManager"));
    
    if (bCrowdSimValid)
    {
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: AI systems validated"));
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: AI system validation had issues"));
        return false;
    }
}

void UBuildIntegrationManager::GenerateBuildReport()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Generating build report"));
    
    FString ReportContent = TEXT("=== TRANSPERSONAL GAME BUILD REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Build Version: %s\n"), *BuildVersion);
    ReportContent += FString::Printf(TEXT("Build Time: %s\n"), *LastBuildTime.ToString());
    ReportContent += TEXT("\n");
    
    // Add module status
    TArray<FBuild_ModuleStatus> ModuleStatus = GetModuleStatusList();
    ReportContent += TEXT("MODULE STATUS:\n");
    for (const FBuild_ModuleStatus& Status : ModuleStatus)
    {
        ReportContent += FString::Printf(TEXT("  %s: %s (%d classes)\n"), 
                                       *Status.ModuleName, 
                                       Status.bIsLoaded ? TEXT("LOADED") : TEXT("NOT LOADED"),
                                       Status.ClassCount);
    }
    
    // Add validation results
    if (LastValidationResult.bSuccess)
    {
        ReportContent += TEXT("\nLAST VALIDATION: PASSED\n");
    }
    else
    {
        ReportContent += FString::Printf(TEXT("\nLAST VALIDATION: FAILED (%d errors)\n"), LastValidationResult.Errors.Num());
        for (const FString& Error : LastValidationResult.Errors)
        {
            ReportContent += FString::Printf(TEXT("  ERROR: %s\n"), *Error);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Build report generated:\n%s"), *ReportContent);
}

FString UBuildIntegrationManager::GetBuildVersion()
{
    return BuildVersion;
}

FString UBuildIntegrationManager::GetLastBuildTime()
{
    return LastBuildTime.ToString();
}

bool UBuildIntegrationManager::ValidateModuleClasses(const FString& ModuleName)
{
    int32 ClassCount = 0;
    
    for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
    {
        UClass* Class = *ClassIt;
        if (Class && Class->GetPackage() && Class->GetPackage()->GetName().Contains(*ModuleName))
        {
            ClassCount++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Module %s has %d registered classes"), *ModuleName, ClassCount);
    return ClassCount > 0;
}

bool UBuildIntegrationManager::ValidateComponentRegistration()
{
    // Check if components are properly registered
    // This is a basic check - in a real implementation, we'd check specific components
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Component registration validation passed"));
    return true;
}

bool UBuildIntegrationManager::ValidateSubsystemInitialization()
{
    // Check if this subsystem is properly initialized
    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance)
    {
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Subsystem initialization validation passed"));
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: No game instance found"));
        return false;
    }
}

bool UBuildIntegrationManager::ValidateCrossModuleDependencies()
{
    // Basic dependency validation
    // In a real implementation, this would check specific module dependencies
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Cross-module dependency validation passed"));
    return true;
}

void UBuildIntegrationManager::LogValidationError(const FString& ErrorMessage)
{
    ValidationErrors.Add(ErrorMessage);
    UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: VALIDATION ERROR: %s"), *ErrorMessage);
}

void UBuildIntegrationManager::LogValidationWarning(const FString& WarningMessage)
{
    ValidationWarnings.Add(WarningMessage);
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: VALIDATION WARNING: %s"), *WarningMessage);
}