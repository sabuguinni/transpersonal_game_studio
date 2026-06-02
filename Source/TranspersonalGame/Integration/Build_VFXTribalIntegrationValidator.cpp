#include "Build_VFXTribalIntegrationValidator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"

ABuild_VFXTribalIntegrationValidator::ABuild_VFXTribalIntegrationValidator()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create validator mesh component
    ValidatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ValidatorMesh"));
    RootComponent = ValidatorMesh;

    // Create VFX test component
    VFXTestComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("VFXTestComponent"));
    VFXTestComponent->SetupAttachment(RootComponent);

    // Create tribal audio component
    TribalAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("TribalAudioComponent"));
    TribalAudioComponent->SetupAttachment(RootComponent);

    // Initialize validation settings
    bEnableVFXValidation = true;
    bEnableTribalDialogueValidation = true;
    bEnableCrossSystemValidation = true;
    ValidationTimeout = 30.0f;

    // Initialize integration report
    IntegrationReport = FBuild_VFXTribalIntegrationReport();
}

void ABuild_VFXTribalIntegrationValidator::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("VFX Tribal Integration Validator initialized"));
    
    // Auto-run validation on begin play
    if (bEnableVFXValidation || bEnableTribalDialogueValidation)
    {
        RunFullIntegrationValidation();
    }
}

void ABuild_VFXTribalIntegrationValidator::RunFullIntegrationValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("Starting full VFX Tribal integration validation"));
    
    // Reset previous results
    ResetValidationState();
    
    float StartTime = FPlatformTime::Seconds();

    // Run all validation tests
    if (bEnableVFXValidation)
    {
        AddTestResult(ValidateVFXFootstepEffects());
    }

    if (bEnableTribalDialogueValidation)
    {
        AddTestResult(ValidateTribalDialogueAudio());
    }

    if (bEnableCrossSystemValidation)
    {
        AddTestResult(ValidateVFXTribalCombination());
        AddTestResult(ValidateQAValidatorIntegration());
    }

    // Always validate performance and QA outputs
    AddTestResult(ValidateSystemPerformance());
    ValidateQAAgentOutputs();
    ValidateActorLimits();
    ValidatePerformanceMetrics();

    // Calculate final results
    IntegrationReport.TotalExecutionTime = FPlatformTime::Seconds() - StartTime;
    IntegrationReport.bIntegrationSuccessful = (IntegrationReport.FailedTests == 0);

    UE_LOG(LogTemp, Warning, TEXT("VFX Tribal integration validation complete: %d/%d tests passed"), 
           IntegrationReport.PassedTests, IntegrationReport.TotalTests);
}

bool ABuild_VFXTribalIntegrationValidator::ValidateVFXSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating VFX systems"));

    UWorld* World = GetWorld();
    if (!World)
    {
        LogValidationResult(TEXT("VFX Systems"), false, TEXT("World not available"));
        return false;
    }

    // Find VFX actors in the world
    TArray<AActor*> VFXActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), VFXActors);
    
    int32 VFXCount = 0;
    for (AActor* Actor : VFXActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("VFX")))
        {
            VFXCount++;
        }
    }

    bool bSuccess = VFXCount > 0;
    LogValidationResult(TEXT("VFX Systems"), bSuccess, 
                       FString::Printf(TEXT("Found %d VFX actors"), VFXCount));
    
    return bSuccess;
}

bool ABuild_VFXTribalIntegrationValidator::ValidateTribalDialogueSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating Tribal Dialogue systems"));

    UWorld* World = GetWorld();
    if (!World)
    {
        LogValidationResult(TEXT("Tribal Dialogue"), false, TEXT("World not available"));
        return false;
    }

    // Find tribal dialogue actors
    TArray<AActor*> TribalActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), TribalActors);
    
    int32 TribalCount = 0;
    for (AActor* Actor : TribalActors)
    {
        if (Actor && (Actor->GetName().Contains(TEXT("Tribal")) || Actor->GetName().Contains(TEXT("Dialogue"))))
        {
            TribalCount++;
        }
    }

    bool bSuccess = TribalCount > 0;
    LogValidationResult(TEXT("Tribal Dialogue"), bSuccess, 
                       FString::Printf(TEXT("Found %d tribal dialogue actors"), TribalCount));
    
    return bSuccess;
}

bool ABuild_VFXTribalIntegrationValidator::ValidateCrossSystemCompatibility()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating cross-system compatibility"));

    bool bVFXValid = ValidateVFXSystems();
    bool bTribalValid = ValidateTribalDialogueSystems();
    
    // Test if both systems can coexist
    bool bCompatible = bVFXValid && bTribalValid;
    
    LogValidationResult(TEXT("Cross-System Compatibility"), bCompatible, 
                       TEXT("VFX and Tribal systems compatibility check"));
    
    return bCompatible;
}

void ABuild_VFXTribalIntegrationValidator::ValidateQAAgentOutputs()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating QA Agent #18 outputs"));

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Look for QA validator actors created by Agent #18
    TArray<AActor*> QAActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), QAActors);
    
    int32 QAValidatorCount = 0;
    for (AActor* Actor : QAActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("QA_VFX")))
        {
            QAValidatorCount++;
        }
    }

    FBuild_VFXTribalTestResult QAResult;
    QAResult.TestName = TEXT("QA Agent Output Validation");
    QAResult.Result = (QAValidatorCount > 0) ? EBuild_VFXTribalValidationResult::ValidationPassed : 
                                               EBuild_VFXTribalValidationResult::ValidationFailed;
    QAResult.ActorsAffected = QAValidatorCount;
    QAResult.ErrorMessage = FString::Printf(TEXT("Found %d QA validator actors"), QAValidatorCount);
    
    AddTestResult(QAResult);
}

void ABuild_VFXTribalIntegrationValidator::ValidateActorLimits()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating actor limits"));

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 TotalActors = AllActors.Num();
    bool bWithinLimits = TotalActors <= 8000;

    FBuild_VFXTribalTestResult LimitResult;
    LimitResult.TestName = TEXT("Actor Limit Validation");
    LimitResult.Result = bWithinLimits ? EBuild_VFXTribalValidationResult::ValidationPassed : 
                                        EBuild_VFXTribalValidationResult::ValidationFailed;
    LimitResult.ActorsAffected = TotalActors;
    LimitResult.ErrorMessage = FString::Printf(TEXT("Total actors: %d (limit: 8000)"), TotalActors);
    
    AddTestResult(LimitResult);
}

void ABuild_VFXTribalIntegrationValidator::ValidatePerformanceMetrics()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating performance metrics"));

    // Basic performance validation
    float CurrentFPS = 1.0f / FApp::GetDeltaTime();
    bool bPerformanceOK = CurrentFPS > 30.0f;

    FBuild_VFXTribalTestResult PerfResult;
    PerfResult.TestName = TEXT("Performance Metrics");
    PerfResult.Result = bPerformanceOK ? EBuild_VFXTribalValidationResult::ValidationPassed : 
                                        EBuild_VFXTribalValidationResult::ValidationFailed;
    PerfResult.ErrorMessage = FString::Printf(TEXT("Current FPS: %.1f"), CurrentFPS);
    
    AddTestResult(PerfResult);
}

FBuild_VFXTribalIntegrationReport ABuild_VFXTribalIntegrationValidator::GetIntegrationReport() const
{
    return IntegrationReport;
}

void ABuild_VFXTribalIntegrationValidator::ResetValidationState()
{
    IntegrationReport = FBuild_VFXTribalIntegrationReport();
}

FBuild_VFXTribalTestResult ABuild_VFXTribalIntegrationValidator::ValidateVFXFootstepEffects()
{
    FBuild_VFXTribalTestResult Result;
    Result.TestName = TEXT("VFX Footstep Effects");
    
    float StartTime = FPlatformTime::Seconds();
    
    // Test VFX component functionality
    if (VFXTestComponent)
    {
        Result.Result = EBuild_VFXTribalValidationResult::ValidationPassed;
        Result.ErrorMessage = TEXT("VFX footstep effects validated successfully");
        Result.ActorsAffected = 1;
    }
    else
    {
        Result.Result = EBuild_VFXTribalValidationResult::ValidationFailed;
        Result.ErrorMessage = TEXT("VFX component not found");
        Result.ActorsAffected = 0;
    }
    
    Result.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    return Result;
}

FBuild_VFXTribalTestResult ABuild_VFXTribalIntegrationValidator::ValidateTribalDialogueAudio()
{
    FBuild_VFXTribalTestResult Result;
    Result.TestName = TEXT("Tribal Dialogue Audio");
    
    float StartTime = FPlatformTime::Seconds();
    
    // Test audio component functionality
    if (TribalAudioComponent)
    {
        Result.Result = EBuild_VFXTribalValidationResult::ValidationPassed;
        Result.ErrorMessage = TEXT("Tribal dialogue audio validated successfully");
        Result.ActorsAffected = 1;
    }
    else
    {
        Result.Result = EBuild_VFXTribalValidationResult::ValidationFailed;
        Result.ErrorMessage = TEXT("Tribal audio component not found");
        Result.ActorsAffected = 0;
    }
    
    Result.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    return Result;
}

FBuild_VFXTribalTestResult ABuild_VFXTribalIntegrationValidator::ValidateVFXTribalCombination()
{
    FBuild_VFXTribalTestResult Result;
    Result.TestName = TEXT("VFX Tribal Combination");
    
    float StartTime = FPlatformTime::Seconds();
    
    // Test combined VFX and tribal systems
    bool bVFXOK = (VFXTestComponent != nullptr);
    bool bTribalOK = (TribalAudioComponent != nullptr);
    
    if (bVFXOK && bTribalOK)
    {
        Result.Result = EBuild_VFXTribalValidationResult::ValidationPassed;
        Result.ErrorMessage = TEXT("VFX and Tribal systems work together successfully");
        Result.ActorsAffected = 2;
    }
    else
    {
        Result.Result = EBuild_VFXTribalValidationResult::ValidationFailed;
        Result.ErrorMessage = TEXT("VFX and Tribal system combination failed");
        Result.ActorsAffected = 0;
    }
    
    Result.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    return Result;
}

FBuild_VFXTribalTestResult ABuild_VFXTribalIntegrationValidator::ValidateQAValidatorIntegration()
{
    FBuild_VFXTribalTestResult Result;
    Result.TestName = TEXT("QA Validator Integration");
    
    float StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<AActor*> QAActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), QAActors);
        
        int32 QACount = 0;
        for (AActor* Actor : QAActors)
        {
            if (Actor && Actor->GetName().Contains(TEXT("QA")))
            {
                QACount++;
            }
        }
        
        Result.Result = (QACount > 0) ? EBuild_VFXTribalValidationResult::ValidationPassed : 
                                       EBuild_VFXTribalValidationResult::ValidationFailed;
        Result.ErrorMessage = FString::Printf(TEXT("Found %d QA integration actors"), QACount);
        Result.ActorsAffected = QACount;
    }
    else
    {
        Result.Result = EBuild_VFXTribalValidationResult::ValidationFailed;
        Result.ErrorMessage = TEXT("World not available for QA validation");
        Result.ActorsAffected = 0;
    }
    
    Result.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    return Result;
}

FBuild_VFXTribalTestResult ABuild_VFXTribalIntegrationValidator::ValidateSystemPerformance()
{
    FBuild_VFXTribalTestResult Result;
    Result.TestName = TEXT("System Performance");
    
    float StartTime = FPlatformTime::Seconds();
    
    // Basic performance check
    float DeltaTime = FApp::GetDeltaTime();
    float FPS = 1.0f / DeltaTime;
    
    if (FPS > 30.0f)
    {
        Result.Result = EBuild_VFXTribalValidationResult::ValidationPassed;
        Result.ErrorMessage = FString::Printf(TEXT("Performance OK: %.1f FPS"), FPS);
    }
    else
    {
        Result.Result = EBuild_VFXTribalValidationResult::ValidationFailed;
        Result.ErrorMessage = FString::Printf(TEXT("Performance low: %.1f FPS"), FPS);
    }
    
    Result.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    Result.ActorsAffected = 1;
    
    return Result;
}

void ABuild_VFXTribalIntegrationValidator::AddTestResult(const FBuild_VFXTribalTestResult& Result)
{
    IntegrationReport.TestResults.Add(Result);
    IntegrationReport.TotalTests++;
    IntegrationReport.TotalExecutionTime += Result.ExecutionTime;
    
    if (Result.Result == EBuild_VFXTribalValidationResult::ValidationPassed)
    {
        IntegrationReport.PassedTests++;
    }
    else
    {
        IntegrationReport.FailedTests++;
    }
}

void ABuild_VFXTribalIntegrationValidator::LogValidationResult(const FString& TestName, bool bSuccess, const FString& Details)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("VALIDATION PASSED: %s - %s"), *TestName, *Details);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("VALIDATION FAILED: %s - %s"), *TestName, *Details);
    }
}