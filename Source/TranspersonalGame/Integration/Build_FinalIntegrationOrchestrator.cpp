#include "Build_FinalIntegrationOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "UObject/UObjectIterator.h"
#include "Engine/Blueprint.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/DateTime.h"

DEFINE_LOG_CATEGORY_STATIC(LogBuildFinalIntegration, Log, All);

UBuild_FinalIntegrationOrchestrator::UBuild_FinalIntegrationOrchestrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f;
    
    // Initialize integration state
    IntegrationState = EBuild_IntegrationState::Initializing;
    TotalSystemsToValidate = 0;
    ValidatedSystems = 0;
    CriticalErrorCount = 0;
    WarningCount = 0;
    
    // Set default validation thresholds
    MaxCriticalErrors = 5;
    MaxWarnings = 20;
    ValidationTimeoutSeconds = 300.0f;
    
    bIsIntegrationComplete = false;
    bHasCriticalFailures = false;
    
    LastValidationTime = FDateTime::MinValue();
    IntegrationStartTime = FDateTime::MinValue();
}

void UBuild_FinalIntegrationOrchestrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogBuildFinalIntegration, Log, TEXT("Build_FinalIntegrationOrchestrator: Starting final integration validation"));
    
    // Initialize integration process
    IntegrationStartTime = FDateTime::Now();
    IntegrationState = EBuild_IntegrationState::ValidatingCore;
    
    // Start validation process
    StartFinalIntegrationValidation();
}

void UBuild_FinalIntegrationOrchestrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update integration process
    UpdateIntegrationProcess();
    
    // Check for timeout
    CheckValidationTimeout();
}

void UBuild_FinalIntegrationOrchestrator::StartFinalIntegrationValidation()
{
    UE_LOG(LogBuildFinalIntegration, Log, TEXT("Starting final integration validation process"));
    
    // Clear previous results
    ValidationResults.Empty();
    SystemValidationStatus.Empty();
    CriticalErrorCount = 0;
    WarningCount = 0;
    ValidatedSystems = 0;
    
    // Identify all systems to validate
    IdentifySystemsToValidate();
    
    // Begin core system validation
    ValidateCoreGameSystems();
}

void UBuild_FinalIntegrationOrchestrator::IdentifySystemsToValidate()
{
    SystemsToValidate.Empty();
    
    // Core game systems
    SystemsToValidate.Add(TEXT("TranspersonalGameState"));
    SystemsToValidate.Add(TEXT("TranspersonalCharacter"));
    SystemsToValidate.Add(TEXT("TranspersonalGameMode"));
    
    // World generation systems
    SystemsToValidate.Add(TEXT("PCGWorldGenerator"));
    SystemsToValidate.Add(TEXT("FoliageManager"));
    SystemsToValidate.Add(TEXT("ProceduralWorldManager"));
    
    // Character and animation systems
    SystemsToValidate.Add(TEXT("PrimitiveAnimationController"));
    SystemsToValidate.Add(TEXT("CharacterMovementEnhancer"));
    
    // AI and behavior systems
    SystemsToValidate.Add(TEXT("NPCBehaviorManager"));
    SystemsToValidate.Add(TEXT("CombatAIController"));
    SystemsToValidate.Add(TEXT("CrowdSimulationManager"));
    
    // Quest and narrative systems
    SystemsToValidate.Add(TEXT("QuestManager"));
    SystemsToValidate.Add(TEXT("NarrativeManager"));
    SystemsToValidate.Add(TEXT("DialogueSystem"));
    
    // Audio and VFX systems
    SystemsToValidate.Add(TEXT("AudioManager"));
    SystemsToValidate.Add(TEXT("VFXManager"));
    SystemsToValidate.Add(TEXT("NiagaraVFXLibrary"));
    
    // QA and testing systems
    SystemsToValidate.Add(TEXT("QA_VFXIntegrationValidator"));
    SystemsToValidate.Add(TEXT("QA_CriticalSystemMonitor"));
    SystemsToValidate.Add(TEXT("QA_TestFramework"));
    
    // Build and integration systems
    SystemsToValidate.Add(TEXT("BuildIntegrationManager"));
    SystemsToValidate.Add(TEXT("BuildValidationActor"));
    
    TotalSystemsToValidate = SystemsToValidate.Num();
    
    UE_LOG(LogBuildFinalIntegration, Log, TEXT("Identified %d systems to validate"), TotalSystemsToValidate);
}

void UBuild_FinalIntegrationOrchestrator::ValidateCoreGameSystems()
{
    UE_LOG(LogBuildFinalIntegration, Log, TEXT("Validating core game systems"));
    
    // Validate each core system
    for (const FString& SystemName : SystemsToValidate)
    {
        ValidateIndividualSystem(SystemName);
    }
    
    // Update integration state
    IntegrationState = EBuild_IntegrationState::ValidatingIntegration;
}

void UBuild_FinalIntegrationOrchestrator::ValidateIndividualSystem(const FString& SystemName)
{
    FBuild_SystemValidationResult Result;
    Result.SystemName = SystemName;
    Result.ValidationTime = FDateTime::Now();
    Result.bIsValid = false;
    Result.ErrorCount = 0;
    Result.WarningCount = 0;
    
    // Try to load the class
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *SystemName);
    UClass* SystemClass = LoadClass<UObject>(nullptr, *ClassPath);
    
    if (SystemClass)
    {
        Result.bIsValid = true;
        Result.ValidationMessage = FString::Printf(TEXT("System %s loaded successfully"), *SystemName);
        
        // Additional validation for specific system types
        if (SystemClass->IsChildOf<AActor>())
        {
            // Validate actor-based systems
            ValidateActorSystem(SystemClass, Result);
        }
        else if (SystemClass->IsChildOf<UActorComponent>())
        {
            // Validate component-based systems
            ValidateComponentSystem(SystemClass, Result);
        }
        else if (SystemClass->IsChildOf<UGameInstanceSubsystem>() || SystemClass->IsChildOf<UWorldSubsystem>())
        {
            // Validate subsystem-based systems
            ValidateSubsystemSystem(SystemClass, Result);
        }
        
        ValidatedSystems++;
    }
    else
    {
        Result.bIsValid = false;
        Result.ErrorCount = 1;
        Result.ValidationMessage = FString::Printf(TEXT("Failed to load system %s"), *SystemName);
        CriticalErrorCount++;
    }
    
    // Store result
    ValidationResults.Add(Result);
    SystemValidationStatus.Add(SystemName, Result.bIsValid);
    
    UE_LOG(LogBuildFinalIntegration, Log, TEXT("System %s validation: %s"), 
           *SystemName, Result.bIsValid ? TEXT("PASS") : TEXT("FAIL"));
}

void UBuild_FinalIntegrationOrchestrator::ValidateActorSystem(UClass* ActorClass, FBuild_SystemValidationResult& Result)
{
    // Check if actor can be spawned
    if (UWorld* World = GetWorld())
    {
        // Try to get default object
        AActor* DefaultActor = Cast<AActor>(ActorClass->GetDefaultObject());
        if (DefaultActor)
        {
            Result.ValidationMessage += TEXT(" - Actor CDO valid");
            
            // Check for required components
            TArray<UActorComponent*> Components = DefaultActor->GetRootComponent() ? 
                DefaultActor->GetRootComponent()->GetAttachChildren() : TArray<UActorComponent*>();
            
            if (Components.Num() > 0)
            {
                Result.ValidationMessage += FString::Printf(TEXT(" - %d components found"), Components.Num());
            }
        }
        else
        {
            Result.WarningCount++;
            Result.ValidationMessage += TEXT(" - Warning: Actor CDO invalid");
        }
    }
}

void UBuild_FinalIntegrationOrchestrator::ValidateComponentSystem(UClass* ComponentClass, FBuild_SystemValidationResult& Result)
{
    // Check component default object
    UActorComponent* DefaultComponent = Cast<UActorComponent>(ComponentClass->GetDefaultObject());
    if (DefaultComponent)
    {
        Result.ValidationMessage += TEXT(" - Component CDO valid");
        
        // Check if component can tick
        if (DefaultComponent->PrimaryComponentTick.bCanEverTick)
        {
            Result.ValidationMessage += TEXT(" - Tick enabled");
        }
    }
    else
    {
        Result.WarningCount++;
        Result.ValidationMessage += TEXT(" - Warning: Component CDO invalid");
    }
}

void UBuild_FinalIntegrationOrchestrator::ValidateSubsystemSystem(UClass* SubsystemClass, FBuild_SystemValidationResult& Result)
{
    // Check subsystem default object
    USubsystem* DefaultSubsystem = Cast<USubsystem>(SubsystemClass->GetDefaultObject());
    if (DefaultSubsystem)
    {
        Result.ValidationMessage += TEXT(" - Subsystem CDO valid");
        
        // Try to get subsystem instance if world exists
        if (UWorld* World = GetWorld())
        {
            if (SubsystemClass->IsChildOf<UWorldSubsystem>())
            {
                UWorldSubsystem* WorldSubsystem = World->GetSubsystem(SubsystemClass->StaticClass());
                if (WorldSubsystem)
                {
                    Result.ValidationMessage += TEXT(" - World subsystem instance found");
                }
            }
        }
    }
    else
    {
        Result.WarningCount++;
        Result.ValidationMessage += TEXT(" - Warning: Subsystem CDO invalid");
    }
}

void UBuild_FinalIntegrationOrchestrator::UpdateIntegrationProcess()
{
    // Update validation progress
    float ValidationProgress = TotalSystemsToValidate > 0 ? 
        (float)ValidatedSystems / (float)TotalSystemsToValidate : 0.0f;
    
    // Check if validation is complete
    if (ValidationProgress >= 1.0f && IntegrationState == EBuild_IntegrationState::ValidatingIntegration)
    {
        CompleteIntegrationValidation();
    }
    
    // Update warning count
    WarningCount = 0;
    for (const FBuild_SystemValidationResult& Result : ValidationResults)
    {
        WarningCount += Result.WarningCount;
    }
    
    // Check for critical failures
    bHasCriticalFailures = (CriticalErrorCount > MaxCriticalErrors) || (WarningCount > MaxWarnings);
}

void UBuild_FinalIntegrationOrchestrator::CompleteIntegrationValidation()
{
    UE_LOG(LogBuildFinalIntegration, Log, TEXT("Completing final integration validation"));
    
    IntegrationState = EBuild_IntegrationState::Complete;
    bIsIntegrationComplete = true;
    LastValidationTime = FDateTime::Now();
    
    // Generate final report
    GenerateFinalIntegrationReport();
    
    // Broadcast completion
    OnIntegrationComplete.Broadcast(bHasCriticalFailures);
}

void UBuild_FinalIntegrationOrchestrator::GenerateFinalIntegrationReport()
{
    FString Report = TEXT("=== FINAL INTEGRATION VALIDATION REPORT ===\n");
    Report += FString::Printf(TEXT("Validation Time: %s\n"), *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Total Systems: %d\n"), TotalSystemsToValidate);
    Report += FString::Printf(TEXT("Validated Systems: %d\n"), ValidatedSystems);
    Report += FString::Printf(TEXT("Critical Errors: %d\n"), CriticalErrorCount);
    Report += FString::Printf(TEXT("Warnings: %d\n"), WarningCount);
    Report += FString::Printf(TEXT("Success Rate: %.1f%%\n"), 
                             TotalSystemsToValidate > 0 ? (float)ValidatedSystems / TotalSystemsToValidate * 100.0f : 0.0f);
    
    Report += TEXT("\n=== SYSTEM VALIDATION DETAILS ===\n");
    for (const FBuild_SystemValidationResult& Result : ValidationResults)
    {
        Report += FString::Printf(TEXT("%s: %s - %s\n"), 
                                 *Result.SystemName,
                                 Result.bIsValid ? TEXT("PASS") : TEXT("FAIL"),
                                 *Result.ValidationMessage);
    }
    
    Report += TEXT("\n=== INTEGRATION STATUS ===\n");
    if (bHasCriticalFailures)
    {
        Report += TEXT("STATUS: CRITICAL FAILURES DETECTED\n");
        Report += TEXT("ACTION REQUIRED: Fix critical errors before deployment\n");
    }
    else
    {
        Report += TEXT("STATUS: INTEGRATION SUCCESSFUL\n");
        Report += TEXT("READY FOR: Next development cycle\n");
    }
    
    FinalIntegrationReport = Report;
    
    UE_LOG(LogBuildFinalIntegration, Log, TEXT("Final Integration Report:\n%s"), *Report);
}

void UBuild_FinalIntegrationOrchestrator::CheckValidationTimeout()
{
    if (IntegrationStartTime != FDateTime::MinValue())
    {
        FTimespan ElapsedTime = FDateTime::Now() - IntegrationStartTime;
        if (ElapsedTime.GetTotalSeconds() > ValidationTimeoutSeconds)
        {
            UE_LOG(LogBuildFinalIntegration, Warning, TEXT("Integration validation timeout reached"));
            
            IntegrationState = EBuild_IntegrationState::Failed;
            bHasCriticalFailures = true;
            
            CompleteIntegrationValidation();
        }
    }
}

FString UBuild_FinalIntegrationOrchestrator::GetIntegrationStatusString() const
{
    switch (IntegrationState)
    {
        case EBuild_IntegrationState::Initializing:
            return TEXT("Initializing");
        case EBuild_IntegrationState::ValidatingCore:
            return TEXT("Validating Core Systems");
        case EBuild_IntegrationState::ValidatingIntegration:
            return TEXT("Validating Integration");
        case EBuild_IntegrationState::Complete:
            return TEXT("Complete");
        case EBuild_IntegrationState::Failed:
            return TEXT("Failed");
        default:
            return TEXT("Unknown");
    }
}

float UBuild_FinalIntegrationOrchestrator::GetValidationProgress() const
{
    return TotalSystemsToValidate > 0 ? (float)ValidatedSystems / (float)TotalSystemsToValidate : 0.0f;
}

bool UBuild_FinalIntegrationOrchestrator::IsSystemValidated(const FString& SystemName) const
{
    const bool* ValidationStatus = SystemValidationStatus.Find(SystemName);
    return ValidationStatus ? *ValidationStatus : false;
}

TArray<FString> UBuild_FinalIntegrationOrchestrator::GetFailedSystems() const
{
    TArray<FString> FailedSystems;
    
    for (const auto& StatusPair : SystemValidationStatus)
    {
        if (!StatusPair.Value)
        {
            FailedSystems.Add(StatusPair.Key);
        }
    }
    
    return FailedSystems;
}