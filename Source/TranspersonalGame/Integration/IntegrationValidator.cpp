#include "IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "EditorLevelLibrary.h"
#include "EditorAssetLibrary.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

void UIntegrationValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("IntegrationValidator subsystem initialized"));
    
    // Initialize singleton lighting classes
    SingletonLightingClasses.Empty();
    SingletonLightingClasses.Add(ADirectionalLight::StaticClass());
    // Note: SkyAtmosphere and SkyLight are components, not actors
    
    LastValidationTime = 0.0f;
}

void UIntegrationValidator::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("IntegrationValidator subsystem deinitialized"));
    Super::Deinitialize();
}

FInteg_ValidationResult UIntegrationValidator::ValidateGameSystems()
{
    FInteg_ValidationResult Result;
    Result.bIsValid = true;
    Result.ErrorMessage = TEXT("System validation completed");
    
    // Validate critical classes
    for (const FString& ClassName : CriticalClasses)
    {
        FString Error;
        if (!ValidateClassLoading(ClassName, Error))
        {
            Result.bIsValid = false;
            Result.ErrorCount++;
            Result.DetailedErrors.Add(FString::Printf(TEXT("Class loading failed: %s - %s"), *ClassName, *Error));
        }
    }
    
    // Validate map integrity
    FInteg_ValidationResult MapResult = ValidateMapIntegrity();
    if (!MapResult.bIsValid)
    {
        Result.bIsValid = false;
        Result.ErrorCount += MapResult.ErrorCount;
        Result.WarningCount += MapResult.WarningCount;
        Result.DetailedErrors.Append(MapResult.DetailedErrors);
    }
    
    // Validate performance
    FInteg_ValidationResult PerfResult = ValidatePerformance();
    if (!PerfResult.bIsValid)
    {
        Result.WarningCount += PerfResult.ErrorCount; // Performance issues are warnings, not errors
        Result.DetailedErrors.Append(PerfResult.DetailedErrors);
    }
    
    if (Result.ErrorCount == 0)
    {
        Result.ErrorMessage = FString::Printf(TEXT("All systems valid. %d warnings."), Result.WarningCount);
    }
    else
    {
        Result.ErrorMessage = FString::Printf(TEXT("Validation failed: %d errors, %d warnings"), Result.ErrorCount, Result.WarningCount);
    }
    
    ValidationHistory.Add(Result);
    LastValidationTime = FPlatformTime::Seconds();
    
    return Result;
}

FInteg_ValidationResult UIntegrationValidator::ValidateMapIntegrity(const FString& MapPath)
{
    FInteg_ValidationResult Result;
    Result.bIsValid = true;
    Result.ErrorMessage = TEXT("Map validation completed");
    
#if WITH_EDITOR
    // Load the map
    if (!UEditorLevelLibrary::LoadLevel(MapPath))
    {
        Result.bIsValid = false;
        Result.ErrorCount++;
        Result.ErrorMessage = FString::Printf(TEXT("Failed to load map: %s"), *MapPath);
        Result.DetailedErrors.Add(Result.ErrorMessage);
        return Result;
    }
    
    // Get all actors
    TArray<AActor*> AllActors = UEditorLevelLibrary::GetAllLevelActors();
    
    if (AllActors.Num() == 0)
    {
        Result.bIsValid = false;
        Result.ErrorCount++;
        Result.ErrorMessage = TEXT("Map is empty - no actors found");
        Result.DetailedErrors.Add(Result.ErrorMessage);
        return Result;
    }
    
    // Count actors by type
    TMap<UClass*, int32> ActorCounts;
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            UClass* ActorClass = Actor->GetClass();
            if (!ActorCounts.Contains(ActorClass))
            {
                ActorCounts.Add(ActorClass, 0);
            }
            ActorCounts[ActorClass]++;
        }
    }
    
    // Check for duplicate lighting actors
    int32 DirectionalLightCount = CountActorsOfType(ADirectionalLight::StaticClass());
    if (DirectionalLightCount > 1)
    {
        Result.WarningCount++;
        Result.DetailedErrors.Add(FString::Printf(TEXT("Multiple DirectionalLights found: %d (should be 1)"), DirectionalLightCount));
    }
    
    // Check for essential actors
    bool bHasPlayerStart = false;
    bool bHasLighting = DirectionalLightCount > 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("PlayerStart")))
        {
            bHasPlayerStart = true;
            break;
        }
    }
    
    if (!bHasPlayerStart)
    {
        Result.ErrorCount++;
        Result.DetailedErrors.Add(TEXT("No PlayerStart found in map"));
        Result.bIsValid = false;
    }
    
    if (!bHasLighting)
    {
        Result.ErrorCount++;
        Result.DetailedErrors.Add(TEXT("No DirectionalLight found in map"));
        Result.bIsValid = false;
    }
    
    Result.ErrorMessage = FString::Printf(TEXT("Map validation: %d actors, %d errors, %d warnings"), 
                                        AllActors.Num(), Result.ErrorCount, Result.WarningCount);
#else
    Result.WarningCount++;
    Result.DetailedErrors.Add(TEXT("Map validation skipped - not in editor"));
#endif
    
    return Result;
}

TArray<FInteg_ModuleStatus> UIntegrationValidator::ValidateModuleStatus()
{
    TArray<FInteg_ModuleStatus> ModuleStatuses;
    
    // Test TranspersonalGame module
    FInteg_ModuleStatus MainModule;
    MainModule.ModuleName = TEXT("TranspersonalGame");
    MainModule.bIsLoaded = true; // If we're running, the module is loaded
    MainModule.bIsCompiled = true; // If we're running, it compiled
    
    // Test critical classes
    for (const FString& ClassName : CriticalClasses)
    {
        FString Error;
        if (ValidateClassLoading(ClassName, Error))
        {
            MainModule.LoadedClasses.Add(ClassName);
            MainModule.ClassCount++;
        }
        else
        {
            MainModule.FailedClasses.Add(ClassName);
        }
    }
    
    ModuleStatuses.Add(MainModule);
    
    return ModuleStatuses;
}

FInteg_ValidationResult UIntegrationValidator::ValidateActorDuplicates()
{
    FInteg_ValidationResult Result;
    Result.bIsValid = true;
    Result.ErrorMessage = TEXT("Duplicate validation completed");
    
#if WITH_EDITOR
    // Check DirectionalLight duplicates
    int32 DirectionalLightCount = CountActorsOfType(ADirectionalLight::StaticClass());
    if (DirectionalLightCount > 1)
    {
        Result.WarningCount++;
        Result.DetailedErrors.Add(FString::Printf(TEXT("Found %d DirectionalLights (should be 1)"), DirectionalLightCount));
    }
    
    // Add more duplicate checks as needed
    Result.ErrorMessage = FString::Printf(TEXT("Duplicate check: %d warnings"), Result.WarningCount);
#endif
    
    return Result;
}

bool UIntegrationValidator::CleanupDuplicateActors()
{
#if WITH_EDITOR
    bool bCleaned = false;
    
    // Clean up DirectionalLights - keep only 1
    CleanupActorsOfType(ADirectionalLight::StaticClass(), 1);
    bCleaned = true;
    
    // Save the level after cleanup
    UEditorLevelLibrary::SaveCurrentLevel();
    
    UE_LOG(LogTemp, Log, TEXT("Duplicate actor cleanup completed"));
    return bCleaned;
#else
    return false;
#endif
}

FInteg_ValidationResult UIntegrationValidator::ValidatePerformance()
{
    FInteg_ValidationResult Result;
    Result.bIsValid = true;
    Result.ErrorMessage = TEXT("Performance validation completed");
    
    // Basic performance checks
    UWorld* World = GetWorld();
    if (World)
    {
        // Check actor count
        int32 ActorCount = World->GetActorCount();
        if (ActorCount > 10000)
        {
            Result.ErrorCount++;
            Result.DetailedErrors.Add(FString::Printf(TEXT("High actor count: %d (may impact performance)"), ActorCount));
        }
        
        // Check for performance warnings
        if (ActorCount > 5000)
        {
            Result.WarningCount++;
            Result.DetailedErrors.Add(FString::Printf(TEXT("Actor count warning: %d actors in world"), ActorCount));
        }
    }
    
    return Result;
}

void UIntegrationValidator::LogValidationResults(const FInteg_ValidationResult& Results)
{
    if (Results.bIsValid)
    {
        UE_LOG(LogTemp, Log, TEXT("Validation PASSED: %s"), *Results.ErrorMessage);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Validation FAILED: %s"), *Results.ErrorMessage);
    }
    
    for (const FString& Error : Results.DetailedErrors)
    {
        UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Error);
    }
}

bool UIntegrationValidator::SaveValidationReport(const FString& FilePath)
{
    if (ValidationHistory.Num() == 0)
    {
        return false;
    }
    
    FString Report;
    Report += FString::Printf(TEXT("Integration Validation Report\n"));
    Report += FString::Printf(TEXT("Generated: %s\n\n"), *FDateTime::Now().ToString());
    
    const FInteg_ValidationResult& LatestResult = ValidationHistory.Last();
    Report += FString::Printf(TEXT("Latest Validation Result:\n"));
    Report += FString::Printf(TEXT("Status: %s\n"), LatestResult.bIsValid ? TEXT("PASS") : TEXT("FAIL"));
    Report += FString::Printf(TEXT("Errors: %d\n"), LatestResult.ErrorCount);
    Report += FString::Printf(TEXT("Warnings: %d\n"), LatestResult.WarningCount);
    Report += FString::Printf(TEXT("Message: %s\n\n"), *LatestResult.ErrorMessage);
    
    Report += TEXT("Detailed Issues:\n");
    for (const FString& Error : LatestResult.DetailedErrors)
    {
        Report += FString::Printf(TEXT("  - %s\n"), *Error);
    }
    
    return FFileHelper::SaveStringToFile(Report, *FilePath);
}

bool UIntegrationValidator::ValidateClassLoading(const FString& ClassName, FString& OutError)
{
    try
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
        if (LoadedClass)
        {
            return true;
        }
        else
        {
            OutError = TEXT("Class not found or failed to load");
            return false;
        }
    }
    catch (...)
    {
        OutError = TEXT("Exception during class loading");
        return false;
    }
}

bool UIntegrationValidator::ValidateActorSpawning(UClass* ActorClass, FString& OutError)
{
#if WITH_EDITOR
    if (!ActorClass || !ActorClass->IsChildOf(AActor::StaticClass()))
    {
        OutError = TEXT("Invalid actor class");
        return false;
    }
    
    try
    {
        AActor* TestActor = UEditorLevelLibrary::SpawnActorFromClass(
            ActorClass, 
            FVector(0, 0, 100), 
            FRotator::ZeroRotator
        );
        
        if (TestActor)
        {
            // Clean up test actor
            UEditorLevelLibrary::DestroyActor(TestActor);
            return true;
        }
        else
        {
            OutError = TEXT("Failed to spawn test actor");
            return false;
        }
    }
    catch (...)
    {
        OutError = TEXT("Exception during actor spawning");
        return false;
    }
#else
    OutError = TEXT("Actor spawning test skipped - not in editor");
    return true; // Don't fail in non-editor builds
#endif
}

int32 UIntegrationValidator::CountActorsOfType(UClass* ActorClass)
{
#if WITH_EDITOR
    if (!ActorClass)
    {
        return 0;
    }
    
    TArray<AActor*> AllActors = UEditorLevelLibrary::GetAllLevelActors();
    int32 Count = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->IsA(ActorClass))
        {
            Count++;
        }
    }
    
    return Count;
#else
    return 0;
#endif
}

void UIntegrationValidator::CleanupActorsOfType(UClass* ActorClass, int32 MaxCount)
{
#if WITH_EDITOR
    if (!ActorClass || MaxCount < 0)
    {
        return;
    }
    
    TArray<AActor*> AllActors = UEditorLevelLibrary::GetAllLevelActors();
    TArray<AActor*> ActorsOfType;
    
    // Find all actors of this type
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->IsA(ActorClass))
        {
            ActorsOfType.Add(Actor);
        }
    }
    
    // Remove excess actors
    int32 ActorsToRemove = ActorsOfType.Num() - MaxCount;
    if (ActorsToRemove > 0)
    {
        for (int32 i = MaxCount; i < ActorsOfType.Num(); i++)
        {
            if (ActorsOfType[i])
            {
                UEditorLevelLibrary::DestroyActor(ActorsOfType[i]);
                UE_LOG(LogTemp, Log, TEXT("Removed duplicate actor: %s"), *ActorsOfType[i]->GetName());
            }
        }
    }
#endif
}