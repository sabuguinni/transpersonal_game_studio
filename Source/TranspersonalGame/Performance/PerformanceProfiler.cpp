// PerformanceProfiler.cpp
// Implementação do sistema de profiling de performance

#include "PerformanceProfiler.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

DEFINE_LOG_CATEGORY(LogTranspersonalPerformance);

UPerformanceProfiler::UPerformanceProfiler()
    : bIsProfilingActive(false)
    , bAutoPerformanceAdjustment(true)
    , ProfilingStartTime(0.0f)
    , CurrentPerformanceLevel(EPerformanceLevel::Auto)
    , LastPerformanceCheckTime(0.0f)
    , bHardwareDetected(false)
    , EstimatedGPUPerformance(5)
    , EstimatedCPUPerformance(5)
    , AvailableMemoryGB(8.0f)
{
    FrameTimeHistory.Reserve(MaxHistorySize);
    FPSHistory.Reserve(MaxHistorySize);
}

void UPerformanceProfiler::Initialize()
{
    UE_LOG(LogTranspersonalPerformance, Log, TEXT("Initializing Performance Profiler"));
    
    DetectHardwareCapabilities();
    StartProfiling();
    
    // Configurar callbacks de engine se necessário
    if (GEngine)
    {
        // Registrar para receber notificações de frame
    }
}

void UPerformanceProfiler::Shutdown()
{
    UE_LOG(LogTranspersonalPerformance, Log, TEXT("Shutting down Performance Profiler"));
    StopProfiling();
}

void UPerformanceProfiler::UpdateMetrics(float DeltaTime)
{
    if (!bIsProfilingActive)
        return;

    CollectFrameMetrics(DeltaTime);
    CollectRenderMetrics();
    CollectMemoryMetrics();
    CollectPhysicsMetrics();

    // Verificar se é hora de atualizar o nível de performance
    float CurrentTime = FPlatformTime::Seconds();
    if (CurrentTime - LastPerformanceCheckTime > PerformanceCheckInterval)
    {
        UpdatePerformanceLevel();
        LastPerformanceCheckTime = CurrentTime;
    }
}

void UPerformanceProfiler::CollectFrameMetrics(float DeltaTime)
{
    CurrentMetrics.FrameTime = DeltaTime * 1000.0f; // Converter para ms
    CurrentMetrics.FPS = (DeltaTime > 0.0f) ? (1.0f / DeltaTime) : 0.0f;

    // Adicionar ao histórico
    AddToHistory(FrameTimeHistory, CurrentMetrics.FrameTime);
    AddToHistory(FPSHistory, CurrentMetrics.FPS);

    // Coletar tempos de thread se disponível
#if STATS
    if (FThreadStats::IsCollectingData())
    {
        // Tentar obter estatísticas de thread
        CurrentMetrics.GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
        CurrentMetrics.RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
    }
#endif
}

void UPerformanceProfiler::CollectRenderMetrics()
{
    if (GEngine && GEngine->GetWorld())
    {
        // Coletar estatísticas de rendering
        UWorld* World = GEngine->GetWorld();
        if (World)
        {
            // Contar atores ativos
            CurrentMetrics.ActiveActors = World->GetActorCount();
        }
    }

    // Tentar obter estatísticas de GPU
#if STATS
    if (FThreadStats::IsCollectingData())
    {
        // Estatísticas de draw calls e triângulos
        // Nota: Estas são aproximações baseadas em estatísticas disponíveis
        CurrentMetrics.DrawCalls = 0; // Seria obtido de RHI stats
        CurrentMetrics.Triangles = 0; // Seria obtido de RHI stats
        CurrentMetrics.GPUTime = 0.0f; // Seria obtido de GPU timing
    }
#endif
}

void UPerformanceProfiler::CollectMemoryMetrics()
{
    // Obter uso de memória do sistema
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    // Converter para MB
    float UsedMemoryMB = static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f);
    float TotalMemoryMB = static_cast<float>(MemStats.TotalPhysical) / (1024.0f * 1024.0f);
    
    CurrentMetrics.MemoryUsage = (TotalMemoryMB > 0.0f) ? (UsedMemoryMB / TotalMemoryMB) : 0.0f;
    
    // Log warning se memória estiver alta
    if (CurrentMetrics.MemoryUsage > MemoryWarningThreshold)
    {
        UE_LOG(LogTranspersonalPerformance, Warning, 
               TEXT("High memory usage detected: %.1f%% (%.1f MB / %.1f MB)"),
               CurrentMetrics.MemoryUsage * 100.0f, UsedMemoryMB, TotalMemoryMB);
    }
}

void UPerformanceProfiler::CollectPhysicsMetrics()
{
    // Coletar tempo de física se disponível
    CurrentMetrics.PhysicsTime = 0.0f;
    
#if STATS
    if (FThreadStats::IsCollectingData())
    {
        // Tentar obter tempo de simulação de física
        // Isto seria implementado com acesso às estatísticas do Chaos Physics
    }
#endif
}

void UPerformanceProfiler::StartProfiling()
{
    if (bIsProfilingActive)
        return;

    bIsProfilingActive = true;
    ProfilingStartTime = FPlatformTime::Seconds();
    LastPerformanceCheckTime = ProfilingStartTime;
    
    // Limpar histórico
    FrameTimeHistory.Empty();
    FPSHistory.Empty();
    
    UE_LOG(LogTranspersonalPerformance, Log, TEXT("Performance profiling started"));
}

void UPerformanceProfiler::StopProfiling()
{
    if (!bIsProfilingActive)
        return;

    bIsProfilingActive = false;
    
    float ProfilingDuration = FPlatformTime::Seconds() - ProfilingStartTime;
    UE_LOG(LogTranspersonalPerformance, Log, 
           TEXT("Performance profiling stopped. Duration: %.2f seconds"), ProfilingDuration);
}

float UPerformanceProfiler::GetAverageFrameTime() const
{
    return GetHistoryAverage(FrameTimeHistory);
}

float UPerformanceProfiler::GetAverageFPS() const
{
    return GetHistoryAverage(FPSHistory);
}

EPerformanceLevel UPerformanceProfiler::GetRecommendedPerformanceLevel() const
{
    float AvgFPS = GetAverageFPS();
    
    if (AvgFPS >= HighPerformanceThreshold)
    {
        return EPerformanceLevel::Ultra;
    }
    else if (AvgFPS >= TargetFPS)
    {
        return EPerformanceLevel::High;
    }
    else if (AvgFPS >= LowPerformanceThreshold)
    {
        return EPerformanceLevel::Medium;
    }
    else
    {
        return EPerformanceLevel::Low;
    }
}

void UPerformanceProfiler::SetAutoPerformanceAdjustment(bool bEnabled)
{
    bAutoPerformanceAdjustment = bEnabled;
    UE_LOG(LogTranspersonalPerformance, Log, 
           TEXT("Auto performance adjustment %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPerformanceProfiler::UpdatePerformanceLevel()
{
    if (!bAutoPerformanceAdjustment)
        return;

    EPerformanceLevel RecommendedLevel = GetRecommendedPerformanceLevel();
    
    if (RecommendedLevel != CurrentPerformanceLevel)
    {
        EPerformanceLevel OldLevel = CurrentPerformanceLevel;
        CurrentPerformanceLevel = RecommendedLevel;
        
        UE_LOG(LogTranspersonalPerformance, Log, 
               TEXT("Performance level changed from %d to %d (Avg FPS: %.1f)"),
               static_cast<int32>(OldLevel), static_cast<int32>(RecommendedLevel), GetAverageFPS());
        
        // Broadcast do evento
        OnPerformanceLevelChanged.Broadcast(CurrentPerformanceLevel);
    }
}

void UPerformanceProfiler::AddToHistory(TArray<float>& History, float Value)
{
    History.Add(Value);
    
    // Manter tamanho máximo do histórico
    if (History.Num() > MaxHistorySize)
    {
        History.RemoveAt(0);
    }
}

float UPerformanceProfiler::GetHistoryAverage(const TArray<float>& History) const
{
    if (History.Num() == 0)
        return 0.0f;

    float Sum = 0.0f;
    for (float Value : History)
    {
        Sum += Value;
    }
    
    return Sum / static_cast<float>(History.Num());
}

void UPerformanceProfiler::DetectHardwareCapabilities()
{
    if (bHardwareDetected)
        return;

    // Detectar capacidades básicas do hardware
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    AvailableMemoryGB = static_cast<float>(MemStats.TotalPhysical) / (1024.0f * 1024.0f * 1024.0f);
    
    // Estimativa básica baseada na memória disponível
    if (AvailableMemoryGB >= 16.0f)
    {
        EstimatedCPUPerformance = 8;
        EstimatedGPUPerformance = 7;
    }
    else if (AvailableMemoryGB >= 8.0f)
    {
        EstimatedCPUPerformance = 6;
        EstimatedGPUPerformance = 5;
    }
    else
    {
        EstimatedCPUPerformance = 4;
        EstimatedGPUPerformance = 3;
    }
    
    bHardwareDetected = true;
    
    UE_LOG(LogTranspersonalPerformance, Log, 
           TEXT("Hardware detected - Memory: %.1f GB, Est. CPU: %d/10, Est. GPU: %d/10"),
           AvailableMemoryGB, EstimatedCPUPerformance, EstimatedGPUPerformance);
}