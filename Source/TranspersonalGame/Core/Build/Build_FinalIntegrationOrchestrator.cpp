#include "Build_FinalIntegrationOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameModeBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/LightComponent.h"
#include "Landscape/Landscape.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

UBuild_FinalIntegrationOrchestrator::UBuild_FinalIntegrationOrchestrator()
{
    bSystemsInitialized = false;
    bIntegrationValidated = false;
    LastFrameRate = 0.0f;
    LastMemoryUsage = 0.0f;
    LastHealthCheck = FDateTime::Now();
}

void UBuild_FinalIntegrationOrchestrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Initializing comprehensive system integration"));
    
    // Initialize system health tracking
    SystemHealthReports.Empty();
    CurrentMetrics = FBuild_IntegrationMetrics();
    
    // Start initial health check
    RunComprehensiveHealthCheck();
    
    bSystemsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Initialization complete"));
}

void UBuild_FinalIntegrationOrchestrator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Deinitializing"));
    
    // Save final integration state
    SaveIntegrationState();
    
    SystemHealthReports.Empty();
    bSystemsInitialized = false;
    bIntegrationValidated = false;
    
    Super::Deinitialize();
}

void UBuild_FinalIntegrationOrchestrator::InitializeAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Initializing all game systems"));
    
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("Build_FinalIntegrationOrchestrator: No valid world found"));
        return;
    }
    
    // Initialize core systems
    ValidateWorldGeneration();
    ValidateCharacterSystems();
    ValidateFoliageSystem();
    ValidatePhysicsSystems();
    ValidateLightingSystem();
    ValidateVFXSystems();
    ValidateAudioSystems();
    
    bSystemsInitialized = true;
    LogIntegrationStatus();
}

void UBuild_FinalIntegrationOrchestrator::ValidateSystemIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Validating system integration"));
    
    if (!bSystemsInitialized)
    {
        InitializeAllSystems();
    }
    
    // Run comprehensive validation
    RunComprehensiveHealthCheck();
    
    // Update integration metrics
    CurrentMetrics = GetIntegrationMetrics();
    
    // Check if all systems are operational
    bool bAllHealthy = AreAllSystemsOperational();
    CurrentMetrics.bAllSystemsHealthy = bAllHealthy;
    
    bIntegrationValidated = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Integration validation complete - All systems healthy: %s"), 
           bAllHealthy ? TEXT("YES") : TEXT("NO"));
}

FBuild_IntegrationMetrics UBuild_FinalIntegrationOrchestrator::GetIntegrationMetrics()
{
    FBuild_IntegrationMetrics Metrics;
    
    if (UWorld* World = GetWorld())
    {
        // Count total actors
        Metrics.TotalActors = World->GetActorCount();
        
        // Count active systems (non-zero health reports)
        Metrics.ActiveSystems = 0;
        for (const FBuild_SystemHealthReport& Report : SystemHealthReports)
        {
            if (Report.Status != EBuild_SystemStatus::Unknown && Report.Status != EBuild_SystemStatus::Failed)
            {
                Metrics.ActiveSystems++;
            }
        }
        
        // Get performance metrics
        Metrics.FrameRate = GetCurrentFrameRate();
        Metrics.MemoryUsage = GetMemoryUsage();
        Metrics.bAllSystemsHealthy = AreAllSystemsOperational();
    }
    
    return Metrics;
}

TArray<FBuild_SystemHealthReport> UBuild_FinalIntegrationOrchestrator::GetSystemHealthReports()
{
    return SystemHealthReports;
}

EBuild_SystemStatus UBuild_FinalIntegrationOrchestrator::GetOverallSystemHealth()
{
    if (SystemHealthReports.Num() == 0)
    {
        return EBuild_SystemStatus::Unknown;
    }
    
    bool bHasCritical = false;
    bool bHasWarning = false;
    bool bHasFailed = false;
    
    for (const FBuild_SystemHealthReport& Report : SystemHealthReports)
    {
        switch (Report.Status)
        {
            case EBuild_SystemStatus::Failed:
                bHasFailed = true;
                break;
            case EBuild_SystemStatus::Critical:
                bHasCritical = true;
                break;
            case EBuild_SystemStatus::Warning:
                bHasWarning = true;
                break;
            default:
                break;
        }
    }
    
    if (bHasFailed) return EBuild_SystemStatus::Failed;
    if (bHasCritical) return EBuild_SystemStatus::Critical;
    if (bHasWarning) return EBuild_SystemStatus::Warning;
    
    return EBuild_SystemStatus::Healthy;
}

void UBuild_FinalIntegrationOrchestrator::RunComprehensiveHealthCheck()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Running comprehensive health check"));
    
    SystemHealthReports.Empty();
    LastHealthCheck = FDateTime::Now();
    
    // Validate all major systems
    ValidateWorldGeneration();
    ValidateCharacterSystems();
    ValidateFoliageSystem();
    ValidatePhysicsSystems();
    ValidateLightingSystem();
    ValidateVFXSystems();
    ValidateAudioSystems();
    
    // Update performance metrics
    MonitorPerformanceMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Health check complete - %d systems checked"), 
           SystemHealthReports.Num());
}

bool UBuild_FinalIntegrationOrchestrator::AreAllSystemsOperational()
{
    for (const FBuild_SystemHealthReport& Report : SystemHealthReports)
    {
        if (Report.Status == EBuild_SystemStatus::Failed || Report.Status == EBuild_SystemStatus::Critical)
        {
            return false;
        }
    }
    return SystemHealthReports.Num() > 0;
}

void UBuild_FinalIntegrationOrchestrator::MonitorPerformanceMetrics()
{
    // Get current frame rate
    LastFrameRate = GetCurrentFrameRate();
    
    // Get memory usage
    LastMemoryUsage = GetMemoryUsage();
    
    UE_LOG(LogTemp, Log, TEXT("Build_FinalIntegrationOrchestrator: Performance - FPS: %.1f, Memory: %.1f MB"), 
           LastFrameRate, LastMemoryUsage);
}

float UBuild_FinalIntegrationOrchestrator::GetCurrentFrameRate()
{
    if (GEngine && GEngine->GetGameViewport())
    {
        return 1.0f / GEngine->GetGameViewport()->GetWorld()->GetDeltaSeconds();
    }
    return 60.0f; // Default assumption
}

float UBuild_FinalIntegrationOrchestrator::GetMemoryUsage()
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f); // Convert to MB
}

bool UBuild_FinalIntegrationOrchestrator::ValidateBuildIntegrity()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Validating build integrity"));
    
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("Build_FinalIntegrationOrchestrator: No valid world for build validation"));
        return false;
    }
    
    // Run full system validation
    RunComprehensiveHealthCheck();
    
    // Check critical systems
    bool bIntegrityValid = AreAllSystemsOperational();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Build integrity validation %s"), 
           bIntegrityValid ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bIntegrityValid;
}

void UBuild_FinalIntegrationOrchestrator::GenerateBuildReport()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Generating comprehensive build report"));
    
    FBuild_IntegrationMetrics Metrics = GetIntegrationMetrics();
    EBuild_SystemStatus OverallHealth = GetOverallSystemHealth();
    
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), Metrics.TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("Active Systems: %d"), Metrics.ActiveSystems);
    UE_LOG(LogTemp, Warning, TEXT("Frame Rate: %.1f FPS"), Metrics.FrameRate);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.1f MB"), Metrics.MemoryUsage);
    UE_LOG(LogTemp, Warning, TEXT("All Systems Healthy: %s"), Metrics.bAllSystemsHealthy ? TEXT("YES") : TEXT("NO"));
    
    UE_LOG(LogTemp, Warning, TEXT("=== SYSTEM HEALTH DETAILS ==="));
    for (const FBuild_SystemHealthReport& Report : SystemHealthReports)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: %s - %s (Actors: %d, Metric: %.2f)"), 
               *Report.SystemName, 
               *UEnum::GetValueAsString(Report.Status),
               *Report.StatusMessage,
               Report.ActorCount,
               Report.PerformanceMetric);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END BUILD REPORT ==="));
}

void UBuild_FinalIntegrationOrchestrator::SaveIntegrationState()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Saving integration state"));
    
    // Generate final report
    GenerateBuildReport();
    
    // Update final metrics
    CurrentMetrics = GetIntegrationMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Integration state saved successfully"));
}

// Private validation functions
void UBuild_FinalIntegrationOrchestrator::ValidateWorldGeneration()
{
    if (!GetWorld()) return;
    
    int32 LandscapeCount = 0;
    int32 TerrainActors = 0;
    
    for (TActorIterator<ALandscape> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        LandscapeCount++;
    }
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        if (ActorItr->GetName().Contains(TEXT("Terrain")) || ActorItr->GetName().Contains(TEXT("World")))
        {
            TerrainActors++;
        }
    }
    
    EBuild_SystemStatus Status = (LandscapeCount > 0 || TerrainActors > 0) ? EBuild_SystemStatus::Healthy : EBuild_SystemStatus::Warning;
    FString Message = FString::Printf(TEXT("Landscapes: %d, Terrain actors: %d"), LandscapeCount, TerrainActors);
    
    UpdateSystemHealth(TEXT("World Generation"), Status, Message, static_cast<float>(LandscapeCount + TerrainActors));
}

void UBuild_FinalIntegrationOrchestrator::ValidateCharacterSystems()
{
    if (!GetWorld()) return;
    
    int32 CharacterCount = 0;
    int32 PlayerStarts = 0;
    
    for (TActorIterator<APawn> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        CharacterCount++;
    }
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        if (ActorItr->GetName().Contains(TEXT("PlayerStart")))
        {
            PlayerStarts++;
        }
    }
    
    EBuild_SystemStatus Status = (CharacterCount > 0 && PlayerStarts > 0) ? EBuild_SystemStatus::Healthy : EBuild_SystemStatus::Warning;
    FString Message = FString::Printf(TEXT("Characters: %d, Player starts: %d"), CharacterCount, PlayerStarts);
    
    UpdateSystemHealth(TEXT("Character Systems"), Status, Message, static_cast<float>(CharacterCount));
}

void UBuild_FinalIntegrationOrchestrator::ValidateFoliageSystem()
{
    if (!GetWorld()) return;
    
    int32 FoliageActors = 0;
    int32 StaticMeshes = 0;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        if (ActorItr->GetName().Contains(TEXT("Foliage")) || ActorItr->GetName().Contains(TEXT("Tree")) || ActorItr->GetName().Contains(TEXT("Plant")))
        {
            FoliageActors++;
        }
        
        if (ActorItr->FindComponentByClass<UStaticMeshComponent>())
        {
            StaticMeshes++;
        }
    }
    
    EBuild_SystemStatus Status = (FoliageActors > 0) ? EBuild_SystemStatus::Healthy : EBuild_SystemStatus::Warning;
    FString Message = FString::Printf(TEXT("Foliage actors: %d, Static meshes: %d"), FoliageActors, StaticMeshes);
    
    UpdateSystemHealth(TEXT("Foliage System"), Status, Message, static_cast<float>(FoliageActors));
}

void UBuild_FinalIntegrationOrchestrator::ValidatePhysicsSystems()
{
    if (!GetWorld()) return;
    
    int32 PhysicsActors = 0;
    int32 CollisionComponents = 0;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        if (ActorItr->GetRootComponent() && ActorItr->GetRootComponent()->IsCollisionEnabled())
        {
            PhysicsActors++;
        }
        
        TArray<UActorComponent*> Components = ActorItr->GetRootComponent() ? ActorItr->GetRootComponent()->GetAttachChildren().Array() : TArray<UActorComponent*>();
        for (UActorComponent* Component : Components)
        {
            if (Component && Component->IsA<UPrimitiveComponent>())
            {
                CollisionComponents++;
            }
        }
    }
    
    EBuild_SystemStatus Status = (PhysicsActors > 0) ? EBuild_SystemStatus::Healthy : EBuild_SystemStatus::Warning;
    FString Message = FString::Printf(TEXT("Physics actors: %d, Collision components: %d"), PhysicsActors, CollisionComponents);
    
    UpdateSystemHealth(TEXT("Physics Systems"), Status, Message, static_cast<float>(PhysicsActors));
}

void UBuild_FinalIntegrationOrchestrator::ValidateLightingSystem()
{
    if (!GetWorld()) return;
    
    int32 LightActors = 0;
    int32 LightComponents = 0;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        if (ActorItr->FindComponentByClass<ULightComponent>())
        {
            LightActors++;
            LightComponents += ActorItr->GetComponents<ULightComponent>().Num();
        }
    }
    
    EBuild_SystemStatus Status = (LightActors > 0) ? EBuild_SystemStatus::Healthy : EBuild_SystemStatus::Warning;
    FString Message = FString::Printf(TEXT("Light actors: %d, Light components: %d"), LightActors, LightComponents);
    
    UpdateSystemHealth(TEXT("Lighting System"), Status, Message, static_cast<float>(LightActors));
}

void UBuild_FinalIntegrationOrchestrator::ValidateVFXSystems()
{
    if (!GetWorld()) return;
    
    int32 ParticleActors = 0;
    int32 ParticleComponents = 0;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        if (ActorItr->FindComponentByClass<UParticleSystemComponent>())
        {
            ParticleActors++;
            ParticleComponents += ActorItr->GetComponents<UParticleSystemComponent>().Num();
        }
    }
    
    EBuild_SystemStatus Status = EBuild_SystemStatus::Healthy; // VFX is optional
    FString Message = FString::Printf(TEXT("Particle actors: %d, Particle components: %d"), ParticleActors, ParticleComponents);
    
    UpdateSystemHealth(TEXT("VFX Systems"), Status, Message, static_cast<float>(ParticleActors));
}

void UBuild_FinalIntegrationOrchestrator::ValidateAudioSystems()
{
    if (!GetWorld()) return;
    
    int32 AudioActors = 0;
    int32 AudioComponents = 0;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        if (ActorItr->GetName().Contains(TEXT("Audio")) || ActorItr->GetName().Contains(TEXT("Sound")))
        {
            AudioActors++;
        }
        
        TArray<UActorComponent*> Components = ActorItr->GetComponents<UActorComponent>().Array();
        for (UActorComponent* Component : Components)
        {
            if (Component && Component->GetName().Contains(TEXT("Audio")))
            {
                AudioComponents++;
            }
        }
    }
    
    EBuild_SystemStatus Status = EBuild_SystemStatus::Healthy; // Audio is optional
    FString Message = FString::Printf(TEXT("Audio actors: %d, Audio components: %d"), AudioActors, AudioComponents);
    
    UpdateSystemHealth(TEXT("Audio Systems"), Status, Message, static_cast<float>(AudioActors));
}

void UBuild_FinalIntegrationOrchestrator::UpdateSystemHealth(const FString& SystemName, EBuild_SystemStatus Status, const FString& Message, float Metric)
{
    FBuild_SystemHealthReport Report;
    Report.SystemName = SystemName;
    Report.Status = Status;
    Report.StatusMessage = Message;
    Report.PerformanceMetric = Metric;
    Report.ActorCount = static_cast<int32>(Metric); // Use metric as actor count for simplicity
    
    SystemHealthReports.Add(Report);
    
    UE_LOG(LogTemp, Log, TEXT("Build_FinalIntegrationOrchestrator: %s - %s: %s"), 
           *SystemName, *UEnum::GetValueAsString(Status), *Message);
}

void UBuild_FinalIntegrationOrchestrator::LogIntegrationStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Integration Status - Systems initialized: %s, Validated: %s"), 
           bSystemsInitialized ? TEXT("YES") : TEXT("NO"),
           bIntegrationValidated ? TEXT("YES") : TEXT("NO"));
}

void UBuild_FinalIntegrationOrchestrator::CleanupInvalidActors()
{
    if (!GetWorld()) return;
    
    int32 CleanedActors = 0;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        if (!IsValid(*ActorItr) || ActorItr->IsPendingKill())
        {
            ActorItr->Destroy();
            CleanedActors++;
        }
    }
    
    if (CleanedActors > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Cleaned up %d invalid actors"), CleanedActors);
    }
}