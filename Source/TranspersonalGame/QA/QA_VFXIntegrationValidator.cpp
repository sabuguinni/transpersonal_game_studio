#include "QA_VFXIntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "NiagaraActor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"

UQA_VFXIntegrationValidator::UQA_VFXIntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize validation thresholds
    MaxAllowedParticleCount = 50000;
    MinRequiredFPS = 30.0f;
    MaxMemoryUsageMB = 512.0f;
    
    // Initialize validation results
    ValidationResults.Empty();
    bValidationPassed = false;
    LastValidationTime = 0.0f;
}

void UQA_VFXIntegrationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    // Start validation timer
    GetWorld()->GetTimerManager().SetTimer(
        ValidationTimerHandle,
        this,
        &UQA_VFXIntegrationValidator::RunPeriodicValidation,
        5.0f,  // Run every 5 seconds
        true   // Loop
    );
}

void UQA_VFXIntegrationValidator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clear validation timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ValidationTimerHandle);
    }
    
    Super::EndPlay(EndPlayReason);
}

bool UQA_VFXIntegrationValidator::ValidateVFXSystems()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("QA_VFXIntegrationValidator: No valid world found"));
        return false;
    }
    
    ValidationResults.Empty();
    bValidationPassed = true;
    
    // Validate Niagara systems
    bool bNiagaraValid = ValidateNiagaraSystems();
    
    // Validate VFX performance
    bool bPerformanceValid = ValidateVFXPerformance();
    
    // Validate VFX integration with gameplay
    bool bIntegrationValid = ValidateGameplayIntegration();
    
    // Overall validation result
    bValidationPassed = bNiagaraValid && bPerformanceValid && bIntegrationValid;
    LastValidationTime = GetWorld()->GetTimeSeconds();
    
    // Log validation summary
    UE_LOG(LogTemp, Warning, TEXT("VFX Integration Validation Complete - Passed: %s"), 
           bValidationPassed ? TEXT("YES") : TEXT("NO"));
    
    return bValidationPassed;
}

bool UQA_VFXIntegrationValidator::ValidateNiagaraSystems()
{
    TArray<AActor*> NiagaraActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANiagaraActor::StaticClass(), NiagaraActors);
    
    FQA_ValidationResult NiagaraResult;
    NiagaraResult.TestName = TEXT("Niagara Systems Validation");
    NiagaraResult.bPassed = true;
    
    int32 ValidSystems = 0;
    int32 TotalSystems = NiagaraActors.Num();
    
    for (AActor* Actor : NiagaraActors)
    {
        if (ANiagaraActor* NiagaraActor = Cast<ANiagaraActor>(Actor))
        {
            UNiagaraComponent* NiagaraComp = NiagaraActor->GetNiagaraComponent();
            if (NiagaraComp && NiagaraComp->GetAsset())
            {
                ValidSystems++;
                
                // Check if system is active
                if (!NiagaraComp->IsActive())
                {
                    UE_LOG(LogTemp, Warning, TEXT("Inactive Niagara system: %s"), 
                           *NiagaraActor->GetName());
                }
            }
            else
            {
                NiagaraResult.bPassed = false;
                NiagaraResult.ErrorMessage += FString::Printf(
                    TEXT("Invalid Niagara system: %s; "), *NiagaraActor->GetName());
            }
        }
    }
    
    NiagaraResult.Details = FString::Printf(
        TEXT("Valid systems: %d/%d"), ValidSystems, TotalSystems);
    
    ValidationResults.Add(NiagaraResult);
    
    UE_LOG(LogTemp, Log, TEXT("Niagara validation: %d/%d systems valid"), 
           ValidSystems, TotalSystems);
    
    return NiagaraResult.bPassed;
}

bool UQA_VFXIntegrationValidator::ValidateVFXPerformance()
{
    FQA_ValidationResult PerformanceResult;
    PerformanceResult.TestName = TEXT("VFX Performance Validation");
    PerformanceResult.bPassed = true;
    
    // Get current FPS
    float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    
    // Check FPS threshold
    if (CurrentFPS < MinRequiredFPS)
    {
        PerformanceResult.bPassed = false;
        PerformanceResult.ErrorMessage += FString::Printf(
            TEXT("FPS below threshold: %.1f < %.1f; "), CurrentFPS, MinRequiredFPS);
    }
    
    // Count total particles (simplified estimation)
    TArray<AActor*> NiagaraActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANiagaraActor::StaticClass(), NiagaraActors);
    
    int32 EstimatedParticleCount = NiagaraActors.Num() * 1000; // Rough estimate
    
    if (EstimatedParticleCount > MaxAllowedParticleCount)
    {
        PerformanceResult.bPassed = false;
        PerformanceResult.ErrorMessage += FString::Printf(
            TEXT("Particle count too high: %d > %d; "), 
            EstimatedParticleCount, MaxAllowedParticleCount);
    }
    
    PerformanceResult.Details = FString::Printf(
        TEXT("FPS: %.1f, Estimated particles: %d"), CurrentFPS, EstimatedParticleCount);
    
    ValidationResults.Add(PerformanceResult);
    
    UE_LOG(LogTemp, Log, TEXT("VFX Performance: FPS=%.1f, Particles=%d"), 
           CurrentFPS, EstimatedParticleCount);
    
    return PerformanceResult.bPassed;
}

bool UQA_VFXIntegrationValidator::ValidateGameplayIntegration()
{
    FQA_ValidationResult IntegrationResult;
    IntegrationResult.TestName = TEXT("VFX Gameplay Integration");
    IntegrationResult.bPassed = true;
    
    // Check if VFX systems respond to gameplay events
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 VFXEnabledActors = 0;
    int32 TotalGameplayActors = 0;
    
    for (AActor* Actor : AllActors)
    {
        // Skip non-gameplay actors
        if (Actor->IsA<ANiagaraActor>() || 
            Actor->GetName().Contains(TEXT("Light")) ||
            Actor->GetName().Contains(TEXT("Sky")))
        {
            continue;
        }
        
        TotalGameplayActors++;
        
        // Check if actor has VFX components or references
        TArray<UActorComponent*> Components = Actor->GetRootComponent()->GetAttachChildren();
        for (UActorComponent* Component : Components)
        {
            if (Component->IsA<UNiagaraComponent>())
            {
                VFXEnabledActors++;
                break;
            }
        }
    }
    
    // Calculate integration percentage
    float IntegrationPercentage = TotalGameplayActors > 0 ? 
        (float)VFXEnabledActors / TotalGameplayActors * 100.0f : 0.0f;
    
    IntegrationResult.Details = FString::Printf(
        TEXT("VFX integration: %.1f%% (%d/%d actors)"), 
        IntegrationPercentage, VFXEnabledActors, TotalGameplayActors);
    
    ValidationResults.Add(IntegrationResult);
    
    UE_LOG(LogTemp, Log, TEXT("VFX Integration: %.1f%% of gameplay actors have VFX"), 
           IntegrationPercentage);
    
    return IntegrationResult.bPassed;
}

void UQA_VFXIntegrationValidator::RunPeriodicValidation()
{
    ValidateVFXSystems();
}

TArray<FQA_ValidationResult> UQA_VFXIntegrationValidator::GetValidationResults() const
{
    return ValidationResults;
}

bool UQA_VFXIntegrationValidator::IsValidationPassing() const
{
    return bValidationPassed;
}

float UQA_VFXIntegrationValidator::GetLastValidationTime() const
{
    return LastValidationTime;
}

void UQA_VFXIntegrationValidator::SetValidationThresholds(int32 MaxParticles, float MinFPS, float MaxMemoryMB)
{
    MaxAllowedParticleCount = MaxParticles;
    MinRequiredFPS = MinFPS;
    MaxMemoryUsageMB = MaxMemoryMB;
    
    UE_LOG(LogTemp, Log, TEXT("VFX validation thresholds updated: Particles=%d, FPS=%.1f, Memory=%.1fMB"), 
           MaxParticles, MinFPS, MaxMemoryMB);
}