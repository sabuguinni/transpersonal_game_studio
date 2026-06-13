#include "Audio_SystemManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

void UAudio_SystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SystemManager: Initializing audio subsystem"));
    
    // Initialize music state
    MusicState = FAudio_MusicState();
    SpatialConfig = FAudio_SpatialConfig();
    
    // Register default footstep sounds
    RegisterFootstepSurface("Grass", nullptr, nullptr);
    RegisterFootstepSurface("Stone", nullptr, nullptr);
    RegisterFootstepSurface("Mud", nullptr, nullptr);
    RegisterFootstepSurface("Sand", nullptr, nullptr);
    
    // Start cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &UAudio_SystemManager::CleanupFinishedComponents,
            1.0f,
            true
        );
    }
}

void UAudio_SystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Audio_SystemManager: Shutting down audio subsystem"));
    
    // Stop all active audio components
    for (UAudioComponent* Component : ActiveAudioComponents)
    {
        if (IsValid(Component))
        {
            Component->Stop();
        }
    }
    
    // Stop all environmental loops
    for (UAudioComponent* Component : EnvironmentalLoops)
    {
        if (IsValid(Component))
        {
            Component->Stop();
        }
    }
    
    ActiveAudioComponents.Empty();
    CategoryComponents.Empty();
    EnvironmentalLoops.Empty();
    
    Super::Deinitialize();
}

void UAudio_SystemManager::SetMusicLayer(EAudio_MusicLayer NewLayer, float FadeTime)
{
    if (MusicState.CurrentLayer == NewLayer)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SystemManager: Changing music layer to %d"), (int32)NewLayer);
    
    MusicState.CurrentLayer = NewLayer;
    MusicState.FadeTime = FadeTime;
    MusicState.bIsTransitioning = true;
    
    UpdateMusicLayers();
    
    // Reset transition flag after fade time
    if (UWorld* World = GetWorld())
    {
        FTimerHandle TransitionTimer;
        World->GetTimerManager().SetTimer(
            TransitionTimer,
            [this]() { MusicState.bIsTransitioning = false; },
            FadeTime,
            false
        );
    }
}

void UAudio_SystemManager::SetTensionLevel(float TensionValue)
{
    MusicState.TensionLevel = FMath::Clamp(TensionValue, 0.0f, 1.0f);
    
    // Auto-switch music layers based on tension
    if (TensionValue > 0.8f && MusicState.CurrentLayer != EAudio_MusicLayer::Combat)
    {
        SetMusicLayer(EAudio_MusicLayer::Combat);
    }
    else if (TensionValue > 0.5f && MusicState.CurrentLayer == EAudio_MusicLayer::Ambient)
    {
        SetMusicLayer(EAudio_MusicLayer::Tension);
    }
    else if (TensionValue < 0.2f && MusicState.CurrentLayer != EAudio_MusicLayer::Ambient)
    {
        SetMusicLayer(EAudio_MusicLayer::Ambient);
    }
}

void UAudio_SystemManager::PlayStinger(USoundCue* StingerSound)
{
    if (!StingerSound)
    {
        return;
    }
    
    UGameplayStatics::PlaySound2D(GetWorld(), StingerSound);
    UE_LOG(LogTemp, Log, TEXT("Audio_SystemManager: Playing stinger sound"));
}

UAudioComponent* UAudio_SystemManager::PlaySFXAtLocation(USoundWave* Sound, FVector Location, float VolumeMultiplier)
{
    if (!Sound || !GetWorld())
    {
        return nullptr;
    }
    
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        Sound,
        Location,
        FRotator::ZeroRotator,
        VolumeMultiplier,
        1.0f,
        0.0f,
        nullptr,
        nullptr,
        true
    );
    
    if (AudioComp)
    {
        ActiveAudioComponents.Add(AudioComp);
        UE_LOG(LogTemp, Log, TEXT("Audio_SystemManager: Playing SFX at location %s"), *Location.ToString());
    }
    
    return AudioComp;
}

UAudioComponent* UAudio_SystemManager::PlaySFXAttached(USoundWave* Sound, USceneComponent* AttachComponent, float VolumeMultiplier)
{
    if (!Sound || !AttachComponent || !GetWorld())
    {
        return nullptr;
    }
    
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAttached(
        Sound,
        AttachComponent,
        NAME_None,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::KeepRelativeOffset,
        true,
        VolumeMultiplier,
        1.0f,
        0.0f,
        nullptr,
        nullptr,
        true
    );
    
    if (AudioComp)
    {
        ActiveAudioComponents.Add(AudioComp);
        UE_LOG(LogTemp, Log, TEXT("Audio_SystemManager: Playing SFX attached to component"));
    }
    
    return AudioComp;
}

void UAudio_SystemManager::StopAllSFXOfCategory(EAudio_SFXCategory Category)
{
    if (TArray<UAudioComponent*>* Components = CategoryComponents.Find(Category))
    {
        for (UAudioComponent* Component : *Components)
        {
            if (IsValid(Component))
            {
                Component->Stop();
            }
        }
        Components->Empty();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SystemManager: Stopped all SFX of category %d"), (int32)Category);
}

void UAudio_SystemManager::PlayFootstepSound(FVector Location, float Weight, bool bIsRunning)
{
    FString SurfaceType = GetSurfaceTypeAtLocation(Location);
    
    USoundWave* FootstepSound = nullptr;
    if (bIsRunning)
    {
        if (USoundWave** RunSound = FootstepRunSounds.Find(SurfaceType))
        {
            FootstepSound = *RunSound;
        }
    }
    else
    {
        if (USoundWave** WalkSound = FootstepWalkSounds.Find(SurfaceType))
        {
            FootstepSound = *WalkSound;
        }
    }
    
    if (FootstepSound)
    {
        float VolumeMultiplier = FMath::Lerp(0.5f, 1.5f, Weight / 100.0f); // Weight in kg
        PlaySFXAtLocation(FootstepSound, Location, VolumeMultiplier);
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Audio_SystemManager: Footstep on %s surface, weight %.1f"), *SurfaceType, Weight);
}

void UAudio_SystemManager::RegisterFootstepSurface(const FString& SurfaceType, USoundWave* WalkSound, USoundWave* RunSound)
{
    if (WalkSound)
    {
        FootstepWalkSounds.Add(SurfaceType, WalkSound);
    }
    if (RunSound)
    {
        FootstepRunSounds.Add(SurfaceType, RunSound);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SystemManager: Registered footstep sounds for surface %s"), *SurfaceType);
}

void UAudio_SystemManager::SetAmbientVolume(float Volume)
{
    Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Apply to all environmental loops
    for (UAudioComponent* Component : EnvironmentalLoops)
    {
        if (IsValid(Component))
        {
            Component->SetVolumeMultiplier(Volume);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SystemManager: Set ambient volume to %.2f"), Volume);
}

void UAudio_SystemManager::PlayEnvironmentalLoop(USoundWave* LoopSound, FVector Location, float Radius)
{
    if (!LoopSound || !GetWorld())
    {
        return;
    }
    
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        LoopSound,
        Location,
        FRotator::ZeroRotator,
        1.0f,
        1.0f,
        0.0f,
        nullptr,
        nullptr,
        true
    );
    
    if (AudioComp)
    {
        EnvironmentalLoops.Add(AudioComp);
        UE_LOG(LogTemp, Log, TEXT("Audio_SystemManager: Started environmental loop at %s"), *Location.ToString());
    }
}

void UAudio_SystemManager::StopEnvironmentalLoop(USoundWave* LoopSound)
{
    for (int32 i = EnvironmentalLoops.Num() - 1; i >= 0; i--)
    {
        UAudioComponent* Component = EnvironmentalLoops[i];
        if (IsValid(Component) && Component->GetSound() == LoopSound)
        {
            Component->Stop();
            EnvironmentalLoops.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Audio_SystemManager: Stopped environmental loop"));
            break;
        }
    }
}

void UAudio_SystemManager::UpdateMusicLayers()
{
    // This would integrate with UE5's MetaSounds system
    // For now, we log the state change
    UE_LOG(LogTemp, Log, TEXT("Audio_SystemManager: Music layer updated - Layer: %d, Tension: %.2f"), 
           (int32)MusicState.CurrentLayer, MusicState.TensionLevel);
}

void UAudio_SystemManager::CleanupFinishedComponents()
{
    // Remove finished audio components
    ActiveAudioComponents.RemoveAll([](UAudioComponent* Component) {
        return !IsValid(Component) || !Component->IsPlaying();
    });
    
    // Clean up category components
    for (auto& CategoryPair : CategoryComponents)
    {
        CategoryPair.Value.RemoveAll([](UAudioComponent* Component) {
            return !IsValid(Component) || !Component->IsPlaying();
        });
    }
    
    // Clean up environmental loops
    EnvironmentalLoops.RemoveAll([](UAudioComponent* Component) {
        return !IsValid(Component) || !Component->IsPlaying();
    });
}

FString UAudio_SystemManager::GetSurfaceTypeAtLocation(FVector Location)
{
    // Simple surface detection - in a full implementation this would
    // use physics materials or landscape layers
    if (Location.Z < 0)
    {
        return "Stone"; // Underground/cave
    }
    else if (Location.Z < 100)
    {
        return "Grass"; // Ground level
    }
    else
    {
        return "Stone"; // Elevated terrain
    }
}