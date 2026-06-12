#include "Build_IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Engine/Blueprint.h"
#include "TranspersonalGameState.h"
#include "TranspersonalCharacter.h"

UBuild_IntegrationValidator::UBuild_IntegrationValidator()
{
    LastValidationTime = 0.0f;
    TotalTestsRun = 0;
    TestsPassed = 0;
    TestsFailed = 0;
}

void UBuild_IntegrationValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator initialized"));
    
    // Run initial validation
    RunFullValidation();
}

void UBuild_IntegrationValidator::Deinitialize()
{
    ClearValidationResults();
    Super::Deinitialize();
}

void UBuild_IntegrationValidator::RunFullValidation()
{
    UE_LOG(LogTemp, Log, TEXT("Starting full integration validation"));
    
    double StartTime = FPlatformTime::Seconds();
    ClearValidationResults();
    
    // Run all validation tests
    ValidateModuleLoading();
    ValidateClassRegistration();
    ValidateCrossSystemIntegration();
    ValidateLevelIntegration();
    
    LastValidationTime = FPlatformTime::Seconds() - StartTime;
    LogValidationSummary();
}

void UBuild_IntegrationValidator::ValidateModuleLoading()
{
    UE_LOG(LogTemp, Log, TEXT("Validating module loading"));
    
    // Check TranspersonalGame module
    FBuild_ModuleStatus ModuleStatus = CheckModuleStatus(TEXT("TranspersonalGame"));
    ModuleStatuses.Add(ModuleStatus);
    
    if (ModuleStatus.bIsLoaded)
    {
        AddValidationResult(TEXT("Module Loading"), EBuild_ValidationStatus::Passed, 
            FString::Printf(TEXT("TranspersonalGame module loaded with %d classes"), ModuleStatus.ClassCount));
    }
    else
    {
        AddValidationResult(TEXT("Module Loading"), EBuild_ValidationStatus::Failed, 
            TEXT("TranspersonalGame module failed to load"));
    }
}

void UBuild_IntegrationValidator::ValidateClassRegistration()
{
    UE_LOG(LogTemp, Log, TEXT("Validating class registration"));
    
    // Test core classes
    TArray<FString> CoreClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager"),
        TEXT("/Script/TranspersonalGame.CrowdSimulationManager")
    };
    
    int32 PassedClasses = 0;
    for (const FString& ClassName : CoreClasses)
    {
        if (ValidateClassExists(ClassName))
        {
            PassedClasses++;
        }
    }
    
    if (PassedClasses == CoreClasses.Num())
    {
        AddValidationResult(TEXT("Class Registration"), EBuild_ValidationStatus::Passed, 
            FString::Printf(TEXT("All %d core classes registered successfully"), CoreClasses.Num()));
    }
    else
    {
        AddValidationResult(TEXT("Class Registration"), EBuild_ValidationStatus::Failed, 
            FString::Printf(TEXT("Only %d/%d core classes registered"), PassedClasses, CoreClasses.Num()));
    }
}

void UBuild_IntegrationValidator::ValidateCrossSystemIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("Validating cross-system integration"));
    
    // Test GameState integration
    UClass* GameStateClass = LoadClass<AGameStateBase>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
    if (GameStateClass)
    {
        ATranspersonalGameState* GameStateCDO = Cast<ATranspersonalGameState>(GameStateClass->GetDefaultObject());
        if (GameStateCDO)
        {
            AddValidationResult(TEXT("GameState Integration"), EBuild_ValidationStatus::Passed, 
                TEXT("TranspersonalGameState CDO accessible"));
        }
        else
        {
            AddValidationResult(TEXT("GameState Integration"), EBuild_ValidationStatus::Failed, 
                TEXT("TranspersonalGameState CDO not accessible"));
        }
    }
    else
    {
        AddValidationResult(TEXT("GameState Integration"), EBuild_ValidationStatus::Failed, 
            TEXT("TranspersonalGameState class not found"));
    }
    
    // Test Character integration
    UClass* CharacterClass = LoadClass<APawn>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    if (CharacterClass)
    {
        ATranspersonalCharacter* CharacterCDO = Cast<ATranspersonalCharacter>(CharacterClass->GetDefaultObject());
        if (CharacterCDO)
        {
            // Test character components
            TArray<UActorComponent*> Components = CharacterCDO->GetRootComponent()->GetAttachChildren();
            AddValidationResult(TEXT("Character Integration"), EBuild_ValidationStatus::Passed, 
                FString::Printf(TEXT("TranspersonalCharacter has %d components"), Components.Num()));
        }
        else
        {
            AddValidationResult(TEXT("Character Integration"), EBuild_ValidationStatus::Failed, 
                TEXT("TranspersonalCharacter CDO not accessible"));
        }
    }
    else
    {
        AddValidationResult(TEXT("Character Integration"), EBuild_ValidationStatus::Failed, 
            TEXT("TranspersonalCharacter class not found"));
    }
}

void UBuild_IntegrationValidator::ValidateLevelIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("Validating level integration"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationResult(TEXT("Level Integration"), EBuild_ValidationStatus::Failed, 
            TEXT("No world context available"));
        return;
    }
    
    // Count actors in level
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }
    
    if (ActorCount > 0)
    {
        AddValidationResult(TEXT("Level Integration"), EBuild_ValidationStatus::Passed, 
            FString::Printf(TEXT("Level contains %d actors"), ActorCount));
    }
    else
    {
        AddValidationResult(TEXT("Level Integration"), EBuild_ValidationStatus::Warning, 
            TEXT("Level appears to be empty"));
    }
    
    // Test for TranspersonalCharacter instances
    int32 CharacterCount = 0;
    for (TActorIterator<ATranspersonalCharacter> CharacterItr(World); CharacterItr; ++CharacterItr)
    {
        CharacterCount++;
    }
    
    AddValidationResult(TEXT("Character Instances"), 
        CharacterCount > 0 ? EBuild_ValidationStatus::Passed : EBuild_ValidationStatus::Warning, 
        FString::Printf(TEXT("Found %d TranspersonalCharacter instances"), CharacterCount));
}

bool UBuild_IntegrationValidator::IsSystemHealthy() const
{
    if (ValidationResults.Num() == 0)
    {
        return false;
    }
    
    int32 FailedTests = 0;
    for (const FBuild_ValidationResult& Result : ValidationResults)
    {
        if (Result.Status == EBuild_ValidationStatus::Failed)
        {
            FailedTests++;
        }
    }
    
    // System is healthy if less than 25% of tests failed
    return (FailedTests * 4) < ValidationResults.Num();
}

void UBuild_IntegrationValidator::RunEditorValidation()
{
    UE_LOG(LogTemp, Log, TEXT("Running editor validation"));
    RunFullValidation();
}

void UBuild_IntegrationValidator::AddValidationResult(const FString& TestName, EBuild_ValidationStatus Status, const FString& Message)
{
    FBuild_ValidationResult Result;
    Result.TestName = TestName;
    Result.Status = Status;
    Result.Message = Message;
    Result.ExecutionTime = LastValidationTime;
    
    ValidationResults.Add(Result);
    LastValidationResult = Result;
    
    TotalTestsRun++;
    if (Status == EBuild_ValidationStatus::Passed)
    {
        TestsPassed++;
    }
    else if (Status == EBuild_ValidationStatus::Failed)
    {
        TestsFailed++;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Validation: %s - %s: %s"), 
        *TestName, 
        Status == EBuild_ValidationStatus::Passed ? TEXT("PASS") : 
        Status == EBuild_ValidationStatus::Failed ? TEXT("FAIL") : TEXT("WARN"),
        *Message);
}

void UBuild_IntegrationValidator::ClearValidationResults()
{
    ValidationResults.Empty();
    ModuleStatuses.Empty();
    TotalTestsRun = 0;
    TestsPassed = 0;
    TestsFailed = 0;
}

FBuild_ModuleStatus UBuild_IntegrationValidator::CheckModuleStatus(const FString& ModuleName)
{
    FBuild_ModuleStatus Status;
    Status.ModuleName = ModuleName;
    
    // Check if module is loaded
    FModuleManager& ModuleManager = FModuleManager::Get();
    Status.bIsLoaded = ModuleManager.IsModuleLoaded(*ModuleName);
    
    if (Status.bIsLoaded)
    {
        // Count classes in module
        for (TObjectIterator<UClass> ClassItr; ClassItr; ++ClassItr)
        {
            UClass* Class = *ClassItr;
            if (Class && Class->GetOutermost()->GetName().Contains(ModuleName))
            {
                Status.ClassCount++;
                Status.LoadedClasses.Add(Class->GetName());
            }
        }
    }
    
    return Status;
}

bool UBuild_IntegrationValidator::ValidateClassExists(const FString& ClassName)
{
    UClass* Class = LoadClass<UObject>(nullptr, *ClassName);
    return Class != nullptr;
}

void UBuild_IntegrationValidator::LogValidationSummary()
{
    UE_LOG(LogTemp, Log, TEXT("=== VALIDATION SUMMARY ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Tests: %d"), TotalTestsRun);
    UE_LOG(LogTemp, Log, TEXT("Passed: %d"), TestsPassed);
    UE_LOG(LogTemp, Log, TEXT("Failed: %d"), TestsFailed);
    UE_LOG(LogTemp, Log, TEXT("Warnings: %d"), TotalTestsRun - TestsPassed - TestsFailed);
    UE_LOG(LogTemp, Log, TEXT("Execution Time: %.3f seconds"), LastValidationTime);
    UE_LOG(LogTemp, Log, TEXT("System Health: %s"), IsSystemHealthy() ? TEXT("HEALTHY") : TEXT("UNHEALTHY"));
}