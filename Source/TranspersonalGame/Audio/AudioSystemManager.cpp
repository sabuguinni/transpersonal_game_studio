#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "AudioMixerBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "AudioDevice.h"
#include "Sound/SoundSubmix.h"

UAudioSystemManager::UAudioSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize default audio layer configurations
    AudioLayers.Add(EAudioLayer::Ambience, FAudioLayerConfig());
    AudioLayers.Add(EAudioLayer::Music, FAudioLayerConfig());
    AudioLayers.Add(EAudioLayer::SFX, FAudioLayerConfig());
    AudioLayers.Add(EAudioLayer::Voice, FAudioLayerConfig());
    AudioLayers.Add(EAudioLayer::UI, FAudioLayerConfig());

    // Set default volumes for each layer
    AudioLayers[EAudioLayer::Ambience].Volume = 0.7f;
    AudioLayers[EAudioLayer::Music].Volume = 0.6f;
    AudioLayers[EAudioLayer::SFX].Volume = 0.8f;
    AudioLayers[EAudioLayer::Voice].Volume = 1.0f;
    AudioLayers[EAudioLayer::UI].Volume = 0.5f;
}

void UAudioSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioSystem();
    SetupDefaultConsciousnessProfiles();
    InitializeAudioComponents();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio System Manager initialized - Ready for consciousness-based audio"));
}

void UAudioSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsTransitioning)
    {
        UpdateConsciousnessTransition(DeltaTime);
    }
    
    UpdateAudioMixerSettings();
}

void UAudioSystemManager::InitializeAudioSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing Transpersonal Audio System..."));
    
    // Verify audio device is available
    FAudioDevice* AudioDevice = GetAudioDevice();
    if (!AudioDevice)
    {
        UE_LOG(LogTemp, Error, TEXT("Audio device not available - cannot initialize audio system"));
        return;
    }
    
    // Initialize audio components for each layer
    InitializeAudioComponents();
    
    // Set initial consciousness state
    SetConsciousnessState(EConsciousnessState::Awakening);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio System initialization complete"));
}

void UAudioSystemManager::SetAudioLayerVolume(EAudioLayer Layer, float Volume, float FadeTime)
{
    if (!AudioLayers.Contains(Layer))
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio layer not found: %d"), (int32)Layer);
        return;
    }
    
    FAudioLayerConfig& LayerConfig = AudioLayers[Layer];
    LayerConfig.Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
    LayerConfig.FadeTime = FadeTime;
    
    // Apply volume to audio component if it exists
    if (AudioComponents.Contains(Layer) && AudioComponents[Layer])
    {
        AudioComponents[Layer]->SetVolumeMultiplier(LayerConfig.Volume);
        AudioComponents[Layer]->FadeIn(FadeTime, LayerConfig.Volume);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Set audio layer %d volume to %f"), (int32)Layer, Volume);
}

void UAudioSystemManager::PlaySoundOnLayer(EAudioLayer Layer, USoundCue* Sound, bool bLoop)
{
    if (!Sound)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempted to play null sound on layer %d"), (int32)Layer);
        return;
    }
    
    if (!AudioComponents.Contains(Layer) || !AudioComponents[Layer])
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio component not found for layer %d"), (int32)Layer);
        return;
    }
    
    UAudioComponent* AudioComp = AudioComponents[Layer];
    
    // Stop current sound if playing
    if (AudioComp->IsPlaying())
    {
        AudioComp->Stop();
    }
    
    // Configure and play new sound
    AudioComp->SetSound(Sound);
    AudioComp->SetLooping(bLoop);
    AudioComp->SetVolumeMultiplier(AudioLayers[Layer].Volume);
    AudioComp->Play();
    
    // Update layer config
    AudioLayers[Layer].CurrentSound = Sound;
    AudioLayers[Layer].bIsActive = true;
    
    UE_LOG(LogTemp, Log, TEXT("Playing sound on layer %d: %s"), (int32)Layer, *Sound->GetName());
}

void UAudioSystemManager::StopSoundOnLayer(EAudioLayer Layer, float FadeTime)
{
    if (!AudioComponents.Contains(Layer) || !AudioComponents[Layer])
    {
        return;
    }
    
    UAudioComponent* AudioComp = AudioComponents[Layer];
    
    if (AudioComp->IsPlaying())
    {
        if (FadeTime > 0.0f)
        {
            AudioComp->FadeOut(FadeTime, 0.0f);
        }
        else
        {
            AudioComp->Stop();
        }
    }
    
    AudioLayers[Layer].bIsActive = false;
    AudioLayers[Layer].CurrentSound = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("Stopped sound on layer %d"), (int32)Layer);
}

void UAudioSystemManager::SetConsciousnessState(EConsciousnessState NewState)
{
    if (CurrentConsciousnessState == NewState)
    {
        return;
    }
    
    CurrentConsciousnessState = NewState;
    TargetConsciousnessState = NewState;
    bIsTransitioning = false;
    TransitionProgress = 1.0f;
    
    // Apply consciousness audio profile immediately
    if (ConsciousnessProfiles.Contains(NewState))
    {
        ApplyConsciousnessAudioProfile(ConsciousnessProfiles[NewState], 1.0f);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Consciousness state changed to: %d"), (int32)NewState);
}

void UAudioSystemManager::TransitionToConsciousnessState(EConsciousnessState NewState, float TransitionTime)
{
    if (CurrentConsciousnessState == NewState)
    {
        return;
    }
    
    TargetConsciousnessState = NewState;
    TransitionDuration = FMath::Max(TransitionTime, 0.1f);
    TransitionProgress = 0.0f;
    bIsTransitioning = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Starting consciousness transition from %d to %d over %f seconds"), 
           (int32)CurrentConsciousnessState, (int32)NewState, TransitionTime);
}

void UAudioSystemManager::SetMetaSoundParameter(EAudioLayer Layer, FName ParameterName, float Value)
{
    if (!ActiveMetaSounds.Contains(Layer) || !ActiveMetaSounds[Layer])
    {
        UE_LOG(LogTemp, Warning, TEXT("No MetaSound active on layer %d"), (int32)Layer);
        return;
    }
    
    // MetaSound parameter setting would be implemented here
    // This requires UE5's MetaSound API which may vary by version
    UE_LOG(LogTemp, Log, TEXT("Setting MetaSound parameter %s to %f on layer %d"), 
           *ParameterName.ToString(), Value, (int32)Layer);
}

void UAudioSystemManager::TriggerMetaSoundEvent(EAudioLayer Layer, FName EventName)
{
    if (!ActiveMetaSounds.Contains(Layer) || !ActiveMetaSounds[Layer])
    {
        UE_LOG(LogTemp, Warning, TEXT("No MetaSound active on layer %d"), (int32)Layer);
        return;
    }
    
    // MetaSound event triggering would be implemented here
    UE_LOG(LogTemp, Log, TEXT("Triggering MetaSound event %s on layer %d"), 
           *EventName.ToString(), (int32)Layer);
}

void UAudioSystemManager::UpdateListenerPosition(FVector Position, FRotator Rotation)
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* Pawn = PC->GetPawn())
            {
                // Update 3D audio listener position
                UGameplayStatics::SetGlobalListenerFocusParameters(World, 0.0f, Position, Rotation.Vector());
            }
        }
    }
}

void UAudioSystemManager::RegisterSpatialAudioSource(AActor* SourceActor, USoundCue* Sound, float MaxDistance)
{
    if (!SourceActor || !Sound)
    {
        return;
    }
    
    // Create audio component on the source actor
    UAudioComponent* SpatialAudioComp = UGameplayStatics::SpawnSoundAttached(
        Sound, SourceActor->GetRootComponent(), NAME_None, FVector::ZeroVector, 
        EAttachLocation::KeepRelativeOffset, false, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true
    );
    
    if (SpatialAudioComp)
    {
        SpatialAudioComp->SetIntParameter(FName("MaxDistance"), MaxDistance);
        UE_LOG(LogTemp, Log, TEXT("Registered spatial audio source: %s"), *SourceActor->GetName());
    }
}

void UAudioSystemManager::SetEnvironmentalReverb(float ReverbAmount, float DecayTime)
{
    // Environmental reverb implementation
    // This would typically involve setting reverb submix parameters
    UE_LOG(LogTemp, Log, TEXT("Setting environmental reverb: Amount=%f, Decay=%f"), ReverbAmount, DecayTime);
}

void UAudioSystemManager::UpdateWeatherAudio(float WindIntensity, float RainIntensity, float ThunderProbability)
{
    // Weather audio updates - would modify MetaSound parameters for weather layers
    UE_LOG(LogTemp, Log, TEXT("Weather audio update: Wind=%f, Rain=%f, Thunder=%f"), 
           WindIntensity, RainIntensity, ThunderProbability);
}

void UAudioSystemManager::UpdateConsciousnessTransition(float DeltaTime)
{
    if (!bIsTransitioning)
    {
        return;
    }
    
    TransitionProgress += DeltaTime / TransitionDuration;
    
    if (TransitionProgress >= 1.0f)
    {
        // Transition complete
        TransitionProgress = 1.0f;
        bIsTransitioning = false;
        CurrentConsciousnessState = TargetConsciousnessState;
        
        UE_LOG(LogTemp, Warning, TEXT("Consciousness transition complete - now in state %d"), (int32)CurrentConsciousnessState);
    }
    
    // Blend between current and target consciousness profiles
    if (ConsciousnessProfiles.Contains(CurrentConsciousnessState) && ConsciousnessProfiles.Contains(TargetConsciousnessState))
    {
        float BlendAlpha = FMath::SmoothStep(0.0f, 1.0f, TransitionProgress);
        
        // Apply blended consciousness profile
        const FConsciousnessAudioProfile& CurrentProfile = ConsciousnessProfiles[CurrentConsciousnessState];
        const FConsciousnessAudioProfile& TargetProfile = ConsciousnessProfiles[TargetConsciousnessState];
        
        // Blend reverb wetness
        float BlendedReverb = FMath::Lerp(CurrentProfile.ReverbWetness, TargetProfile.ReverbWetness, BlendAlpha);
        SetEnvironmentalReverb(BlendedReverb, 2.0f);
    }
}

void UAudioSystemManager::ApplyConsciousnessAudioProfile(const FConsciousnessAudioProfile& Profile, float BlendWeight)
{
    // Apply reverb settings
    SetEnvironmentalReverb(Profile.ReverbWetness * BlendWeight, 2.0f);
    
    // Apply MetaSound assignments
    if (Profile.AmbienceMetaSound)
    {
        ActiveMetaSounds[EAudioLayer::Ambience] = Profile.AmbienceMetaSound;
    }
    
    if (Profile.MusicMetaSound)
    {
        ActiveMetaSounds[EAudioLayer::Music] = Profile.MusicMetaSound;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied consciousness audio profile with blend weight %f"), BlendWeight);
}

void UAudioSystemManager::InitializeAudioComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot initialize audio components - no owner actor"));
        return;
    }
    
    // Create audio components for each layer
    for (auto& LayerPair : AudioLayers)
    {
        EAudioLayer Layer = LayerPair.Key;
        
        UAudioComponent* AudioComp = NewObject<UAudioComponent>(Owner);
        if (AudioComp)
        {
            AudioComp->AttachToComponent(Owner->GetRootComponent(), 
                FAttachmentTransformRules::KeepRelativeTransform);
            AudioComp->SetVolumeMultiplier(LayerPair.Value.Volume);
            AudioComp->RegisterComponent();
            
            AudioComponents.Add(Layer, AudioComp);
            
            UE_LOG(LogTemp, Log, TEXT("Created audio component for layer %d"), (int32)Layer);
        }
    }
}

void UAudioSystemManager::SetupDefaultConsciousnessProfiles()
{
    // Awakening State - Minimal, mysterious
    FConsciousnessAudioProfile AwakeningProfile;
    AwakeningProfile.ReverbWetness = 0.2f;
    AwakeningProfile.LowPassFilter = 0.7f;
    AwakeningProfile.SpatialBlend = 0.3f;
    ConsciousnessProfiles.Add(EConsciousnessState::Awakening, AwakeningProfile);
    
    // Grounded State - Natural, present
    FConsciousnessAudioProfile GroundedProfile;
    GroundedProfile.ReverbWetness = 0.4f;
    GroundedProfile.LowPassFilter = 1.0f;
    GroundedProfile.SpatialBlend = 0.6f;
    ConsciousnessProfiles.Add(EConsciousnessState::Grounded, GroundedProfile);
    
    // Elevated State - Expanded, flowing
    FConsciousnessAudioProfile ElevatedProfile;
    ElevatedProfile.ReverbWetness = 0.6f;
    ElevatedProfile.LowPassFilter = 1.0f;
    ElevatedProfile.SpatialBlend = 0.8f;
    ConsciousnessProfiles.Add(EConsciousnessState::Elevated, ElevatedProfile);
    
    // Transcendent State - Ethereal, boundless
    FConsciousnessAudioProfile TranscendentProfile;
    TranscendentProfile.ReverbWetness = 0.8f;
    TranscendentProfile.LowPassFilter = 0.9f;
    TranscendentProfile.SpatialBlend = 1.0f;
    ConsciousnessProfiles.Add(EConsciousnessState::Transcendent, TranscendentProfile);
    
    // Unity State - Infinite, unified
    FConsciousnessAudioProfile UnityProfile;
    UnityProfile.ReverbWetness = 1.0f;
    UnityProfile.LowPassFilter = 0.8f;
    UnityProfile.SpatialBlend = 1.0f;
    ConsciousnessProfiles.Add(EConsciousnessState::Unity, UnityProfile);
    
    UE_LOG(LogTemp, Warning, TEXT("Default consciousness audio profiles configured"));
}

FAudioDevice* UAudioSystemManager::GetAudioDevice() const
{
    if (UWorld* World = GetWorld())
    {
        return World->GetAudioDeviceRaw();
    }
    return nullptr;
}

void UAudioSystemManager::UpdateAudioMixerSettings()
{
    // Continuous audio mixer updates based on current state
    // This would be called every tick to maintain smooth audio transitions
}