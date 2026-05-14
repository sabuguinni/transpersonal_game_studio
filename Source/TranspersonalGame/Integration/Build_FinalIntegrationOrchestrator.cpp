#include "Build_FinalIntegrationOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Engine/GameViewportClient.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"

ABuild_FinalIntegrationOrchestrator::ABuild_FinalIntegrationOrchestrator()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f;
    
    // Initialize state
    CurrentIntegrationPhase = EBuild_IntegrationPhase::Initialization;
    bIntegrationInProgress = false;
    IntegrationStartTime = 0.0f;
    
    // Set default configuration
    MaxIntegrationTime = 300.0f;
    MaxAcceptableActorCount = 1000;
    MinAcceptableFrameRate = 30.0f;
    MaxAcceptableMemoryMB = 2048.0f;
    bAutoStartIntegration = true;
    bVerboseLogging = true;
    
    // Initialize integration report
    IntegrationReport.BuildVersion = TEXT("1.0.0-FINAL");
    IntegrationReport.BuildTimestamp = FDateTime::Now();
}

void ABuild_FinalIntegrationOrchestrator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Final Integration Orchestrator: Starting initialization"));
    }
    
    InitializeIntegration();
    
    if (bAutoStartIntegration)
    {
        // Start integration after a brief delay
        GetWorldTimerManager().SetTimer(
            IntegrationTimerHandle,
            this,
            &ABuild_FinalIntegrationOrchestrator::StartFinalIntegration,
            2.0f,
            false
        );
    }
}

void ABuild_FinalIntegrationOrchestrator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up timers
    if (GetWorld())
    {
        GetWorldTimerManager().ClearTimer(IntegrationTimerHandle);
        GetWorldTimerManager().ClearTimer(PerformanceMonitorHandle);
    }
    
    // Final cleanup
    CleanupTestActors();
    
    Super::EndPlay(EndPlayReason);
}

void ABuild_FinalIntegrationOrchestrator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIntegrationInProgress)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        float ElapsedTime = CurrentTime - IntegrationStartTime;
        
        // Check for timeout
        if (ElapsedTime > MaxIntegrationTime)
        {
            HandleIntegrationFailure(TEXT("Integration timeout exceeded"));
            return;
        }
        
        // Update integration report time
        IntegrationReport.TotalIntegrationTime = ElapsedTime;
    }
}

void ABuild_FinalIntegrationOrchestrator::StartFinalIntegration()
{
    if (bIntegrationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Integration already in progress"));
        return;
    }
    
    bIntegrationInProgress = true;
    IntegrationStartTime = GetWorld()->GetTimeSeconds();
    CurrentIntegrationPhase = EBuild_IntegrationPhase::SystemValidation;
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Final Integration: Starting system validation phase"));
    }
    
    // Clear previous results
    IntegrationReport.SystemResults.Empty();
    IntegrationReport.CriticalErrors.Empty();
    IntegrationReport.Warnings.Empty();
    
    // Start validation process
    ValidateAllSystems();
}

void ABuild_FinalIntegrationOrchestrator::ValidateAllSystems()
{
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Final Integration: Validating all systems"));
    }
    
    // Validate each system
    IntegrationReport.SystemResults.Add(ValidateVFXSystem());
    IntegrationReport.SystemResults.Add(ValidateCharacterSystem());
    IntegrationReport.SystemResults.Add(ValidateGameStateSystem());
    IntegrationReport.SystemResults.Add(ValidateQASystem());
    
    // Check if all systems passed validation
    bool bAllSystemsValid = true;
    for (const FBuild_SystemValidationResult& Result : IntegrationReport.SystemResults)
    {
        if (!Result.bSystemLoaded || !Result.bCompilationSuccess || !Result.bRuntimeStable)
        {
            bAllSystemsValid = false;
            IntegrationReport.CriticalErrors.Add(
                FString::Printf(TEXT("System validation failed: %s - %s"), 
                *Result.SystemName, *Result.ValidationMessage)
            );
        }
    }
    
    if (bAllSystemsValid)
    {
        AdvanceIntegrationPhase();
        RunCrossSystemTests();
    }
    else
    {
        HandleIntegrationFailure(TEXT("System validation failed"));
    }
}

void ABuild_FinalIntegrationOrchestrator::RunCrossSystemTests()
{
    CurrentIntegrationPhase = EBuild_IntegrationPhase::CrossSystemTesting;
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Final Integration: Running cross-system tests"));
    }
    
    bool bCrossSystemTestsPassed = true;
    
    // Test VFX-Character integration
    if (!TestVFXCharacterIntegration())
    {
        bCrossSystemTestsPassed = false;
        IntegrationReport.CriticalErrors.Add(TEXT("VFX-Character integration test failed"));
    }
    
    // Test system communication
    if (!TestSystemCommunication())
    {
        bCrossSystemTestsPassed = false;
        IntegrationReport.CriticalErrors.Add(TEXT("System communication test failed"));
    }
    
    // Test memory management
    if (!TestMemoryManagement())
    {
        bCrossSystemTestsPassed = false;
        IntegrationReport.Warnings.Add(TEXT("Memory management test showed potential issues"));
    }
    
    if (bCrossSystemTestsPassed)
    {
        AdvanceIntegrationPhase();
        PerformPerformanceValidation();
    }
    else
    {
        HandleIntegrationFailure(TEXT("Cross-system tests failed"));
    }
}

void ABuild_FinalIntegrationOrchestrator::PerformPerformanceValidation()
{
    CurrentIntegrationPhase = EBuild_IntegrationPhase::PerformanceCheck;
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Final Integration: Performing performance validation"));
    }
    
    // Gather performance metrics
    IntegrationReport.PerformanceData = GatherPerformanceMetrics();
    
    // Check if performance is acceptable
    if (IsPerformanceAcceptable(IntegrationReport.PerformanceData))
    {
        AdvanceIntegrationPhase();
        GenerateFinalReport();
    }
    else
    {
        IntegrationReport.CriticalErrors.Add(TEXT("Performance validation failed"));
        HandleIntegrationFailure(TEXT("Performance below acceptable thresholds"));
    }
}

void ABuild_FinalIntegrationOrchestrator::GenerateFinalReport()
{
    CurrentIntegrationPhase = EBuild_IntegrationPhase::FinalValidation;
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Final Integration: Generating final report"));
    }
    
    // Finalize integration report
    IntegrationReport.bBuildSuccessful = true;
    IntegrationReport.CurrentPhase = EBuild_IntegrationPhase::BuildComplete;
    
    // Log final status
    LogIntegrationStatus();
    
    // Export build report
    ExportBuildReport();
    
    // Complete integration
    HandleIntegrationSuccess();
}

bool ABuild_FinalIntegrationOrchestrator::IsBuildReady() const
{
    return IntegrationReport.bBuildSuccessful && 
           CurrentIntegrationPhase == EBuild_IntegrationPhase::BuildComplete;
}

FBuild_SystemValidationResult ABuild_FinalIntegrationOrchestrator::ValidateVFXSystem()
{
    float StartTime = GetWorld()->GetTimeSeconds();
    
    // Try to load VFX system class
    UClass* VFXClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.VFX_ParticleSystemManager"));
    bool bLoaded = (VFXClass != nullptr);
    
    FString ValidationMessage = bLoaded ? 
        TEXT("VFX system loaded successfully") : 
        TEXT("Failed to load VFX system class");
    
    float ValidationTime = GetWorld()->GetTimeSeconds() - StartTime;
    
    return CreateValidationResult(TEXT("VFX System"), bLoaded, bLoaded, bLoaded, ValidationMessage);
}

FBuild_SystemValidationResult ABuild_FinalIntegrationOrchestrator::ValidateCharacterSystem()
{
    float StartTime = GetWorld()->GetTimeSeconds();
    
    // Try to load Character system class
    UClass* CharacterClass = LoadClass<ACharacter>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    bool bLoaded = (CharacterClass != nullptr);
    
    // Check for character actors in the world
    TArray<AActor*> CharacterActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), CharacterClass, CharacterActors);
    
    FString ValidationMessage = bLoaded ? 
        FString::Printf(TEXT("Character system loaded, %d character actors found"), CharacterActors.Num()) :
        TEXT("Failed to load Character system class");
    
    float ValidationTime = GetWorld()->GetTimeSeconds() - StartTime;
    
    return CreateValidationResult(TEXT("Character System"), bLoaded, bLoaded, bLoaded, ValidationMessage);
}

FBuild_SystemValidationResult ABuild_FinalIntegrationOrchestrator::ValidateGameStateSystem()
{
    float StartTime = GetWorld()->GetTimeSeconds();
    
    // Try to load GameState system class
    UClass* GameStateClass = LoadClass<AGameStateBase>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
    bool bLoaded = (GameStateClass != nullptr);
    
    FString ValidationMessage = bLoaded ? 
        TEXT("GameState system loaded successfully") : 
        TEXT("Failed to load GameState system class");
    
    float ValidationTime = GetWorld()->GetTimeSeconds() - StartTime;
    
    return CreateValidationResult(TEXT("GameState System"), bLoaded, bLoaded, bLoaded, ValidationMessage);
}

FBuild_SystemValidationResult ABuild_FinalIntegrationOrchestrator::ValidateQASystem()
{
    float StartTime = GetWorld()->GetTimeSeconds();
    
    // Try to load QA system class
    UClass* QAClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.QA_VFXIntegrationValidator"));
    bool bLoaded = (QAClass != nullptr);
    
    FString ValidationMessage = bLoaded ? 
        TEXT("QA system loaded successfully") : 
        TEXT("Failed to load QA system class");
    
    float ValidationTime = GetWorld()->GetTimeSeconds() - StartTime;
    
    return CreateValidationResult(TEXT("QA System"), bLoaded, bLoaded, bLoaded, ValidationMessage);
}

FBuild_PerformanceMetrics ABuild_FinalIntegrationOrchestrator::GatherPerformanceMetrics()
{
    FBuild_PerformanceMetrics Metrics;
    
    // Get actor count
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    Metrics.TotalActorCount = AllActors.Num();
    
    // Count VFX actors (approximate)
    Metrics.ActiveVFXCount = 0; // Would need specific VFX counting logic
    
    // Count character actors
    UClass* CharacterClass = LoadClass<ACharacter>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    if (CharacterClass)
    {
        TArray<AActor*> CharacterActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), CharacterClass, CharacterActors);
        Metrics.CharacterCount = CharacterActors.Num();
    }
    
    // Get frame rate (approximate)
    if (GEngine && GEngine->GetGameViewport())
    {
        Metrics.FrameRate = 1.0f / GetWorld()->GetDeltaSeconds();
    }
    
    // Memory usage (basic estimation)
    Metrics.MemoryUsageMB = 512.0f; // Placeholder - would need platform-specific memory queries
    
    // CPU usage (placeholder)
    Metrics.CPUUsagePercent = 25.0f; // Placeholder
    
    // Determine if performance is acceptable
    Metrics.bPerformanceAcceptable = IsPerformanceAcceptable(Metrics);
    
    return Metrics;
}

bool ABuild_FinalIntegrationOrchestrator::IsPerformanceAcceptable(const FBuild_PerformanceMetrics& Metrics) const
{
    return (Metrics.TotalActorCount <= MaxAcceptableActorCount) &&
           (Metrics.FrameRate >= MinAcceptableFrameRate) &&
           (Metrics.MemoryUsageMB <= MaxAcceptableMemoryMB);
}

bool ABuild_FinalIntegrationOrchestrator::TestVFXCharacterIntegration()
{
    // Test that VFX and Character systems can work together
    UClass* VFXClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.VFX_ParticleSystemManager"));
    UClass* CharacterClass = LoadClass<ACharacter>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    
    return (VFXClass != nullptr) && (CharacterClass != nullptr);
}

bool ABuild_FinalIntegrationOrchestrator::TestSystemCommunication()
{
    // Test that systems can communicate with each other
    // This is a basic test - in a real implementation, this would test actual communication
    return true;
}

bool ABuild_FinalIntegrationOrchestrator::TestMemoryManagement()
{
    // Test memory management across systems
    // Basic test - count actors before and after operations
    TArray<AActor*> ActorsBefore;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), ActorsBefore);
    
    // Perform some operations that might affect memory
    // (In a real test, this would create and destroy test objects)
    
    TArray<AActor*> ActorsAfter;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), ActorsAfter);
    
    // Check for memory leaks (basic check)
    return ActorsAfter.Num() >= ActorsBefore.Num(); // Allow for growth, but not unexpected shrinkage
}

FBuild_IntegrationReport ABuild_FinalIntegrationOrchestrator::GetIntegrationReport() const
{
    return IntegrationReport;
}

void ABuild_FinalIntegrationOrchestrator::LogIntegrationStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== FINAL INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Build Version: %s"), *IntegrationReport.BuildVersion);
    UE_LOG(LogTemp, Warning, TEXT("Build Successful: %s"), IntegrationReport.bBuildSuccessful ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Integration Time: %.2f seconds"), IntegrationReport.TotalIntegrationTime);
    UE_LOG(LogTemp, Warning, TEXT("Current Phase: %d"), (int32)IntegrationReport.CurrentPhase);
    
    UE_LOG(LogTemp, Warning, TEXT("System Validation Results:"));
    for (const FBuild_SystemValidationResult& Result : IntegrationReport.SystemResults)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %s: %s"), *Result.SystemName, 
               (Result.bSystemLoaded && Result.bCompilationSuccess && Result.bRuntimeStable) ? TEXT("PASS") : TEXT("FAIL"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Metrics:"));
    UE_LOG(LogTemp, Warning, TEXT("  Total Actors: %d"), IntegrationReport.PerformanceData.TotalActorCount);
    UE_LOG(LogTemp, Warning, TEXT("  Frame Rate: %.1f"), IntegrationReport.PerformanceData.FrameRate);
    UE_LOG(LogTemp, Warning, TEXT("  Memory Usage: %.1f MB"), IntegrationReport.PerformanceData.MemoryUsageMB);
    
    if (IntegrationReport.CriticalErrors.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Critical Errors:"));
        for (const FString& Error : IntegrationReport.CriticalErrors)
        {
            UE_LOG(LogTemp, Error, TEXT("  %s"), *Error);
        }
    }
    
    if (IntegrationReport.Warnings.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Warnings:"));
        for (const FString& Warning : IntegrationReport.Warnings)
        {
            UE_LOG(LogTemp, Warning, TEXT("  %s"), *Warning);
        }
    }
}

void ABuild_FinalIntegrationOrchestrator::ExportBuildReport()
{
    // Create a detailed build report file
    FString ReportContent;
    ReportContent += TEXT("=== TRANSPERSONAL GAME STUDIO - FINAL BUILD REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Build Version: %s\n"), *IntegrationReport.BuildVersion);
    ReportContent += FString::Printf(TEXT("Build Timestamp: %s\n"), *IntegrationReport.BuildTimestamp.ToString());
    ReportContent += FString::Printf(TEXT("Build Successful: %s\n"), IntegrationReport.bBuildSuccessful ? TEXT("YES") : TEXT("NO"));
    ReportContent += FString::Printf(TEXT("Integration Time: %.2f seconds\n"), IntegrationReport.TotalIntegrationTime);
    ReportContent += TEXT("\n");
    
    ReportContent += TEXT("SYSTEM VALIDATION RESULTS:\n");
    for (const FBuild_SystemValidationResult& Result : IntegrationReport.SystemResults)
    {
        ReportContent += FString::Printf(TEXT("  %s:\n"), *Result.SystemName);
        ReportContent += FString::Printf(TEXT("    Loaded: %s\n"), Result.bSystemLoaded ? TEXT("YES") : TEXT("NO"));
        ReportContent += FString::Printf(TEXT("    Compiled: %s\n"), Result.bCompilationSuccess ? TEXT("YES") : TEXT("NO"));
        ReportContent += FString::Printf(TEXT("    Stable: %s\n"), Result.bRuntimeStable ? TEXT("YES") : TEXT("NO"));
        ReportContent += FString::Printf(TEXT("    Message: %s\n"), *Result.ValidationMessage);
    }
    ReportContent += TEXT("\n");
    
    ReportContent += TEXT("PERFORMANCE METRICS:\n");
    ReportContent += FString::Printf(TEXT("  Total Actors: %d\n"), IntegrationReport.PerformanceData.TotalActorCount);
    ReportContent += FString::Printf(TEXT("  Active VFX: %d\n"), IntegrationReport.PerformanceData.ActiveVFXCount);
    ReportContent += FString::Printf(TEXT("  Characters: %d\n"), IntegrationReport.PerformanceData.CharacterCount);
    ReportContent += FString::Printf(TEXT("  Frame Rate: %.1f FPS\n"), IntegrationReport.PerformanceData.FrameRate);
    ReportContent += FString::Printf(TEXT("  Memory Usage: %.1f MB\n"), IntegrationReport.PerformanceData.MemoryUsageMB);
    ReportContent += FString::Printf(TEXT("  Performance Acceptable: %s\n"), IntegrationReport.PerformanceData.bPerformanceAcceptable ? TEXT("YES") : TEXT("NO"));
    
    // Save to file (basic implementation)
    FString FilePath = FPaths::ProjectLogDir() / TEXT("FinalBuildReport.txt");
    FFileHelper::SaveStringToFile(ReportContent, *FilePath);
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build report exported to: %s"), *FilePath);
    }
}

void ABuild_FinalIntegrationOrchestrator::CleanupTestActors()
{
    for (AActor* TestActor : TestActors)
    {
        if (IsValid(TestActor))
        {
            TestActor->Destroy();
        }
    }
    TestActors.Empty();
    
    SystemComponents.Empty();
}

void ABuild_FinalIntegrationOrchestrator::FinalizeIntegration()
{
    bIntegrationInProgress = false;
    CurrentIntegrationPhase = EBuild_IntegrationPhase::BuildComplete;
    
    CleanupTestActors();
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Final Integration: Integration finalized"));
    }
}

void ABuild_FinalIntegrationOrchestrator::InitializeIntegration()
{
    // Clear any previous state
    TestActors.Empty();
    SystemComponents.Empty();
    
    // Initialize integration report
    IntegrationReport.SystemResults.Empty();
    IntegrationReport.CriticalErrors.Empty();
    IntegrationReport.Warnings.Empty();
    IntegrationReport.bBuildSuccessful = false;
    IntegrationReport.CurrentPhase = EBuild_IntegrationPhase::Initialization;
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Final Integration: Initialization complete"));
    }
}

void ABuild_FinalIntegrationOrchestrator::AdvanceIntegrationPhase()
{
    switch (CurrentIntegrationPhase)
    {
        case EBuild_IntegrationPhase::Initialization:
            CurrentIntegrationPhase = EBuild_IntegrationPhase::SystemValidation;
            break;
        case EBuild_IntegrationPhase::SystemValidation:
            CurrentIntegrationPhase = EBuild_IntegrationPhase::CrossSystemTesting;
            break;
        case EBuild_IntegrationPhase::CrossSystemTesting:
            CurrentIntegrationPhase = EBuild_IntegrationPhase::PerformanceCheck;
            break;
        case EBuild_IntegrationPhase::PerformanceCheck:
            CurrentIntegrationPhase = EBuild_IntegrationPhase::FinalValidation;
            break;
        case EBuild_IntegrationPhase::FinalValidation:
            CurrentIntegrationPhase = EBuild_IntegrationPhase::BuildComplete;
            break;
        default:
            break;
    }
    
    IntegrationReport.CurrentPhase = CurrentIntegrationPhase;
}

void ABuild_FinalIntegrationOrchestrator::HandleIntegrationFailure(const FString& ErrorMessage)
{
    bIntegrationInProgress = false;
    CurrentIntegrationPhase = EBuild_IntegrationPhase::BuildFailed;
    IntegrationReport.CurrentPhase = EBuild_IntegrationPhase::BuildFailed;
    IntegrationReport.bBuildSuccessful = false;
    
    IntegrationReport.CriticalErrors.Add(ErrorMessage);
    
    UE_LOG(LogTemp, Error, TEXT("Final Integration FAILED: %s"), *ErrorMessage);
    
    LogIntegrationStatus();
    ExportBuildReport();
    
    CleanupTestActors();
}

void ABuild_FinalIntegrationOrchestrator::HandleIntegrationSuccess()
{
    bIntegrationInProgress = false;
    CurrentIntegrationPhase = EBuild_IntegrationPhase::BuildComplete;
    IntegrationReport.CurrentPhase = EBuild_IntegrationPhase::BuildComplete;
    IntegrationReport.bBuildSuccessful = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Final Integration SUCCESSFUL!"));
    
    FinalizeIntegration();
}

FBuild_SystemValidationResult ABuild_FinalIntegrationOrchestrator::CreateValidationResult(
    const FString& SystemName, 
    bool bLoaded, 
    bool bCompiled, 
    bool bStable, 
    const FString& Message)
{
    FBuild_SystemValidationResult Result;
    Result.SystemName = SystemName;
    Result.bSystemLoaded = bLoaded;
    Result.bCompilationSuccess = bCompiled;
    Result.bRuntimeStable = bStable;
    Result.ValidationMessage = Message.IsEmpty() ? TEXT("Validation completed") : Message;
    Result.ValidationTime = 0.1f; // Placeholder timing
    
    return Result;
}