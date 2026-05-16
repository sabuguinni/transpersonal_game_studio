#include "Build_FinalIntegrationOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "GameFramework/Character.h"
#include "Components/LightComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogBuildIntegration, Log, All);

UBuild_FinalIntegrationOrchestrator::UBuild_FinalIntegrationOrchestrator()
{
    bIsInitialized = false;
}

void UBuild_FinalIntegrationOrchestrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Final Integration Orchestrator initialized"));
    
    // Initialize system metrics
    SystemMetrics.Empty();
    bIsInitialized = true;
    
    // Perform initial system validation
    RefreshSystemMetrics();
}

void UBuild_FinalIntegrationOrchestrator::Deinitialize()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Final Integration Orchestrator deinitialized"));
    
    SystemMetrics.Empty();
    bIsInitialized = false;
    
    Super::Deinitialize();
}

FBuild_IntegrationReport UBuild_FinalIntegrationOrchestrator::GenerateIntegrationReport()
{
    FBuild_IntegrationReport Report;
    
    if (!bIsInitialized)
    {
        UE_LOG(LogBuildIntegration, Warning, TEXT("Integration orchestrator not initialized"));
        return Report;
    }
    
    // Refresh metrics before generating report
    RefreshSystemMetrics();
    
    Report.SystemMetrics = SystemMetrics;
    Report.TotalActorCount = 0;
    Report.bAllSystemsOperational = true;
    
    // Calculate totals and check overall status
    for (const FBuild_SystemMetrics& Metric : SystemMetrics)
    {
        Report.TotalActorCount += Metric.ActorCount;
        if (Metric.Status != EBuild_SystemStatus::Operational)
        {
            Report.bAllSystemsOperational = false;
        }
    }
    
    Report.BuildVersion = TEXT("PROD_CYCLE_AUTO_20260516_006");
    Report.GeneratedAt = FDateTime::Now();
    
    LastReport = Report;
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Integration report generated - Total actors: %d, All systems operational: %s"), 
           Report.TotalActorCount, Report.bAllSystemsOperational ? TEXT("Yes") : TEXT("No"));
    
    return Report;
}

bool UBuild_FinalIntegrationOrchestrator::ValidateAllSystems()
{
    if (!bIsInitialized)
    {
        return false;
    }
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Starting comprehensive system validation"));
    
    ValidateCharacterSystem();
    ValidateDinosaurSystem();
    ValidateEnvironmentSystem();
    ValidateLightingSystem();
    ValidateVFXSystem();
    ValidateAudioSystem();
    ValidatePhysicsSystem();
    
    // Check if all systems are operational
    bool bAllOperational = true;
    for (const FBuild_SystemMetrics& Metric : SystemMetrics)
    {
        if (Metric.Status != EBuild_SystemStatus::Operational)
        {
            bAllOperational = false;
            break;
        }
    }
    
    UE_LOG(LogBuildIntegration, Log, TEXT("System validation complete - All operational: %s"), 
           bAllOperational ? TEXT("Yes") : TEXT("No"));
    
    return bAllOperational;
}

EBuild_SystemStatus UBuild_FinalIntegrationOrchestrator::GetSystemStatus(const FString& SystemName)
{
    for (const FBuild_SystemMetrics& Metric : SystemMetrics)
    {
        if (Metric.SystemName == SystemName)
        {
            return Metric.Status;
        }
    }
    return EBuild_SystemStatus::Unknown;
}

void UBuild_FinalIntegrationOrchestrator::RefreshSystemMetrics()
{
    if (!bIsInitialized)
    {
        return;
    }
    
    SystemMetrics.Empty();
    
    ValidateCharacterSystem();
    ValidateDinosaurSystem();
    ValidateEnvironmentSystem();
    ValidateLightingSystem();
    ValidateVFXSystem();
    ValidateAudioSystem();
    ValidatePhysicsSystem();
}

void UBuild_FinalIntegrationOrchestrator::ValidateCharacterSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        SystemMetrics.Add(CreateSystemMetric(TEXT("Character"), 0, false));
        return;
    }
    
    int32 CharacterCount = 0;
    for (TActorIterator<ACharacter> ActorItr(World); ActorItr; ++ActorItr)
    {
        CharacterCount++;
    }
    
    bool bIsOperational = CharacterCount > 0;
    SystemMetrics.Add(CreateSystemMetric(TEXT("Character"), CharacterCount, bIsOperational));
    
    LogSystemStatus(TEXT("Character"), bIsOperational ? EBuild_SystemStatus::Operational : EBuild_SystemStatus::Failed);
}

void UBuild_FinalIntegrationOrchestrator::ValidateDinosaurSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        SystemMetrics.Add(CreateSystemMetric(TEXT("Dinosaur"), 0, false));
        return;
    }
    
    int32 DinosaurCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && (Actor->GetActorLabel().Contains(TEXT("Dinosaur")) || 
                     Actor->GetActorLabel().Contains(TEXT("TRex")) ||
                     Actor->GetActorLabel().Contains(TEXT("Raptor"))))
        {
            DinosaurCount++;
        }
    }
    
    bool bIsOperational = DinosaurCount >= 5;
    SystemMetrics.Add(CreateSystemMetric(TEXT("Dinosaur"), DinosaurCount, bIsOperational));
    
    LogSystemStatus(TEXT("Dinosaur"), bIsOperational ? EBuild_SystemStatus::Operational : EBuild_SystemStatus::Degraded);
}

void UBuild_FinalIntegrationOrchestrator::ValidateEnvironmentSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        SystemMetrics.Add(CreateSystemMetric(TEXT("Environment"), 0, false));
        return;
    }
    
    int32 EnvironmentCount = 0;
    for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && (Actor->GetActorLabel().Contains(TEXT("Tree")) || 
                     Actor->GetActorLabel().Contains(TEXT("Rock")) ||
                     Actor->GetActorLabel().Contains(TEXT("Vegetation"))))
        {
            EnvironmentCount++;
        }
    }
    
    bool bIsOperational = EnvironmentCount >= 10;
    SystemMetrics.Add(CreateSystemMetric(TEXT("Environment"), EnvironmentCount, bIsOperational));
    
    LogSystemStatus(TEXT("Environment"), bIsOperational ? EBuild_SystemStatus::Operational : EBuild_SystemStatus::Degraded);
}

void UBuild_FinalIntegrationOrchestrator::ValidateLightingSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        SystemMetrics.Add(CreateSystemMetric(TEXT("Lighting"), 0, false));
        return;
    }
    
    int32 LightCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->FindComponentByClass<ULightComponent>())
        {
            LightCount++;
        }
    }
    
    bool bIsOperational = LightCount > 0;
    SystemMetrics.Add(CreateSystemMetric(TEXT("Lighting"), LightCount, bIsOperational));
    
    LogSystemStatus(TEXT("Lighting"), bIsOperational ? EBuild_SystemStatus::Operational : EBuild_SystemStatus::Failed);
}

void UBuild_FinalIntegrationOrchestrator::ValidateVFXSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        SystemMetrics.Add(CreateSystemMetric(TEXT("VFX"), 0, true)); // VFX is optional
        return;
    }
    
    int32 VFXCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->FindComponentByClass<UNiagaraComponent>())
        {
            VFXCount++;
        }
    }
    
    // VFX system is always operational (optional)
    SystemMetrics.Add(CreateSystemMetric(TEXT("VFX"), VFXCount, true));
    
    LogSystemStatus(TEXT("VFX"), EBuild_SystemStatus::Operational);
}

void UBuild_FinalIntegrationOrchestrator::ValidateAudioSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        SystemMetrics.Add(CreateSystemMetric(TEXT("Audio"), 0, true)); // Audio is optional
        return;
    }
    
    int32 AudioCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->FindComponentByClass<UAudioComponent>())
        {
            AudioCount++;
        }
    }
    
    // Audio system is always operational (optional)
    SystemMetrics.Add(CreateSystemMetric(TEXT("Audio"), AudioCount, true));
    
    LogSystemStatus(TEXT("Audio"), EBuild_SystemStatus::Operational);
}

void UBuild_FinalIntegrationOrchestrator::ValidatePhysicsSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        SystemMetrics.Add(CreateSystemMetric(TEXT("Physics"), 0, false));
        return;
    }
    
    int32 PhysicsCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent())
        {
            PhysicsCount++;
        }
    }
    
    bool bIsOperational = PhysicsCount > 0;
    SystemMetrics.Add(CreateSystemMetric(TEXT("Physics"), PhysicsCount, bIsOperational));
    
    LogSystemStatus(TEXT("Physics"), bIsOperational ? EBuild_SystemStatus::Operational : EBuild_SystemStatus::Failed);
}

FBuild_SystemMetrics UBuild_FinalIntegrationOrchestrator::CreateSystemMetric(const FString& SystemName, int32 ActorCount, bool bIsOperational)
{
    FBuild_SystemMetrics Metric;
    Metric.SystemName = SystemName;
    Metric.Status = bIsOperational ? EBuild_SystemStatus::Operational : EBuild_SystemStatus::Failed;
    Metric.ActorCount = ActorCount;
    Metric.PerformanceScore = bIsOperational ? 100.0f : 0.0f;
    Metric.LastValidated = FDateTime::Now();
    
    return Metric;
}

void UBuild_FinalIntegrationOrchestrator::LogSystemStatus(const FString& SystemName, EBuild_SystemStatus Status)
{
    FString StatusText;
    switch (Status)
    {
        case EBuild_SystemStatus::Operational:
            StatusText = TEXT("OPERATIONAL");
            break;
        case EBuild_SystemStatus::Degraded:
            StatusText = TEXT("DEGRADED");
            break;
        case EBuild_SystemStatus::Failed:
            StatusText = TEXT("FAILED");
            break;
        default:
            StatusText = TEXT("UNKNOWN");
            break;
    }
    
    UE_LOG(LogBuildIntegration, Log, TEXT("System %s: %s"), *SystemName, *StatusText);
}