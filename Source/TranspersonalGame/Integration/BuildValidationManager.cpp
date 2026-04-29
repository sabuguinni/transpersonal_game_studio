#include "BuildValidationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Engine/Level.h"
#include "Engine/LevelStreaming.h"
#include "Misc/PackageName.h"

UBuildValidationManager::UBuildValidationManager()
{
    // Initialize core class paths for validation
    CoreClassPaths = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameMode"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.TranspersonalPlayerController")
    };

    GameplayClassPaths = {
        TEXT("/Script/TranspersonalGame.QATestManager"),
        TEXT("/Script/TranspersonalGame.BuildValidationManager")
    };

    SubsystemClassPaths = {
        TEXT("/Script/TranspersonalGame.BuildValidationManager")
    };

    bContinuousValidationActive = false;
    ValidationInterval = 30.0f;
    bInitialized = false;
    ValidationRunCount = 0;
    LastValidationTime = 0.0;
}

void UBuildValidationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Initializing..."));
    
    bInitialized = true;
    
    // Perform initial validation
    LastValidationResult = ValidateFullBuild();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Initialized successfully"));
}

void UBuildValidationManager::Deinitialize()
{
    StopContinuousValidation();
    bInitialized = false;
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Deinitialized"));
    
    Super::Deinitialize();
}

FBuild_ValidationResult UBuildValidationManager::ValidateFullBuild()
{
    FBuild_ValidationResult Result;
    ValidationRunCount++;
    LastValidationTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Starting full build validation (Run #%d)"), ValidationRunCount);
    
    // Validate TranspersonalGame module
    bool bModuleValid = ValidateTranspersonalGameModule();
    if (!bModuleValid)
    {
        Result.ErrorMessages.Add(TEXT("TranspersonalGame module validation failed"));
        Result.ErrorCount++;
    }
    
    // Validate core classes
    bool bCoreValid = ValidateCoreClasses();
    if (!bCoreValid)
    {
        Result.ErrorMessages.Add(TEXT("Core classes validation failed"));
        Result.ErrorCount++;
    }
    
    // Validate gameplay classes
    bool bGameplayValid = ValidateGameplayClasses();
    if (!bGameplayValid)
    {
        Result.WarningMessages.Add(TEXT("Some gameplay classes failed validation"));
        Result.WarningCount++;
    }
    
    // Validate subsystems
    bool bSubsystemsValid = ValidateSubsystems();
    if (!bSubsystemsValid)
    {
        Result.WarningMessages.Add(TEXT("Some subsystems failed validation"));
        Result.WarningCount++;
    }
    
    // Determine overall result
    Result.bIsValid = (Result.ErrorCount == 0);
    
    if (Result.bIsValid)
    {
        Result.ValidationMessage = FString::Printf(TEXT("Build validation PASSED (Warnings: %d)"), Result.WarningCount);
    }
    else
    {
        Result.ValidationMessage = FString::Printf(TEXT("Build validation FAILED (Errors: %d, Warnings: %d)"), Result.ErrorCount, Result.WarningCount);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: %s"), *Result.ValidationMessage);
    
    return Result;
}

FBuild_ValidationResult UBuildValidationManager::ValidateModuleIntegrity()
{
    FBuild_ValidationResult Result;
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Validating module integrity..."));
    
    // Check if TranspersonalGame module is loaded
    FModuleManager& ModuleManager = FModuleManager::Get();
    bool bModuleLoaded = ModuleManager.IsModuleLoaded(TEXT("TranspersonalGame"));
    
    if (bModuleLoaded)
    {
        Result.ValidationMessage = TEXT("TranspersonalGame module is loaded and accessible");
        Result.bIsValid = true;
        UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Module integrity check PASSED"));
    }
    else
    {
        Result.ValidationMessage = TEXT("TranspersonalGame module is not loaded");
        Result.ErrorMessages.Add(TEXT("Module not loaded"));
        Result.ErrorCount++;
        Result.bIsValid = false;
        UE_LOG(LogTemp, Error, TEXT("BuildValidationManager: Module integrity check FAILED"));
    }
    
    return Result;
}

FBuild_ValidationResult UBuildValidationManager::ValidateClassRegistration()
{
    FBuild_ValidationResult Result;
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Validating class registration..."));
    
    TArray<FString> AllClassPaths = CoreClassPaths;
    AllClassPaths.Append(GameplayClassPaths);
    
    int32 LoadedCount = 0;
    int32 FailedCount = 0;
    
    for (const FString& ClassPath : AllClassPaths)
    {
        if (TestClassLoading(ClassPath))
        {
            LoadedCount++;
            UE_LOG(LogTemp, Log, TEXT("BuildValidationManager: Class loaded successfully: %s"), *ClassPath);
        }
        else
        {
            FailedCount++;
            Result.ErrorMessages.Add(FString::Printf(TEXT("Failed to load class: %s"), *ClassPath));
            UE_LOG(LogTemp, Error, TEXT("BuildValidationManager: Failed to load class: %s"), *ClassPath);
        }
    }
    
    Result.ErrorCount = FailedCount;
    Result.bIsValid = (FailedCount == 0);
    Result.ValidationMessage = FString::Printf(TEXT("Class registration: %d loaded, %d failed"), LoadedCount, FailedCount);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Class registration validation complete: %s"), *Result.ValidationMessage);
    
    return Result;
}

TArray<FBuild_ModuleStatus> UBuildValidationManager::GetModuleStatusReport()
{
    TArray<FBuild_ModuleStatus> StatusReport;
    
    // TranspersonalGame module status
    FBuild_ModuleStatus TranspersonalStatus;
    TranspersonalStatus.ModuleName = TEXT("TranspersonalGame");
    TranspersonalStatus.bIsLoaded = FModuleManager::Get().IsModuleLoaded(TEXT("TranspersonalGame"));
    
    // Test core classes
    for (const FString& ClassPath : CoreClassPaths)
    {
        if (TestClassLoading(ClassPath))
        {
            TranspersonalStatus.LoadedClasses.Add(ClassPath);
        }
        else
        {
            TranspersonalStatus.FailedClasses.Add(ClassPath);
            TranspersonalStatus.bHasErrors = true;
        }
    }
    
    // Test gameplay classes
    for (const FString& ClassPath : GameplayClassPaths)
    {
        if (TestClassLoading(ClassPath))
        {
            TranspersonalStatus.LoadedClasses.Add(ClassPath);
        }
        else
        {
            TranspersonalStatus.FailedClasses.Add(ClassPath);
        }
    }
    
    TranspersonalStatus.ClassCount = TranspersonalStatus.LoadedClasses.Num();
    StatusReport.Add(TranspersonalStatus);
    
    CachedModuleStatus = StatusReport;
    return StatusReport;
}

void UBuildValidationManager::StartContinuousValidation()
{
    if (bContinuousValidationActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Continuous validation already active"));
        return;
    }
    
    UGameInstance* GameInstance = GetGameInstance();
    if (!GameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildValidationManager: Cannot start continuous validation - no GameInstance"));
        return;
    }
    
    UWorld* World = GameInstance->GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildValidationManager: Cannot start continuous validation - no World"));
        return;
    }
    
    bContinuousValidationActive = true;
    
    World->GetTimerManager().SetTimer(
        ValidationTimerHandle,
        this,
        &UBuildValidationManager::PerformValidationTick,
        ValidationInterval,
        true // Loop
    );
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Continuous validation started (interval: %.1fs)"), ValidationInterval);
}

void UBuildValidationManager::StopContinuousValidation()
{
    if (!bContinuousValidationActive)
    {
        return;
    }
    
    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance)
    {
        UWorld* World = GameInstance->GetWorld();
        if (World)
        {
            World->GetTimerManager().ClearTimer(ValidationTimerHandle);
        }
    }
    
    bContinuousValidationActive = false;
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Continuous validation stopped"));
}

FBuild_ValidationResult UBuildValidationManager::TestCrossModuleIntegration()
{
    FBuild_ValidationResult Result;
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Testing cross-module integration..."));
    
    // Test basic integration scenarios
    bool bIntegrationValid = true;
    
    // Test 1: Can we access core game classes?
    if (!TestClassLoading(TEXT("/Script/TranspersonalGame.TranspersonalCharacter")))
    {
        Result.ErrorMessages.Add(TEXT("Cannot access TranspersonalCharacter"));
        bIntegrationValid = false;
    }
    
    // Test 2: Can we access game mode?
    if (!TestClassLoading(TEXT("/Script/TranspersonalGame.TranspersonalGameMode")))
    {
        Result.ErrorMessages.Add(TEXT("Cannot access TranspersonalGameMode"));
        bIntegrationValid = false;
    }
    
    Result.bIsValid = bIntegrationValid;
    Result.ErrorCount = Result.ErrorMessages.Num();
    Result.ValidationMessage = bIntegrationValid ? TEXT("Cross-module integration PASSED") : TEXT("Cross-module integration FAILED");
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Cross-module integration test: %s"), *Result.ValidationMessage);
    
    return Result;
}

FBuild_ValidationResult UBuildValidationManager::ValidateMinPlayableMap()
{
    FBuild_ValidationResult Result;
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Validating MinPlayableMap..."));
    
    // This is a placeholder - in a real implementation we would:
    // 1. Load the MinPlayableMap
    // 2. Check for required actors (PlayerStart, etc.)
    // 3. Validate level streaming
    // 4. Test gameplay functionality
    
    Result.bIsValid = true;
    Result.ValidationMessage = TEXT("MinPlayableMap validation not fully implemented yet");
    Result.WarningMessages.Add(TEXT("Map validation is placeholder"));
    Result.WarningCount = 1;
    
    return Result;
}

void UBuildValidationManager::LogValidationResults(const FBuild_ValidationResult& Results)
{
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD VALIDATION RESULTS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Status: %s"), Results.bIsValid ? TEXT("VALID") : TEXT("INVALID"));
    UE_LOG(LogTemp, Warning, TEXT("Message: %s"), *Results.ValidationMessage);
    UE_LOG(LogTemp, Warning, TEXT("Errors: %d, Warnings: %d"), Results.ErrorCount, Results.WarningCount);
    
    for (const FString& Error : Results.ErrorMessages)
    {
        UE_LOG(LogTemp, Error, TEXT("ERROR: %s"), *Error);
    }
    
    for (const FString& Warning : Results.WarningMessages)
    {
        UE_LOG(LogTemp, Warning, TEXT("WARNING: %s"), *Warning);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END VALIDATION RESULTS ==="));
}

FString UBuildValidationManager::GetBuildStatusSummary()
{
    if (!bInitialized)
    {
        return TEXT("BuildValidationManager not initialized");
    }
    
    FString Summary = FString::Printf(
        TEXT("Build Status: %s | Validation Runs: %d | Last Run: %.1fs ago"),
        LastValidationResult.bIsValid ? TEXT("VALID") : TEXT("INVALID"),
        ValidationRunCount,
        FPlatformTime::Seconds() - LastValidationTime
    );
    
    return Summary;
}

bool UBuildValidationManager::ValidateTranspersonalGameModule()
{
    FModuleManager& ModuleManager = FModuleManager::Get();
    return ModuleManager.IsModuleLoaded(TEXT("TranspersonalGame"));
}

bool UBuildValidationManager::ValidateCoreClasses()
{
    for (const FString& ClassPath : CoreClassPaths)
    {
        if (!TestClassLoading(ClassPath))
        {
            UE_LOG(LogTemp, Error, TEXT("BuildValidationManager: Core class validation failed: %s"), *ClassPath);
            return false;
        }
    }
    return true;
}

bool UBuildValidationManager::ValidateGameplayClasses()
{
    int32 FailedCount = 0;
    for (const FString& ClassPath : GameplayClassPaths)
    {
        if (!TestClassLoading(ClassPath))
        {
            FailedCount++;
            UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Gameplay class validation failed: %s"), *ClassPath);
        }
    }
    
    // Allow some gameplay classes to fail (they might not be implemented yet)
    return (FailedCount < GameplayClassPaths.Num());
}

bool UBuildValidationManager::ValidateSubsystems()
{
    // Test if this subsystem itself is working
    return IsValid(this) && bInitialized;
}

TArray<FString> UBuildValidationManager::GetExpectedClasses()
{
    TArray<FString> Expected = CoreClassPaths;
    Expected.Append(GameplayClassPaths);
    Expected.Append(SubsystemClassPaths);
    return Expected;
}

bool UBuildValidationManager::TestClassLoading(const FString& ClassName)
{
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
    return (LoadedClass != nullptr);
}

bool UBuildValidationManager::TestClassSpawning(const FString& ClassName)
{
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
    if (!LoadedClass)
    {
        return false;
    }
    
    // For now, just test that we can load the class
    // Actual spawning would require more context (world, etc.)
    return true;
}

void UBuildValidationManager::PerformValidationTick()
{
    if (!bContinuousValidationActive)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("BuildValidationManager: Performing validation tick..."));
    
    LastValidationResult = ValidateFullBuild();
    
    if (!LastValidationResult.bIsValid)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildValidationManager: Validation tick found errors!"));
        LogValidationResults(LastValidationResult);
    }
}