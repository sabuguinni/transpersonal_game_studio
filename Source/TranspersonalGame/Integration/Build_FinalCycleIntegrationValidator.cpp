#include "Build_FinalCycleIntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Kismet/GameplayStatics.h"

UBuild_FinalCycleIntegrationValidator::UBuild_FinalCycleIntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    bAutoValidateOnBeginPlay = true;
    MaxActorCountLimit = 100;
    ValidationInterval = 30.0f;

    // Initialize default values
    QAResults.bVFXFrameworkLoaded = false;
    QAResults.bSystemValidatorActive = false;
    QAResults.bBuildValidationComplete = false;
    QAResults.TotalTestsPassed = 0;
    QAResults.TotalTestsFailed = 0;

    ModuleStatus.CoreModulesLoaded = 0;
    ModuleStatus.IntegrationModulesLoaded = 0;
    ModuleStatus.QAModulesLoaded = 0;
    ModuleStatus.TotalActorsInLevel = 0;
    ModuleStatus.OverallStatus = EBuild_IntegrationStatus::Unknown;
}

void UBuild_FinalCycleIntegrationValidator::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoValidateOnBeginPlay)
    {
        RunFullValidationSuite();
    }

    // Set up periodic validation
    if (ValidationInterval > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            ValidationTimerHandle,
            this,
            &UBuild_FinalCycleIntegrationValidator::RunFullValidationSuite,
            ValidationInterval,
            true
        );
    }
}

bool UBuild_FinalCycleIntegrationValidator::ValidateQAIntegration()
{
    QAResults.TotalTestsPassed = 0;
    QAResults.TotalTestsFailed = 0;

    // Test VFX Framework
    UClass* VFXFrameworkClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Script/TranspersonalGame.QA_VFXTestFramework"));
    if (VFXFrameworkClass)
    {
        QAResults.bVFXFrameworkLoaded = true;
        QAResults.TotalTestsPassed++;
        UE_LOG(LogTemp, Log, TEXT("QA VFX Framework: LOADED"));
    }
    else
    {
        QAResults.bVFXFrameworkLoaded = false;
        QAResults.TotalTestsFailed++;
        UE_LOG(LogTemp, Warning, TEXT("QA VFX Framework: NOT FOUND"));
    }

    // Test System Validator
    UClass* ValidatorClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Script/TranspersonalGame.QA_SystemValidator"));
    if (ValidatorClass)
    {
        QAResults.bSystemValidatorActive = true;
        QAResults.TotalTestsPassed++;
        UE_LOG(LogTemp, Log, TEXT("QA System Validator: LOADED"));
    }
    else
    {
        QAResults.bSystemValidatorActive = false;
        QAResults.TotalTestsFailed++;
        UE_LOG(LogTemp, Warning, TEXT("QA System Validator: NOT FOUND"));
    }

    UpdateValidationTimestamp();
    QAResults.bBuildValidationComplete = true;

    return QAResults.TotalTestsFailed == 0;
}

bool UBuild_FinalCycleIntegrationValidator::ValidateModuleStatus()
{
    ModuleStatus.CoreModulesLoaded = 0;
    ModuleStatus.IntegrationModulesLoaded = 0;
    ModuleStatus.QAModulesLoaded = 0;

    // Validate core modules
    if (ValidateCoreModules())
    {
        ModuleStatus.CoreModulesLoaded++;
    }

    // Validate integration modules
    if (ValidateIntegrationModules())
    {
        ModuleStatus.IntegrationModulesLoaded++;
    }

    // Validate QA modules
    if (ValidateQAModules())
    {
        ModuleStatus.QAModulesLoaded++;
    }

    // Count actors in level
    if (GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
        ModuleStatus.TotalActorsInLevel = AllActors.Num();
    }

    // Determine overall status
    if (ModuleStatus.CoreModulesLoaded > 0 && ModuleStatus.IntegrationModulesLoaded > 0 && ModuleStatus.QAModulesLoaded > 0)
    {
        ModuleStatus.OverallStatus = EBuild_IntegrationStatus::Pass;
    }
    else if (ModuleStatus.CoreModulesLoaded > 0)
    {
        ModuleStatus.OverallStatus = EBuild_IntegrationStatus::Warning;
    }
    else
    {
        ModuleStatus.OverallStatus = EBuild_IntegrationStatus::Fail;
    }

    return ModuleStatus.OverallStatus == EBuild_IntegrationStatus::Pass;
}

bool UBuild_FinalCycleIntegrationValidator::ValidateVFXSystemIntegration()
{
    bool bVFXIntegrationValid = true;

    // Check if VFX framework is loaded
    if (!QAResults.bVFXFrameworkLoaded)
    {
        UE_LOG(LogTemp, Error, TEXT("VFX Integration FAIL: VFX Framework not loaded"));
        bVFXIntegrationValid = false;
    }

    // Check if QA modules are sufficient
    if (ModuleStatus.QAModulesLoaded < 1)
    {
        UE_LOG(LogTemp, Error, TEXT("VFX Integration FAIL: QA modules incomplete"));
        bVFXIntegrationValid = false;
    }

    // Check if core modules are sufficient
    if (ModuleStatus.CoreModulesLoaded < 1)
    {
        UE_LOG(LogTemp, Error, TEXT("VFX Integration FAIL: Core modules incomplete"));
        bVFXIntegrationValid = false;
    }

    if (bVFXIntegrationValid)
    {
        UE_LOG(LogTemp, Log, TEXT("VFX Integration Status: PASS"));
    }

    return bVFXIntegrationValid;
}

EBuild_IntegrationStatus UBuild_FinalCycleIntegrationValidator::GetOverallIntegrationStatus() const
{
    return ModuleStatus.OverallStatus;
}

void UBuild_FinalCycleIntegrationValidator::RunFullValidationSuite()
{
    UE_LOG(LogTemp, Log, TEXT("Running full validation suite..."));

    ModuleStatus.OverallStatus = EBuild_IntegrationStatus::Validating;

    bool bQAValid = ValidateQAIntegration();
    bool bModulesValid = ValidateModuleStatus();
    bool bVFXValid = ValidateVFXSystemIntegration();
    bool bActorCountValid = CheckActorCountLimits();

    if (bQAValid && bModulesValid && bVFXValid && bActorCountValid)
    {
        ModuleStatus.OverallStatus = EBuild_IntegrationStatus::Pass;
        UE_LOG(LogTemp, Log, TEXT("Full validation suite: PASS"));
    }
    else if (bModulesValid && bActorCountValid)
    {
        ModuleStatus.OverallStatus = EBuild_IntegrationStatus::Warning;
        UE_LOG(LogTemp, Warning, TEXT("Full validation suite: WARNING"));
    }
    else
    {
        ModuleStatus.OverallStatus = EBuild_IntegrationStatus::Fail;
        UE_LOG(LogTemp, Error, TEXT("Full validation suite: FAIL"));
    }

    GenerateIntegrationReport();
}

bool UBuild_FinalCycleIntegrationValidator::CheckActorCountLimits()
{
    if (ModuleStatus.TotalActorsInLevel > MaxActorCountLimit)
    {
        UE_LOG(LogTemp, Warning, TEXT("Actor count %d exceeds recommended limit of %d"), 
               ModuleStatus.TotalActorsInLevel, MaxActorCountLimit);
        return false;
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Actor count %d within limits"), ModuleStatus.TotalActorsInLevel);
        return true;
    }
}

void UBuild_FinalCycleIntegrationValidator::GenerateIntegrationReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== INTEGRATION VALIDATION REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("QA Integration: VFX Framework=%s, System Validator=%s"), 
           QAResults.bVFXFrameworkLoaded ? TEXT("LOADED") : TEXT("MISSING"),
           QAResults.bSystemValidatorActive ? TEXT("ACTIVE") : TEXT("INACTIVE"));
    UE_LOG(LogTemp, Log, TEXT("Module Status: Core=%d, Integration=%d, QA=%d"), 
           ModuleStatus.CoreModulesLoaded, ModuleStatus.IntegrationModulesLoaded, ModuleStatus.QAModulesLoaded);
    UE_LOG(LogTemp, Log, TEXT("Actor Count: %d/%d"), ModuleStatus.TotalActorsInLevel, MaxActorCountLimit);
    UE_LOG(LogTemp, Log, TEXT("Overall Status: %s"), 
           ModuleStatus.OverallStatus == EBuild_IntegrationStatus::Pass ? TEXT("PASS") :
           ModuleStatus.OverallStatus == EBuild_IntegrationStatus::Warning ? TEXT("WARNING") :
           ModuleStatus.OverallStatus == EBuild_IntegrationStatus::Fail ? TEXT("FAIL") : TEXT("UNKNOWN"));
    UE_LOG(LogTemp, Log, TEXT("Validation Timestamp: %s"), *QAResults.LastValidationTimestamp);
    UE_LOG(LogTemp, Log, TEXT("=== END REPORT ==="));
}

bool UBuild_FinalCycleIntegrationValidator::ValidateCoreModules()
{
    TArray<FString> CoreModules = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager")
    };

    int32 LoadedCount = 0;
    for (const FString& ModulePath : CoreModules)
    {
        UClass* ModuleClass = StaticLoadClass(UObject::StaticClass(), nullptr, *ModulePath);
        if (ModuleClass)
        {
            LoadedCount++;
        }
    }

    ModuleStatus.CoreModulesLoaded = LoadedCount;
    return LoadedCount >= 3; // At least 3 out of 4 core modules should be loaded
}

bool UBuild_FinalCycleIntegrationValidator::ValidateIntegrationModules()
{
    TArray<FString> IntegrationModules = {
        TEXT("/Script/TranspersonalGame.BuildIntegrationManager"),
        TEXT("/Script/TranspersonalGame.BuildValidationManager"),
        TEXT("/Script/TranspersonalGame.BuildValidator")
    };

    int32 LoadedCount = 0;
    for (const FString& ModulePath : IntegrationModules)
    {
        UClass* ModuleClass = StaticLoadClass(UObject::StaticClass(), nullptr, *ModulePath);
        if (ModuleClass)
        {
            LoadedCount++;
        }
    }

    ModuleStatus.IntegrationModulesLoaded = LoadedCount;
    return LoadedCount >= 2; // At least 2 out of 3 integration modules should be loaded
}

bool UBuild_FinalCycleIntegrationValidator::ValidateQAModules()
{
    TArray<FString> QAModules = {
        TEXT("/Script/TranspersonalGame.QA_VFXTestFramework"),
        TEXT("/Script/TranspersonalGame.QA_SystemValidator")
    };

    int32 LoadedCount = 0;
    for (const FString& ModulePath : QAModules)
    {
        UClass* ModuleClass = StaticLoadClass(UObject::StaticClass(), nullptr, *ModulePath);
        if (ModuleClass)
        {
            LoadedCount++;
        }
    }

    ModuleStatus.QAModulesLoaded = LoadedCount;
    return LoadedCount >= 1; // At least 1 out of 2 QA modules should be loaded
}

void UBuild_FinalCycleIntegrationValidator::UpdateValidationTimestamp()
{
    FDateTime Now = FDateTime::Now();
    QAResults.LastValidationTimestamp = Now.ToString(TEXT("%Y-%m-%d %H:%M:%S"));
}