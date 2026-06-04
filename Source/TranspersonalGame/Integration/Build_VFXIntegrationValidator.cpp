#include "Build_VFXIntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h"

ABuild_VFXIntegrationValidator::ABuild_VFXIntegrationValidator()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create validator mesh component
    ValidatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ValidatorMesh"));
    RootComponent = ValidatorMesh;

    // Create test VFX component
    TestVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TestVFXComponent"));
    TestVFXComponent->SetupAttachment(RootComponent);

    // Set default values
    bAutoValidateOnBeginPlay = true;
    ValidationInterval = 5.0f;
}

void ABuild_VFXIntegrationValidator::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoValidateOnBeginPlay)
    {
        // Run initial validation
        ValidateVFXIntegration();

        // Set up periodic validation
        if (ValidationInterval > 0.0f)
        {
            GetWorldTimerManager().SetTimer(
                ValidationTimerHandle,
                this,
                &ABuild_VFXIntegrationValidator::PerformPeriodicValidation,
                ValidationInterval,
                true
            );
        }
    }
}

FBuild_VFXValidationReport ABuild_VFXIntegrationValidator::ValidateVFXIntegration()
{
    FBuild_VFXValidationReport Report;
    Report.ValidationTimestamp = GetWorld()->GetTimeSeconds();
    Report.ValidationErrors.Empty();

    UE_LOG(LogTemp, Log, TEXT("Build VFX Integration Validator: Starting validation"));

    // Find all VFX actors in the world
    TArray<AActor*> VFXActors = FindAllVFXActors();
    Report.TotalVFXActors = VFXActors.Num();

    // Validate each VFX actor
    int32 ValidCount = 0;
    int32 InvalidCount = 0;

    for (AActor* Actor : VFXActors)
    {
        if (ValidateVFXActor(Actor))
        {
            ValidCount++;
        }
        else
        {
            InvalidCount++;
            Report.ValidationErrors.Add(FString::Printf(TEXT("Invalid VFX Actor: %s"), *Actor->GetName()));
        }
    }

    Report.ValidVFXActors = ValidCount;
    Report.InvalidVFXActors = InvalidCount;

    // Test VFX performance
    bool PerformanceOK = TestVFXPerformance();
    if (!PerformanceOK)
    {
        Report.ValidationErrors.Add(TEXT("VFX Performance test failed"));
    }

    // Determine overall result
    if (Report.ValidationErrors.Num() == 0)
    {
        Report.OverallResult = EBuild_VFXValidationResult::Passed;
    }
    else if (Report.InvalidVFXActors == 0 && !PerformanceOK)
    {
        Report.OverallResult = EBuild_VFXValidationResult::Warning;
    }
    else
    {
        Report.OverallResult = EBuild_VFXValidationResult::Failed;
    }

    LastValidationReport = Report;
    LogValidationResult(Report);

    return Report;
}

bool ABuild_VFXIntegrationValidator::ValidateNiagaraSystem(UNiagaraSystem* NiagaraSystem)
{
    if (!NiagaraSystem)
    {
        return false;
    }

    // Check if the Niagara system is valid and can be activated
    if (!NiagaraSystem->IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid Niagara System: %s"), *NiagaraSystem->GetName());
        return false;
    }

    // Additional validation checks can be added here
    return true;
}

TArray<AActor*> ABuild_VFXIntegrationValidator::FindAllVFXActors()
{
    TArray<AActor*> VFXActors;
    TArray<AActor*> AllActors;

    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            // Check if actor has Niagara components
            TArray<UNiagaraComponent*> NiagaraComponents;
            Actor->GetComponents<UNiagaraComponent>(NiagaraComponents);

            if (NiagaraComponents.Num() > 0)
            {
                VFXActors.Add(Actor);
            }

            // Check if actor class name contains VFX-related keywords
            FString ClassName = Actor->GetClass()->GetName();
            if (ClassName.Contains(TEXT("VFX")) || 
                ClassName.Contains(TEXT("Niagara")) || 
                ClassName.Contains(TEXT("Particle")) ||
                ClassName.Contains(TEXT("Effect")))
            {
                VFXActors.AddUnique(Actor);
            }
        }
    }

    return VFXActors;
}

bool ABuild_VFXIntegrationValidator::TestVFXPerformance()
{
    // Basic performance test - check if we can activate test VFX component
    if (TestVFXComponent && TestVFXComponent->GetAsset())
    {
        TestVFXComponent->Activate();
        
        // Wait a frame and check if it's still active
        return TestVFXComponent->IsActive();
    }

    // If no test component, assume performance is OK
    return true;
}

void ABuild_VFXIntegrationValidator::RunVFXValidationTest()
{
    FBuild_VFXValidationReport Report = ValidateVFXIntegration();
    
    FString ResultString;
    switch (Report.OverallResult)
    {
        case EBuild_VFXValidationResult::Passed:
            ResultString = TEXT("PASSED");
            break;
        case EBuild_VFXValidationResult::Warning:
            ResultString = TEXT("WARNING");
            break;
        case EBuild_VFXValidationResult::Failed:
            ResultString = TEXT("FAILED");
            break;
        default:
            ResultString = TEXT("NOT TESTED");
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("VFX Validation Test Result: %s"), *ResultString);
    UE_LOG(LogTemp, Log, TEXT("Total VFX Actors: %d, Valid: %d, Invalid: %d"), 
           Report.TotalVFXActors, Report.ValidVFXActors, Report.InvalidVFXActors);

    for (const FString& Error : Report.ValidationErrors)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Validation Error: %s"), *Error);
    }
}

void ABuild_VFXIntegrationValidator::PerformPeriodicValidation()
{
    ValidateVFXIntegration();
}

bool ABuild_VFXIntegrationValidator::ValidateVFXActor(AActor* Actor)
{
    if (!Actor)
    {
        return false;
    }

    // Check if actor has valid Niagara components
    TArray<UNiagaraComponent*> NiagaraComponents;
    Actor->GetComponents<UNiagaraComponent>(NiagaraComponents);

    for (UNiagaraComponent* Component : NiagaraComponents)
    {
        if (Component && Component->GetAsset())
        {
            if (!ValidateNiagaraSystem(Component->GetAsset()))
            {
                return false;
            }
        }
    }

    return true;
}

void ABuild_VFXIntegrationValidator::LogValidationResult(const FBuild_VFXValidationReport& Report)
{
    FString ResultString;
    switch (Report.OverallResult)
    {
        case EBuild_VFXValidationResult::Passed:
            ResultString = TEXT("PASSED");
            break;
        case EBuild_VFXValidationResult::Warning:
            ResultString = TEXT("WARNING");
            break;
        case EBuild_VFXValidationResult::Failed:
            ResultString = TEXT("FAILED");
            break;
        default:
            ResultString = TEXT("NOT TESTED");
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("=== VFX INTEGRATION VALIDATION REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Result: %s"), *ResultString);
    UE_LOG(LogTemp, Log, TEXT("Total VFX Actors: %d"), Report.TotalVFXActors);
    UE_LOG(LogTemp, Log, TEXT("Valid VFX Actors: %d"), Report.ValidVFXActors);
    UE_LOG(LogTemp, Log, TEXT("Invalid VFX Actors: %d"), Report.InvalidVFXActors);
    UE_LOG(LogTemp, Log, TEXT("Validation Timestamp: %.2f"), Report.ValidationTimestamp);

    if (Report.ValidationErrors.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Validation Errors:"));
        for (const FString& Error : Report.ValidationErrors)
        {
            UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Error);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("=== END VFX VALIDATION REPORT ==="));
}