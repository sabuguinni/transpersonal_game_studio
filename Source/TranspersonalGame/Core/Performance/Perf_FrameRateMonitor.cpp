#include "Perf_FrameRateMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

void UPerf_FrameRateMonitor::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize frame rate monitoring
    FrameSamples.Reserve(MaxSamples);
    ResetStatistics();
    
    // Set up tick delegate
    TickDelegate = FTickerDelegate::CreateUObject(this, &UPerf_FrameRateMonitor::Tick);
    TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(TickDelegate, SampleInterval);
    
    bInitialized = true;
    
    UE_LOG(LogTemp, Log, TEXT("Performance Frame Rate Monitor Initialized - Target FPS: %.1f"), TargetFrameRate);
}

void UPerf_FrameRateMonitor::Deinitialize()
{
    if (TickDelegateHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
    }
    
    bInitialized = false;
    Super::Deinitialize();
}

FPerf_FrameData UPerf_FrameRateMonitor::GetCurrentFrameData() const
{
    FPerf_FrameData FrameData;
    
    if (GEngine && GEngine->GetGameViewport())
    {
        // Get current frame rate
        FrameData.FrameRate = CalculateCurrentFrameRate();
        FrameData.FrameTime = 1.0f / FMath::Max(FrameData.FrameRate, 0.001f);
        
        // Get rendering stats (approximated)
        FrameData.GameThreadTime = GGameThreadTime;
        FrameData.RenderThreadTime = GRenderThreadTime;
        FrameData.GPUTime = GGPUFrameTime;
        
        // Get draw call stats (estimated based on frame rate)
        if (FrameData.FrameRate > 0)
        {
            FrameData.DrawCalls = FMath::RoundToInt(1000.0f * (60.0f / FMath::Max(FrameData.FrameRate, 1.0f)));
            FrameData.Triangles = FrameData.DrawCalls * 100; // Rough estimate
        }
    }
    
    return FrameData;
}

void UPerf_FrameRateMonitor::ResetStatistics()
{
    FrameSamples.Empty();
    AverageFrameRate = 0.0f;
    MinFrameRate = 0.0f;
    MaxFrameRate = 0.0f;
    LastSampleTime = 0.0f;
}

bool UPerf_FrameRateMonitor::Tick(float DeltaTime)
{
    if (!bInitialized)
    {
        return true;
    }
    
    float CurrentTime = FPlatformTime::Seconds();
    if (CurrentTime - LastSampleTime >= SampleInterval)
    {
        float CurrentFrameRate = CalculateCurrentFrameRate();
        AddFrameSample(CurrentFrameRate);
        UpdateFrameStatistics();
        CheckPerformanceAlerts();
        
        LastSampleTime = CurrentTime;
    }
    
    return true; // Continue ticking
}

void UPerf_FrameRateMonitor::UpdateFrameStatistics()
{
    if (FrameSamples.Num() == 0)
    {
        return;
    }
    
    // Calculate average
    float Sum = 0.0f;
    MinFrameRate = FrameSamples[0];
    MaxFrameRate = FrameSamples[0];
    
    for (float Sample : FrameSamples)
    {
        Sum += Sample;
        MinFrameRate = FMath::Min(MinFrameRate, Sample);
        MaxFrameRate = FMath::Max(MaxFrameRate, Sample);
    }
    
    AverageFrameRate = Sum / FrameSamples.Num();
}

void UPerf_FrameRateMonitor::CheckPerformanceAlerts()
{
    if (!bPerformanceAlertsEnabled)
    {
        return;
    }
    
    float CurrentFPS = CalculateCurrentFrameRate();
    float AlertThreshold = TargetFrameRate * PerformanceAlertThreshold;
    
    if (CurrentFPS < AlertThreshold)
    {
        OnPerformanceAlert.Broadcast(CurrentFPS);
        
        UE_LOG(LogTemp, Warning, TEXT("Performance Alert: FPS dropped to %.1f (Target: %.1f, Threshold: %.1f)"), 
               CurrentFPS, TargetFrameRate, AlertThreshold);
    }
}

void UPerf_FrameRateMonitor::AddFrameSample(float FrameRate)
{
    FrameSamples.Add(FrameRate);
    
    // Remove old samples if we exceed the maximum
    if (FrameSamples.Num() > MaxSamples)
    {
        FrameSamples.RemoveAt(0);
    }
}

float UPerf_FrameRateMonitor::CalculateCurrentFrameRate() const
{
    if (GEngine && GEngine->GetGameViewport())
    {
        UWorld* World = GEngine->GetGameViewport()->GetWorld();
        if (World)
        {
            float DeltaTime = World->GetDeltaSeconds();
            if (DeltaTime > 0.0f)
            {
                return 1.0f / DeltaTime;
            }
        }
    }
    
    // Fallback to engine stats
    return FMath::Clamp(1.0f / FApp::GetDeltaTime(), 1.0f, 200.0f);
}