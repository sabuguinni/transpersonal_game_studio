#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"

DEFINE_LOG_CATEGORY_STATIC(LogBuildIntegration, Log, All);

UBuildIntegrationManager::UBuildIntegrationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Check every 5 seconds
    
    bAutoActivate = true;
    bWantsInitializeComponent = true;
    
    // Initialize validation counters
    LastValidationTime = 0.0f;
    ValidationInterval = 10.0f;
    
    // System health tracking
    SystemHealthStatus.Add(TEXT("Core"), EBuild_SystemHealth::Healthy);
    SystemHealthStatus.Add(TEXT("Physics"), EBuild_SystemHealth::Healthy);
    SystemHealthStatus.Add(TEXT("AI"), EBuild_SystemHealth::Healthy);
    SystemHealthStatus.Add(TEXT("Audio"), EBuild_SystemHealth::Healthy);
    SystemHealthStatus.Add(TEXT("VFX"), EBuild_SystemHealth::Healthy);
}

void UBuildIntegrationManager::InitializeComponent()
{
    Super::InitializeComponent();
    
    UE_LOG(LogBuildIntegration, Log, TEXT("BuildIntegrationManager initialized"));
    
    // Start validation timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ValidationTimerHandle,
            this,
            &UBuildIntegrationManager::PerformSystemValidation,
            ValidationInterval,
            true
        );
    }
}

void UBuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Perform initial system check
    PerformSystemValidation();
}

void UBuildIntegrationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update validation timer
    LastValidationTime += DeltaTime;
}

void UBuildIntegrationManager::PerformSystemValidation()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Performing system validation..."));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogBuildIntegration, Error, TEXT("No valid world for system validation"));
        return;
    }
    
    // Reset validation results
    ValidationResults.Empty();
    
    // Validate core systems
    ValidateCoreSystem();
    ValidatePhysicsSystem();
    ValidateAISystem();
    ValidateAudioSystem();
    ValidateVFXSystem();
    
    // Update overall health status
    UpdateSystemHealth();
    
    // Log validation summary
    LogValidationSummary();
}

void UBuildIntegrationManager::ValidateCoreSystem()
{
    FBuild_ValidationResult Result;
    Result.SystemName = TEXT("Core");
    Result.bIsHealthy = true;
    Result.ErrorCount = 0;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        Result.bIsHealthy = false;
        Result.ErrorCount++;
        Result.ErrorMessages.Add(TEXT("No valid world reference"));
    }
    
    // Check for game mode
    if (World && !World->GetAuthGameMode())
    {
        Result.ErrorCount++;
        Result.ErrorMessages.Add(TEXT("No GameMode found"));
    }
    
    // Check actor count (should have reasonable number)
    if (World)
    {
        int32 ActorCount = World->GetActorCount();
        if (ActorCount < 5)
        {
            Result.ErrorCount++;
            Result.ErrorMessages.Add(FString::Printf(TEXT("Low actor count: %d"), ActorCount));
        }
        else
        {
            Result.SuccessMessages.Add(FString::Printf(TEXT("Actor count: %d"), ActorCount));
        }
    }
    
    Result.bIsHealthy = (Result.ErrorCount == 0);
    SystemHealthStatus[TEXT("Core")] = Result.bIsHealthy ? EBuild_SystemHealth::Healthy : EBuild_SystemHealth::Critical;
    
    ValidationResults.Add(Result);
}

void UBuildIntegrationManager::ValidatePhysicsSystem()
{
    FBuild_ValidationResult Result;
    Result.SystemName = TEXT("Physics");
    Result.bIsHealthy = true;
    Result.ErrorCount = 0;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        Result.bIsHealthy = false;
        Result.ErrorCount++;
        Result.ErrorMessages.Add(TEXT("No world for physics validation"));
        ValidationResults.Add(Result);
        return;
    }
    
    // Check physics scene
    if (!World->GetPhysicsScene())
    {
        Result.ErrorCount++;
        Result.ErrorMessages.Add(TEXT("No physics scene"));
    }
    else
    {
        Result.SuccessMessages.Add(TEXT("Physics scene active"));
    }
    
    // Count physics actors
    int32 PhysicsActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent() && Actor->GetRootComponent()->IsSimulatingPhysics())
        {
            PhysicsActorCount++;
        }
    }
    
    Result.SuccessMessages.Add(FString::Printf(TEXT("Physics actors: %d"), PhysicsActorCount));
    
    Result.bIsHealthy = (Result.ErrorCount == 0);
    SystemHealthStatus[TEXT("Physics")] = Result.bIsHealthy ? EBuild_SystemHealth::Healthy : EBuild_SystemHealth::Warning;
    
    ValidationResults.Add(Result);
}

void UBuildIntegrationManager::ValidateAISystem()
{
    FBuild_ValidationResult Result;
    Result.SystemName = TEXT("AI");
    Result.bIsHealthy = true;
    Result.ErrorCount = 0;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        Result.bIsHealthy = false;
        Result.ErrorCount++;
        Result.ErrorMessages.Add(TEXT("No world for AI validation"));
        ValidationResults.Add(Result);
        return;
    }
    
    // Count AI controllers
    int32 AIControllerCount = 0;
    for (TActorIterator<AController> ControllerItr(World); ControllerItr; ++ControllerItr)
    {
        AController* Controller = *ControllerItr;
        if (Controller && !Controller->IsPlayerController())
        {
            AIControllerCount++;
        }
    }
    
    Result.SuccessMessages.Add(FString::Printf(TEXT("AI Controllers: %d"), AIControllerCount));
    
    if (AIControllerCount == 0)
    {
        Result.ErrorCount++;
        Result.ErrorMessages.Add(TEXT("No AI controllers found"));
    }
    
    Result.bIsHealthy = (Result.ErrorCount == 0);
    SystemHealthStatus[TEXT("AI")] = Result.bIsHealthy ? EBuild_SystemHealth::Healthy : EBuild_SystemHealth::Warning;
    
    ValidationResults.Add(Result);
}

void UBuildIntegrationManager::ValidateAudioSystem()
{
    FBuild_ValidationResult Result;
    Result.SystemName = TEXT("Audio");
    Result.bIsHealthy = true;
    Result.ErrorCount = 0;
    
    // Basic audio system validation
    if (!GEngine || !GEngine->GetAudioDeviceManager())
    {
        Result.ErrorCount++;
        Result.ErrorMessages.Add(TEXT("No audio device manager"));
    }
    else
    {
        Result.SuccessMessages.Add(TEXT("Audio device manager active"));
    }
    
    Result.bIsHealthy = (Result.ErrorCount == 0);
    SystemHealthStatus[TEXT("Audio")] = Result.bIsHealthy ? EBuild_SystemHealth::Healthy : EBuild_SystemHealth::Warning;
    
    ValidationResults.Add(Result);
}

void UBuildIntegrationManager::ValidateVFXSystem()
{
    FBuild_ValidationResult Result;
    Result.SystemName = TEXT("VFX");
    Result.bIsHealthy = true;
    Result.ErrorCount = 0;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        Result.bIsHealthy = false;
        Result.ErrorCount++;
        Result.ErrorMessages.Add(TEXT("No world for VFX validation"));
        ValidationResults.Add(Result);
        return;
    }
    
    // Count particle systems
    int32 ParticleSystemCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->FindComponentByClass<UParticleSystemComponent>())
        {
            ParticleSystemCount++;
        }
    }
    
    Result.SuccessMessages.Add(FString::Printf(TEXT("Particle systems: %d"), ParticleSystemCount));
    
    Result.bIsHealthy = (Result.ErrorCount == 0);
    SystemHealthStatus[TEXT("VFX")] = Result.bIsHealthy ? EBuild_SystemHealth::Healthy : EBuild_SystemHealth::Healthy;
    
    ValidationResults.Add(Result);
}

void UBuildIntegrationManager::UpdateSystemHealth()
{
    // Calculate overall system health
    int32 HealthyCount = 0;
    int32 WarningCount = 0;
    int32 CriticalCount = 0;
    
    for (const auto& HealthPair : SystemHealthStatus)
    {
        switch (HealthPair.Value)
        {
            case EBuild_SystemHealth::Healthy:
                HealthyCount++;
                break;
            case EBuild_SystemHealth::Warning:
                WarningCount++;
                break;
            case EBuild_SystemHealth::Critical:
                CriticalCount++;
                break;
        }
    }
    
    // Determine overall health
    if (CriticalCount > 0)
    {
        OverallSystemHealth = EBuild_SystemHealth::Critical;
    }
    else if (WarningCount > 0)
    {
        OverallSystemHealth = EBuild_SystemHealth::Warning;
    }
    else
    {
        OverallSystemHealth = EBuild_SystemHealth::Healthy;
    }
    
    UE_LOG(LogBuildIntegration, Log, TEXT("System Health - Healthy: %d, Warning: %d, Critical: %d"), 
           HealthyCount, WarningCount, CriticalCount);
}

void UBuildIntegrationManager::LogValidationSummary()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("=== VALIDATION SUMMARY ==="));
    
    for (const FBuild_ValidationResult& Result : ValidationResults)
    {
        UE_LOG(LogBuildIntegration, Log, TEXT("System: %s - Health: %s - Errors: %d"), 
               *Result.SystemName, 
               Result.bIsHealthy ? TEXT("HEALTHY") : TEXT("UNHEALTHY"),
               Result.ErrorCount);
        
        for (const FString& Success : Result.SuccessMessages)
        {
            UE_LOG(LogBuildIntegration, Log, TEXT("  ✓ %s"), *Success);
        }
        
        for (const FString& Error : Result.ErrorMessages)
        {
            UE_LOG(LogBuildIntegration, Warning, TEXT("  ✗ %s"), *Error);
        }
    }
    
    const TCHAR* OverallHealthStr = TEXT("UNKNOWN");
    switch (OverallSystemHealth)
    {
        case EBuild_SystemHealth::Healthy: OverallHealthStr = TEXT("HEALTHY"); break;
        case EBuild_SystemHealth::Warning: OverallHealthStr = TEXT("WARNING"); break;
        case EBuild_SystemHealth::Critical: OverallHealthStr = TEXT("CRITICAL"); break;
    }
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Overall System Health: %s"), OverallHealthStr);
}

EBuild_SystemHealth UBuildIntegrationManager::GetSystemHealth(const FString& SystemName) const
{
    if (const EBuild_SystemHealth* Health = SystemHealthStatus.Find(SystemName))
    {
        return *Health;
    }
    return EBuild_SystemHealth::Warning;
}

TArray<FBuild_ValidationResult> UBuildIntegrationManager::GetLastValidationResults() const
{
    return ValidationResults;
}

void UBuildIntegrationManager::ForceSystemValidation()
{
    PerformSystemValidation();
}