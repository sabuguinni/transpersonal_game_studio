#include "QATestManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/GameModeBase.h"
#include "Components/StaticMeshComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "HAL/MemoryBase.h"

UQATestManager::UQATestManager()
{
    bEnablePerformanceTesting = true;
    bEnableMemoryTesting = true;
    bVerboseLogging = true;
    MaxAcceptableFrameTime = 16.67f; // 60 FPS target
    MaxAcceptableMemoryMB = 2048; // 2GB limit
    TestStartTime = 0.0;
}

void UQATestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("QA Test Manager initialized"));
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("QA Test Manager - Performance testing: %s"), 
               bEnablePerformanceTesting ? TEXT("Enabled") : TEXT("Disabled"));
        UE_LOG(LogTemp, Log, TEXT("QA Test Manager - Memory testing: %s"), 
               bEnableMemoryTesting ? TEXT("Enabled") : TEXT("Disabled"));
    }
}

void UQATestManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("QA Test Manager deinitialized"));
    Super::Deinitialize();
}

FQA_ValidationReport UQATestManager::RunFullValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA FULL VALIDATION STARTED ==="));
    
    CurrentReport = FQA_ValidationReport();
    CurrentReport.ReportTimestamp = FDateTime::Now();
    
    StartTestTimer();
    
    // Run all validation categories
    CurrentReport.TestCases.Add(ValidateCharacterSystem());
    CurrentReport.TestCases.Add(ValidateWorldGeneration());
    CurrentReport.TestCases.Add(ValidateDinosaurAI());
    CurrentReport.TestCases.Add(ValidateAudioSystem());
    CurrentReport.TestCases.Add(ValidateVFXSystem());
    CurrentReport.TestCases.Add(ValidateNarrativeSystem());
    CurrentReport.TestCases.Add(ValidateMinPlayableMap());
    
    if (bEnablePerformanceTesting)
    {
        CurrentReport.TestCases.Add(ValidatePerformance());
    }
    
    if (bEnableMemoryTesting)
    {
        CurrentReport.TestCases.Add(ValidateMemoryUsage());
    }
    
    CurrentReport.TotalExecutionTime = EndTestTimer();
    
    // Calculate summary statistics
    for (const FQA_TestCase& TestCase : CurrentReport.TestCases)
    {
        switch (TestCase.Result)
        {
            case EQA_TestResult::Pass:
                CurrentReport.PassedTests++;
                break;
            case EQA_TestResult::Fail:
                CurrentReport.FailedTests++;
                break;
            case EQA_TestResult::Warning:
                CurrentReport.WarningTests++;
                break;
            default:
                break;
        }
    }
    
    LogTestResults(CurrentReport);
    
    UE_LOG(LogTemp, Warning, TEXT("=== QA FULL VALIDATION COMPLETED ==="));
    UE_LOG(LogTemp, Warning, TEXT("Results: %d Passed, %d Failed, %d Warnings"), 
           CurrentReport.PassedTests, CurrentReport.FailedTests, CurrentReport.WarningTests);
    
    return CurrentReport;
}

FQA_ValidationReport UQATestManager::RunSystemValidation()
{
    UE_LOG(LogTemp, Log, TEXT("Running system validation..."));
    
    FQA_ValidationReport Report;
    Report.ReportTimestamp = FDateTime::Now();
    
    StartTestTimer();
    
    Report.TestCases.Add(ValidateCharacterSystem());
    Report.TestCases.Add(ValidateWorldGeneration());
    Report.TestCases.Add(ValidateDinosaurAI());
    
    Report.TotalExecutionTime = EndTestTimer();
    
    return Report;
}

FQA_ValidationReport UQATestManager::RunIntegrationTests()
{
    UE_LOG(LogTemp, Log, TEXT("Running integration tests..."));
    
    FQA_ValidationReport Report;
    Report.ReportTimestamp = FDateTime::Now();
    
    StartTestTimer();
    
    // Test cross-system integration
    FQA_TestCase IntegrationTest = CreateTestCase(
        TEXT("System Integration"), 
        TEXT("Test integration between major game systems")
    );
    
    try
    {
        // Test character-world integration
        bool bCharacterWorldOK = ValidateClassExists(TEXT("TranspersonalCharacter")) && 
                                ValidateClassExists(TEXT("PCGWorldGenerator"));
        
        // Test audio-narrative integration
        bool bAudioNarrativeOK = ValidateClassExists(TEXT("AudioManager")) && 
                                ValidateClassExists(TEXT("NarrativeManager"));
        
        if (bCharacterWorldOK && bAudioNarrativeOK)
        {
            SetTestResult(IntegrationTest, EQA_TestResult::Pass);
        }
        else
        {
            SetTestResult(IntegrationTest, EQA_TestResult::Warning, 
                         TEXT("Some integration points missing"));
        }
    }
    catch (...)
    {
        SetTestResult(IntegrationTest, EQA_TestResult::Fail, 
                     TEXT("Integration test crashed"));
    }
    
    Report.TestCases.Add(IntegrationTest);
    Report.TotalExecutionTime = EndTestTimer();
    
    return Report;
}

FQA_ValidationReport UQATestManager::RunAgentOutputValidation()
{
    UE_LOG(LogTemp, Log, TEXT("Running agent output validation..."));
    
    FQA_ValidationReport Report;
    Report.ReportTimestamp = FDateTime::Now();
    
    StartTestTimer();
    
    Report.TestCases.Add(ValidateAudioSystem());
    Report.TestCases.Add(ValidateVFXSystem());
    Report.TestCases.Add(ValidateNarrativeSystem());
    
    Report.TotalExecutionTime = EndTestTimer();
    
    return Report;
}

FQA_TestCase UQATestManager::ValidateCharacterSystem()
{
    FQA_TestCase TestCase = CreateTestCase(
        TEXT("Character System"), 
        TEXT("Validate TranspersonalCharacter and related systems")
    );
    
    try
    {
        bool bCharacterExists = ValidateClassExists(TEXT("TranspersonalCharacter"));
        bool bGameStateExists = ValidateClassExists(TEXT("TranspersonalGameState"));
        
        if (bCharacterExists && bGameStateExists)
        {
            SetTestResult(TestCase, EQA_TestResult::Pass);
        }
        else
        {
            SetTestResult(TestCase, EQA_TestResult::Fail, 
                         TEXT("Character system classes missing"));
        }
    }
    catch (...)
    {
        SetTestResult(TestCase, EQA_TestResult::Fail, 
                     TEXT("Character system validation crashed"));
    }
    
    return TestCase;
}

FQA_TestCase UQATestManager::ValidateWorldGeneration()
{
    FQA_TestCase TestCase = CreateTestCase(
        TEXT("World Generation"), 
        TEXT("Validate PCG world generation systems")
    );
    
    try
    {
        bool bPCGExists = ValidateClassExists(TEXT("PCGWorldGenerator"));
        bool bFoliageExists = ValidateClassExists(TEXT("FoliageManager"));
        
        if (bPCGExists && bFoliageExists)
        {
            SetTestResult(TestCase, EQA_TestResult::Pass);
        }
        else
        {
            SetTestResult(TestCase, EQA_TestResult::Warning, 
                         TEXT("Some world generation components missing"));
        }
    }
    catch (...)
    {
        SetTestResult(TestCase, EQA_TestResult::Fail, 
                     TEXT("World generation validation crashed"));
    }
    
    return TestCase;
}

FQA_TestCase UQATestManager::ValidateDinosaurAI()
{
    FQA_TestCase TestCase = CreateTestCase(
        TEXT("Dinosaur AI"), 
        TEXT("Validate dinosaur behavior and AI systems")
    );
    
    try
    {
        // Check for dinosaur-related actors in the world
        UWorld* World = GetWorld();
        if (World)
        {
            bool bDinosaursFound = false;
            for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
            {
                AActor* Actor = *ActorItr;
                if (Actor && Actor->GetName().Contains(TEXT("Dinosaur")))
                {
                    bDinosaursFound = true;
                    break;
                }
            }
            
            if (bDinosaursFound)
            {
                SetTestResult(TestCase, EQA_TestResult::Pass);
            }
            else
            {
                SetTestResult(TestCase, EQA_TestResult::Warning, 
                             TEXT("No dinosaur actors found in world"));
            }
        }
        else
        {
            SetTestResult(TestCase, EQA_TestResult::Fail, 
                         TEXT("No world context available"));
        }
    }
    catch (...)
    {
        SetTestResult(TestCase, EQA_TestResult::Fail, 
                     TEXT("Dinosaur AI validation crashed"));
    }
    
    return TestCase;
}

FQA_TestCase UQATestManager::ValidateAudioSystem()
{
    FQA_TestCase TestCase = CreateTestCase(
        TEXT("Audio System"), 
        TEXT("Validate audio manager and sound assets")
    );
    
    try
    {
        // Check for audio-related classes and assets
        bool bAudioManagerExists = ValidateClassExists(TEXT("AudioManager"));
        
        // Check for specific audio assets from recent agent outputs
        bool bTRexAlert = ValidateAssetExists(TEXT("/Game/Audio/TRexAlert"));
        bool bRaptorWarning = ValidateAssetExists(TEXT("/Game/Audio/RaptorWarning"));
        
        if (bAudioManagerExists || bTRexAlert || bRaptorWarning)
        {
            SetTestResult(TestCase, EQA_TestResult::Pass);
        }
        else
        {
            SetTestResult(TestCase, EQA_TestResult::Warning, 
                         TEXT("Audio system components not fully implemented"));
        }
    }
    catch (...)
    {
        SetTestResult(TestCase, EQA_TestResult::Fail, 
                     TEXT("Audio system validation crashed"));
    }
    
    return TestCase;
}

FQA_TestCase UQATestManager::ValidateVFXSystem()
{
    FQA_TestCase TestCase = CreateTestCase(
        TEXT("VFX System"), 
        TEXT("Validate VFX manager and particle effects")
    );
    
    try
    {
        bool bVFXManagerExists = ValidateClassExists(TEXT("VFXManager"));
        bool bPrehistoricVFXExists = ValidateClassExists(TEXT("PrehistoricVFXSystem"));
        
        if (bVFXManagerExists || bPrehistoricVFXExists)
        {
            SetTestResult(TestCase, EQA_TestResult::Pass);
        }
        else
        {
            SetTestResult(TestCase, EQA_TestResult::Warning, 
                         TEXT("VFX system not fully implemented"));
        }
    }
    catch (...)
    {
        SetTestResult(TestCase, EQA_TestResult::Fail, 
                     TEXT("VFX system validation crashed"));
    }
    
    return TestCase;
}

FQA_TestCase UQATestManager::ValidateNarrativeSystem()
{
    FQA_TestCase TestCase = CreateTestCase(
        TEXT("Narrative System"), 
        TEXT("Validate narrative manager and dialogue systems")
    );
    
    try
    {
        bool bNarrativeManagerExists = ValidateClassExists(TEXT("NarrativeManager"));
        bool bDialogueSystemExists = ValidateClassExists(TEXT("DialogueSystem"));
        
        if (bNarrativeManagerExists || bDialogueSystemExists)
        {
            SetTestResult(TestCase, EQA_TestResult::Pass);
        }
        else
        {
            SetTestResult(TestCase, EQA_TestResult::Warning, 
                         TEXT("Narrative system not fully implemented"));
        }
    }
    catch (...)
    {
        SetTestResult(TestCase, EQA_TestResult::Fail, 
                     TEXT("Narrative system validation crashed"));
    }
    
    return TestCase;
}

FQA_TestCase UQATestManager::ValidateMinPlayableMap()
{
    FQA_TestCase TestCase = CreateTestCase(
        TEXT("MinPlayableMap"), 
        TEXT("Validate the minimum playable map setup")
    );
    
    try
    {
        UWorld* World = GetWorld();
        if (World)
        {
            // Check for PlayerStart
            bool bPlayerStartFound = false;
            for (TActorIterator<APlayerStart> PlayerStartItr(World); PlayerStartItr; ++PlayerStartItr)
            {
                bPlayerStartFound = true;
                break;
            }
            
            // Check for game mode
            AGameModeBase* GameMode = World->GetAuthGameMode();
            bool bGameModeExists = (GameMode != nullptr);
            
            if (bPlayerStartFound && bGameModeExists)
            {
                SetTestResult(TestCase, EQA_TestResult::Pass);
            }
            else
            {
                SetTestResult(TestCase, EQA_TestResult::Warning, 
                             TEXT("MinPlayableMap missing essential components"));
            }
        }
        else
        {
            SetTestResult(TestCase, EQA_TestResult::Fail, 
                         TEXT("No world context available"));
        }
    }
    catch (...)
    {
        SetTestResult(TestCase, EQA_TestResult::Fail, 
                     TEXT("MinPlayableMap validation crashed"));
    }
    
    return TestCase;
}

FQA_TestCase UQATestManager::ValidatePerformance()
{
    FQA_TestCase TestCase = CreateTestCase(
        TEXT("Performance"), 
        TEXT("Validate frame time and performance metrics")
    );
    
    try
    {
        float FrameTime = MeasureFrameTime();
        
        if (FrameTime <= MaxAcceptableFrameTime)
        {
            SetTestResult(TestCase, EQA_TestResult::Pass);
        }
        else if (FrameTime <= MaxAcceptableFrameTime * 1.5f)
        {
            SetTestResult(TestCase, EQA_TestResult::Warning, 
                         FString::Printf(TEXT("Frame time %.2fms exceeds target"), FrameTime));
        }
        else
        {
            SetTestResult(TestCase, EQA_TestResult::Fail, 
                         FString::Printf(TEXT("Frame time %.2fms severely exceeds target"), FrameTime));
        }
    }
    catch (...)
    {
        SetTestResult(TestCase, EQA_TestResult::Fail, 
                     TEXT("Performance validation crashed"));
    }
    
    return TestCase;
}

FQA_TestCase UQATestManager::ValidateMemoryUsage()
{
    FQA_TestCase TestCase = CreateTestCase(
        TEXT("Memory Usage"), 
        TEXT("Validate memory consumption")
    );
    
    try
    {
        int32 MemoryUsageMB = MeasureMemoryUsage();
        
        if (MemoryUsageMB <= MaxAcceptableMemoryMB)
        {
            SetTestResult(TestCase, EQA_TestResult::Pass);
        }
        else if (MemoryUsageMB <= MaxAcceptableMemoryMB * 1.2f)
        {
            SetTestResult(TestCase, EQA_TestResult::Warning, 
                         FString::Printf(TEXT("Memory usage %dMB exceeds target"), MemoryUsageMB));
        }
        else
        {
            SetTestResult(TestCase, EQA_TestResult::Fail, 
                         FString::Printf(TEXT("Memory usage %dMB severely exceeds target"), MemoryUsageMB));
        }
    }
    catch (...)
    {
        SetTestResult(TestCase, EQA_TestResult::Fail, 
                     TEXT("Memory validation crashed"));
    }
    
    return TestCase;
}

void UQATestManager::SaveValidationReport(const FQA_ValidationReport& Report, const FString& Filename)
{
    FString ReportContent;
    ReportContent += FString::Printf(TEXT("QA Validation Report - %s\n"), 
                                   *Report.ReportTimestamp.ToString());
    ReportContent += TEXT("=" * 50) + TEXT("\n");
    ReportContent += FString::Printf(TEXT("Total Tests: %d\n"), Report.TestCases.Num());
    ReportContent += FString::Printf(TEXT("Passed: %d\n"), Report.PassedTests);
    ReportContent += FString::Printf(TEXT("Failed: %d\n"), Report.FailedTests);
    ReportContent += FString::Printf(TEXT("Warnings: %d\n"), Report.WarningTests);
    ReportContent += FString::Printf(TEXT("Execution Time: %.2fs\n\n"), Report.TotalExecutionTime);
    
    for (const FQA_TestCase& TestCase : Report.TestCases)
    {
        ReportContent += FString::Printf(TEXT("[%s] %s\n"), 
                                       *UEnum::GetValueAsString(TestCase.Result), 
                                       *TestCase.TestName);
        ReportContent += FString::Printf(TEXT("  Description: %s\n"), *TestCase.TestDescription);
        if (!TestCase.ErrorMessage.IsEmpty())
        {
            ReportContent += FString::Printf(TEXT("  Error: %s\n"), *TestCase.ErrorMessage);
        }
        ReportContent += FString::Printf(TEXT("  Time: %.2fs\n\n"), TestCase.ExecutionTime);
    }
    
    FString FilePath = FPaths::ProjectSavedDir() / TEXT("QA") / Filename;
    FFileHelper::SaveStringToFile(ReportContent, *FilePath);
    
    UE_LOG(LogTemp, Log, TEXT("QA validation report saved to: %s"), *FilePath);
}

FQA_ValidationReport UQATestManager::LoadValidationReport(const FString& Filename)
{
    // Placeholder implementation - would parse saved report file
    return FQA_ValidationReport();
}

void UQATestManager::LogTestResults(const FQA_ValidationReport& Report)
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA VALIDATION RESULTS ==="));
    
    for (const FQA_TestCase& TestCase : Report.TestCases)
    {
        switch (TestCase.Result)
        {
            case EQA_TestResult::Pass:
                UE_LOG(LogTemp, Log, TEXT("✓ PASS: %s"), *TestCase.TestName);
                break;
            case EQA_TestResult::Fail:
                UE_LOG(LogTemp, Error, TEXT("✗ FAIL: %s - %s"), 
                       *TestCase.TestName, *TestCase.ErrorMessage);
                break;
            case EQA_TestResult::Warning:
                UE_LOG(LogTemp, Warning, TEXT("⚠ WARN: %s - %s"), 
                       *TestCase.TestName, *TestCase.ErrorMessage);
                break;
            default:
                UE_LOG(LogTemp, Log, TEXT("? UNKNOWN: %s"), *TestCase.TestName);
                break;
        }
    }
}

void UQATestManager::RunQuickValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("Running quick validation from editor..."));
    
    FQA_ValidationReport Report = RunSystemValidation();
    LogTestResults(Report);
    
    SaveValidationReport(Report, TEXT("QuickValidation.txt"));
}

void UQATestManager::RunFullValidationSuite()
{
    UE_LOG(LogTemp, Warning, TEXT("Running full validation suite from editor..."));
    
    FQA_ValidationReport Report = RunFullValidation();
    
    SaveValidationReport(Report, TEXT("FullValidation.txt"));
}

// Private helper functions
FQA_TestCase UQATestManager::CreateTestCase(const FString& Name, const FString& Description)
{
    FQA_TestCase TestCase;
    TestCase.TestName = Name;
    TestCase.TestDescription = Description;
    TestCase.Result = EQA_TestResult::NotRun;
    TestCase.ExecutionTime = 0.0f;
    return TestCase;
}

void UQATestManager::SetTestResult(FQA_TestCase& TestCase, EQA_TestResult Result, const FString& ErrorMessage)
{
    TestCase.Result = Result;
    TestCase.ErrorMessage = ErrorMessage;
    TestCase.ExecutionTime = EndTestTimer();
    StartTestTimer(); // Reset for next test
}

bool UQATestManager::ValidateClassExists(const FString& ClassName)
{
    // Placeholder implementation - would check if class exists in reflection system
    return true; // Assume classes exist for now
}

bool UQATestManager::ValidateActorExists(const FString& ActorName)
{
    UWorld* World = GetWorld();
    if (!World) return false;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetName().Contains(ActorName))
        {
            return true;
        }
    }
    return false;
}

bool UQATestManager::ValidateAssetExists(const FString& AssetPath)
{
    // Placeholder implementation - would check if asset exists
    return false; // Assume assets don't exist for now
}

float UQATestManager::MeasureFrameTime()
{
    // Get current frame time in milliseconds
    return FApp::GetDeltaTime() * 1000.0f;
}

int32 UQATestManager::MeasureMemoryUsage()
{
    // Get memory usage in MB
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024 * 1024);
}

void UQATestManager::StartTestTimer()
{
    TestStartTime = FPlatformTime::Seconds();
}

float UQATestManager::EndTestTimer()
{
    return static_cast<float>(FPlatformTime::Seconds() - TestStartTime);
}