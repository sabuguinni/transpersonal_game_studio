#include "Audio_MetaSoundManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UAudio_MetaSoundManager::UAudio_MetaSoundManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default values
    CurrentBiome = EAudio_BiomeType::Forest;
    RandomSoundTimer = 0.0f;
    HeartbeatTimer = 0.0f;
    bIsInDanger = false;
    HeartbeatBaseVolume = 0.3f;
    DangerWarningVolume = 0.8f;
    
    // Initialize sound states for all layers
    for (int32 i = 0; i < (int32)EAudio_SoundLayer::UI + 1; i++)
    {
        EAudio_SoundLayer Layer = (EAudio_SoundLayer)i;
        FAudio_SoundState& State = SoundStates.Add(Layer);
        State.Layer = Layer;
        State.Volume = 1.0f;
        State.Pitch = 1.0f;
        State.bIsPlaying = false;
    }
    
    // Initialize biome audio data with default values
    for (int32 i = 0; i < (int32)EAudio_BiomeType::Mountains + 1; i++)
    {
        EAudio_BiomeType Biome = (EAudio_BiomeType)i;
        FAudio_BiomeAudioData& Data = BiomeAudioData.Add(Biome);
        Data.AmbientVolume = 0.7f;
        Data.RandomSoundChance = 0.1f;
    }
}

void UAudio_MetaSoundManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioComponents();
    SetCurrentBiome(CurrentBiome);
}

void UAudio_MetaSoundManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateBiomeAudio(DeltaTime);
    UpdateRandomSounds(DeltaTime);
}

void UAudio_MetaSoundManager::InitializeAudioComponents()
{
    if (!GetOwner())
    {
        return;
    }
    
    // Create audio components for each layer
    for (auto& StatePair : SoundStates)
    {
        EAudio_SoundLayer Layer = StatePair.Key;
        UAudioComponent* AudioComp = GetOrCreateAudioComponent(Layer);
        
        if (AudioComp)
        {
            LayerComponents.Add(Layer, AudioComp);
            
            // Set default properties based on layer type
            switch (Layer)
            {
                case EAudio_SoundLayer::Ambient:
                    AudioComp->bAutoActivate = true;
                    AudioComp->VolumeMultiplier = 0.7f;
                    break;
                case EAudio_SoundLayer::Music:
                    AudioComp->bAutoActivate = false;
                    AudioComp->VolumeMultiplier = 0.5f;
                    break;
                case EAudio_SoundLayer::SFX:
                    AudioComp->bAutoActivate = false;
                    AudioComp->VolumeMultiplier = 0.8f;
                    break;
                case EAudio_SoundLayer::Dialogue:
                    AudioComp->bAutoActivate = false;
                    AudioComp->VolumeMultiplier = 1.0f;
                    break;
                case EAudio_SoundLayer::UI:
                    AudioComp->bAutoActivate = false;
                    AudioComp->VolumeMultiplier = 0.6f;
                    break;
            }
        }
    }
}

UAudioComponent* UAudio_MetaSoundManager::GetOrCreateAudioComponent(EAudio_SoundLayer Layer)
{
    if (!GetOwner())
    {
        return nullptr;
    }
    
    // Check if component already exists
    if (UAudioComponent** ExistingComp = LayerComponents.Find(Layer))
    {
        if (*ExistingComp && IsValid(*ExistingComp))
        {
            return *ExistingComp;
        }
    }
    
    // Create new audio component
    FString ComponentName = FString::Printf(TEXT("AudioComp_%s"), 
        *UEnum::GetValueAsString(Layer));
    
    UAudioComponent* NewAudioComp = NewObject<UAudioComponent>(GetOwner(), 
        UAudioComponent::StaticClass(), *ComponentName);
    
    if (NewAudioComp)
    {
        NewAudioComp->AttachToComponent(GetOwner()->GetRootComponent(), 
            FAttachmentTransformRules::KeepRelativeTransform);
        NewAudioComp->RegisterComponent();
    }
    
    return NewAudioComp;
}

void UAudio_MetaSoundManager::PlaySound(USoundCue* SoundCue, EAudio_SoundLayer Layer, float Volume, float Pitch)
{
    if (!SoundCue)
    {
        return;
    }
    
    UAudioComponent* AudioComp = GetOrCreateAudioComponent(Layer);
    if (!AudioComp)
    {
        return;
    }
    
    // Update sound state
    if (FAudio_SoundState* State = SoundStates.Find(Layer))
    {
        State->Volume = Volume;
        State->Pitch = Pitch;
        State->bIsPlaying = true;
    }
    
    // Configure and play sound
    AudioComp->SetSound(SoundCue);
    AudioComp->SetVolumeMultiplier(Volume);
    AudioComp->SetPitchMultiplier(Pitch);
    AudioComp->Play();
}

void UAudio_MetaSoundManager::StopSound(EAudio_SoundLayer Layer)
{
    UAudioComponent* AudioComp = GetOrCreateAudioComponent(Layer);
    if (AudioComp && AudioComp->IsPlaying())
    {
        AudioComp->Stop();
    }
    
    // Update sound state
    if (FAudio_SoundState* State = SoundStates.Find(Layer))
    {
        State->bIsPlaying = false;
    }
}

void UAudio_MetaSoundManager::SetLayerVolume(EAudio_SoundLayer Layer, float Volume)
{
    UAudioComponent* AudioComp = GetOrCreateAudioComponent(Layer);
    if (AudioComp)
    {
        AudioComp->SetVolumeMultiplier(Volume);
    }
    
    // Update sound state
    if (FAudio_SoundState* State = SoundStates.Find(Layer))
    {
        State->Volume = Volume;
    }
}

void UAudio_MetaSoundManager::SetCurrentBiome(EAudio_BiomeType BiomeType)
{
    if (CurrentBiome == BiomeType)
    {
        return;
    }
    
    CurrentBiome = BiomeType;
    
    // Stop current ambient audio
    StopSound(EAudio_SoundLayer::Ambient);
    
    // Start new biome ambient audio
    if (FAudio_BiomeAudioData* BiomeData = BiomeAudioData.Find(CurrentBiome))
    {
        if (BiomeData->AmbientLoop.IsValid())
        {
            USoundCue* AmbientCue = BiomeData->AmbientLoop.LoadSynchronous();
            if (AmbientCue)
            {
                PlaySound(AmbientCue, EAudio_SoundLayer::Ambient, BiomeData->AmbientVolume);
            }
        }
    }
}

void UAudio_MetaSoundManager::UpdateBiomeAudio(float DeltaTime)
{
    // Ensure ambient audio is playing for current biome
    if (FAudio_SoundState* AmbientState = SoundStates.Find(EAudio_SoundLayer::Ambient))
    {
        if (!AmbientState->bIsPlaying)
        {
            SetCurrentBiome(CurrentBiome); // Restart ambient audio
        }
    }
}

void UAudio_MetaSoundManager::UpdateRandomSounds(float DeltaTime)
{
    RandomSoundTimer += DeltaTime;
    
    // Check for random sound trigger every 5 seconds
    if (RandomSoundTimer >= 5.0f)
    {
        RandomSoundTimer = 0.0f;
        
        if (FAudio_BiomeAudioData* BiomeData = BiomeAudioData.Find(CurrentBiome))
        {
            float RandomValue = FMath::RandRange(0.0f, 1.0f);
            if (RandomValue <= BiomeData->RandomSoundChance && BiomeData->RandomSounds.Num() > 0)
            {
                int32 RandomIndex = FMath::RandRange(0, BiomeData->RandomSounds.Num() - 1);
                if (BiomeData->RandomSounds[RandomIndex].IsValid())
                {
                    USoundCue* RandomCue = BiomeData->RandomSounds[RandomIndex].LoadSynchronous();
                    if (RandomCue)
                    {
                        PlaySound(RandomCue, EAudio_SoundLayer::SFX, 0.6f);
                    }
                }
            }
        }
    }
}

void UAudio_MetaSoundManager::PlayDialogue(USoundCue* DialogueCue, float Volume)
{
    if (DialogueCue)
    {
        // Stop any current dialogue
        StopSound(EAudio_SoundLayer::Dialogue);
        
        // Play new dialogue
        PlaySound(DialogueCue, EAudio_SoundLayer::Dialogue, Volume);
    }
}

void UAudio_MetaSoundManager::PlayNarration(USoundCue* NarrationCue, float Volume)
{
    if (NarrationCue)
    {
        PlaySound(NarrationCue, EAudio_SoundLayer::Music, Volume);
    }
}

void UAudio_MetaSoundManager::PlayDangerWarning(float ThreatLevel)
{
    if (DangerWarningSound.IsValid())
    {
        USoundCue* WarningSoundCue = DangerWarningSound.LoadSynchronous();
        if (WarningSoundCue)
        {
            float WarningVolume = DangerWarningVolume * FMath::Clamp(ThreatLevel, 0.3f, 1.0f);
            PlaySound(WarningSoundCue, EAudio_SoundLayer::SFX, WarningVolume);
        }
    }
    
    bIsInDanger = ThreatLevel > 0.5f;
}

void UAudio_MetaSoundManager::PlayHeartbeat(float Intensity)
{
    if (HeartbeatSound.IsValid())
    {
        USoundCue* HeartbeatCue = HeartbeatSound.LoadSynchronous();
        if (HeartbeatCue)
        {
            float HeartbeatVolume = HeartbeatBaseVolume * FMath::Clamp(Intensity, 0.1f, 1.0f);
            float HeartbeatPitch = 1.0f + (Intensity * 0.5f); // Faster when more intense
            PlaySound(HeartbeatCue, EAudio_SoundLayer::SFX, HeartbeatVolume, HeartbeatPitch);
        }
    }
}

void UAudio_MetaSoundManager::UpdateSurvivalAudio(float Health, float Hunger, float Thirst, float Fear)
{
    // Calculate overall stress level
    float StressLevel = 0.0f;
    
    if (Health < 0.3f) StressLevel += (0.3f - Health) * 2.0f;
    if (Hunger > 0.8f) StressLevel += (Hunger - 0.8f) * 1.5f;
    if (Thirst > 0.8f) StressLevel += (Thirst - 0.8f) * 1.5f;
    if (Fear > 0.5f) StressLevel += (Fear - 0.5f) * 2.0f;
    
    StressLevel = FMath::Clamp(StressLevel, 0.0f, 1.0f);
    
    // Update heartbeat based on stress
    UpdateHeartbeatAudio(GetWorld()->GetDeltaSeconds(), StressLevel);
    
    // Adjust ambient volume based on fear
    if (Fear > 0.7f)
    {
        SetLayerVolume(EAudio_SoundLayer::Ambient, 0.3f); // Muffled when terrified
    }
    else
    {
        SetLayerVolume(EAudio_SoundLayer::Ambient, 0.7f); // Normal ambient volume
    }
}

void UAudio_MetaSoundManager::UpdateHeartbeatAudio(float DeltaTime, float FearLevel)
{
    HeartbeatTimer += DeltaTime;
    
    // Heartbeat frequency based on fear level
    float HeartbeatInterval = FMath::Lerp(2.0f, 0.8f, FearLevel); // 2s to 0.8s between beats
    
    if (HeartbeatTimer >= HeartbeatInterval && FearLevel > 0.2f)
    {
        HeartbeatTimer = 0.0f;
        PlayHeartbeat(FearLevel);
    }
}