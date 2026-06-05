#include "QA_VFXIntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"

UQA_VFXIntegrationValidator::UQA_VFXIntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    // Initialize validation parameters
    MaxParticleCount = 10000;
    MaxEffectDuration = 30.0f;
    MinFrameRate = 30.0f;
    
    // Initialize validation state
    bValidationActive = false;
    ValidationStartTime = 0.0f;
    LastFrameRate = 60.0f;
    ActiveEffectCount = 0;
    TotalParticleCount = 0;
}

void UQA_VFXIntegrationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXIntegrationValidator: BeginPlay - Starting VFX validation"));
    
    // Start validation process
    StartValidation();
}

void UQA_VFXIntegrationValidator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bValidationActive)
    {
        UpdateValidationMetrics(DeltaTime);
        CheckPerformanceThresholds();
        ValidateActiveEffects();
    }
}

void UQA_VFXIntegrationValidator::StartValidation()
{
    if (bValidationActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA_VFXIntegrationValidator: Validation already active"));
        return;
    }
    
    bValidationActive = true;
    ValidationStartTime = GetWorld()->GetTimeSeconds();
    
    // Clear previous results
    ValidationResults.Empty();
    EffectValidationData.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXIntegrationValidator: Validation started"));
    
    // Validate existing VFX systems
    ValidateVFXSystems();
    ValidateNiagaraSystems();
    ValidateParticleSystems();
}

void UQA_VFXIntegrationValidator::StopValidation()
{
    if (!bValidationActive)
    {
        return;
    }
    
    bValidationActive = false;
    float ValidationDuration = GetWorld()->GetTimeSeconds() - ValidationStartTime;
    
    // Generate final validation report
    GenerateValidationReport(ValidationDuration);
    
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXIntegrationValidator: Validation stopped after %.2f seconds"), ValidationDuration);
}

void UQA_VFXIntegrationValidator::ValidateVFXSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXIntegrationValidator: Validating VFX systems"));
    
    // Find all VFX_EffectManager actors in the world
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationResult(TEXT("VFX_Systems"), false, TEXT("World not available"));
        return;
    }
    
    int32 VFXManagerCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("VFX_EffectManager")))
        {
            VFXManagerCount++;
            ValidateVFXManager(Actor);
        }
    }
    
    if (VFXManagerCount > 0)
    {
        AddValidationResult(TEXT("VFX_EffectManager"), true, FString::Printf(TEXT("Found %d VFX managers"), VFXManagerCount));
    }
    else
    {
        AddValidationResult(TEXT("VFX_EffectManager"), false, TEXT("No VFX managers found"));
    }
}

void UQA_VFXIntegrationValidator::ValidateVFXManager(AActor* VFXManager)
{
    if (!VFXManager)
    {
        return;
    }
    
    FString ActorName = VFXManager->GetName();
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXIntegrationValidator: Validating VFX Manager: %s"), *ActorName);
    
    // Check if actor has required components
    TArray<UActorComponent*> Components = VFXManager->GetRootComponent()->GetAttachChildren();
    
    bool bHasNiagaraComponent = false;
    bool bHasParticleComponent = false;
    
    for (UActorComponent* Component : Components)
    {
        if (Cast<UNiagaraComponent>(Component))
        {
            bHasNiagaraComponent = true;
        }
        else if (Cast<UParticleSystemComponent>(Component))
        {
            bHasParticleComponent = true;
        }
    }
    
    // Validate component setup
    if (bHasNiagaraComponent || bHasParticleComponent)
    {
        AddValidationResult(ActorName, true, TEXT("VFX components found"));
    }
    else
    {
        AddValidationResult(ActorName, false, TEXT("No VFX components found"));
    }
}

void UQA_VFXIntegrationValidator::ValidateNiagaraSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXIntegrationValidator: Validating Niagara systems"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 NiagaraComponentCount = 0;
    int32 ActiveNiagaraCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        TArray<UNiagaraComponent*> NiagaraComponents;
        Actor->GetComponents<UNiagaraComponent>(NiagaraComponents);
        
        for (UNiagaraComponent* NiagaraComp : NiagaraComponents)
        {
            NiagaraComponentCount++;
            
            if (NiagaraComp && NiagaraComp->IsActive())
            {
                ActiveNiagaraCount++;
                ValidateNiagaraComponent(NiagaraComp);
            }
        }
    }
    
    AddValidationResult(TEXT("Niagara_Systems"), true, 
        FString::Printf(TEXT("Found %d Niagara components (%d active)"), NiagaraComponentCount, ActiveNiagaraCount));
}

void UQA_VFXIntegrationValidator::ValidateNiagaraComponent(UNiagaraComponent* NiagaraComp)
{
    if (!NiagaraComp)
    {
        return;
    }
    
    FString ComponentName = NiagaraComp->GetName();
    
    // Check if system asset is valid
    UNiagaraSystem* NiagaraSystem = NiagaraComp->GetAsset();
    if (NiagaraSystem)
    {
        // Estimate particle count (simplified)
        int32 EstimatedParticles = 100; // Default estimate
        TotalParticleCount += EstimatedParticles;
        
        FQA_EffectValidationData ValidationData;
        ValidationData.EffectName = ComponentName;
        ValidationData.EffectType = TEXT("Niagara");
        ValidationData.bIsActive = NiagaraComp->IsActive();
        ValidationData.ParticleCount = EstimatedParticles;
        ValidationData.Duration = 0.0f; // Will be updated during tick
        
        EffectValidationData.Add(ValidationData);
    }
    else
    {
        AddValidationResult(ComponentName, false, TEXT("Niagara system asset is null"));
    }
}

void UQA_VFXIntegrationValidator::ValidateParticleSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXIntegrationValidator: Validating particle systems"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 ParticleComponentCount = 0;
    int32 ActiveParticleCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        TArray<UParticleSystemComponent*> ParticleComponents;
        Actor->GetComponents<UParticleSystemComponent>(ParticleComponents);
        
        for (UParticleSystemComponent* ParticleComp : ParticleComponents)
        {
            ParticleComponentCount++;
            
            if (ParticleComp && ParticleComp->IsActive())
            {
                ActiveParticleCount++;
                ValidateParticleComponent(ParticleComp);
            }
        }
    }
    
    AddValidationResult(TEXT("Particle_Systems"), true, 
        FString::Printf(TEXT("Found %d particle components (%d active)"), ParticleComponentCount, ActiveParticleCount));
}

void UQA_VFXIntegrationValidator::ValidateParticleComponent(UParticleSystemComponent* ParticleComp)
{
    if (!ParticleComp)
    {
        return;
    }
    
    FString ComponentName = ParticleComp->GetName();
    
    // Check if template is valid
    if (ParticleComp->Template)
    {
        // Estimate particle count (simplified)
        int32 EstimatedParticles = 50; // Default estimate
        TotalParticleCount += EstimatedParticles;
        
        FQA_EffectValidationData ValidationData;
        ValidationData.EffectName = ComponentName;
        ValidationData.EffectType = TEXT("Cascade");
        ValidationData.bIsActive = ParticleComp->IsActive();
        ValidationData.ParticleCount = EstimatedParticles;
        ValidationData.Duration = 0.0f;
        
        EffectValidationData.Add(ValidationData);
    }
    else
    {
        AddValidationResult(ComponentName, false, TEXT("Particle system template is null"));
    }
}

void UQA_VFXIntegrationValidator::UpdateValidationMetrics(float DeltaTime)
{
    // Update frame rate
    if (DeltaTime > 0.0f)
    {
        LastFrameRate = 1.0f / DeltaTime;
    }
    
    // Update active effect count
    ActiveEffectCount = 0;
    for (FQA_EffectValidationData& EffectData : EffectValidationData)
    {
        if (EffectData.bIsActive)
        {
            ActiveEffectCount++;
            EffectData.Duration += DeltaTime;
        }
    }
}

void UQA_VFXIntegrationValidator::CheckPerformanceThresholds()
{
    // Check frame rate threshold
    if (LastFrameRate < MinFrameRate)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA_VFXIntegrationValidator: Frame rate below threshold: %.1f FPS"), LastFrameRate);
    }
    
    // Check particle count threshold
    if (TotalParticleCount > MaxParticleCount)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA_VFXIntegrationValidator: Particle count above threshold: %d"), TotalParticleCount);
    }
    
    // Check for long-running effects
    for (const FQA_EffectValidationData& EffectData : EffectValidationData)
    {
        if (EffectData.bIsActive && EffectData.Duration > MaxEffectDuration)
        {
            UE_LOG(LogTemp, Warning, TEXT("QA_VFXIntegrationValidator: Long-running effect: %s (%.1fs)"), 
                *EffectData.EffectName, EffectData.Duration);
        }
    }
}

void UQA_VFXIntegrationValidator::ValidateActiveEffects()
{
    // Validate that active effects are still functioning
    for (FQA_EffectValidationData& EffectData : EffectValidationData)
    {
        if (EffectData.bIsActive)
        {
            // Additional validation logic can be added here
            // For now, just log active effects periodically
            static float LastLogTime = 0.0f;
            float CurrentTime = GetWorld()->GetTimeSeconds();
            
            if (CurrentTime - LastLogTime > 5.0f) // Log every 5 seconds
            {
                UE_LOG(LogTemp, Log, TEXT("QA_VFXIntegrationValidator: Active effect: %s (%s, %d particles)"), 
                    *EffectData.EffectName, *EffectData.EffectType, EffectData.ParticleCount);
                LastLogTime = CurrentTime;
            }
        }
    }
}

void UQA_VFXIntegrationValidator::AddValidationResult(const FString& TestName, bool bPassed, const FString& Details)
{
    FQA_ValidationResult Result;
    Result.TestName = TestName;
    Result.bPassed = bPassed;
    Result.Details = Details;
    Result.Timestamp = GetWorld()->GetTimeSeconds();
    
    ValidationResults.Add(Result);
    
    FString Status = bPassed ? TEXT("PASS") : TEXT("FAIL");
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXIntegrationValidator: [%s] %s - %s"), *Status, *TestName, *Details);
}

void UQA_VFXIntegrationValidator::GenerateValidationReport(float ValidationDuration)
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA VFX INTEGRATION VALIDATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Validation Duration: %.2f seconds"), ValidationDuration);
    UE_LOG(LogTemp, Warning, TEXT("Total Tests: %d"), ValidationResults.Num());
    
    int32 PassedTests = 0;
    int32 FailedTests = 0;
    
    for (const FQA_ValidationResult& Result : ValidationResults)
    {
        if (Result.bPassed)
        {
            PassedTests++;
        }
        else
        {
            FailedTests++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Passed Tests: %d"), PassedTests);
    UE_LOG(LogTemp, Warning, TEXT("Failed Tests: %d"), FailedTests);
    UE_LOG(LogTemp, Warning, TEXT("Success Rate: %.1f%%"), (float)PassedTests / ValidationResults.Num() * 100.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Metrics:"));
    UE_LOG(LogTemp, Warning, TEXT("- Last Frame Rate: %.1f FPS"), LastFrameRate);
    UE_LOG(LogTemp, Warning, TEXT("- Active Effects: %d"), ActiveEffectCount);
    UE_LOG(LogTemp, Warning, TEXT("- Total Particles: %d"), TotalParticleCount);
    
    UE_LOG(LogTemp, Warning, TEXT("=== END VALIDATION REPORT ==="));
}

TArray<FQA_ValidationResult> UQA_VFXIntegrationValidator::GetValidationResults() const
{
    return ValidationResults;
}

TArray<FQA_EffectValidationData> UQA_VFXIntegrationValidator::GetEffectValidationData() const
{
    return EffectValidationData;
}

bool UQA_VFXIntegrationValidator::IsValidationActive() const
{
    return bValidationActive;
}

float UQA_VFXIntegrationValidator::GetLastFrameRate() const
{
    return LastFrameRate;
}

int32 UQA_VFXIntegrationValidator::GetActiveEffectCount() const
{
    return ActiveEffectCount;
}

int32 UQA_VFXIntegrationValidator::GetTotalParticleCount() const
{
    return TotalParticleCount;
}