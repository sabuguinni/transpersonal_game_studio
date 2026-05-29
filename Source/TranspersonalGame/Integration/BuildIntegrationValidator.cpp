#include "BuildIntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"

UBuildIntegrationValidator::UBuildIntegrationValidator()
{
    // Initialize core class paths for validation
    CoreClassPaths = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager"),
        TEXT("/Script/TranspersonalGame.CrowdSimulationManager"),
        TEXT("/Script/TranspersonalGame.ProceduralWorldManager"),
        TEXT("/Script/TranspersonalGame.BuildIntegrationManager")
    };
}

void UBuildIntegrationValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationValidator initialized"));
    
    // Run initial validation
    FBuild_ValidationResult InitialResult = ValidateAllModules();
    if (InitialResult.bIsValid)
    {
        UE_LOG(LogTemp, Log, TEXT("Initial build validation PASSED - %d/%d classes loaded"), 
               InitialResult.LoadedClasses, InitialResult.TotalClasses);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Initial build validation FAILED - %d/%d classes loaded"), 
               InitialResult.LoadedClasses, InitialResult.TotalClasses);
    }
}

void UBuildIntegrationValidator::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationValidator deinitialized"));
    Super::Deinitialize();
}

FBuild_ValidationResult UBuildIntegrationValidator::ValidateAllModules()
{
    ValidationStartTime = FPlatformTime::Seconds();
    
    FBuild_ValidationResult Result;
    Result.TotalClasses = CoreClassPaths.Num();
    Result.LoadedClasses = 0;
    Result.FailedClasses.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("=== BUILD VALIDATION STARTED ==="));
    
    // Validate each core class
    for (const FString& ClassPath : CoreClassPaths)
    {
        if (ValidateClass(ClassPath))
        {
            Result.LoadedClasses++;
            UE_LOG(LogTemp, Log, TEXT("✓ %s loaded successfully"), *ClassPath);
        }
        else
        {
            Result.FailedClasses.Add(ClassPath);
            UE_LOG(LogTemp, Error, TEXT("✗ %s failed to load"), *ClassPath);
        }
    }
    
    // Calculate validation result
    Result.bIsValid = (Result.LoadedClasses == Result.TotalClasses);
    Result.ValidationTime = FPlatformTime::Seconds() - ValidationStartTime;
    
    if (Result.bIsValid)
    {
        Result.ValidationMessage = FString::Printf(TEXT("BUILD VALIDATION PASSED - All %d classes loaded successfully in %.2fs"), 
                                                  Result.LoadedClasses, Result.ValidationTime);
    }
    else
    {
        Result.ValidationMessage = FString::Printf(TEXT("BUILD VALIDATION FAILED - %d/%d classes loaded in %.2fs"), 
                                                  Result.LoadedClasses, Result.TotalClasses, Result.ValidationTime);
    }
    
    // Cache result
    LastValidationResult = Result;
    
    // Log final result
    LogValidationResult(Result);
    
    UE_LOG(LogTemp, Log, TEXT("=== BUILD VALIDATION COMPLETED ==="));
    
    return Result;
}

FBuild_ModuleStatus UBuildIntegrationValidator::ValidateModule(const FString& ModuleName)
{
    FBuild_ModuleStatus Status;
    Status.ModuleName = ModuleName;
    Status.bIsLoaded = false;
    Status.ClassCount = 0;
    Status.LoadedClasses.Empty();
    
    // Check if module is loaded
    if (FModuleManager::Get().IsModuleLoaded(*ModuleName))
    {
        Status.bIsLoaded = true;
        UE_LOG(LogTemp, Log, TEXT("Module %s is loaded"), *ModuleName);
        
        // Count classes in this module
        for (const FString& ClassPath : CoreClassPaths)
        {
            if (ClassPath.Contains(ModuleName))
            {
                if (ValidateClass(ClassPath))
                {
                    Status.ClassCount++;
                    Status.LoadedClasses.Add(ClassPath);
                }
            }
        }
    }
    else
    {
        Status.LastError = FString::Printf(TEXT("Module %s is not loaded"), *ModuleName);
        UE_LOG(LogTemp, Warning, TEXT("%s"), *Status.LastError);
    }
    
    return Status;
}

bool UBuildIntegrationValidator::ValidateCoreClasses()
{
    int32 LoadedCount = 0;
    
    for (const FString& ClassPath : CoreClassPaths)
    {
        if (ValidateClass(ClassPath))
        {
            LoadedCount++;
        }
    }
    
    bool bAllLoaded = (LoadedCount == CoreClassPaths.Num());
    UE_LOG(LogTemp, Log, TEXT("Core class validation: %d/%d classes loaded"), LoadedCount, CoreClassPaths.Num());
    
    return bAllLoaded;
}

bool UBuildIntegrationValidator::ValidateWorldState(UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("World validation failed - World is null"));
        return false;
    }
    
    // Check world state
    if (!World->IsGameWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("World validation - Not a game world"));
        return false;
    }
    
    // Check game mode
    AGameModeBase* GameMode = World->GetAuthGameMode();
    if (!GameMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("World validation - No game mode found"));
        return false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("World validation PASSED - GameMode: %s"), *GameMode->GetClass()->GetName());
    return true;
}

FBuild_ValidationResult UBuildIntegrationValidator::RunIntegrationTests()
{
    UE_LOG(LogTemp, Log, TEXT("=== INTEGRATION TESTS STARTED ==="));
    
    FBuild_ValidationResult Result = ValidateAllModules();
    
    // Additional integration tests
    if (Result.bIsValid)
    {
        // Test world state
        UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
        if (!World && GEngine)
        {
            World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
        }
        
        if (World)
        {
            bool bWorldValid = ValidateWorldState(World);
            if (!bWorldValid)
            {
                Result.ValidationMessage += TEXT(" | World state validation failed");
            }
        }
        
        // Test module dependencies
        FBuild_ModuleStatus TranspersonalGameStatus = ValidateModule(TEXT("TranspersonalGame"));
        if (!TranspersonalGameStatus.bIsLoaded)
        {
            Result.bIsValid = false;
            Result.ValidationMessage += TEXT(" | TranspersonalGame module not loaded");
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== INTEGRATION TESTS COMPLETED ==="));
    
    return Result;
}

bool UBuildIntegrationValidator::ValidateClass(const FString& ClassPath)
{
    try
    {
        // Attempt to load the class
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        
        if (LoadedClass)
        {
            // Verify class is valid and properly constructed
            if (LoadedClass->IsValidLowLevel() && !LoadedClass->HasAnyFlags(RF_BeginDestroyed))
            {
                return true;
            }
        }
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("Exception while loading class: %s"), *ClassPath);
    }
    
    return false;
}

TArray<FString> UBuildIntegrationValidator::GetCoreClassPaths() const
{
    return CoreClassPaths;
}

void UBuildIntegrationValidator::LogValidationResult(const FBuild_ValidationResult& Result)
{
    UE_LOG(LogTemp, Log, TEXT("=== VALIDATION RESULT ==="));
    UE_LOG(LogTemp, Log, TEXT("Valid: %s"), Result.bIsValid ? TEXT("TRUE") : TEXT("FALSE"));
    UE_LOG(LogTemp, Log, TEXT("Classes: %d/%d loaded"), Result.LoadedClasses, Result.TotalClasses);
    UE_LOG(LogTemp, Log, TEXT("Time: %.2f seconds"), Result.ValidationTime);
    UE_LOG(LogTemp, Log, TEXT("Message: %s"), *Result.ValidationMessage);
    
    if (Result.FailedClasses.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed classes:"));
        for (const FString& FailedClass : Result.FailedClasses)
        {
            UE_LOG(LogTemp, Warning, TEXT("  - %s"), *FailedClass);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("========================"));
}