#include "IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "Modules/ModuleManager.h"
#include "UObject/UObjectGlobals.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

void UIntegrationValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Initialize known modules
    KnownModules = {
        TEXT("TranspersonalGame"),
        TEXT("Core"),
        TEXT("Engine"),
        TEXT("UnrealEd")
    };

    // Initialize critical classes to test
    CriticalClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager"),
        TEXT("/Script/TranspersonalGame.CrowdSimulationManager"),
        TEXT("/Script/TranspersonalGame.ProceduralWorldManager"),
        TEXT("/Script/TranspersonalGame.BuildIntegrationManager")
    };

    UE_LOG(LogTemp, Log, TEXT("IntegrationValidator initialized with %d critical classes"), CriticalClasses.Num());
}

void UIntegrationValidator::Deinitialize()
{
    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("IntegrationValidator deinitialized"));
}

FInteg_ValidationResult UIntegrationValidator::ValidateFullSystem()
{
    FInteg_ValidationResult Result;
    Result.bIsValid = true;
    Result.ErrorCount = 0;
    Result.WarningCount = 0;

    if (bValidationInProgress)
    {
        Result.bIsValid = false;
        Result.ErrorMessage = TEXT("Validation already in progress");
        return Result;
    }

    bValidationInProgress = true;
    LastValidationTime = FPlatformTime::Seconds();

    UE_LOG(LogTemp, Log, TEXT("Starting full system validation..."));

    // Validate modules
    for (const FString& ModuleName : KnownModules)
    {
        FInteg_ModuleStatus ModuleStatus = ValidateModule(ModuleName);
        if (ModuleStatus.bHasErrors)
        {
            Result.ErrorCount++;
            Result.Details.Add(FString::Printf(TEXT("Module %s has errors"), *ModuleName));
        }
    }

    // Validate critical classes
    for (const FString& ClassName : CriticalClasses)
    {
        if (!TestClassLoading(ClassName))
        {
            Result.ErrorCount++;
            Result.Details.Add(FString::Printf(TEXT("Failed to load critical class: %s"), *ClassName));
        }
    }

    // Validate MinPlayableMap
    FInteg_LevelStatus LevelStatus = ValidateLevel(TEXT("/Game/Maps/MinPlayableMap"));
    if (LevelStatus.DuplicateActors > 0)
    {
        Result.WarningCount++;
        Result.Details.Add(FString::Printf(TEXT("Level has %d duplicate actors"), LevelStatus.DuplicateActors));
    }

    if (!LevelStatus.bHasPlayerStart)
    {
        Result.ErrorCount++;
        Result.Details.Add(TEXT("Level missing PlayerStart"));
    }

    if (!LevelStatus.bHasCharacter)
    {
        Result.WarningCount++;
        Result.Details.Add(TEXT("Level missing player character"));
    }

    // Check performance
    if (!IsPerformanceAcceptable())
    {
        Result.WarningCount++;
        Result.Details.Add(TEXT("Performance below acceptable threshold"));
    }

    // Final validation result
    Result.bIsValid = (Result.ErrorCount == 0);
    if (!Result.bIsValid)
    {
        Result.ErrorMessage = FString::Printf(TEXT("Validation failed with %d errors and %d warnings"), 
                                            Result.ErrorCount, Result.WarningCount);
    }

    LogValidationResults(Result);
    bValidationInProgress = false;

    return Result;
}

FInteg_ModuleStatus UIntegrationValidator::ValidateModule(const FString& ModuleName)
{
    FInteg_ModuleStatus Status;
    Status.ModuleName = ModuleName;
    Status.bIsLoaded = FModuleManager::Get().IsModuleLoaded(*ModuleName);
    Status.ClassCount = 0;
    Status.bHasErrors = false;

    if (!Status.bIsLoaded)
    {
        Status.bHasErrors = true;
        UE_LOG(LogTemp, Warning, TEXT("Module %s is not loaded"), *ModuleName);
        return Status;
    }

    // Test class loading for this module
    ValidateModuleClasses(ModuleName, Status);

    UE_LOG(LogTemp, Log, TEXT("Module %s validation: %d classes, %d loaded, %d failed"), 
           *ModuleName, Status.ClassCount, Status.LoadedClasses.Num(), Status.FailedClasses.Num());

    return Status;
}

FInteg_LevelStatus UIntegrationValidator::ValidateLevel(const FString& LevelPath)
{
    FInteg_LevelStatus Status;
    Status.LevelName = LevelPath;
    Status.TotalActors = 0;
    Status.DuplicateActors = 0;
    Status.bHasPlayerStart = false;
    Status.bHasCharacter = false;
    Status.bHasLighting = false;

    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot validate level - no world context"));
        return Status;
    }

    ValidateLevelActors(World, Status);

    UE_LOG(LogTemp, Log, TEXT("Level validation: %d actors, %d duplicates"), 
           Status.TotalActors, Status.DuplicateActors);

    return Status;
}

bool UIntegrationValidator::CleanupDuplicateActors(const FString& LevelPath)
{
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return false;
    }

    CleanupLightingDuplicates(World);
    return true;
}

TArray<FString> UIntegrationValidator::GetLoadedModules()
{
    TArray<FString> LoadedModules;
    
    for (const FString& ModuleName : KnownModules)
    {
        if (FModuleManager::Get().IsModuleLoaded(*ModuleName))
        {
            LoadedModules.Add(ModuleName);
        }
    }

    return LoadedModules;
}

bool UIntegrationValidator::TestClassLoading(const FString& ClassName)
{
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
    bool bSuccess = (LoadedClass != nullptr);
    
    if (bSuccess)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Successfully loaded class: %s"), *ClassName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load class: %s"), *ClassName);
    }

    return bSuccess;
}

void UIntegrationValidator::GenerateIntegrationReport()
{
    FInteg_ValidationResult FullResult = ValidateFullSystem();
    
    UE_LOG(LogTemp, Log, TEXT("=== INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Validation Status: %s"), FullResult.bIsValid ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Log, TEXT("Errors: %d, Warnings: %d"), FullResult.ErrorCount, FullResult.WarningCount);
    
    for (const FString& Detail : FullResult.Details)
    {
        UE_LOG(LogTemp, Log, TEXT("  - %s"), *Detail);
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== END REPORT ==="));
}

float UIntegrationValidator::GetCurrentFramerate()
{
    if (GEngine && GEngine->GetGameViewport())
    {
        return 1.0f / GEngine->GetGameViewport()->GetWorld()->GetDeltaSeconds();
    }
    return 0.0f;
}

bool UIntegrationValidator::IsPerformanceAcceptable()
{
    float CurrentFPS = GetCurrentFramerate();
    return CurrentFPS >= 30.0f; // Minimum acceptable framerate
}

bool UIntegrationValidator::ValidateModuleClasses(const FString& ModuleName, FInteg_ModuleStatus& OutStatus)
{
    // Test critical classes for TranspersonalGame module
    if (ModuleName == TEXT("TranspersonalGame"))
    {
        for (const FString& ClassName : CriticalClasses)
        {
            OutStatus.ClassCount++;
            if (TestClassLoading(ClassName))
            {
                OutStatus.LoadedClasses.Add(ClassName);
            }
            else
            {
                OutStatus.FailedClasses.Add(ClassName);
                OutStatus.bHasErrors = true;
            }
        }
    }

    return !OutStatus.bHasErrors;
}

bool UIntegrationValidator::ValidateLevelActors(UWorld* World, FInteg_LevelStatus& OutStatus)
{
    if (!World || !World->GetCurrentLevel())
    {
        return false;
    }

    TMap<FString, int32> ActorTypeCounts;
    
    for (AActor* Actor : World->GetCurrentLevel()->Actors)
    {
        if (!Actor)
        {
            continue;
        }

        OutStatus.TotalActors++;
        
        FString ActorClassName = Actor->GetClass()->GetName();
        int32& Count = ActorTypeCounts.FindOrAdd(ActorClassName);
        Count++;

        // Check for specific actor types
        if (Actor->IsA<APlayerStart>())
        {
            OutStatus.bHasPlayerStart = true;
        }
        else if (Actor->IsA<ACharacter>())
        {
            OutStatus.bHasCharacter = true;
        }
        else if (Actor->IsA<ADirectionalLight>() || Actor->IsA<ASkyLight>())
        {
            OutStatus.bHasLighting = true;
        }
    }

    // Count duplicates for lighting actors
    TArray<FString> LightingTypes = {
        TEXT("DirectionalLight"),
        TEXT("SkyLight"),
        TEXT("SkyAtmosphere"),
        TEXT("ExponentialHeightFog")
    };

    for (const FString& LightingType : LightingTypes)
    {
        if (int32* Count = ActorTypeCounts.Find(LightingType))
        {
            if (*Count > 1)
            {
                OutStatus.DuplicateActors += (*Count - 1);
            }
        }
    }

    OutStatus.ActorCounts = ActorTypeCounts;
    return true;
}

void UIntegrationValidator::CleanupLightingDuplicates(UWorld* World)
{
    if (!World || !World->GetCurrentLevel())
    {
        return;
    }

    TMap<FString, AActor*> FirstOfType;
    TArray<AActor*> ActorsToDestroy;

    for (AActor* Actor : World->GetCurrentLevel()->Actors)
    {
        if (!Actor)
        {
            continue;
        }

        FString ActorClassName = Actor->GetClass()->GetName();
        
        // Only clean up lighting actors
        if (ActorClassName == TEXT("DirectionalLight") || 
            ActorClassName == TEXT("SkyLight") ||
            ActorClassName == TEXT("SkyAtmosphere") ||
            ActorClassName == TEXT("ExponentialHeightFog"))
        {
            if (FirstOfType.Contains(ActorClassName))
            {
                // This is a duplicate, mark for destruction
                ActorsToDestroy.Add(Actor);
                UE_LOG(LogTemp, Log, TEXT("Marking duplicate %s for destruction: %s"), 
                       *ActorClassName, *Actor->GetName());
            }
            else
            {
                // This is the first of this type, keep it
                FirstOfType.Add(ActorClassName, Actor);
                UE_LOG(LogTemp, Log, TEXT("Keeping first %s: %s"), 
                       *ActorClassName, *Actor->GetName());
            }
        }
    }

    // Destroy duplicates
    for (AActor* Actor : ActorsToDestroy)
    {
        if (Actor && IsValid(Actor))
        {
            Actor->Destroy();
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Cleaned up %d duplicate lighting actors"), ActorsToDestroy.Num());
}

void UIntegrationValidator::LogValidationResults(const FInteg_ValidationResult& Results)
{
    UE_LOG(LogTemp, Log, TEXT("=== VALIDATION RESULTS ==="));
    UE_LOG(LogTemp, Log, TEXT("Status: %s"), Results.bIsValid ? TEXT("VALID") : TEXT("INVALID"));
    UE_LOG(LogTemp, Log, TEXT("Errors: %d"), Results.ErrorCount);
    UE_LOG(LogTemp, Log, TEXT("Warnings: %d"), Results.WarningCount);
    
    if (!Results.ErrorMessage.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Error: %s"), *Results.ErrorMessage);
    }

    for (const FString& Detail : Results.Details)
    {
        UE_LOG(LogTemp, Log, TEXT("  %s"), *Detail);
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== END VALIDATION ==="));
}