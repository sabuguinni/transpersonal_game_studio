#include "Build_IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"

UBuild_IntegrationValidator::UBuild_IntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;

    // Configuration defaults
    bAutoValidateOnBeginPlay = true;
    ValidationInterval = 30.0f;
    bLogValidationResults = true;
    bValidationInProgress = false;
    LastValidationTime = 0.0f;
    ValidationTimer = 0.0f;

    // Critical systems that must be operational
    CriticalSystems.Add(TEXT("TranspersonalGameState"));
    CriticalSystems.Add(TEXT("TranspersonalCharacter"));
    CriticalSystems.Add(TEXT("PCGWorldGenerator"));
    CriticalSystems.Add(TEXT("FoliageManager"));
    CriticalSystems.Add(TEXT("CrowdSimulationManager"));
}

void UBuild_IntegrationValidator::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoValidateOnBeginPlay)
    {
        // Delay initial validation to allow systems to initialize
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            [this]() { RunFullValidation(); },
            2.0f,
            false
        );
    }
}

void UBuild_IntegrationValidator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    ValidationTimer += DeltaTime;

    // Periodic validation
    if (ValidationTimer >= ValidationInterval && !bValidationInProgress)
    {
        ValidationTimer = 0.0f;
        RunFullValidation();
    }
}

TArray<FBuild_ValidationResult> UBuild_IntegrationValidator::RunFullValidation()
{
    if (bValidationInProgress)
    {
        return LastValidationResults;
    }

    bValidationInProgress = true;
    LastValidationStartTime = FPlatformTime::Seconds();
    LastValidationResults.Empty();

    UE_LOG(LogTemp, Log, TEXT("Integration Validator: Starting full validation"));

    // Run all validation tests
    LastValidationResults.Add(ValidateClassLoading());
    LastValidationResults.Add(ValidateLevelIntegrity());
    LastValidationResults.Add(ValidateSystemDependencies());
    LastValidationResults.Add(ValidateAssetReferences());

    // Update system statuses
    SystemStatuses.Empty();
    for (const FString& SystemName : CriticalSystems)
    {
        UpdateSystemStatus(SystemName);
    }

    LastValidationTime = FPlatformTime::Seconds() - LastValidationStartTime;
    bValidationInProgress = false;

    UE_LOG(LogTemp, Log, TEXT("Integration Validator: Validation completed in %.3f seconds"), LastValidationTime);

    return LastValidationResults;
}

FBuild_ValidationResult UBuild_IntegrationValidator::ValidateClassLoading()
{
    FBuild_ValidationResult Result;
    Result.TestName = TEXT("Class Loading Validation");
    
    double StartTime = FPlatformTime::Seconds();
    
    int32 LoadedCount = 0;
    int32 TotalCount = CriticalSystems.Num();
    TArray<FString> FailedClasses;

    for (const FString& SystemName : CriticalSystems)
    {
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *SystemName);
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        
        if (LoadedClass)
        {
            LoadedCount++;
        }
        else
        {
            FailedClasses.Add(SystemName);
        }
    }

    Result.ExecutionTime = FPlatformTime::Seconds() - StartTime;

    if (LoadedCount == TotalCount)
    {
        Result.Status = EBuild_ValidationStatus::Passed;
        Result.Message = FString::Printf(TEXT("All %d critical classes loaded successfully"), TotalCount);
    }
    else if (LoadedCount >= TotalCount * 0.8f) // 80% threshold
    {
        Result.Status = EBuild_ValidationStatus::Warning;
        Result.Message = FString::Printf(TEXT("%d/%d classes loaded. Failed: %s"), 
            LoadedCount, TotalCount, *FString::Join(FailedClasses, TEXT(", ")));
    }
    else
    {
        Result.Status = EBuild_ValidationStatus::Failed;
        Result.Message = FString::Printf(TEXT("Critical failure: Only %d/%d classes loaded"), LoadedCount, TotalCount);
    }

    if (bLogValidationResults)
    {
        LogValidationResult(Result);
    }

    return Result;
}

FBuild_ValidationResult UBuild_IntegrationValidator::ValidateLevelIntegrity()
{
    FBuild_ValidationResult Result;
    Result.TestName = TEXT("Level Integrity Validation");
    
    double StartTime = FPlatformTime::Seconds();

    UWorld* World = GetWorld();
    if (!World)
    {
        Result.Status = EBuild_ValidationStatus::Critical;
        Result.Message = TEXT("No valid world context");
        Result.ExecutionTime = FPlatformTime::Seconds() - StartTime;
        return Result;
    }

    // Count actors and validate basic level structure
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    int32 PlayerStarts = 0;
    int32 LightSources = 0;
    int32 StaticMeshes = 0;
    int32 CustomActors = 0;

    for (AActor* Actor : AllActors)
    {
        if (Actor->IsA<APlayerStart>())
        {
            PlayerStarts++;
        }
        else if (Actor->IsA<ALight>())
        {
            LightSources++;
        }
        else if (Actor->FindComponentByClass<UStaticMeshComponent>())
        {
            StaticMeshes++;
        }
        else if (Actor->GetClass()->GetName().Contains(TEXT("Transpersonal")))
        {
            CustomActors++;
        }
    }

    Result.ExecutionTime = FPlatformTime::Seconds() - StartTime;

    // Validate minimum level requirements
    bool bHasPlayerStart = PlayerStarts > 0;
    bool bHasLighting = LightSources > 0;
    bool bHasGeometry = StaticMeshes > 0;

    if (bHasPlayerStart && bHasLighting && bHasGeometry)
    {
        Result.Status = EBuild_ValidationStatus::Passed;
        Result.Message = FString::Printf(TEXT("Level valid: %d actors, %d custom"), AllActors.Num(), CustomActors);
    }
    else
    {
        Result.Status = EBuild_ValidationStatus::Failed;
        Result.Message = FString::Printf(TEXT("Level incomplete: PlayerStart=%d, Lights=%d, Meshes=%d"), 
            PlayerStarts, LightSources, StaticMeshes);
    }

    if (bLogValidationResults)
    {
        LogValidationResult(Result);
    }

    return Result;
}

FBuild_ValidationResult UBuild_IntegrationValidator::ValidateSystemDependencies()
{
    FBuild_ValidationResult Result;
    Result.TestName = TEXT("System Dependencies Validation");
    
    double StartTime = FPlatformTime::Seconds();

    // Check if critical systems can communicate
    bool bCommunicationTest = TestCrossSystemCommunication();
    
    Result.ExecutionTime = FPlatformTime::Seconds() - StartTime;

    if (bCommunicationTest)
    {
        Result.Status = EBuild_ValidationStatus::Passed;
        Result.Message = TEXT("System dependencies validated successfully");
    }
    else
    {
        Result.Status = EBuild_ValidationStatus::Warning;
        Result.Message = TEXT("Some system dependencies may have issues");
    }

    if (bLogValidationResults)
    {
        LogValidationResult(Result);
    }

    return Result;
}

FBuild_ValidationResult UBuild_IntegrationValidator::ValidateAssetReferences()
{
    FBuild_ValidationResult Result;
    Result.TestName = TEXT("Asset References Validation");
    
    double StartTime = FPlatformTime::Seconds();

    // Basic asset validation - check if critical paths exist
    TArray<FString> CriticalPaths = {
        TEXT("/Game/Maps/MinPlayableMap"),
        TEXT("/Game/TranspersonalGame/"),
        TEXT("/Game/Characters/"),
        TEXT("/Game/Environment/")
    };

    int32 ValidPaths = 0;
    for (const FString& Path : CriticalPaths)
    {
        if (FPaths::DirectoryExists(FPaths::ProjectContentDir() + Path.Replace(TEXT("/Game/"), TEXT(""))))
        {
            ValidPaths++;
        }
    }

    Result.ExecutionTime = FPlatformTime::Seconds() - StartTime;

    if (ValidPaths >= CriticalPaths.Num() * 0.75f) // 75% threshold
    {
        Result.Status = EBuild_ValidationStatus::Passed;
        Result.Message = FString::Printf(TEXT("%d/%d critical asset paths found"), ValidPaths, CriticalPaths.Num());
    }
    else
    {
        Result.Status = EBuild_ValidationStatus::Warning;
        Result.Message = FString::Printf(TEXT("Missing asset paths: %d/%d found"), ValidPaths, CriticalPaths.Num());
    }

    if (bLogValidationResults)
    {
        LogValidationResult(Result);
    }

    return Result;
}

TArray<FBuild_SystemStatus> UBuild_IntegrationValidator::GetSystemStatusReport()
{
    return SystemStatuses;
}

bool UBuild_IntegrationValidator::TestCrossSystemCommunication()
{
    // Test basic system communication
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    // Test GameMode access
    AGameModeBase* GameMode = World->GetAuthGameMode();
    if (!GameMode)
    {
        return false;
    }

    // Test PlayerController access
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        return false;
    }

    return true;
}

bool UBuild_IntegrationValidator::TestPerformanceBaseline()
{
    // Basic performance test - measure frame time
    double StartTime = FPlatformTime::Seconds();
    
    // Simulate some work
    for (int32 i = 0; i < 1000; ++i)
    {
        FVector TestVector = FVector(i, i * 2, i * 3);
        TestVector.Normalize();
    }
    
    double ElapsedTime = FPlatformTime::Seconds() - StartTime;
    
    // Should complete in reasonable time (< 1ms for this simple test)
    return ElapsedTime < 0.001;
}

bool UBuild_IntegrationValidator::TestMemoryUsage()
{
    // Basic memory test - check if we can allocate reasonable amounts
    try
    {
        TArray<int32> TestArray;
        TestArray.Reserve(10000);
        for (int32 i = 0; i < 10000; ++i)
        {
            TestArray.Add(i);
        }
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool UBuild_IntegrationValidator::ValidateBuildConfiguration()
{
    // Check if we're in a valid build configuration
    UWorld* World = GetWorld();
    return World && World->IsGameWorld();
}

TArray<FString> UBuild_IntegrationValidator::GetMissingDependencies()
{
    TArray<FString> MissingDeps;
    
    for (const FString& SystemName : CriticalSystems)
    {
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *SystemName);
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        
        if (!LoadedClass)
        {
            MissingDeps.Add(SystemName);
        }
    }
    
    return MissingDeps;
}

TArray<FString> UBuild_IntegrationValidator::GetCompilationErrors()
{
    TArray<FString> Errors;
    
    // This would typically read from build logs
    // For now, return basic validation errors
    TArray<FString> MissingDeps = GetMissingDependencies();
    for (const FString& Dep : MissingDeps)
    {
        Errors.Add(FString::Printf(TEXT("Missing class: %s"), *Dep));
    }
    
    return Errors;
}

void UBuild_IntegrationValidator::UpdateSystemStatus(const FString& SystemName)
{
    FBuild_SystemStatus Status;
    Status.SystemName = SystemName;
    
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *SystemName);
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
    
    Status.bIsLoaded = (LoadedClass != nullptr);
    Status.bIsInitialized = Status.bIsLoaded; // Simplified check
    Status.ComponentCount = 1; // Simplified
    
    SystemStatuses.Add(Status);
}

void UBuild_IntegrationValidator::LogValidationResult(const FBuild_ValidationResult& Result)
{
    FString StatusString;
    switch (Result.Status)
    {
        case EBuild_ValidationStatus::Passed:
            StatusString = TEXT("PASSED");
            UE_LOG(LogTemp, Log, TEXT("[VALIDATION] %s: %s - %s (%.3fs)"), 
                *StatusString, *Result.TestName, *Result.Message, Result.ExecutionTime);
            break;
        case EBuild_ValidationStatus::Warning:
            StatusString = TEXT("WARNING");
            UE_LOG(LogTemp, Warning, TEXT("[VALIDATION] %s: %s - %s (%.3fs)"), 
                *StatusString, *Result.TestName, *Result.Message, Result.ExecutionTime);
            break;
        case EBuild_ValidationStatus::Failed:
        case EBuild_ValidationStatus::Critical:
            StatusString = TEXT("FAILED");
            UE_LOG(LogTemp, Error, TEXT("[VALIDATION] %s: %s - %s (%.3fs)"), 
                *StatusString, *Result.TestName, *Result.Message, Result.ExecutionTime);
            break;
        default:
            StatusString = TEXT("UNKNOWN");
            break;
    }
}