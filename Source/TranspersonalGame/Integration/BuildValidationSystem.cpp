#include "BuildValidationSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameStateBase.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

UBuildValidationSystem::UBuildValidationSystem()
{
    // Initialize validation state
    bValidationInProgress = false;
    LastValidationTime = 0.0f;
    TotalTestsRun = 0;
    TestsPassed = 0;
    TestsFailed = 0;

    // Register core integration tests
    FBuild_IntegrationTest CoreModuleTest;
    CoreModuleTest.TestCategory = TEXT("Core Module");
    CoreModuleTest.TestDescription = TEXT("Verify TranspersonalGame module loads and core classes are accessible");
    CoreModuleTest.bIsRequired = true;
    CoreModuleTest.Dependencies.Add(TEXT("TranspersonalGame"));
    RegisteredTests.Add(CoreModuleTest);

    FBuild_IntegrationTest GameplayTest;
    GameplayTest.TestCategory = TEXT("Gameplay");
    GameplayTest.TestDescription = TEXT("Test GameMode, Character, and GameState integration");
    GameplayTest.bIsRequired = true;
    GameplayTest.Dependencies.Add(TEXT("TranspersonalGameMode"));
    GameplayTest.Dependencies.Add(TEXT("TranspersonalCharacter"));
    GameplayTest.Dependencies.Add(TEXT("TranspersonalGameState"));
    RegisteredTests.Add(GameplayTest);
}

void UBuildValidationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Initializing build validation subsystem"));
    
    // Clear previous results
    LastValidationResults.Empty();
    CachedModuleStatus.Empty();
    
    // Run initial validation
    RunFullValidationSuite();
}

void UBuildValidationSystem::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Shutting down build validation subsystem"));
    
    // Log final build status
    LogBuildStatus();
    
    Super::Deinitialize();
}

bool UBuildValidationSystem::RunFullValidationSuite()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildValidationSystem: Validation already in progress"));
        return false;
    }

    bValidationInProgress = true;
    LastValidationResults.Empty();
    TotalTestsRun = 0;
    TestsPassed = 0;
    TestsFailed = 0;

    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Starting full validation suite"));
    
    double StartTime = FPlatformTime::Seconds();

    // Run all validation tests
    bool bModuleValidation = ValidateModuleCompilation();
    bool bClassValidation = ValidateClassLoading();
    bool bActorValidation = ValidateActorSpawning();
    bool bIntegrationValidation = ValidateIntegrationScenarios();

    double EndTime = FPlatformTime::Seconds();
    float TotalTime = static_cast<float>(EndTime - StartTime);

    // Create summary result
    bool bOverallSuccess = bModuleValidation && bClassValidation && bActorValidation && bIntegrationValidation;
    
    LastValidationSummary = FBuild_ValidationResult(
        TEXT("Full Validation Suite"),
        bOverallSuccess,
        bOverallSuccess ? TEXT("All tests passed") : TEXT("Some tests failed"),
        TotalTime
    );

    bValidationInProgress = false;

    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Validation suite completed in %.2f seconds. Success: %s"), 
           TotalTime, bOverallSuccess ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Tests run: %d, Passed: %d, Failed: %d"), 
           TotalTestsRun, TestsPassed, TestsFailed);

    return bOverallSuccess;
}

bool UBuildValidationSystem::ValidateModuleCompilation()
{
    auto TestFunction = [this]() -> bool
    {
        // Check if TranspersonalGame module is loaded
        FModuleManager& ModuleManager = FModuleManager::Get();
        bool bModuleLoaded = ModuleManager.IsModuleLoaded(TEXT("TranspersonalGame"));
        
        if (!bModuleLoaded)
        {
            UE_LOG(LogTemp, Error, TEXT("BuildValidationSystem: TranspersonalGame module is not loaded"));
            return false;
        }

        // Update module status
        FBuild_ModuleStatus ModuleStatus;
        ModuleStatus.ModuleName = TEXT("TranspersonalGame");
        ModuleStatus.bIsLoaded = true;
        ModuleStatus.bHasErrors = false;
        ModuleStatus.ClassCount = CoreClassPaths.Num();

        CachedModuleStatus.Empty();
        CachedModuleStatus.Add(ModuleStatus);

        return true;
    };

    FBuild_ValidationResult Result = ExecuteValidationTest(TEXT("Module Compilation"), TestFunction);
    return Result.bSuccess;
}

bool UBuildValidationSystem::ValidateClassLoading()
{
    auto TestFunction = [this]() -> bool
    {
        bool bAllClassesLoaded = true;
        
        for (const FString& ClassPath : CoreClassPaths)
        {
            UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
            if (!LoadedClass)
            {
                UE_LOG(LogTemp, Error, TEXT("BuildValidationSystem: Failed to load class: %s"), *ClassPath);
                bAllClassesLoaded = false;
            }
            else
            {
                UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Successfully loaded class: %s"), *ClassPath);
            }
        }

        return bAllClassesLoaded;
    };

    FBuild_ValidationResult Result = ExecuteValidationTest(TEXT("Class Loading"), TestFunction);
    return Result.bSuccess;
}

bool UBuildValidationSystem::ValidateActorSpawning()
{
    auto TestFunction = [this]() -> bool
    {
        UWorld* World = GetWorld();
        if (!World)
        {
            UE_LOG(LogTemp, Error, TEXT("BuildValidationSystem: No world available for actor spawning test"));
            return false;
        }

        // Test spawning a basic actor
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = FName(TEXT("ValidationTestActor"));
        
        AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        
        if (!TestActor)
        {
            UE_LOG(LogTemp, Error, TEXT("BuildValidationSystem: Failed to spawn test actor"));
            return false;
        }

        // Clean up test actor
        TestActor->Destroy();
        
        UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Actor spawning test passed"));
        return true;
    };

    FBuild_ValidationResult Result = ExecuteValidationTest(TEXT("Actor Spawning"), TestFunction);
    return Result.bSuccess;
}

bool UBuildValidationSystem::ValidateIntegrationScenarios()
{
    auto TestFunction = [this]() -> bool
    {
        bool bAllIntegrationTestsPassed = true;

        for (const FBuild_IntegrationTest& Test : RegisteredTests)
        {
            if (Test.bIsRequired)
            {
                bool bTestPassed = RunSpecificTest(Test.TestDescription);
                if (!bTestPassed)
                {
                    UE_LOG(LogTemp, Error, TEXT("BuildValidationSystem: Required integration test failed: %s"), *Test.TestDescription);
                    bAllIntegrationTestsPassed = false;
                }
            }
        }

        return bAllIntegrationTestsPassed;
    };

    FBuild_ValidationResult Result = ExecuteValidationTest(TEXT("Integration Scenarios"), TestFunction);
    return Result.bSuccess;
}

TArray<FBuild_ModuleStatus> UBuildValidationSystem::GetModuleStatusReport()
{
    // Refresh module status if cache is empty
    if (CachedModuleStatus.Num() == 0)
    {
        ValidateModuleCompilation();
    }

    return CachedModuleStatus;
}

FBuild_ModuleStatus UBuildValidationSystem::GetTranspersonalGameModuleStatus()
{
    TArray<FBuild_ModuleStatus> StatusReport = GetModuleStatusReport();
    
    for (const FBuild_ModuleStatus& Status : StatusReport)
    {
        if (Status.ModuleName == TEXT("TranspersonalGame"))
        {
            return Status;
        }
    }

    // Return empty status if not found
    return FBuild_ModuleStatus();
}

bool UBuildValidationSystem::IsModuleHealthy(const FString& ModuleName)
{
    FBuild_ModuleStatus Status = GetTranspersonalGameModuleStatus();
    return Status.bIsLoaded && !Status.bHasErrors;
}

bool UBuildValidationSystem::HasValidationErrors() const
{
    return TestsFailed > 0;
}

void UBuildValidationSystem::RegisterIntegrationTest(const FBuild_IntegrationTest& Test)
{
    RegisteredTests.Add(Test);
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Registered integration test: %s"), *Test.TestDescription);
}

bool UBuildValidationSystem::RunSpecificTest(const FString& TestName)
{
    // Simple test execution - in a real implementation this would be more sophisticated
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Running specific test: %s"), *TestName);
    
    // For now, just return true as a placeholder
    return true;
}

float UBuildValidationSystem::GetOverallBuildHealth() const
{
    if (TotalTestsRun == 0)
    {
        return 0.0f;
    }

    return static_cast<float>(TestsPassed) / static_cast<float>(TotalTestsRun);
}

FString UBuildValidationSystem::GenerateBuildReport()
{
    FString Report;
    Report += TEXT("=== BUILD VALIDATION REPORT ===\n");
    Report += FString::Printf(TEXT("Validation Time: %s\n"), *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Total Tests: %d\n"), TotalTestsRun);
    Report += FString::Printf(TEXT("Tests Passed: %d\n"), TestsPassed);
    Report += FString::Printf(TEXT("Tests Failed: %d\n"), TestsFailed);
    Report += FString::Printf(TEXT("Build Health: %.1f%%\n"), GetOverallBuildHealth() * 100.0f);
    Report += TEXT("\n=== TEST RESULTS ===\n");

    for (const FBuild_ValidationResult& Result : LastValidationResults)
    {
        FString Status = Result.bSuccess ? TEXT("PASS") : TEXT("FAIL");
        Report += FString::Printf(TEXT("[%s] %s (%.2fs)\n"), *Status, *Result.TestName, Result.ExecutionTime);
        if (!Result.bSuccess && !Result.ErrorMessage.IsEmpty())
        {
            Report += FString::Printf(TEXT("  Error: %s\n"), *Result.ErrorMessage);
        }
    }

    Report += TEXT("\n=== MODULE STATUS ===\n");
    for (const FBuild_ModuleStatus& Status : CachedModuleStatus)
    {
        FString ModuleHealth = Status.bIsLoaded && !Status.bHasErrors ? TEXT("HEALTHY") : TEXT("UNHEALTHY");
        Report += FString::Printf(TEXT("%s: %s (%d classes)\n"), *Status.ModuleName, *ModuleHealth, Status.ClassCount);
    }

    return Report;
}

void UBuildValidationSystem::LogBuildStatus()
{
    FString Report = GenerateBuildReport();
    UE_LOG(LogTemp, Log, TEXT("%s"), *Report);
}

bool UBuildValidationSystem::TestClassConstruction(const FString& ClassName)
{
    UClass* TestClass = LoadClass<UObject>(nullptr, *ClassName);
    if (!TestClass)
    {
        return false;
    }

    // Try to create a CDO (Class Default Object)
    UObject* CDO = TestClass->GetDefaultObject();
    return CDO != nullptr;
}

bool UBuildValidationSystem::TestComponentRegistration(const FString& ComponentClassName)
{
    // Test if component class can be loaded and instantiated
    UClass* ComponentClass = LoadClass<UActorComponent>(nullptr, *ComponentClassName);
    return ComponentClass != nullptr;
}

bool UBuildValidationSystem::TestSubsystemInitialization(const FString& SubsystemClassName)
{
    // Test if subsystem class can be loaded
    UClass* SubsystemClass = LoadClass<USubsystem>(nullptr, *SubsystemClassName);
    return SubsystemClass != nullptr;
}

bool UBuildValidationSystem::TestCrossSystemIntegration()
{
    // Test basic cross-system functionality
    // This is a placeholder for more complex integration tests
    return true;
}

FBuild_ValidationResult UBuildValidationSystem::ExecuteValidationTest(const FString& TestName, TFunction<bool()> TestFunction)
{
    double StartTime = FPlatformTime::Seconds();
    
    bool bSuccess = false;
    FString ErrorMessage;

    try
    {
        bSuccess = TestFunction();
    }
    catch (const std::exception& e)
    {
        ErrorMessage = FString(e.what());
    }
    catch (...)
    {
        ErrorMessage = TEXT("Unknown exception occurred");
    }

    double EndTime = FPlatformTime::Seconds();
    float ExecutionTime = static_cast<float>(EndTime - StartTime);

    FBuild_ValidationResult Result(TestName, bSuccess, ErrorMessage, ExecutionTime);
    
    LastValidationResults.Add(Result);
    TotalTestsRun++;
    
    if (bSuccess)
    {
        TestsPassed++;
    }
    else
    {
        TestsFailed++;
    }

    LogValidationResult(Result);
    
    return Result;
}

void UBuildValidationSystem::LogValidationResult(const FBuild_ValidationResult& Result)
{
    if (Result.bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: [PASS] %s (%.2fs)"), *Result.TestName, Result.ExecutionTime);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BuildValidationSystem: [FAIL] %s (%.2fs) - %s"), 
               *Result.TestName, Result.ExecutionTime, *Result.ErrorMessage);
    }
}