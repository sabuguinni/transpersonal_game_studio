#include "DinosaurAudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

UDinosaurAudioComponent::UDinosaurAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    DinosaurSpecies = EAudio_DinosaurSpecies::TRex;
    CurrentThreatLevel = 0.0f;
    SoundCooldownTime = 2.0f;
    LastSoundTime = 0.0f;
    bUseDistanceAttenuation = true;
    MinPitchVariation = 0.8f;
    MaxPitchVariation = 1.2f;
    
    // Set default audio component properties
    bAutoActivate = false;
    bStopWhenOwnerDestroyed = true;
    VolumeMultiplier = 1.0f;
    PitchMultiplier = 1.0f;
}

void UDinosaurAudioComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeSpeciesSoundSets();
    
    // Create breathing audio component
    if (!BreathingComponent)
    {
        BreathingComponent = NewObject<UAudioComponent>(this);
        if (BreathingComponent)
        {
            BreathingComponent->bAutoActivate = false;
            BreathingComponent->bStopWhenOwnerDestroyed = true;
            BreathingComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
            BreathingComponent->RegisterComponent();
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurAudioComponent: Initialized for species %d"), (int32)DinosaurSpecies);
}

void UDinosaurAudioComponent::PlayDinosaurSound(EAudio_DinosaurSoundType SoundType, float VolumeMultiplier, float PitchVariation)
{
    if (!CanPlaySound())
    {
        return;
    }
    
    USoundBase* Sound = GetSoundForType(SoundType);
    if (!Sound)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurAudioComponent: No sound found for type %d"), (int32)SoundType);
        return;
    }
    
    // Calculate volume based on threat level
    float FinalVolume = CalculateVolumeFromThreat(VolumeMultiplier);
    
    // Calculate pitch with variation
    float BasePitch = CalculatePitchFromThreat(1.0f);
    float PitchRange = FMath::RandRange(-PitchVariation, PitchVariation);
    float FinalPitch = FMath::Clamp(BasePitch + PitchRange, MinPitchVariation, MaxPitchVariation);
    
    // Play the sound
    SetSound(Sound);
    SetVolumeMultiplier(FinalVolume);
    SetPitchMultiplier(FinalPitch);
    Play();
    
    LastSoundTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurAudioComponent: Played %d sound with volume %f, pitch %f"), 
           (int32)SoundType, FinalVolume, FinalPitch);
}

void UDinosaurAudioComponent::SetDinosaurSpecies(EAudio_DinosaurSpecies Species)
{
    DinosaurSpecies = Species;
    UE_LOG(LogTemp, Log, TEXT("DinosaurAudioComponent: Species set to %d"), (int32)Species);
}

void UDinosaurAudioComponent::PlayFootstepSound(float VolumeMultiplier)
{
    PlayDinosaurSound(EAudio_DinosaurSoundType::Footstep, VolumeMultiplier, 0.05f);
}

void UDinosaurAudioComponent::StartBreathingLoop()
{
    if (!BreathingComponent)
    {
        return;
    }
    
    USoundBase* BreathingSound = GetSoundForType(EAudio_DinosaurSoundType::Breathing);
    if (BreathingSound)
    {
        BreathingComponent->SetSound(BreathingSound);
        BreathingComponent->SetVolumeMultiplier(0.3f);
        BreathingComponent->SetPitchMultiplier(1.0f);
        BreathingComponent->Play();
        
        UE_LOG(LogTemp, Log, TEXT("DinosaurAudioComponent: Started breathing loop"));
    }
}

void UDinosaurAudioComponent::StopBreathingLoop()
{
    if (BreathingComponent && BreathingComponent->IsPlaying())
    {
        BreathingComponent->Stop();
        UE_LOG(LogTemp, Log, TEXT("DinosaurAudioComponent: Stopped breathing loop"));
    }
}

void UDinosaurAudioComponent::SetThreatLevel(float ThreatLevel)
{
    CurrentThreatLevel = FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("DinosaurAudioComponent: Threat level set to %f"), CurrentThreatLevel);
}

bool UDinosaurAudioComponent::IsPlayingDinosaurSound() const
{
    return IsPlaying();
}

void UDinosaurAudioComponent::SetAudioDistance(float Distance)
{
    if (bUseDistanceAttenuation)
    {
        FAudio_DinosaurSoundSet* SoundSet = SpeciesSoundSets.Find(DinosaurSpecies);
        if (SoundSet)
        {
            float DistanceRatio = FMath::Clamp(Distance / SoundSet->MaxAudibleDistance, 0.0f, 1.0f);
            float AttenuatedVolume = 1.0f - DistanceRatio;
            SetVolumeMultiplier(AttenuatedVolume);
        }
    }
}

void UDinosaurAudioComponent::InitializeSpeciesSoundSets()
{
    // Initialize T-Rex sound set
    FAudio_DinosaurSoundSet TRexSounds;
    TRexSounds.BaseVolume = 1.0f;
    TRexSounds.BasePitch = 0.8f;
    TRexSounds.MaxAudibleDistance = 8000.0f;
    SpeciesSoundSets.Add(EAudio_DinosaurSpecies::TRex, TRexSounds);
    
    // Initialize Velociraptor sound set
    FAudio_DinosaurSoundSet RaptorSounds;
    RaptorSounds.BaseVolume = 0.7f;
    RaptorSounds.BasePitch = 1.2f;
    RaptorSounds.MaxAudibleDistance = 3000.0f;
    SpeciesSoundSets.Add(EAudio_DinosaurSpecies::Velociraptor, RaptorSounds);
    
    // Initialize Triceratops sound set
    FAudio_DinosaurSoundSet TriceratopsSounds;
    TriceratopsSounds.BaseVolume = 0.9f;
    TriceratopsSounds.BasePitch = 0.7f;
    TriceratopsSounds.MaxAudibleDistance = 6000.0f;
    SpeciesSoundSets.Add(EAudio_DinosaurSpecies::Triceratops, TriceratopsSounds);
    
    // Initialize Brachiosaurus sound set
    FAudio_DinosaurSoundSet BrachiosaurusSounds;
    BrachiosaurusSounds.BaseVolume = 1.2f;
    BrachiosaurusSounds.BasePitch = 0.5f;
    BrachiosaurusSounds.MaxAudibleDistance = 10000.0f;
    SpeciesSoundSets.Add(EAudio_DinosaurSpecies::Brachiosaurus, BrachiosaurusSounds);
    
    // Initialize Ankylosaurus sound set
    FAudio_DinosaurSoundSet AnkylosaurusSounds;
    AnkylosaurusSounds.BaseVolume = 0.8f;
    AnkylosaurusSounds.BasePitch = 0.9f;
    AnkylosaurusSounds.MaxAudibleDistance = 4000.0f;
    SpeciesSoundSets.Add(EAudio_DinosaurSpecies::Ankylosaurus, AnkylosaurusSounds);
    
    // Initialize Parasaurolophus sound set
    FAudio_DinosaurSoundSet ParasaurolophSounds;
    ParasaurolophSounds.BaseVolume = 0.8f;
    ParasaurolophSounds.BasePitch = 1.1f;
    ParasaurolophSounds.MaxAudibleDistance = 7000.0f;
    SpeciesSoundSets.Add(EAudio_DinosaurSpecies::Parasaurolophus, ParasaurolophSounds);
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurAudioComponent: Species sound sets initialized"));
}

USoundBase* UDinosaurAudioComponent::GetSoundForType(EAudio_DinosaurSoundType SoundType)
{
    FAudio_DinosaurSoundSet* SoundSet = SpeciesSoundSets.Find(DinosaurSpecies);
    if (!SoundSet)
    {
        return nullptr;
    }
    
    TSoftObjectPtr<USoundBase>* SoundPtr = nullptr;
    
    switch (SoundType)
    {
        case EAudio_DinosaurSoundType::Idle:
            SoundPtr = &SoundSet->IdleSound;
            break;
        case EAudio_DinosaurSoundType::Alert:
            SoundPtr = &SoundSet->AlertSound;
            break;
        case EAudio_DinosaurSoundType::Aggressive:
            SoundPtr = &SoundSet->AggressiveSound;
            break;
        case EAudio_DinosaurSoundType::Pain:
            SoundPtr = &SoundSet->PainSound;
            break;
        case EAudio_DinosaurSoundType::Death:
            SoundPtr = &SoundSet->DeathSound;
            break;
        case EAudio_DinosaurSoundType::Footstep:
            SoundPtr = &SoundSet->FootstepSound;
            break;
        case EAudio_DinosaurSoundType::Breathing:
            SoundPtr = &SoundSet->BreathingSound;
            break;
        default:
            return nullptr;
    }
    
    if (SoundPtr && !SoundPtr->IsNull())
    {
        return SoundPtr->LoadSynchronous();
    }
    
    return nullptr;
}

float UDinosaurAudioComponent::CalculateVolumeFromThreat(float BaseVolume)
{
    FAudio_DinosaurSoundSet* SoundSet = SpeciesSoundSets.Find(DinosaurSpecies);
    if (!SoundSet)
    {
        return BaseVolume;
    }
    
    // Increase volume with threat level
    float ThreatMultiplier = 1.0f + (CurrentThreatLevel * 0.5f);
    return BaseVolume * SoundSet->BaseVolume * ThreatMultiplier;
}

float UDinosaurAudioComponent::CalculatePitchFromThreat(float BasePitch)
{
    FAudio_DinosaurSoundSet* SoundSet = SpeciesSoundSets.Find(DinosaurSpecies);
    if (!SoundSet)
    {
        return BasePitch;
    }
    
    // Slightly increase pitch with threat level
    float ThreatPitchShift = CurrentThreatLevel * 0.2f;
    return BasePitch * SoundSet->BasePitch * (1.0f + ThreatPitchShift);
}

bool UDinosaurAudioComponent::CanPlaySound() const
{
    if (!GetWorld())
    {
        return false;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastSoundTime) >= SoundCooldownTime;
}