#include "AudioLODManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UAudioLODManager::UAudioLODManager()
{
    CurrentLODLevel = EAudio_LODLevel::High;
    LODUpdateInterval = 0.5f;
    TimeSinceLastLODUpdate = 0.0f;
    bAutoLODEnabled = true;
    PerformanceThreshold = 60.0f; // Target 60 FPS
    
    InitializeLODSettings();
}

void UAudioLODManager::InitializeLODSystem(UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioLODManager: Cannot initialize - World is null"));
        return;
    }

    // Clear existing registrations
    RegisteredAudioComponents.Empty();
    
    // Find and register all existing audio components
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            TArray<UAudioComponent*> AudioComponents;
            Actor->GetComponents<UAudioComponent>(AudioComponents);
            
            for (UAudioComponent* AudioComp : AudioComponents)
            {
                if (AudioComp)
                {
                    RegisterAudioComponent(AudioComp);
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("AudioLODManager: Initialized with %d audio components"), RegisteredAudioComponents.Num());
}

void UAudioLODManager::SetLODLevel(EAudio_LODLevel NewLODLevel)
{
    if (CurrentLODLevel != NewLODLevel)
    {
        CurrentLODLevel = NewLODLevel;
        ApplyLODSettings();
        
        UE_LOG(LogTemp, Log, TEXT("AudioLODManager: LOD Level changed to %d"), (int32)CurrentLODLevel);
    }
}

void UAudioLODManager::UpdateAudioLOD(float DeltaTime)
{
    TimeSinceLastLODUpdate += DeltaTime;
    
    if (TimeSinceLastLODUpdate >= LODUpdateInterval)
    {
        TimeSinceLastLODUpdate = 0.0f;
        
        // Clean up invalid weak pointers
        RegisteredAudioComponents.RemoveAll([](const TWeakObjectPtr<UAudioComponent>& WeakPtr)
        {
            return !WeakPtr.IsValid();
        });
        
        // Cull distant audio sources
        CullDistantAudioSources();
        
        // Auto-adjust LOD if enabled
        if (bAutoLODEnabled)
        {
            EAudio_LODLevel OptimalLOD = DetermineOptimalLODLevel();
            SetLODLevel(OptimalLOD);
        }
        
        // Optimize active audio sources
        OptimizeActiveAudioSources();
    }
}

void UAudioLODManager::RegisterAudioComponent(UAudioComponent* AudioComp)
{
    if (AudioComp && !RegisteredAudioComponents.Contains(AudioComp))
    {
        RegisteredAudioComponents.Add(AudioComp);
    }
}

void UAudioLODManager::UnregisterAudioComponent(UAudioComponent* AudioComp)
{
    if (AudioComp)
    {
        RegisteredAudioComponents.Remove(AudioComp);
    }
}

void UAudioLODManager::SetPerformanceMode(bool bLowPerformanceMode)
{
    if (bLowPerformanceMode)
    {
        SetLODLevel(EAudio_LODLevel::Low);
        bAutoLODEnabled = false;
    }
    else
    {
        SetLODLevel(EAudio_LODLevel::High);
        bAutoLODEnabled = true;
    }
}

float UAudioLODManager::GetCurrentAudioLoad() const
{
    int32 ActiveSounds = 0;
    
    for (const TWeakObjectPtr<UAudioComponent>& WeakAudioComp : RegisteredAudioComponents)
    {
        if (UAudioComponent* AudioComp = WeakAudioComp.Get())
        {
            if (AudioComp->IsPlaying())
            {
                ActiveSounds++;
            }
        }
    }
    
    const FAudio_LODSettings& CurrentSettings = LODSettings[CurrentLODLevel];
    return (float)ActiveSounds / (float)CurrentSettings.MaxSimultaneousSounds;
}

void UAudioLODManager::InitializeLODSettings()
{
    // High Quality LOD
    FAudio_LODSettings HighLOD;
    HighLOD.MaxSimultaneousSounds = 64;
    HighLOD.MaxAudibleDistance = 8000.0f;
    HighLOD.VolumeMultiplier = 1.0f;
    HighLOD.bEnableReverb = true;
    HighLOD.bEnableOcclusion = true;
    LODSettings.Add(EAudio_LODLevel::High, HighLOD);
    
    // Medium Quality LOD
    FAudio_LODSettings MediumLOD;
    MediumLOD.MaxSimultaneousSounds = 32;
    MediumLOD.MaxAudibleDistance = 5000.0f;
    MediumLOD.VolumeMultiplier = 0.8f;
    MediumLOD.bEnableReverb = true;
    MediumLOD.bEnableOcclusion = false;
    LODSettings.Add(EAudio_LODLevel::Medium, MediumLOD);
    
    // Low Quality LOD
    FAudio_LODSettings LowLOD;
    LowLOD.MaxSimultaneousSounds = 16;
    LowLOD.MaxAudibleDistance = 3000.0f;
    LowLOD.VolumeMultiplier = 0.6f;
    LowLOD.bEnableReverb = false;
    LowLOD.bEnableOcclusion = false;
    LODSettings.Add(EAudio_LODLevel::Low, LowLOD);
    
    // Minimal Quality LOD
    FAudio_LODSettings MinimalLOD;
    MinimalLOD.MaxSimultaneousSounds = 8;
    MinimalLOD.MaxAudibleDistance = 1500.0f;
    MinimalLOD.VolumeMultiplier = 0.4f;
    MinimalLOD.bEnableReverb = false;
    MinimalLOD.bEnableOcclusion = false;
    LODSettings.Add(EAudio_LODLevel::Minimal, MinimalLOD);
}

void UAudioLODManager::ApplyLODSettings()
{
    const FAudio_LODSettings& CurrentSettings = LODSettings[CurrentLODLevel];
    
    for (const TWeakObjectPtr<UAudioComponent>& WeakAudioComp : RegisteredAudioComponents)
    {
        if (UAudioComponent* AudioComp = WeakAudioComp.Get())
        {
            // Apply volume multiplier
            AudioComp->SetVolumeMultiplier(AudioComp->VolumeMultiplier * CurrentSettings.VolumeMultiplier);
            
            // Apply attenuation settings
            if (AudioComp->AttenuationSettings)
            {
                // Note: In a real implementation, you'd modify attenuation settings
                // For now, we'll just log the change
                UE_LOG(LogTemp, Verbose, TEXT("AudioLODManager: Applied LOD settings to audio component"));
            }
        }
    }
}

void UAudioLODManager::CullDistantAudioSources()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    const FAudio_LODSettings& CurrentSettings = LODSettings[CurrentLODLevel];
    
    for (const TWeakObjectPtr<UAudioComponent>& WeakAudioComp : RegisteredAudioComponents)
    {
        if (UAudioComponent* AudioComp = WeakAudioComp.Get())
        {
            float Distance = FVector::Dist(AudioComp->GetComponentLocation(), PlayerLocation);
            
            if (Distance > CurrentSettings.MaxAudibleDistance)
            {
                if (AudioComp->IsPlaying())
                {
                    AudioComp->Stop();
                }
            }
        }
    }
}

void UAudioLODManager::OptimizeActiveAudioSources()
{
    const FAudio_LODSettings& CurrentSettings = LODSettings[CurrentLODLevel];
    
    // Count currently playing sounds
    TArray<UAudioComponent*> PlayingComponents;
    for (const TWeakObjectPtr<UAudioComponent>& WeakAudioComp : RegisteredAudioComponents)
    {
        if (UAudioComponent* AudioComp = WeakAudioComp.Get())
        {
            if (AudioComp->IsPlaying())
            {
                PlayingComponents.Add(AudioComp);
            }
        }
    }
    
    // If we're over the limit, stop the least important sounds
    if (PlayingComponents.Num() > CurrentSettings.MaxSimultaneousSounds)
    {
        // Sort by priority (distance from player, volume, etc.)
        UWorld* World = GetWorld();
        if (World)
        {
            APlayerController* PlayerController = World->GetFirstPlayerController();
            if (PlayerController && PlayerController->GetPawn())
            {
                FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
                
                PlayingComponents.Sort([PlayerLocation](const UAudioComponent& A, const UAudioComponent& B)
                {
                    float DistA = FVector::Dist(A.GetComponentLocation(), PlayerLocation);
                    float DistB = FVector::Dist(B.GetComponentLocation(), PlayerLocation);
                    return DistA < DistB; // Closer sounds have higher priority
                });
                
                // Stop the furthest sounds
                int32 SoundsToStop = PlayingComponents.Num() - CurrentSettings.MaxSimultaneousSounds;
                for (int32 i = PlayingComponents.Num() - SoundsToStop; i < PlayingComponents.Num(); i++)
                {
                    PlayingComponents[i]->Stop();
                }
            }
        }
    }
}

EAudio_LODLevel UAudioLODManager::DetermineOptimalLODLevel()
{
    // Get current frame rate
    float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    
    // Calculate audio complexity
    float AudioComplexity = CalculateAudioComplexity();
    
    // Determine optimal LOD based on performance
    if (CurrentFPS < 30.0f || AudioComplexity > 0.9f)
    {
        return EAudio_LODLevel::Minimal;
    }
    else if (CurrentFPS < 45.0f || AudioComplexity > 0.7f)
    {
        return EAudio_LODLevel::Low;
    }
    else if (CurrentFPS < 55.0f || AudioComplexity > 0.5f)
    {
        return EAudio_LODLevel::Medium;
    }
    
    return EAudio_LODLevel::High;
}

float UAudioLODManager::CalculateAudioComplexity() const
{
    int32 ActiveSounds = 0;
    int32 TotalSounds = 0;
    
    for (const TWeakObjectPtr<UAudioComponent>& WeakAudioComp : RegisteredAudioComponents)
    {
        if (UAudioComponent* AudioComp = WeakAudioComp.Get())
        {
            TotalSounds++;
            if (AudioComp->IsPlaying())
            {
                ActiveSounds++;
            }
        }
    }
    
    if (TotalSounds == 0)
    {
        return 0.0f;
    }
    
    return (float)ActiveSounds / (float)TotalSounds;
}