#include "Audio_AdaptiveMusicSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UAudio_AdaptiveMusicComponent::UAudio_AdaptiveMusicComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    CurrentBiome = EAudio_BiomeType::Forest;
    CurrentMusicLayer = EAudio_MusicLayer::Ambient;
    CurrentThreatLevel = 0.0f;
    LayerTransitionSpeed = 1.0f;
    ThreatDecayRate = 0.1f;
}

void UAudio_AdaptiveMusicComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeProfiles();
    
    // Registar com o subsystem global
    if (UAudio_AdaptiveMusicSubsystem* MusicSubsystem = GetWorld()->GetSubsystem<UAudio_AdaptiveMusicSubsystem>())
    {
        MusicSubsystem->RegisterMusicComponent(this);
    }
    
    // Iniciar com música ambiente
    TransitionToLayer(EAudio_MusicLayer::Ambient);
}

void UAudio_AdaptiveMusicComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateMusicIntensity(DeltaTime);
}

void UAudio_AdaptiveMusicComponent::InitializeBiomeProfiles()
{
    // Forest Biome Profile
    FAudio_BiomeMusicProfile ForestProfile;
    ForestProfile.BiomeType = EAudio_BiomeType::Forest;
    ForestProfile.BaseIntensity = 0.3f;
    ForestProfile.ThreatMultiplier = 1.8f;
    
    // Swamp Biome Profile
    FAudio_BiomeMusicProfile SwampProfile;
    SwampProfile.BiomeType = EAudio_BiomeType::Swamp;
    SwampProfile.BaseIntensity = 0.4f;
    SwampProfile.ThreatMultiplier = 2.2f;
    
    // Savanna Biome Profile
    FAudio_BiomeMusicProfile SavannaProfile;
    SavannaProfile.BiomeType = EAudio_BiomeType::Savanna;
    SavannaProfile.BaseIntensity = 0.2f;
    SavannaProfile.ThreatMultiplier = 1.5f;
    
    // Desert Biome Profile
    FAudio_BiomeMusicProfile DesertProfile;
    DesertProfile.BiomeType = EAudio_BiomeType::Desert;
    DesertProfile.BaseIntensity = 0.1f;
    DesertProfile.ThreatMultiplier = 2.5f;
    
    // Mountain Biome Profile
    FAudio_BiomeMusicProfile MountainProfile;
    MountainProfile.BiomeType = EAudio_BiomeType::Mountain;
    MountainProfile.BaseIntensity = 0.25f;
    MountainProfile.ThreatMultiplier = 2.0f;
    
    BiomeProfiles.Add(EAudio_BiomeType::Forest, ForestProfile);
    BiomeProfiles.Add(EAudio_BiomeType::Swamp, SwampProfile);
    BiomeProfiles.Add(EAudio_BiomeType::Savanna, SavannaProfile);
    BiomeProfiles.Add(EAudio_BiomeType::Desert, DesertProfile);
    BiomeProfiles.Add(EAudio_BiomeType::Mountain, MountainProfile);
}

void UAudio_AdaptiveMusicComponent::SetBiome(EAudio_BiomeType NewBiome)
{
    if (CurrentBiome != NewBiome)
    {
        CurrentBiome = NewBiome;
        
        // Recalcular layer baseado no novo bioma
        EAudio_MusicLayer TargetLayer = CalculateTargetLayer();
        TransitionToLayer(TargetLayer);
        
        UE_LOG(LogTemp, Log, TEXT("Music system changed to biome: %d"), (int32)NewBiome);
    }
}

void UAudio_AdaptiveMusicComponent::SetThreatLevel(float ThreatLevel)
{
    CurrentThreatLevel = FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
    
    // Calcular layer apropriado baseado no threat level
    EAudio_MusicLayer TargetLayer = CalculateTargetLayer();
    
    if (TargetLayer != CurrentMusicLayer)
    {
        TransitionToLayer(TargetLayer);
    }
}

EAudio_MusicLayer UAudio_AdaptiveMusicComponent::CalculateTargetLayer()
{
    if (!BiomeProfiles.Contains(CurrentBiome))
    {
        return EAudio_MusicLayer::Ambient;
    }
    
    const FAudio_BiomeMusicProfile& Profile = BiomeProfiles[CurrentBiome];
    float AdjustedThreat = CurrentThreatLevel * Profile.ThreatMultiplier;
    
    if (AdjustedThreat >= 0.8f)
    {
        return EAudio_MusicLayer::Combat;
    }
    else if (AdjustedThreat >= 0.6f)
    {
        return EAudio_MusicLayer::Danger;
    }
    else if (AdjustedThreat >= 0.3f)
    {
        return EAudio_MusicLayer::Tension;
    }
    else if (AdjustedThreat >= 0.1f)
    {
        return EAudio_MusicLayer::Exploration;
    }
    else
    {
        return EAudio_MusicLayer::Ambient;
    }
}

void UAudio_AdaptiveMusicComponent::TransitionToLayer(EAudio_MusicLayer NewLayer)
{
    if (CurrentMusicLayer == NewLayer)
    {
        return;
    }
    
    // Fade out current layer
    if (ActiveAudioComponents.Contains(CurrentMusicLayer))
    {
        UAudioComponent* CurrentAudio = ActiveAudioComponents[CurrentMusicLayer];
        if (CurrentAudio && CurrentAudio->IsPlaying())
        {
            FadeAudioComponent(CurrentAudio, 0.0f, 2.0f);
        }
    }
    
    // Fade in new layer
    if (!ActiveAudioComponents.Contains(NewLayer))
    {
        CreateAudioComponentForLayer(NewLayer);
    }
    
    if (ActiveAudioComponents.Contains(NewLayer))
    {
        UAudioComponent* NewAudio = ActiveAudioComponents[NewLayer];
        if (NewAudio)
        {
            if (!NewAudio->IsPlaying())
            {
                NewAudio->Play();
            }
            FadeAudioComponent(NewAudio, 1.0f, 2.0f);
        }
    }
    
    CurrentMusicLayer = NewLayer;
    UE_LOG(LogTemp, Log, TEXT("Music transitioned to layer: %d"), (int32)NewLayer);
}

void UAudio_AdaptiveMusicComponent::CreateAudioComponentForLayer(EAudio_MusicLayer Layer)
{
    if (!BiomeProfiles.Contains(CurrentBiome))
    {
        return;
    }
    
    const FAudio_BiomeMusicProfile& Profile = BiomeProfiles[CurrentBiome];
    
    if (!Profile.MusicLayers.Contains(Layer))
    {
        return;
    }
    
    const FAudio_MusicLayerData& LayerData = Profile.MusicLayers[Layer];
    
    if (!LayerData.SoundAsset)
    {
        return;
    }
    
    UAudioComponent* AudioComp = UGameplayStatics::CreateSound2D(GetWorld(), LayerData.SoundAsset);
    if (AudioComp)
    {
        AudioComp->SetVolumeMultiplier(0.0f); // Start muted for fade in
        AudioComp->bIsUISound = false;
        AudioComp->bAutoDestroy = false;
        
        ActiveAudioComponents.Add(Layer, AudioComp);
    }
}

void UAudio_AdaptiveMusicComponent::FadeAudioComponent(UAudioComponent* AudioComp, float TargetVolume, float FadeTime)
{
    if (!AudioComp)
    {
        return;
    }
    
    // Simple fade implementation - in production would use timeline or tween
    float CurrentVolume = AudioComp->VolumeMultiplier;
    float VolumeStep = (TargetVolume - CurrentVolume) / (FadeTime * 60.0f); // Assuming 60 FPS
    
    // Store fade parameters for tick update
    // For now, set directly - would implement proper fade in production
    AudioComp->SetVolumeMultiplier(TargetVolume);
    
    if (TargetVolume <= 0.0f)
    {
        // Schedule stop after fade
        FTimerHandle StopTimer;
        GetWorld()->GetTimerManager().SetTimer(StopTimer, [AudioComp]()
        {
            if (AudioComp)
            {
                AudioComp->Stop();
            }
        }, FadeTime, false);
    }
}

void UAudio_AdaptiveMusicComponent::UpdateMusicIntensity(float DeltaTime)
{
    // Decay threat level over time
    if (CurrentThreatLevel > 0.0f)
    {
        CurrentThreatLevel = FMath::Max(0.0f, CurrentThreatLevel - (ThreatDecayRate * DeltaTime));
        
        // Check if we need to transition to a lower intensity layer
        EAudio_MusicLayer TargetLayer = CalculateTargetLayer();
        if (TargetLayer != CurrentMusicLayer)
        {
            TransitionToLayer(TargetLayer);
        }
    }
}

// Subsystem Implementation
void UAudio_AdaptiveMusicSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    GlobalMusicComponent = nullptr;
}

void UAudio_AdaptiveMusicSubsystem::Deinitialize()
{
    GlobalMusicComponent = nullptr;
    Super::Deinitialize();
}

void UAudio_AdaptiveMusicSubsystem::RegisterMusicComponent(UAudio_AdaptiveMusicComponent* MusicComponent)
{
    if (MusicComponent)
    {
        GlobalMusicComponent = MusicComponent;
        UE_LOG(LogTemp, Log, TEXT("Global music component registered"));
    }
}

void UAudio_AdaptiveMusicSubsystem::SetGlobalThreatLevel(float ThreatLevel)
{
    if (GlobalMusicComponent)
    {
        GlobalMusicComponent->SetThreatLevel(ThreatLevel);
    }
}

void UAudio_AdaptiveMusicSubsystem::SetGlobalBiome(EAudio_BiomeType Biome)
{
    if (GlobalMusicComponent)
    {
        GlobalMusicComponent->SetBiome(Biome);
    }
}