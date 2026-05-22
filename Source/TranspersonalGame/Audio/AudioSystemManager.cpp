#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

UAudioSystemManager::UAudioSystemManager()
{
    CurrentAmbientComponent = nullptr;
    CurrentBiome = EBiomeType::Savana;
    MasterVolume = 1.0f;
    AmbientVolume = 0.7f;
    SFXVolume = 0.8f;
    
    PlayerFootstepSound = nullptr;
    DamageSound = nullptr;
    HeartbeatSound = nullptr;
}

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeAudioSystem();
}

void UAudioSystemManager::Deinitialize()
{
    CleanupInactiveComponents();
    
    if (CurrentAmbientComponent && IsValid(CurrentAmbientComponent))
    {
        CurrentAmbientComponent->Stop();
        CurrentAmbientComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UAudioSystemManager::InitializeAudioSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Initializing audio system"));
    
    // Initialize biome audio data
    FAudio_BiomeAudioData SavanaAudio;
    SavanaAudio.Volume = 0.6f;
    SavanaAudio.FadeTime = 3.0f;
    BiomeAudioMap.Add(EBiomeType::Savana, SavanaAudio);
    
    FAudio_BiomeAudioData FlorestaAudio;
    FlorestaAudio.Volume = 0.8f;
    FlorestaAudio.FadeTime = 2.5f;
    BiomeAudioMap.Add(EBiomeType::Floresta, FlorestaAudio);
    
    FAudio_BiomeAudioData DesertoAudio;
    DesertoAudio.Volume = 0.5f;
    DesertoAudio.FadeTime = 4.0f;
    BiomeAudioMap.Add(EBiomeType::Deserto, DesertoAudio);
    
    FAudio_BiomeAudioData PantanoAudio;
    PantanoAudio.Volume = 0.9f;
    PantanoAudio.FadeTime = 2.0f;
    BiomeAudioMap.Add(EBiomeType::Pantano, PantanoAudio);
    
    FAudio_BiomeAudioData MontanhaAudio;
    MontanhaAudio.Volume = 0.4f;
    MontanhaAudio.FadeTime = 5.0f;
    BiomeAudioMap.Add(EBiomeType::Montanha, MontanhaAudio);
    
    // Initialize dinosaur audio data
    FAudio_DinosaurAudioData TRexAudio;
    TRexAudio.ThreatRadius = 8000.0f;
    TRexAudio.FootstepVolume = 1.0f;
    DinosaurAudioMap.Add(EDinosaurSpecies::TRex, TRexAudio);
    
    FAudio_DinosaurAudioData VelociraptorAudio;
    VelociraptorAudio.ThreatRadius = 3000.0f;
    VelociraptorAudio.FootstepVolume = 0.4f;
    DinosaurAudioMap.Add(EDinosaurSpecies::Velociraptor, VelociraptorAudio);
    
    FAudio_DinosaurAudioData BrachiosaurusAudio;
    BrachiosaurusAudio.ThreatRadius = 6000.0f;
    BrachiosaurusAudio.FootstepVolume = 0.9f;
    DinosaurAudioMap.Add(EDinosaurSpecies::Brachiosaurus, BrachiosaurusAudio);
    
    FAudio_DinosaurAudioData TriceratopsAudio;
    TriceratopsAudio.ThreatRadius = 4000.0f;
    TriceratopsAudio.FootstepVolume = 0.7f;
    DinosaurAudioMap.Add(EDinosaurSpecies::Triceratops, TriceratopsAudio);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Audio system initialized with %d biomes and %d dinosaur species"), 
           BiomeAudioMap.Num(), DinosaurAudioMap.Num());
}

void UAudioSystemManager::UpdateBiomeAudio(EBiomeType BiomeType, const FVector& PlayerLocation)
{
    if (CurrentBiome == BiomeType)
    {
        return;
    }
    
    CurrentBiome = BiomeType;
    
    // Stop current ambient audio
    if (CurrentAmbientComponent && IsValid(CurrentAmbientComponent))
    {
        CurrentAmbientComponent->FadeOut(2.0f, 0.0f);
        CurrentAmbientComponent = nullptr;
    }
    
    // Start new biome ambient audio
    if (BiomeAudioMap.Contains(BiomeType))
    {
        const FAudio_BiomeAudioData& AudioData = BiomeAudioMap[BiomeType];
        if (AudioData.AmbientSound)
        {
            CurrentAmbientComponent = CreateAudioComponent(AudioData.AmbientSound, PlayerLocation, 
                                                         AudioData.Volume * AmbientVolume, true);
            if (CurrentAmbientComponent)
            {
                CurrentAmbientComponent->FadeIn(AudioData.FadeTime, AudioData.Volume * AmbientVolume);
                UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Switched to biome audio for %d"), (int32)BiomeType);
            }
        }
    }
}

void UAudioSystemManager::PlayDinosaurAudio(EDinosaurSpecies Species, const FVector& DinosaurLocation, const FVector& PlayerLocation)
{
    if (!DinosaurAudioMap.Contains(Species))
    {
        return;
    }
    
    const FAudio_DinosaurAudioData& AudioData = DinosaurAudioMap[Species];
    float Distance = FVector::Dist(DinosaurLocation, PlayerLocation);
    
    if (Distance > AudioData.ThreatRadius)
    {
        return;
    }
    
    float Attenuation = CalculateDistanceAttenuation(DinosaurLocation, PlayerLocation, AudioData.ThreatRadius);
    
    // Play roar sound
    if (AudioData.RoarSound)
    {
        UAudioComponent* RoarComponent = CreateAudioComponent(AudioData.RoarSound, DinosaurLocation, 
                                                            Attenuation * SFXVolume, false);
        if (RoarComponent)
        {
            ActiveAudioComponents.Add(RoarComponent);
            UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Playing roar for species %d at distance %.1f"), 
                   (int32)Species, Distance);
        }
    }
}

void UAudioSystemManager::PlayFootstepAudio(const FVector& Location, float Intensity)
{
    if (!PlayerFootstepSound)
    {
        return;
    }
    
    float Volume = FMath::Clamp(Intensity * SFXVolume, 0.1f, 1.0f);
    UAudioComponent* FootstepComponent = CreateAudioComponent(PlayerFootstepSound, Location, Volume, false);
    
    if (FootstepComponent)
    {
        ActiveAudioComponents.Add(FootstepComponent);
    }
}

void UAudioSystemManager::PlayDamageAudio(float DamageAmount, const FVector& ImpactLocation)
{
    if (!DamageSound)
    {
        return;
    }
    
    float Volume = FMath::Clamp(DamageAmount / 100.0f * SFXVolume, 0.3f, 1.0f);
    UAudioComponent* DamageComponent = CreateAudioComponent(DamageSound, ImpactLocation, Volume, false);
    
    if (DamageComponent)
    {
        ActiveAudioComponents.Add(DamageComponent);
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Playing damage audio with volume %.2f"), Volume);
    }
    
    // Play heartbeat if damage is severe
    if (DamageAmount > 50.0f && HeartbeatSound)
    {
        UAudioComponent* HeartbeatComponent = CreateAudioComponent(HeartbeatSound, ImpactLocation, 
                                                                 0.8f * SFXVolume, true);
        if (HeartbeatComponent)
        {
            ActiveAudioComponents.Add(HeartbeatComponent);
            // Stop heartbeat after 10 seconds
            FTimerHandle HeartbeatTimer;
            GetWorld()->GetTimerManager().SetTimer(HeartbeatTimer, [HeartbeatComponent]()
            {
                if (IsValid(HeartbeatComponent))
                {
                    HeartbeatComponent->FadeOut(2.0f, 0.0f);
                }
            }, 10.0f, false);
        }
    }
}

void UAudioSystemManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Master volume set to %.2f"), MasterVolume);
}

void UAudioSystemManager::SetAmbientVolume(float Volume)
{
    AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (CurrentAmbientComponent && IsValid(CurrentAmbientComponent))
    {
        CurrentAmbientComponent->SetVolumeMultiplier(AmbientVolume * MasterVolume);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Ambient volume set to %.2f"), AmbientVolume);
}

void UAudioSystemManager::SetSFXVolume(float Volume)
{
    SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: SFX volume set to %.2f"), SFXVolume);
}

void UAudioSystemManager::CleanupInactiveComponents()
{
    ActiveAudioComponents.RemoveAll([](UAudioComponent* Component)
    {
        if (!IsValid(Component) || !Component->IsPlaying())
        {
            if (IsValid(Component))
            {
                Component->DestroyComponent();
            }
            return true;
        }
        return false;
    });
}

UAudioComponent* UAudioSystemManager::CreateAudioComponent(USoundCue* SoundCue, const FVector& Location, float Volume, bool bLooping)
{
    if (!SoundCue || !GetWorld())
    {
        return nullptr;
    }
    
    UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(), SoundCue, Location, FRotator::ZeroRotator, Volume * MasterVolume);
    
    if (AudioComponent)
    {
        AudioComponent->bAutoDestroy = !bLooping;
        AudioComponent->SetUISound(false);
        
        if (bLooping)
        {
            AudioComponent->SetIntParameter(FName("Loop"), 1);
        }
    }
    
    return AudioComponent;
}

float UAudioSystemManager::CalculateDistanceAttenuation(const FVector& SourceLocation, const FVector& ListenerLocation, float MaxDistance)
{
    float Distance = FVector::Dist(SourceLocation, ListenerLocation);
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }
    
    return 1.0f - (Distance / MaxDistance);
}