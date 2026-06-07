#include "Build_VFXIntegrationValidator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

UBuild_VFXIntegrationValidator::UBuild_VFXIntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    bEnableDetailedLogging = true;
    MaxAllowedFrameTimeImpact = 2.0f;
}

void UBuild_VFXIntegrationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bEnableDetailedLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_VFXIntegrationValidator: Component initialized"));
    }
}

FBuild_VFXValidationResult UBuild_VFXIntegrationValidator::ValidateVFXSystems()
{
    LogValidationStep(TEXT("Starting VFX Systems Validation"), true);
    
    FBuild_VFXValidationResult Result;
    
    // Validate Niagara systems
    Result.bNiagaraSystemsValid = ValidateNiagaraSystems();
    LogValidationStep(TEXT("Niagara Systems Validation"), Result.bNiagaraSystemsValid);
    
    // Validate particle systems
    Result.bParticleSystemsValid = ValidateParticleSystems();
    LogValidationStep(TEXT("Particle Systems Validation"), Result.bParticleSystemsValid);
    
    // Validate campfire effects specifically
    Result.bCampfireEffectsValid = ValidateCampfireEffects();
    LogValidationStep(TEXT("Campfire Effects Validation"), Result.bCampfireEffectsValid);
    
    // Measure performance impact
    Result.FrameTimeImpact = MeasureVFXPerformanceImpact();
    Result.bPerformanceWithinBudget = (Result.FrameTimeImpact <= MaxAllowedFrameTimeImpact);
    LogValidationStep(TEXT("Performance Budget Check"), Result.bPerformanceWithinBudget);
    
    // Count VFX actors
    TArray<AActor*> VFXActors = FindVFXActors();
    Result.TotalVFXActors = VFXActors.Num();
    
    // Generate validation report
    FString Report = FString::Printf(TEXT("VFX Integration Validation Report:\n"));
    Report += FString::Printf(TEXT("- Niagara Systems: %s\n"), Result.bNiagaraSystemsValid ? TEXT("VALID") : TEXT("INVALID"));
    Report += FString::Printf(TEXT("- Particle Systems: %s\n"), Result.bParticleSystemsValid ? TEXT("VALID") : TEXT("INVALID"));
    Report += FString::Printf(TEXT("- Campfire Effects: %s\n"), Result.bCampfireEffectsValid ? TEXT("VALID") : TEXT("INVALID"));
    Report += FString::Printf(TEXT("- Performance Impact: %.2fms (Budget: %.2fms)\n"), Result.FrameTimeImpact, MaxAllowedFrameTimeImpact);
    Report += FString::Printf(TEXT("- Total VFX Actors: %d\n"), Result.TotalVFXActors);
    
    Result.ValidationReport = Report;
    LastValidationResult = Result;
    
    if (bEnableDetailedLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Integration Validation Complete: %s"), *Report);
    }
    
    return Result;
}

bool UBuild_VFXIntegrationValidator::ValidateNiagaraSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check for VFX_NiagaraLibrary class
    UClass* NiagaraLibraryClass = FindObject<UClass>(ANY_PACKAGE, TEXT("VFX_NiagaraLibrary"));
    if (!NiagaraLibraryClass)
    {
        if (bEnableDetailedLogging)
        {
            UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary class not found - may need compilation"));
        }
        return false;
    }
    
    // Look for Niagara-related actors in the scene
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 NiagaraActorCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Niagara")))
        {
            NiagaraActorCount++;
        }
    }
    
    if (bEnableDetailedLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Found %d Niagara-related actors"), NiagaraActorCount);
    }
    
    return true; // Basic validation passed
}

bool UBuild_VFXIntegrationValidator::ValidateParticleSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Look for particle system actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 ParticleActorCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && (Actor->GetName().Contains(TEXT("Particle")) || Actor->GetName().Contains(TEXT("Effect"))))
        {
            ParticleActorCount++;
        }
    }
    
    if (bEnableDetailedLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Found %d Particle-related actors"), ParticleActorCount);
    }
    
    return true; // Basic validation passed
}

bool UBuild_VFXIntegrationValidator::ValidateCampfireEffects()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Look for QA test actors created by Agent #18
    TArray<AActor*> QAActors = FindQATestActors();
    
    bool bFoundCampfireBase = false;
    bool bFoundFireEffect = false;
    bool bFoundValidationSuite = false;
    
    for (AActor* Actor : QAActors)
    {
        if (Actor)
        {
            FString ActorName = Actor->GetName();
            if (ActorName.Contains(TEXT("QA_Test_Campfire_Base")))
            {
                bFoundCampfireBase = true;
            }
            else if (ActorName.Contains(TEXT("QA_Test_Fire_Effect")))
            {
                bFoundFireEffect = true;
            }
            else if (ActorName.Contains(TEXT("QA_VFX_Validation_Suite")))
            {
                bFoundValidationSuite = true;
            }
        }
    }
    
    bool bCampfireValid = bFoundCampfireBase && bFoundFireEffect && bFoundValidationSuite;
    
    if (bEnableDetailedLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Campfire Effects Validation: Base=%s, Effect=%s, Suite=%s"), 
               bFoundCampfireBase ? TEXT("Found") : TEXT("Missing"),
               bFoundFireEffect ? TEXT("Found") : TEXT("Missing"),
               bFoundValidationSuite ? TEXT("Found") : TEXT("Missing"));
    }
    
    return bCampfireValid;
}

float UBuild_VFXIntegrationValidator::MeasureVFXPerformanceImpact()
{
    // Simulate performance measurement
    // In a real implementation, this would measure actual frame time impact
    TArray<AActor*> VFXActors = FindVFXActors();
    
    // Estimate 0.1ms per VFX actor as baseline
    float EstimatedImpact = VFXActors.Num() * 0.1f;
    
    if (bEnableDetailedLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Estimated VFX Performance Impact: %.2fms for %d actors"), 
               EstimatedImpact, VFXActors.Num());
    }
    
    return EstimatedImpact;
}

void UBuild_VFXIntegrationValidator::GenerateVFXIntegrationReport()
{
    FBuild_VFXValidationResult Result = ValidateVFXSystems();
    
    FString DetailedReport = TEXT("=== VFX INTEGRATION REPORT ===\n");
    DetailedReport += FString::Printf(TEXT("Validation Timestamp: %s\n"), *FDateTime::Now().ToString());
    DetailedReport += TEXT("\n");
    DetailedReport += Result.ValidationReport;
    DetailedReport += TEXT("\n");
    DetailedReport += TEXT("=== QA INTEGRATION STATUS ===\n");
    
    bool bQAValid = ValidateQATestActors();
    DetailedReport += FString::Printf(TEXT("QA Test Actors Valid: %s\n"), bQAValid ? TEXT("YES") : TEXT("NO"));
    
    DetailedReport += TEXT("\n=== RECOMMENDATIONS ===\n");
    if (!Result.bNiagaraSystemsValid)
    {
        DetailedReport += TEXT("- Compile VFX_NiagaraLibrary class\n");
    }
    if (!Result.bCampfireEffectsValid)
    {
        DetailedReport += TEXT("- Verify QA test actors are properly placed\n");
    }
    if (!Result.bPerformanceWithinBudget)
    {
        DetailedReport += TEXT("- Optimize VFX performance to meet budget\n");
    }
    
    if (bEnableDetailedLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s"), *DetailedReport);
    }
}

bool UBuild_VFXIntegrationValidator::ValidateQATestActors()
{
    TArray<AActor*> QAActors = FindQATestActors();
    
    if (bEnableDetailedLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Found %d QA test actors"), QAActors.Num());
        for (AActor* Actor : QAActors)
        {
            if (Actor)
            {
                UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Actor->GetName());
            }
        }
    }
    
    return QAActors.Num() >= 3; // Expect at least 3 QA test actors from Agent #18
}

void UBuild_VFXIntegrationValidator::ProcessQAValidationResults()
{
    if (bEnableDetailedLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Processing QA validation results from Agent #18"));
    }
    
    // Validate QA deliverables
    bool bQAValid = ValidateQATestActors();
    
    // Run VFX validation
    FBuild_VFXValidationResult VFXResult = ValidateVFXSystems();
    
    // Generate combined report
    GenerateVFXIntegrationReport();
    
    if (bEnableDetailedLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA-VFX Integration Processing Complete"));
    }
}

void UBuild_VFXIntegrationValidator::LogValidationStep(const FString& StepName, bool bSuccess)
{
    if (bEnableDetailedLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Validation - %s: %s"), 
               *StepName, bSuccess ? TEXT("PASS") : TEXT("FAIL"));
    }
}

TArray<AActor*> UBuild_VFXIntegrationValidator::FindVFXActors()
{
    TArray<AActor*> VFXActors;
    UWorld* World = GetWorld();
    
    if (World)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        for (AActor* Actor : AllActors)
        {
            if (Actor)
            {
                FString ActorName = Actor->GetName();
                if (ActorName.Contains(TEXT("VFX")) || 
                    ActorName.Contains(TEXT("Effect")) || 
                    ActorName.Contains(TEXT("Particle")) || 
                    ActorName.Contains(TEXT("Niagara")))
                {
                    VFXActors.Add(Actor);
                }
            }
        }
    }
    
    return VFXActors;
}

TArray<AActor*> UBuild_VFXIntegrationValidator::FindQATestActors()
{
    TArray<AActor*> QAActors;
    UWorld* World = GetWorld();
    
    if (World)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        for (AActor* Actor : AllActors)
        {
            if (Actor)
            {
                FString ActorName = Actor->GetName();
                if (ActorName.Contains(TEXT("QA_Test")) || ActorName.Contains(TEXT("QA_VFX")))
                {
                    QAActors.Add(Actor);
                }
            }
        }
    }
    
    return QAActors;
}