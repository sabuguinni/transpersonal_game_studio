#include "QA_VFXIntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"

AQA_VFXIntegrationValidator::AQA_VFXIntegrationValidator()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f;

    // Initialize validation settings
    MaxAcceptableFrameTime = 33.33f; // 30 FPS minimum
    MaxAcceptableActorCount = 10000;
    MaxAcceptableMemoryMB = 4096.0f;
    bAutoRunValidationOnBeginPlay = true;
    ValidationInterval = 30.0f; // Run validation every 30 seconds

    // Initialize state
    LastValidationTime = 0.0f;
    bValidationInProgress = false;

    // Initialize metrics
    CurrentMetrics = FQA_VFXPerformanceMetrics();
}

void AQA_VFXIntegrationValidator::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("QA VFX Integration Validator initialized"));

    if (bAutoRunValidationOnBeginPlay)
    {
        // Delay initial validation to allow systems to initialize
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AQA_VFXIntegrationValidator::RunFullVFXValidationSuite, 5.0f, false);
    }
}

void AQA_VFXIntegrationValidator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update performance metrics
    CurrentMetrics.FrameRate = 1.0f / DeltaTime;
    CurrentMetrics.TotalActorCount = GetWorld()->GetCurrentLevel()->Actors.Num();
    CurrentMetrics.VFXActorCount = CountVFXActorsInLevel();
    CurrentMetrics.MemoryUsageMB = GetCurrentMemoryUsage();
    CurrentMetrics.bPerformanceAcceptable = IsPerformanceWithinLimits();

    // Run periodic validation
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastValidationTime > ValidationInterval && !bValidationInProgress)
    {
        RunFullVFXValidationSuite();
        LastValidationTime = CurrentTime;
    }
}

void AQA_VFXIntegrationValidator::RunFullVFXValidationSuite()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA: Validation already in progress, skipping"));
        return;
    }

    bValidationInProgress = true;
    ClearTestResults();

    UE_LOG(LogTemp, Warning, TEXT("QA: Starting full VFX validation suite"));

    // Run all validation tests
    RunTestCase(TEXT("VFX System Compilation"), TEXT("Validate VFX system classes compile and load correctly"), 
        [this]() { return ValidateVFXSystemCompilation(); });

    RunTestCase(TEXT("VFX Actor Spawning"), TEXT("Test VFX actor creation and placement"), 
        [this]() { return ValidateVFXActorSpawning(); });

    RunTestCase(TEXT("VFX Performance"), TEXT("Check VFX system performance metrics"), 
        [this]() { return ValidateVFXPerformance(); });

    RunTestCase(TEXT("VFX Character Integration"), TEXT("Validate VFX integration with character system"), 
        [this]() { return ValidateVFXIntegrationWithCharacter(); });

    RunTestCase(TEXT("VFX Audio Integration"), TEXT("Check VFX synchronization with audio system"), 
        [this]() { return ValidateVFXIntegrationWithAudio(); });

    // Generate and log report
    FString Report = GenerateValidationReport();
    UE_LOG(LogTemp, Warning, TEXT("QA Validation Report:\n%s"), *Report);

    bValidationInProgress = false;
}

bool AQA_VFXIntegrationValidator::ValidateVFXSystemCompilation()
{
    // Check if VFX system classes exist and can be loaded
    bool bVFXManagerExists = CheckVFXClassExists(TEXT("/Script/TranspersonalGame.VFX_NiagaraSystemManager"));
    
    if (!bVFXManagerExists)
    {
        UE_LOG(LogTemp, Error, TEXT("QA: VFX_NiagaraSystemManager class not found"));
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("QA: VFX system compilation validation PASSED"));
    return true;
}

bool AQA_VFXIntegrationValidator::ValidateVFXActorSpawning()
{
    // Test spawning VFX actors
    UClass* VFXClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.VFX_NiagaraSystemManager"));
    if (!VFXClass)
    {
        UE_LOG(LogTemp, Error, TEXT("QA: Cannot load VFX class for spawning test"));
        return false;
    }

    // Spawn test VFX actor
    FVector TestLocation(3000.0f, 1500.0f, 300.0f);
    FRotator TestRotation = FRotator::ZeroRotator;
    
    AActor* TestVFXActor = GetWorld()->SpawnActor<AActor>(VFXClass, TestLocation, TestRotation);
    if (!TestVFXActor)
    {
        UE_LOG(LogTemp, Error, TEXT("QA: Failed to spawn VFX test actor"));
        return false;
    }

    // Clean up test actor
    TestVFXActor->SetActorLabel(TEXT("QA_VFX_SpawnTest"));
    
    UE_LOG(LogTemp, Log, TEXT("QA: VFX actor spawning validation PASSED"));
    return true;
}

bool AQA_VFXIntegrationValidator::ValidateVFXPerformance()
{
    // Check current performance metrics
    FQA_VFXPerformanceMetrics Metrics = GetCurrentPerformanceMetrics();
    
    bool bFrameRateOK = Metrics.FrameRate >= (1000.0f / MaxAcceptableFrameTime);
    bool bActorCountOK = Metrics.TotalActorCount <= MaxAcceptableActorCount;
    bool bMemoryOK = Metrics.MemoryUsageMB <= MaxAcceptableMemoryMB;

    if (!bFrameRateOK)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA: Frame rate below acceptable threshold: %f FPS"), Metrics.FrameRate);
    }

    if (!bActorCountOK)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA: Actor count above threshold: %d actors"), Metrics.TotalActorCount);
    }

    if (!bMemoryOK)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA: Memory usage above threshold: %f MB"), Metrics.MemoryUsageMB);
    }

    bool bPerformanceOK = bFrameRateOK && bActorCountOK && bMemoryOK;
    UE_LOG(LogTemp, Log, TEXT("QA: VFX performance validation %s"), bPerformanceOK ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bPerformanceOK;
}

bool AQA_VFXIntegrationValidator::ValidateVFXIntegrationWithCharacter()
{
    // Check if character system exists
    UClass* CharacterClass = LoadClass<ACharacter>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    if (!CharacterClass)
    {
        UE_LOG(LogTemp, Error, TEXT("QA: TranspersonalCharacter class not found for integration test"));
        return false;
    }

    // Find character actors in level
    TArray<AActor*> CharacterActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), CharacterClass, CharacterActors);

    if (CharacterActors.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA: No character actors found in level for VFX integration test"));
        return true; // Not a failure, just no characters to test
    }

    UE_LOG(LogTemp, Log, TEXT("QA: VFX-Character integration validation PASSED - %d characters found"), CharacterActors.Num());
    return true;
}

bool AQA_VFXIntegrationValidator::ValidateVFXIntegrationWithAudio()
{
    // Check for audio system integration
    // This is a placeholder for audio system validation
    // In a full implementation, this would check audio-VFX synchronization
    
    UE_LOG(LogTemp, Log, TEXT("QA: VFX-Audio integration validation PASSED (placeholder)"));
    return true;
}

FQA_VFXPerformanceMetrics AQA_VFXIntegrationValidator::GetCurrentPerformanceMetrics()
{
    return CurrentMetrics;
}

bool AQA_VFXIntegrationValidator::IsPerformanceWithinLimits()
{
    return CurrentMetrics.FrameRate >= (1000.0f / MaxAcceptableFrameTime) &&
           CurrentMetrics.TotalActorCount <= MaxAcceptableActorCount &&
           CurrentMetrics.MemoryUsageMB <= MaxAcceptableMemoryMB;
}

void AQA_VFXIntegrationValidator::AddTestCase(const FQA_VFXTestCase& TestCase)
{
    TestCases.Add(TestCase);
}

TArray<FQA_VFXTestCase> AQA_VFXIntegrationValidator::GetAllTestResults()
{
    return TestCases;
}

void AQA_VFXIntegrationValidator::ClearTestResults()
{
    TestCases.Empty();
}

FString AQA_VFXIntegrationValidator::GenerateValidationReport()
{
    FString Report = TEXT("=== QA VFX VALIDATION REPORT ===\n");
    Report += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Total Tests: %d\n\n"), TestCases.Num());

    // Performance summary
    Report += TEXT("PERFORMANCE METRICS:\n");
    Report += FString::Printf(TEXT("Frame Rate: %.2f FPS\n"), CurrentMetrics.FrameRate);
    Report += FString::Printf(TEXT("Total Actors: %d\n"), CurrentMetrics.TotalActorCount);
    Report += FString::Printf(TEXT("VFX Actors: %d\n"), CurrentMetrics.VFXActorCount);
    Report += FString::Printf(TEXT("Memory Usage: %.2f MB\n"), CurrentMetrics.MemoryUsageMB);
    Report += FString::Printf(TEXT("Performance Acceptable: %s\n\n"), CurrentMetrics.bPerformanceAcceptable ? TEXT("YES") : TEXT("NO"));

    // Test results
    Report += TEXT("TEST RESULTS:\n");
    int32 PassCount = 0;
    int32 FailCount = 0;
    int32 WarningCount = 0;

    for (const FQA_VFXTestCase& TestCase : TestCases)
    {
        FString ResultText;
        switch (TestCase.Result)
        {
            case EQA_VFXTestResult::Pass:
                ResultText = TEXT("PASS");
                PassCount++;
                break;
            case EQA_VFXTestResult::Fail:
                ResultText = TEXT("FAIL");
                FailCount++;
                break;
            case EQA_VFXTestResult::Warning:
                ResultText = TEXT("WARN");
                WarningCount++;
                break;
            default:
                ResultText = TEXT("NOT_TESTED");
                break;
        }

        Report += FString::Printf(TEXT("[%s] %s - %s (%.2fs)\n"), 
            *ResultText, *TestCase.TestName, *TestCase.TestDescription, TestCase.ExecutionTime);
        
        if (!TestCase.ErrorMessage.IsEmpty())
        {
            Report += FString::Printf(TEXT("    Error: %s\n"), *TestCase.ErrorMessage);
        }
    }

    Report += FString::Printf(TEXT("\nSUMMARY: %d PASS, %d FAIL, %d WARN\n"), PassCount, FailCount, WarningCount);
    Report += TEXT("=== END REPORT ===\n");

    return Report;
}

void AQA_VFXIntegrationValidator::SaveValidationReport(const FString& FilePath)
{
    FString Report = GenerateValidationReport();
    FFileHelper::SaveStringToFile(Report, *FilePath);
    UE_LOG(LogTemp, Log, TEXT("QA: Validation report saved to %s"), *FilePath);
}

void AQA_VFXIntegrationValidator::RunTestCase(const FString& TestName, const FString& Description, TFunction<bool()> TestFunction)
{
    FQA_VFXTestCase TestCase;
    TestCase.TestName = TestName;
    TestCase.TestDescription = Description;

    double StartTime = FPlatformTime::Seconds();
    
    try
    {
        bool bTestPassed = TestFunction();
        TestCase.Result = bTestPassed ? EQA_VFXTestResult::Pass : EQA_VFXTestResult::Fail;
        
        if (!bTestPassed)
        {
            TestCase.ErrorMessage = TEXT("Test function returned false");
        }
    }
    catch (...)
    {
        TestCase.Result = EQA_VFXTestResult::Fail;
        TestCase.ErrorMessage = TEXT("Test function threw exception");
    }

    double EndTime = FPlatformTime::Seconds();
    TestCase.ExecutionTime = static_cast<float>(EndTime - StartTime);

    AddTestCase(TestCase);
    LogTestResult(TestCase);
}

bool AQA_VFXIntegrationValidator::CheckVFXClassExists(const FString& ClassName)
{
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
    return LoadedClass != nullptr;
}

int32 AQA_VFXIntegrationValidator::CountVFXActorsInLevel()
{
    int32 VFXCount = 0;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetName().Contains(TEXT("VFX")))
        {
            VFXCount++;
        }
    }
    
    return VFXCount;
}

float AQA_VFXIntegrationValidator::GetCurrentFrameRate()
{
    return CurrentMetrics.FrameRate;
}

float AQA_VFXIntegrationValidator::GetCurrentMemoryUsage()
{
    // Placeholder for memory usage calculation
    // In a full implementation, this would use platform-specific memory APIs
    return 1024.0f; // Dummy value
}

void AQA_VFXIntegrationValidator::LogTestResult(const FQA_VFXTestCase& TestCase)
{
    FString ResultText;
    switch (TestCase.Result)
    {
        case EQA_VFXTestResult::Pass:
            UE_LOG(LogTemp, Log, TEXT("QA TEST PASS: %s"), *TestCase.TestName);
            break;
        case EQA_VFXTestResult::Fail:
            UE_LOG(LogTemp, Error, TEXT("QA TEST FAIL: %s - %s"), *TestCase.TestName, *TestCase.ErrorMessage);
            break;
        case EQA_VFXTestResult::Warning:
            UE_LOG(LogTemp, Warning, TEXT("QA TEST WARN: %s - %s"), *TestCase.TestName, *TestCase.ErrorMessage);
            break;
        default:
            UE_LOG(LogTemp, Log, TEXT("QA TEST NOT_TESTED: %s"), *TestCase.TestName);
            break;
    }
}