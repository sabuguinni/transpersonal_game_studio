#include "Build_QAIntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ABuild_QAIntegrationValidator::ABuild_QAIntegrationValidator()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create validator mesh component
    ValidatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ValidatorMesh"));
    RootComponent = ValidatorMesh;

    // Set default mesh (cube)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        ValidatorMesh->SetStaticMesh(CubeMeshAsset.Object);
        ValidatorMesh->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
    }

    // Initialize validation settings
    MaxActorLimit = 200;
    bAutoValidateOnBeginPlay = true;
    ValidationInterval = 30.0f;

    // Initialize validation result
    CurrentValidationResult = FBuild_QAIntegrationResult();
}

void ABuild_QAIntegrationValidator::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoValidateOnBeginPlay)
    {
        // Run initial validation
        ValidateQAIntegration();

        // Set up periodic validation
        if (ValidationInterval > 0.0f)
        {
            GetWorldTimerManager().SetTimer(
                ValidationTimerHandle,
                this,
                &ABuild_QAIntegrationValidator::PerformPeriodicValidation,
                ValidationInterval,
                true
            );
        }
    }
}

FBuild_QAIntegrationResult ABuild_QAIntegrationValidator::ValidateQAIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Integration Validator: Starting validation"));

    // Reset validation result
    CurrentValidationResult = FBuild_QAIntegrationResult();
    CurrentValidationResult.ValidationStatus = EBuild_QAValidationStatus::InProgress;

    // Validate performance limits
    CurrentValidationResult.bPerformanceLimitsOK = ValidatePerformanceLimits();

    // Validate module integration
    CurrentValidationResult.bModuleIntegrationOK = ValidateModuleIntegration();

    // Validate VFX integration
    CurrentValidationResult.bVFXIntegrationOK = ValidateVFXIntegration();

    // Count actors by type
    CurrentValidationResult.QAActorsFound = CountActorsByType(TEXT("QA"));
    CurrentValidationResult.VFXActorsFound = CountActorsByType(TEXT("VFX"));

    // Update overall status
    UpdateValidationStatus();

    // Generate report
    GenerateValidationReport();

    UE_LOG(LogTemp, Warning, TEXT("QA Integration Validator: Validation complete - Status: %d"), 
           (int32)CurrentValidationResult.ValidationStatus);

    return CurrentValidationResult;
}

bool ABuild_QAIntegrationValidator::ValidatePerformanceLimits()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("QA Integration Validator: No world found"));
        return false;
    }

    // Count total actors
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }

    CurrentValidationResult.TotalActorsValidated = ActorCount;

    bool bWithinLimits = ActorCount <= MaxActorLimit;
    
    if (bWithinLimits)
    {
        UE_LOG(LogTemp, Log, TEXT("QA Integration Validator: Performance OK - %d/%d actors"), 
               ActorCount, MaxActorLimit);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("QA Integration Validator: Performance WARNING - %d/%d actors"), 
               ActorCount, MaxActorLimit);
    }

    return bWithinLimits;
}

bool ABuild_QAIntegrationValidator::ValidateModuleIntegration()
{
    // Check if core TranspersonalGame classes are available
    bool bCoreClassesOK = true;

    // Test character class
    UClass* CharacterClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter"));
    if (!CharacterClass)
    {
        UE_LOG(LogTemp, Error, TEXT("QA Integration Validator: TranspersonalCharacter class not found"));
        bCoreClassesOK = false;
    }

    // Test game state class
    UClass* GameStateClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalGameState"));
    if (!GameStateClass)
    {
        UE_LOG(LogTemp, Error, TEXT("QA Integration Validator: TranspersonalGameState class not found"));
        bCoreClassesOK = false;
    }

    // Test QA framework class
    UClass* QAFrameworkClass = FindObject<UClass>(ANY_PACKAGE, TEXT("QA_TestFramework"));
    if (!QAFrameworkClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA Integration Validator: QA_TestFramework class not found"));
        // Not critical for core module integration
    }

    if (bCoreClassesOK)
    {
        UE_LOG(LogTemp, Log, TEXT("QA Integration Validator: Module integration OK"));
    }

    return bCoreClassesOK;
}

bool ABuild_QAIntegrationValidator::ValidateVFXIntegration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    // Count VFX-related actors
    int32 VFXActorCount = 0;
    int32 NiagaraActorCount = 0;

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        FString ActorName = Actor->GetClass()->GetName();
        
        if (ActorName.Contains(TEXT("VFX")))
        {
            VFXActorCount++;
        }
        
        if (ActorName.Contains(TEXT("Niagara")))
        {
            NiagaraActorCount++;
        }
    }

    bool bVFXIntegrationOK = (VFXActorCount > 0 || NiagaraActorCount > 0);

    UE_LOG(LogTemp, Log, TEXT("QA Integration Validator: VFX Integration - %d VFX actors, %d Niagara actors"), 
           VFXActorCount, NiagaraActorCount);

    return bVFXIntegrationOK;
}

void ABuild_QAIntegrationValidator::GenerateValidationReport()
{
    FString Report = TEXT("=== QA Integration Validation Report ===\n");
    
    Report += FString::Printf(TEXT("Total Actors: %d\n"), CurrentValidationResult.TotalActorsValidated);
    Report += FString::Printf(TEXT("QA Actors: %d\n"), CurrentValidationResult.QAActorsFound);
    Report += FString::Printf(TEXT("VFX Actors: %d\n"), CurrentValidationResult.VFXActorsFound);
    Report += FString::Printf(TEXT("Performance Limits: %s\n"), 
                             CurrentValidationResult.bPerformanceLimitsOK ? TEXT("PASS") : TEXT("FAIL"));
    Report += FString::Printf(TEXT("Module Integration: %s\n"), 
                             CurrentValidationResult.bModuleIntegrationOK ? TEXT("PASS") : TEXT("FAIL"));
    Report += FString::Printf(TEXT("VFX Integration: %s\n"), 
                             CurrentValidationResult.bVFXIntegrationOK ? TEXT("PASS") : TEXT("FAIL"));
    
    FString StatusText;
    switch (CurrentValidationResult.ValidationStatus)
    {
        case EBuild_QAValidationStatus::Passed:
            StatusText = TEXT("PASSED");
            break;
        case EBuild_QAValidationStatus::Failed:
            StatusText = TEXT("FAILED");
            break;
        case EBuild_QAValidationStatus::Critical:
            StatusText = TEXT("CRITICAL");
            break;
        default:
            StatusText = TEXT("PENDING");
            break;
    }
    
    Report += FString::Printf(TEXT("Overall Status: %s\n"), *StatusText);
    
    CurrentValidationResult.ValidationReport = Report;
    
    UE_LOG(LogTemp, Warning, TEXT("QA Integration Validation Report:\n%s"), *Report);
}

void ABuild_QAIntegrationValidator::RunFullValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("QA Integration Validator: Running full validation (Editor)"));
    ValidateQAIntegration();
}

bool ABuild_QAIntegrationValidator::IsValidationPassing() const
{
    return CurrentValidationResult.ValidationStatus == EBuild_QAValidationStatus::Passed;
}

void ABuild_QAIntegrationValidator::PerformPeriodicValidation()
{
    UE_LOG(LogTemp, Log, TEXT("QA Integration Validator: Performing periodic validation"));
    ValidateQAIntegration();
}

void ABuild_QAIntegrationValidator::UpdateValidationStatus()
{
    // Determine overall validation status
    if (!CurrentValidationResult.bModuleIntegrationOK)
    {
        CurrentValidationResult.ValidationStatus = EBuild_QAValidationStatus::Critical;
    }
    else if (!CurrentValidationResult.bPerformanceLimitsOK || !CurrentValidationResult.bVFXIntegrationOK)
    {
        CurrentValidationResult.ValidationStatus = EBuild_QAValidationStatus::Failed;
    }
    else
    {
        CurrentValidationResult.ValidationStatus = EBuild_QAValidationStatus::Passed;
    }
}

int32 ABuild_QAIntegrationValidator::CountActorsByType(const FString& TypeFilter)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }

    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        FString ActorName = Actor->GetClass()->GetName();
        
        if (ActorName.Contains(TypeFilter))
        {
            Count++;
        }
    }

    return Count;
}