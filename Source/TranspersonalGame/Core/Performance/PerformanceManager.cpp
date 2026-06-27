// PerformanceManager.cpp
// Agent #04 — Performance Optimizer
// Enforces 60fps PC / 30fps console frame budget.
// Controls LOD distances, tick intervals, shadow quality, texture streaming.

#include "PerformanceManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

APerformanceManager::APerformanceManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Self-ticks at 2Hz — low overhead
}

void APerformanceManager::BeginPlay()
{
    Super::BeginPlay();
    ApplyQualityTier(QualityTier);
    ApplyConsoleCommands();
    UE_LOG(LogTemp, Log, TEXT("[PerformanceManager] Initialized. Target: %.0f FPS. Tier: %d"),
           TargetFPS_PC, (int32)QualityTier);
}

void APerformanceManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateFrameStats(DeltaTime);

    TimeSinceLastTickUpdate += DeltaTime;
    if (TimeSinceLastTickUpdate >= TickUpdateInterval)
    {
        TimeSinceLastTickUpdate = 0.0f;
        UpdateTickIntervalsForAllActors();
    }
}

// ============================================================
// UpdateFrameStats — rolling average FPS
// ============================================================
void APerformanceManager::UpdateFrameStats(float DeltaTime)
{
    FrameTimeAccumulator += DeltaTime * 1000.0f; // ms
    FrameCount++;

    if (FrameCount >= 60)
    {
        FrameStats.LastFrameTimeMs = FrameTimeAccumulator / FrameCount;
        FrameStats.AverageFPS = (FrameStats.LastFrameTimeMs > 0.0f)
            ? (1000.0f / FrameStats.LastFrameTimeMs)
            : 0.0f;
        FrameTimeAccumulator = 0.0f;
        FrameCount = 0;

        if (!IsFrameBudgetHealthy())
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[PerformanceManager] Frame budget exceeded! FPS=%.1f (target=%.1f). "
                     "Active dinos=%d, crowd=%d"),
                FrameStats.AverageFPS, TargetFPS_PC,
                FrameStats.ActiveDinoCount, FrameStats.ActiveCrowdCount);
        }
    }
}

// ============================================================
// UpdateTickIntervalsForAllActors — LOD-based tick throttling
// ============================================================
void APerformanceManager::UpdateTickIntervalsForAllActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Get player location for distance checks
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;

    int32 DinoCount = 0;
    int32 NearCrowd = 0;
    int32 MidCrowd = 0;
    int32 DormantCrowd = 0;

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor || Actor == this) continue;

        FString ClassName = Actor->GetClass()->GetName();
        bool bIsDino = ClassName.Contains(TEXT("Dino")) || ClassName.Contains(TEXT("TRex"))
                    || ClassName.Contains(TEXT("Raptor")) || ClassName.Contains(TEXT("Brach"))
                    || ClassName.Contains(TEXT("Tyrann")) || ClassName.Contains(TEXT("Veloci"));
        bool bIsCrowd = ClassName.Contains(TEXT("Crowd")) || ClassName.Contains(TEXT("NPC"))
                     || ClassName.Contains(TEXT("Primitive"));

        if (bIsDino)
        {
            Actor->SetActorTickInterval(TickBudget.DinosaurTickInterval);
            DinoCount++;
        }
        else if (bIsCrowd && PlayerPawn)
        {
            float Dist = GetDistanceToPlayer(Actor);
            if (Dist < TickBudget.NearDistanceThreshold)
            {
                Actor->SetActorTickEnabled(true);
                Actor->SetActorTickInterval(TickBudget.CrowdNearTickInterval);
                NearCrowd++;
            }
            else if (Dist < TickBudget.FarDistanceThreshold)
            {
                Actor->SetActorTickEnabled(true);
                Actor->SetActorTickInterval(TickBudget.CrowdMidTickInterval);
                MidCrowd++;
            }
            else
            {
                Actor->SetActorTickEnabled(false); // Dormant
                DormantCrowd++;
            }
        }
    }

    FrameStats.ActiveDinoCount = DinoCount;
    FrameStats.ActiveCrowdCount = NearCrowd + MidCrowd;
    FrameStats.DormantCrowdCount = DormantCrowd;
}

// ============================================================
// ApplyQualityTier
// ============================================================
void APerformanceManager::ApplyQualityTier(EPerf_QualityTier NewTier)
{
    QualityTier = NewTier;
    switch (NewTier)
    {
        case EPerf_QualityTier::Low:    ApplyQualityPreset_Low();    break;
        case EPerf_QualityTier::Medium: ApplyQualityPreset_Medium(); break;
        case EPerf_QualityTier::High:   ApplyQualityPreset_High();   break;
        case EPerf_QualityTier::Ultra:  ApplyQualityPreset_Ultra();  break;
    }
    ApplyConsoleCommands();
}

void APerformanceManager::ApplyQualityPreset_Low()
{
    StaticMeshLODScale    = 0.7f;
    SkeletalMeshLODScale  = 0.7f;
    MaxShadowResolution   = 512;
    TextureStreamingPoolMB = 512;
    TickBudget.DinosaurTickInterval  = 0.1f;  // 10Hz
    TickBudget.CrowdNearTickInterval = 0.2f;  // 5Hz
    TickBudget.CrowdMidTickInterval  = 1.0f;  // 1Hz
    TickBudget.NearDistanceThreshold = 300.0f;
    TickBudget.FarDistanceThreshold  = 1000.0f;
}

void APerformanceManager::ApplyQualityPreset_Medium()
{
    StaticMeshLODScale    = 1.0f;
    SkeletalMeshLODScale  = 1.0f;
    MaxShadowResolution   = 1024;
    TextureStreamingPoolMB = 768;
    TickBudget.DinosaurTickInterval  = 0.05f; // 20Hz
    TickBudget.CrowdNearTickInterval = 0.1f;  // 10Hz
    TickBudget.CrowdMidTickInterval  = 0.5f;  // 2Hz
    TickBudget.NearDistanceThreshold = 500.0f;
    TickBudget.FarDistanceThreshold  = 1500.0f;
}

void APerformanceManager::ApplyQualityPreset_High()
{
    StaticMeshLODScale    = 1.0f;
    SkeletalMeshLODScale  = 1.0f;
    MaxShadowResolution   = 2048;
    TextureStreamingPoolMB = 1024;
    TickBudget.DinosaurTickInterval  = 0.05f; // 20Hz
    TickBudget.CrowdNearTickInterval = 0.1f;  // 10Hz
    TickBudget.CrowdMidTickInterval  = 0.5f;  // 2Hz
    TickBudget.NearDistanceThreshold = 500.0f;
    TickBudget.FarDistanceThreshold  = 2000.0f;
}

void APerformanceManager::ApplyQualityPreset_Ultra()
{
    StaticMeshLODScale    = 1.5f;
    SkeletalMeshLODScale  = 1.5f;
    MaxShadowResolution   = 4096;
    TextureStreamingPoolMB = 2048;
    TickBudget.DinosaurTickInterval  = 0.033f; // 30Hz
    TickBudget.CrowdNearTickInterval = 0.05f;  // 20Hz
    TickBudget.CrowdMidTickInterval  = 0.2f;   // 5Hz
    TickBudget.NearDistanceThreshold = 800.0f;
    TickBudget.FarDistanceThreshold  = 3000.0f;
}

// ============================================================
// ApplyConsoleCommands — push all settings to UE5 renderer
// ============================================================
void APerformanceManager::ApplyConsoleCommands()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // LOD
    UKismetSystemLibrary::ExecuteConsoleCommand(World,
        FString::Printf(TEXT("r.StaticMeshLODDistanceScale %.2f"), StaticMeshLODScale));
    UKismetSystemLibrary::ExecuteConsoleCommand(World,
        FString::Printf(TEXT("r.SkeletalMeshLODDistanceScale %.2f"), SkeletalMeshLODScale));

    // Shadows
    UKismetSystemLibrary::ExecuteConsoleCommand(World,
        FString::Printf(TEXT("r.Shadow.MaxResolution %d"), MaxShadowResolution));
    UKismetSystemLibrary::ExecuteConsoleCommand(World,
        TEXT("r.Shadow.RadiusThreshold 0.03"));

    // Texture streaming
    UKismetSystemLibrary::ExecuteConsoleCommand(World,
        FString::Printf(TEXT("r.Streaming.PoolSize %d"), TextureStreamingPoolMB));

    // Occlusion
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.HZBOcclusion 1"));

    // Sky performance
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.SkyAtmosphere.FastSkyLUT 1"));

    // Lumen
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.Reflections.Allow 1"));
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.DynamicGlobalIlluminationMethod 1"));

    // Niagara GPU particle budget
    UKismetSystemLibrary::ExecuteConsoleCommand(World,
        TEXT("fx.Niagara.MaxGPUParticlesSpawnPerFrame 2000"));

    UE_LOG(LogTemp, Log, TEXT("[PerformanceManager] Console commands applied. "
        "LOD=%.1f Shadow=%d Streaming=%dMB"),
        StaticMeshLODScale, MaxShadowResolution, TextureStreamingPoolMB);
}

// ============================================================
// Public API
// ============================================================
void APerformanceManager::SetTargetFPS(float NewTargetFPS)
{
    TargetFPS_PC = FMath::Clamp(NewTargetFPS, 20.0f, 144.0f);
    UE_LOG(LogTemp, Log, TEXT("[PerformanceManager] Target FPS set to %.0f"), TargetFPS_PC);
}

FPerf_FrameStats APerformanceManager::GetFrameStats() const
{
    return FrameStats;
}

bool APerformanceManager::IsFrameBudgetHealthy() const
{
    if (FrameStats.AverageFPS <= 0.0f) return true; // Not enough data yet
    return FrameStats.AverageFPS >= (TargetFPS_PC * 0.9f); // 10% tolerance
}

float APerformanceManager::GetDistanceToPlayer(AActor* Actor) const
{
    if (!Actor) return 0.0f;
    UWorld* World = GetWorld();
    if (!World) return 0.0f;
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
    if (!PlayerPawn) return 0.0f;
    return FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
}
