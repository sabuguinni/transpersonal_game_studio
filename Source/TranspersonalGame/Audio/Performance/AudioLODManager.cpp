#include "AudioLODManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "AudioDevice.h"
#include "Engine/GameInstance.h"

UAudioLODManager::UAudioLODManager()
{
    TargetFrameRate = 60.0f;
    MaxAudioCPUBudget = 15.0f;
    MaxAudioMemoryBudgetMB = 256.0f;
    LODUpdateFrequency = 0.1f;
    LastLODUpdateTime = 0.0f;
}

void UAudioLODManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("AudioLODManager: Initializing audio performance optimization system"));
    
    InitializeLODSettings();
    
    // Initialize performance metrics
    CurrentMetrics = FAudio_PerformanceMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("AudioLODManager: Initialization complete"));
}

void UAudioLODManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("AudioLODManager: Shutting down"));
    
    RegisteredComponents.Empty();
    LODSettings.Empty();
    
    Super::Deinitialize();
}

void UAudioLODManager::InitializeLODSettings()
{
    // Highest quality settings
    FAudio_LODSettings HighestSettings;
    HighestSettings.MaxDistance = 10000.0f;
    HighestSettings.VolumeMultiplier = 1.0f;
    HighestSettings.MaxConcurrentSounds = 64;
    HighestSettings.bEnableReverb = true;
    HighestSettings.bEnableOcclusion = true;
    HighestSettings.bEnableSpatialization = true;
    LODSettings.Add(EAudio_LODLevel::Highest, HighestSettings);

    // High quality settings
    FAudio_LODSettings HighSettings;
    HighSettings.MaxDistance = 7500.0f;
    HighSettings.VolumeMultiplier = 0.9f;
    HighSettings.MaxConcurrentSounds = 48;
    HighSettings.bEnableReverb = true;
    HighSettings.bEnableOcclusion = true;
    HighSettings.bEnableSpatialization = true;
    LODSettings.Add(EAudio_LODLevel::High, HighSettings);

    // Medium quality settings
    FAudio_LODSettings MediumSettings;
    MediumSettings.MaxDistance = 5000.0f;
    MediumSettings.VolumeMultiplier = 0.8f;
    MediumSettings.MaxConcurrentSounds = 32;
    MediumSettings.bEnableReverb = true;
    MediumSettings.bEnableOcclusion = false;
    MediumSettings.bEnableSpatialization = true;
    LODSettings.Add(EAudio_LODLevel::Medium, MediumSettings);

    // Low quality settings
    FAudio_LODSettings LowSettings;
    LowSettings.MaxDistance = 2500.0f;
    LowSettings.VolumeMultiplier = 0.6f;
    LowSettings.MaxConcurrentSounds = 16;
    LowSettings.bEnableReverb = false;
    LowSettings.bEnableOcclusion = false;
    LowSettings.bEnableSpatialization = true;
    LODSettings.Add(EAudio_LODLevel::Low, LowSettings);

    // Lowest quality settings
    FAudio_LODSettings LowestSettings;
    LowestSettings.MaxDistance = 1000.0f;
    LowestSettings.VolumeMultiplier = 0.4f;
    LowestSettings.MaxConcurrentSounds = 8;
    LowestSettings.bEnableReverb = false;
    LowestSettings.bEnableOcclusion = false;
    LowestSettings.bEnableSpatialization = false;
    LODSettings.Add(EAudio_LODLevel::Lowest, LowestSettings);

    UE_LOG(LogTemp, Log, TEXT("AudioLODManager: LOD settings initialized"));
}

void UAudioLODManager::UpdateAudioLOD(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check if it's time to update LOD
    if (CurrentTime - LastLODUpdateTime < LODUpdateFrequency)
    {
        return;
    }
    
    LastLODUpdateTime = CurrentTime;
    
    // Get player location
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return;
    }
    
    FVector ListenerLocation = PlayerController->GetPawn()->GetActorLocation();
    
    // Update LOD for all registered components
    for (auto& ComponentPair : RegisteredComponents)
    {
        UAudioComponent* AudioComponent = ComponentPair.Key;
        if (IsValid(AudioComponent) && AudioComponent->GetOwner())
        {
            EAudio_LODLevel NewLODLevel = CalculateLODLevel(AudioComponent->GetOwner(), ListenerLocation);
            SetAudioLODLevel(AudioComponent, NewLODLevel);
        }
    }
    
    // Update distance culling
    UpdateDistanceCulling(ListenerLocation);
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Apply performance optimizations if needed
    if (CurrentMetrics.AudioCPUUsagePercent > MaxAudioCPUBudget || 
        CurrentMetrics.AudioMemoryUsageMB > MaxAudioMemoryBudgetMB)
    {
        ApplyPerformanceOptimizations();
    }
}

EAudio_LODLevel UAudioLODManager::CalculateLODLevel(AActor* AudioSource, const FVector& ListenerLocation) const
{
    if (!AudioSource)
    {
        return EAudio_LODLevel::Lowest;
    }
    
    float Distance = FVector::Dist(AudioSource->GetActorLocation(), ListenerLocation);
    
    // Distance-based LOD calculation
    if (Distance < 500.0f)
    {
        return EAudio_LODLevel::Highest;
    }
    else if (Distance < 1500.0f)
    {
        return EAudio_LODLevel::High;
    }
    else if (Distance < 3000.0f)
    {
        return EAudio_LODLevel::Medium;
    }
    else if (Distance < 5000.0f)
    {
        return EAudio_LODLevel::Low;
    }
    else
    {
        return EAudio_LODLevel::Lowest;
    }
}

void UAudioLODManager::SetAudioLODLevel(UAudioComponent* AudioComponent, EAudio_LODLevel LODLevel)
{
    if (!IsValid(AudioComponent))
    {
        return;
    }
    
    const FAudio_LODSettings* Settings = LODSettings.Find(LODLevel);
    if (!Settings)
    {
        return;
    }
    
    // Apply LOD settings to audio component
    AudioComponent->SetVolumeMultiplier(Settings->VolumeMultiplier);
    
    // Enable/disable spatialization
    if (Settings->bEnableSpatialization)
    {
        AudioComponent->SetBoolParameter(FName("EnableSpatialization"), true);
    }
    else
    {
        AudioComponent->SetBoolParameter(FName("EnableSpatialization"), false);
    }
    
    // Set attenuation distance
    if (AudioComponent->GetAttenuationSettings())
    {
        // Note: In a real implementation, you'd modify the attenuation settings
        // This is a simplified version
    }
}

void UAudioLODManager::RegisterAudioComponent(UAudioComponent* AudioComponent, EAudio_SourceType SourceType)
{
    if (IsValid(AudioComponent))
    {
        RegisteredComponents.Add(AudioComponent, SourceType);
        UE_LOG(LogTemp, VeryVerbose, TEXT("AudioLODManager: Registered audio component %s"), 
               *AudioComponent->GetName());
    }
}

void UAudioLODManager::UnregisterAudioComponent(UAudioComponent* AudioComponent)
{
    if (RegisteredComponents.Contains(AudioComponent))
    {
        RegisteredComponents.Remove(AudioComponent);
        UE_LOG(LogTemp, VeryVerbose, TEXT("AudioLODManager: Unregistered audio component %s"), 
               AudioComponent ? *AudioComponent->GetName() : TEXT("NULL"));
    }
}

void UAudioLODManager::SetTargetFrameRate(float TargetFPS)
{
    TargetFrameRate = FMath::Clamp(TargetFPS, 30.0f, 120.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioLODManager: Target frame rate set to %f"), TargetFrameRate);
}

FAudio_PerformanceMetrics UAudioLODManager::GetPerformanceMetrics() const
{
    return CurrentMetrics;
}

void UAudioLODManager::OptimizeForPerformance()
{
    UE_LOG(LogTemp, Log, TEXT("AudioLODManager: Applying performance optimizations"));
    
    // Reduce LOD level for distant sources
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController && PlayerController->GetPawn())
    {
        FVector ListenerLocation = PlayerController->GetPawn()->GetActorLocation();
        
        for (auto& ComponentPair : RegisteredComponents)
        {
            UAudioComponent* AudioComponent = ComponentPair.Key;
            if (IsValid(AudioComponent) && AudioComponent->GetOwner())
            {
                float Distance = FVector::Dist(AudioComponent->GetOwner()->GetActorLocation(), ListenerLocation);
                
                // More aggressive culling during performance issues
                if (Distance > 2000.0f)
                {
                    SetAudioLODLevel(AudioComponent, EAudio_LODLevel::Lowest);
                }
                else if (Distance > 1000.0f)
                {
                    SetAudioLODLevel(AudioComponent, EAudio_LODLevel::Low);
                }
            }
        }
    }
}

void UAudioLODManager::UpdateDistanceCulling(const FVector& ListenerLocation)
{
    int32 CulledCount = 0;
    
    for (auto& ComponentPair : RegisteredComponents)
    {
        UAudioComponent* AudioComponent = ComponentPair.Key;
        EAudio_SourceType SourceType = ComponentPair.Value;
        
        if (IsValid(AudioComponent) && AudioComponent->GetOwner())
        {
            FVector SourceLocation = AudioComponent->GetOwner()->GetActorLocation();
            
            if (ShouldCullAudioSource(SourceLocation, ListenerLocation, SourceType))
            {
                if (AudioComponent->IsPlaying())
                {
                    AudioComponent->Stop();
                    CulledCount++;
                }
            }
            else
            {
                if (!AudioComponent->IsPlaying() && AudioComponent->GetSound())
                {
                    AudioComponent->Play();
                }
            }
        }
    }
    
    CurrentMetrics.CulledAudioSources = CulledCount;
}

bool UAudioLODManager::ShouldCullAudioSource(const FVector& SourceLocation, const FVector& ListenerLocation, EAudio_SourceType SourceType) const
{
    float Distance = FVector::Dist(SourceLocation, ListenerLocation);
    
    // Different culling distances based on source type
    switch (SourceType)
    {
        case EAudio_SourceType::Music:
            return false; // Never cull music
            
        case EAudio_SourceType::Dialogue:
            return Distance > 2000.0f; // Keep dialogue closer
            
        case EAudio_SourceType::SFX:
            return Distance > 5000.0f;
            
        case EAudio_SourceType::Ambience:
            return Distance > 7500.0f;
            
        case EAudio_SourceType::UI:
            return false; // Never cull UI sounds
            
        default:
            return Distance > 3000.0f;
    }
}

void UAudioLODManager::SetLODSettings(EAudio_LODLevel LODLevel, const FAudio_LODSettings& Settings)
{
    LODSettings.Add(LODLevel, Settings);
    UE_LOG(LogTemp, Log, TEXT("AudioLODManager: Updated LOD settings for level %d"), (int32)LODLevel);
}

FAudio_LODSettings UAudioLODManager::GetLODSettings(EAudio_LODLevel LODLevel) const
{
    const FAudio_LODSettings* Settings = LODSettings.Find(LODLevel);
    return Settings ? *Settings : FAudio_LODSettings();
}

void UAudioLODManager::SetGlobalAudioQuality(EAudio_LODLevel Quality)
{
    UE_LOG(LogTemp, Log, TEXT("AudioLODManager: Setting global audio quality to %d"), (int32)Quality);
    
    // Apply quality settings to all registered components
    for (auto& ComponentPair : RegisteredComponents)
    {
        UAudioComponent* AudioComponent = ComponentPair.Key;
        if (IsValid(AudioComponent))
        {
            SetAudioLODLevel(AudioComponent, Quality);
        }
    }
}

void UAudioLODManager::UpdatePerformanceMetrics()
{
    // Count active audio sources
    int32 ActiveCount = 0;
    for (auto& ComponentPair : RegisteredComponents)
    {
        if (IsValid(ComponentPair.Key) && ComponentPair.Key->IsPlaying())
        {
            ActiveCount++;
        }
    }
    
    CurrentMetrics.ActiveAudioSources = ActiveCount;
    
    // Estimate memory usage (simplified)
    CurrentMetrics.AudioMemoryUsageMB = ActiveCount * 2.0f; // Rough estimate
    
    // Estimate CPU usage based on active sources and quality settings
    CurrentMetrics.AudioCPUUsagePercent = FMath::Min(ActiveCount * 0.5f, 25.0f);
    
    // Estimate latency (simplified)
    CurrentMetrics.AverageLatencyMS = FMath::RandRange(5.0f, 15.0f);
}

void UAudioLODManager::ApplyPerformanceOptimizations()
{
    UE_LOG(LogTemp, Warning, TEXT("AudioLODManager: Performance budget exceeded, applying optimizations"));
    
    // Reduce quality globally
    if (CurrentMetrics.AudioCPUUsagePercent > MaxAudioCPUBudget)
    {
        SetGlobalAudioQuality(EAudio_LODLevel::Low);
    }
    
    // Cull more aggressively
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController && PlayerController->GetPawn())
    {
        FVector ListenerLocation = PlayerController->GetPawn()->GetActorLocation();
        CullDistantAudioSources(ListenerLocation);
    }
}

float UAudioLODManager::CalculateDistanceFactor(float Distance, float MaxDistance) const
{
    return FMath::Clamp(1.0f - (Distance / MaxDistance), 0.0f, 1.0f);
}

void UAudioLODManager::CullDistantAudioSources(const FVector& ListenerLocation)
{
    for (auto& ComponentPair : RegisteredComponents)
    {
        UAudioComponent* AudioComponent = ComponentPair.Key;
        EAudio_SourceType SourceType = ComponentPair.Value;
        
        if (IsValid(AudioComponent) && AudioComponent->GetOwner())
        {
            // More aggressive culling during performance issues
            float CullDistance = 2000.0f; // Reduced from normal culling distances
            
            float Distance = FVector::Dist(AudioComponent->GetOwner()->GetActorLocation(), ListenerLocation);
            
            if (Distance > CullDistance && SourceType != EAudio_SourceType::Music && SourceType != EAudio_SourceType::UI)
            {
                if (AudioComponent->IsPlaying())
                {
                    AudioComponent->Stop();
                }
            }
        }
    }
}