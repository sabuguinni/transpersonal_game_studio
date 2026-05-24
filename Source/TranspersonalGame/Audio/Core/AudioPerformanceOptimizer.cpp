#include "AudioPerformanceOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Engine/GameViewportClient.h"
#include "AudioDevice.h"
#include "HAL/PlatformFilemanager.h"

UAudioPerformanceOptimizer::UAudioPerformanceOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default LOD settings
    LODSettings.HighQualityDistance = 500.0f;
    LODSettings.MediumQualityDistance = 1500.0f;
    LODSettings.LowQualityDistance = 3000.0f;
    LODSettings.CullingDistance = 5000.0f;
    LODSettings.MaxSimultaneousAudioSources = 64;
    LODSettings.MaxStreamingAudioSources = 16;
    
    TargetFrameRate = 60;
    PerformanceUpdateInterval = 0.1f;
    MaxFrameTimeHistorySize = 60;
    
    // Initialize frame time history
    FrameTimeHistory.Reserve(MaxFrameTimeHistorySize);
}

void UAudioPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("AudioPerformanceOptimizer: Initialized with target %d FPS"), TargetFrameRate);
    
    // Pre-allocate frame time history
    FrameTimeHistory.SetNum(MaxFrameTimeHistorySize);
    for (int32 i = 0; i < MaxFrameTimeHistorySize; i++)
    {
        FrameTimeHistory[i] = 1.0f / TargetFrameRate; // Initialize with target frame time
    }
}

void UAudioPerformanceOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update frame time history
    if (FrameTimeHistory.Num() >= MaxFrameTimeHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    FrameTimeHistory.Add(DeltaTime);
    
    // Update performance metrics periodically
    LastPerformanceUpdate += DeltaTime;
    if (LastPerformanceUpdate >= PerformanceUpdateInterval)
    {
        UpdatePerformanceMetrics();
        ApplyLODOptimizations();
        CullDistantAudioSources();
        ManageAudioStreaming();
        LastPerformanceUpdate = 0.0f;
    }
}

FAudio_PerformanceMetrics UAudioPerformanceOptimizer::GetCurrentPerformanceMetrics() const
{
    return CurrentMetrics;
}

void UAudioPerformanceOptimizer::SetLODSettings(const FAudio_LODSettings& NewSettings)
{
    LODSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("AudioPerformanceOptimizer: Updated LOD settings"));
}

EAudio_LODLevel UAudioPerformanceOptimizer::CalculateAudioLOD(const FVector& ListenerLocation, const FVector& AudioSourceLocation) const
{
    float Distance = FVector::Dist(ListenerLocation, AudioSourceLocation);
    
    if (Distance <= LODSettings.HighQualityDistance)
    {
        return EAudio_LODLevel::High;
    }
    else if (Distance <= LODSettings.MediumQualityDistance)
    {
        return EAudio_LODLevel::Medium;
    }
    else if (Distance <= LODSettings.LowQualityDistance)
    {
        return EAudio_LODLevel::Low;
    }
    else
    {
        return EAudio_LODLevel::Disabled;
    }
}

void UAudioPerformanceOptimizer::RegisterAudioComponent(UAudioComponent* AudioComponent)
{
    if (AudioComponent && !RegisteredAudioComponents.Contains(AudioComponent))
    {
        RegisteredAudioComponents.Add(AudioComponent);
        UE_LOG(LogTemp, VeryVerbose, TEXT("AudioPerformanceOptimizer: Registered audio component %s"), 
               *AudioComponent->GetName());
    }
}

void UAudioPerformanceOptimizer::UnregisterAudioComponent(UAudioComponent* AudioComponent)
{
    if (AudioComponent)
    {
        RegisteredAudioComponents.Remove(AudioComponent);
        UE_LOG(LogTemp, VeryVerbose, TEXT("AudioPerformanceOptimizer: Unregistered audio component %s"), 
               *AudioComponent->GetName());
    }
}

void UAudioPerformanceOptimizer::OptimizeAudioSources()
{
    FVector ListenerLocation = GetListenerLocation();
    int32 ActiveSources = 0;
    int32 CulledSources = 0;
    
    // Clean up invalid weak pointers
    RegisteredAudioComponents.RemoveAll([](const TWeakObjectPtr<UAudioComponent>& WeakPtr) {
        return !WeakPtr.IsValid();
    });
    
    for (const TWeakObjectPtr<UAudioComponent>& WeakAudioComp : RegisteredAudioComponents)
    {
        if (UAudioComponent* AudioComp = WeakAudioComp.Get())
        {
            if (AudioComp->IsPlaying())
            {
                FVector AudioLocation = AudioComp->GetComponentLocation();
                EAudio_LODLevel LODLevel = CalculateAudioLOD(ListenerLocation, AudioLocation);
                
                if (LODLevel == EAudio_LODLevel::Disabled)
                {
                    AudioComp->Stop();
                    CulledSources++;
                }
                else
                {
                    OptimizeAudioComponentSettings(AudioComp, LODLevel);
                    ActiveSources++;
                }
            }
        }
    }
    
    CurrentMetrics.ActiveAudioSources = ActiveSources;
    CurrentMetrics.CulledAudioSources = CulledSources;
}

void UAudioPerformanceOptimizer::EnableAudioStreaming(UAudioComponent* AudioComponent, bool bEnable)
{
    if (AudioComponent && AudioComponent->GetSound())
    {
        // Note: In a full implementation, this would interface with UE5's audio streaming system
        // For now, we'll adjust the component's settings to simulate streaming behavior
        if (bEnable)
        {
            AudioComponent->SetVolumeMultiplier(0.8f); // Slightly reduce volume for streaming
        }
        else
        {
            AudioComponent->SetVolumeMultiplier(1.0f); // Full volume for non-streaming
        }
    }
}

void UAudioPerformanceOptimizer::PreloadCriticalAudioAssets()
{
    UE_LOG(LogTemp, Log, TEXT("AudioPerformanceOptimizer: Preloading critical audio assets"));
    
    // In a full implementation, this would preload essential audio assets
    // For now, we'll simulate by ensuring registered components are ready
    for (const TWeakObjectPtr<UAudioComponent>& WeakAudioComp : RegisteredAudioComponents)
    {
        if (UAudioComponent* AudioComp = WeakAudioComp.Get())
        {
            if (AudioComp->GetSound())
            {
                // Simulate preloading by accessing the sound object
                USoundBase* Sound = AudioComp->GetSound();
                if (Sound)
                {
                    UE_LOG(LogTemp, VeryVerbose, TEXT("Preloaded: %s"), *Sound->GetName());
                }
            }
        }
    }
}

void UAudioPerformanceOptimizer::RunPerformanceProfiler()
{
    UE_LOG(LogTemp, Warning, TEXT("=== AUDIO PERFORMANCE PROFILER ==="));
    UE_LOG(LogTemp, Warning, TEXT("Active Audio Sources: %d"), CurrentMetrics.ActiveAudioSources);
    UE_LOG(LogTemp, Warning, TEXT("Streaming Audio Sources: %d"), CurrentMetrics.StreamingAudioSources);
    UE_LOG(LogTemp, Warning, TEXT("Culled Audio Sources: %d"), CurrentMetrics.CulledAudioSources);
    UE_LOG(LogTemp, Warning, TEXT("Audio Memory Usage: %.2f MB"), CurrentMetrics.AudioMemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("CPU Usage: %.2f%%"), CurrentMetrics.CPUUsagePercent);
    UE_LOG(LogTemp, Warning, TEXT("Average Frame Time: %.4f ms"), CalculateAverageFrameTime() * 1000.0f);
    UE_LOG(LogTemp, Warning, TEXT("Target Frame Rate: %d FPS"), TargetFrameRate);
    UE_LOG(LogTemp, Warning, TEXT("Registered Components: %d"), RegisteredAudioComponents.Num());
    UE_LOG(LogTemp, Warning, TEXT("=== END PROFILER ==="));
}

void UAudioPerformanceOptimizer::SetPerformanceTarget(int32 TargetFPS)
{
    TargetFrameRate = FMath::Clamp(TargetFPS, 30, 120);
    UE_LOG(LogTemp, Log, TEXT("AudioPerformanceOptimizer: Set target frame rate to %d FPS"), TargetFrameRate);
}

void UAudioPerformanceOptimizer::UpdatePerformanceMetrics()
{
    // Update active audio sources count
    int32 ActiveSources = 0;
    int32 StreamingSources = 0;
    
    for (const TWeakObjectPtr<UAudioComponent>& WeakAudioComp : RegisteredAudioComponents)
    {
        if (UAudioComponent* AudioComp = WeakAudioComp.Get())
        {
            if (AudioComp->IsPlaying())
            {
                ActiveSources++;
                // Simple heuristic: consider sources beyond medium distance as streaming
                FVector ListenerLocation = GetListenerLocation();
                FVector AudioLocation = AudioComp->GetComponentLocation();
                float Distance = FVector::Dist(ListenerLocation, AudioLocation);
                
                if (Distance > LODSettings.MediumQualityDistance)
                {
                    StreamingSources++;
                }
            }
        }
    }
    
    CurrentMetrics.ActiveAudioSources = ActiveSources;
    CurrentMetrics.StreamingAudioSources = StreamingSources;
    
    // Calculate CPU usage based on frame time
    float AverageFrameTime = CalculateAverageFrameTime();
    float TargetFrameTime = 1.0f / TargetFrameRate;
    CurrentMetrics.CPUUsagePercent = FMath::Clamp((AverageFrameTime / TargetFrameTime) * 100.0f, 0.0f, 100.0f);
    
    // Estimate audio memory usage (simplified)
    CurrentMetrics.AudioMemoryUsageMB = ActiveSources * 2.5f; // Rough estimate: 2.5MB per active source
}

void UAudioPerformanceOptimizer::ApplyLODOptimizations()
{
    FVector ListenerLocation = GetListenerLocation();
    
    for (const TWeakObjectPtr<UAudioComponent>& WeakAudioComp : RegisteredAudioComponents)
    {
        if (UAudioComponent* AudioComp = WeakAudioComp.Get())
        {
            if (AudioComp->IsPlaying())
            {
                FVector AudioLocation = AudioComp->GetComponentLocation();
                EAudio_LODLevel LODLevel = CalculateAudioLOD(ListenerLocation, AudioLocation);
                OptimizeAudioComponentSettings(AudioComp, LODLevel);
            }
        }
    }
}

void UAudioPerformanceOptimizer::CullDistantAudioSources()
{
    FVector ListenerLocation = GetListenerLocation();
    int32 CulledCount = 0;
    
    for (const TWeakObjectPtr<UAudioComponent>& WeakAudioComp : RegisteredAudioComponents)
    {
        if (UAudioComponent* AudioComp = WeakAudioComp.Get())
        {
            if (AudioComp->IsPlaying())
            {
                FVector AudioLocation = AudioComp->GetComponentLocation();
                float Distance = FVector::Dist(ListenerLocation, AudioLocation);
                
                if (Distance > LODSettings.CullingDistance)
                {
                    AudioComp->Stop();
                    CulledCount++;
                }
            }
        }
    }
    
    CurrentMetrics.CulledAudioSources = CulledCount;
}

void UAudioPerformanceOptimizer::ManageAudioStreaming()
{
    // Limit the number of simultaneous audio sources
    int32 PlayingCount = 0;
    TArray<UAudioComponent*> PlayingComponents;
    
    for (const TWeakObjectPtr<UAudioComponent>& WeakAudioComp : RegisteredAudioComponents)
    {
        if (UAudioComponent* AudioComp = WeakAudioComp.Get())
        {
            if (AudioComp->IsPlaying())
            {
                PlayingComponents.Add(AudioComp);
                PlayingCount++;
            }
        }
    }
    
    // If we exceed the maximum, stop the furthest audio sources
    if (PlayingCount > LODSettings.MaxSimultaneousAudioSources)
    {
        FVector ListenerLocation = GetListenerLocation();
        
        // Sort by distance from listener
        PlayingComponents.Sort([ListenerLocation](const UAudioComponent& A, const UAudioComponent& B) {
            float DistA = FVector::Dist(ListenerLocation, A.GetComponentLocation());
            float DistB = FVector::Dist(ListenerLocation, B.GetComponentLocation());
            return DistA > DistB; // Sort furthest first
        });
        
        // Stop the furthest sources
        int32 ToStop = PlayingCount - LODSettings.MaxSimultaneousAudioSources;
        for (int32 i = 0; i < ToStop && i < PlayingComponents.Num(); i++)
        {
            PlayingComponents[i]->Stop();
        }
    }
}

float UAudioPerformanceOptimizer::CalculateAverageFrameTime() const
{
    if (FrameTimeHistory.Num() == 0)
    {
        return 1.0f / TargetFrameRate;
    }
    
    float Sum = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Sum += FrameTime;
    }
    
    return Sum / FrameTimeHistory.Num();
}

FVector UAudioPerformanceOptimizer::GetListenerLocation() const
{
    UWorld* World = GetWorld();
    if (World)
    {
        APlayerController* PC = World->GetFirstPlayerController();
        if (PC && PC->GetPawn())
        {
            return PC->GetPawn()->GetActorLocation();
        }
    }
    
    return FVector::ZeroVector;
}

void UAudioPerformanceOptimizer::OptimizeAudioComponentSettings(UAudioComponent* AudioComponent, EAudio_LODLevel LODLevel)
{
    if (!AudioComponent)
    {
        return;
    }
    
    switch (LODLevel)
    {
        case EAudio_LODLevel::High:
            AudioComponent->SetVolumeMultiplier(1.0f);
            AudioComponent->SetPitchMultiplier(1.0f);
            break;
            
        case EAudio_LODLevel::Medium:
            AudioComponent->SetVolumeMultiplier(0.8f);
            AudioComponent->SetPitchMultiplier(1.0f);
            break;
            
        case EAudio_LODLevel::Low:
            AudioComponent->SetVolumeMultiplier(0.6f);
            AudioComponent->SetPitchMultiplier(0.95f);
            break;
            
        case EAudio_LODLevel::Disabled:
            AudioComponent->Stop();
            break;
    }
}