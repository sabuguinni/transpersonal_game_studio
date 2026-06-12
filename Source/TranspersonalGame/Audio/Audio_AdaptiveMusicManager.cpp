#include "Audio_AdaptiveMusicManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UAudio_AdaptiveMusicManager::UAudio_AdaptiveMusicManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    CurrentMusicState = EAudio_MusicState::Calm;
    TargetMusicState = EAudio_MusicState::Calm;
    StateTransitionTime = 3.0f;
    CurrentTransitionTime = 0.0f;
    
    DangerDetectionRadius = 2000.0f;
    TensionDetectionRadius = 3000.0f;
    
    // Initialize music layers
    MusicLayers.SetNum(8); // One for each music state
    AudioComponents.SetNum(8);
}

void UAudio_AdaptiveMusicManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeAudioComponents();
}

void UAudio_AdaptiveMusicManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    ProcessMusicTransition(DeltaTime);
    UpdateLayerVolumes(DeltaTime);
}

void UAudio_AdaptiveMusicManager::SetMusicState(EAudio_MusicState NewState)
{
    if (NewState != CurrentMusicState)
    {
        TargetMusicState = NewState;
        CurrentTransitionTime = 0.0f;
    }
}

void UAudio_AdaptiveMusicManager::EnableMusicLayer(int32 LayerIndex, float FadeInTime)
{
    if (MusicLayers.IsValidIndex(LayerIndex) && AudioComponents.IsValidIndex(LayerIndex))
    {
        MusicLayers[LayerIndex].bIsActive = true;
        MusicLayers[LayerIndex].FadeTime = FadeInTime;
        
        if (AudioComponents[LayerIndex] && MusicLayers[LayerIndex].SoundCue.LoadSynchronous())
        {
            AudioComponents[LayerIndex]->SetSound(MusicLayers[LayerIndex].SoundCue.LoadSynchronous());
            AudioComponents[LayerIndex]->Play();
        }
    }
}

void UAudio_AdaptiveMusicManager::DisableMusicLayer(int32 LayerIndex, float FadeOutTime)
{
    if (MusicLayers.IsValidIndex(LayerIndex))
    {
        MusicLayers[LayerIndex].bIsActive = false;
        MusicLayers[LayerIndex].FadeTime = FadeOutTime;
    }
}

void UAudio_AdaptiveMusicManager::UpdateProximityMusic(const TArray<AActor*>& NearbyActors)
{
    EAudio_MusicState ProximityState = DetermineMusicStateFromProximity(NearbyActors);
    SetMusicState(ProximityState);
}

void UAudio_AdaptiveMusicManager::UpdateTimeOfDayMusic(float TimeOfDay)
{
    EAudio_MusicState TimeState = DetermineMusicStateFromTimeOfDay(TimeOfDay);
    
    // Only override if not in danger/combat
    if (CurrentMusicState != EAudio_MusicState::Danger && CurrentMusicState != EAudio_MusicState::Combat)
    {
        SetMusicState(TimeState);
    }
}

void UAudio_AdaptiveMusicManager::InitializeAudioComponents()
{
    for (int32 i = 0; i < AudioComponents.Num(); i++)
    {
        if (!AudioComponents[i])
        {
            AudioComponents[i] = GetOwner()->CreateDefaultSubobject<UAudioComponent>(
                FName(*FString::Printf(TEXT("MusicLayer_%d"), i))
            );
            
            if (AudioComponents[i])
            {
                AudioComponents[i]->bAutoActivate = false;
                AudioComponents[i]->SetVolumeMultiplier(0.0f);
            }
        }
    }
}

void UAudio_AdaptiveMusicManager::ProcessMusicTransition(float DeltaTime)
{
    if (CurrentMusicState != TargetMusicState)
    {
        CurrentTransitionTime += DeltaTime;
        
        if (CurrentTransitionTime >= StateTransitionTime)
        {
            CurrentMusicState = TargetMusicState;
            CurrentTransitionTime = 0.0f;
            
            // Activate target layer
            int32 TargetLayerIndex = static_cast<int32>(TargetMusicState);
            EnableMusicLayer(TargetLayerIndex, 2.0f);
            
            // Disable other layers
            for (int32 i = 0; i < MusicLayers.Num(); i++)
            {
                if (i != TargetLayerIndex)
                {
                    DisableMusicLayer(i, 2.0f);
                }
            }
        }
    }
}

void UAudio_AdaptiveMusicManager::UpdateLayerVolumes(float DeltaTime)
{
    for (int32 i = 0; i < MusicLayers.Num(); i++)
    {
        if (AudioComponents.IsValidIndex(i) && AudioComponents[i])
        {
            float CurrentVolume = AudioComponents[i]->GetVolumeMultiplier();
            float TargetVolume = MusicLayers[i].bIsActive ? MusicLayers[i].Volume : 0.0f;
            
            if (!FMath::IsNearlyEqual(CurrentVolume, TargetVolume, 0.01f))
            {
                float FadeSpeed = 1.0f / FMath::Max(MusicLayers[i].FadeTime, 0.1f);
                float NewVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, FadeSpeed);
                AudioComponents[i]->SetVolumeMultiplier(NewVolume);
                
                // Stop audio component if volume reaches zero
                if (NewVolume <= 0.01f && !MusicLayers[i].bIsActive)
                {
                    AudioComponents[i]->Stop();
                }
            }
        }
    }
}

EAudio_MusicState UAudio_AdaptiveMusicManager::DetermineMusicStateFromProximity(const TArray<AActor*>& NearbyActors)
{
    float ClosestDangerDistance = FLT_MAX;
    bool bFoundDanger = false;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Actor : NearbyActors)
    {
        if (!Actor) continue;
        
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        
        // Check for dangerous dinosaurs
        if (ActorLabel.Contains(TEXT("trex")) || ActorLabel.Contains(TEXT("raptor")))
        {
            float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
            
            if (Distance < DangerDetectionRadius)
            {
                bFoundDanger = true;
                ClosestDangerDistance = FMath::Min(ClosestDangerDistance, Distance);
            }
        }
    }
    
    if (bFoundDanger)
    {
        if (ClosestDangerDistance < DangerDetectionRadius * 0.5f)
        {
            return EAudio_MusicState::Danger;
        }
        else
        {
            return EAudio_MusicState::Tension;
        }
    }
    
    return EAudio_MusicState::Exploration;
}

EAudio_MusicState UAudio_AdaptiveMusicManager::DetermineMusicStateFromTimeOfDay(float TimeOfDay)
{
    // TimeOfDay assumed to be 0.0-24.0
    if (TimeOfDay >= 5.0f && TimeOfDay < 7.0f)
    {
        return EAudio_MusicState::Dawn;
    }
    else if (TimeOfDay >= 18.0f && TimeOfDay < 20.0f)
    {
        return EAudio_MusicState::Dusk;
    }
    else if (TimeOfDay >= 20.0f || TimeOfDay < 5.0f)
    {
        return EAudio_MusicState::Night;
    }
    else
    {
        return EAudio_MusicState::Calm;
    }
}