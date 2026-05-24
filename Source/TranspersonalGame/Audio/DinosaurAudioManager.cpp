#include "DinosaurAudioManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AAudio_DinosaurAudioManager::AAudio_DinosaurAudioManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Initialize default settings
    DinosaurVolumeMultiplier = 1.0f;
    FootstepVolumeBySize = 1.0f;
    RoarEchoDelay = 0.5f;
}

void AAudio_DinosaurAudioManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDinosaurSounds();
}

void AAudio_DinosaurAudioManager::InitializeDinosaurSounds()
{
    // Initialize T-Rex sounds
    FAudio_DinosaurSoundData TRexRoar;
    TRexRoar.Species = EAudio_DinosaurSpecies::TRex;
    TRexRoar.SoundType = EAudio_DinosaurSoundType::Roar;
    TRexRoar.BaseVolume = 1.5f;
    TRexRoar.BasePitch = 0.8f;
    TRexRoar.MaxDistance = 10000.0f;
    DinosaurSounds.Add(TRexRoar);
    
    FAudio_DinosaurSoundData TRexFootstep;
    TRexFootstep.Species = EAudio_DinosaurSpecies::TRex;
    TRexFootstep.SoundType = EAudio_DinosaurSoundType::Footstep;
    TRexFootstep.BaseVolume = 1.2f;
    TRexFootstep.BasePitch = 0.7f;
    TRexFootstep.MaxDistance = 8000.0f;
    DinosaurSounds.Add(TRexFootstep);
    
    // Initialize Velociraptor sounds
    FAudio_DinosaurSoundData RaptorGrowl;
    RaptorGrowl.Species = EAudio_DinosaurSpecies::Velociraptor;
    RaptorGrowl.SoundType = EAudio_DinosaurSoundType::Growl;
    RaptorGrowl.BaseVolume = 0.8f;
    RaptorGrowl.BasePitch = 1.2f;
    RaptorGrowl.MaxDistance = 3000.0f;
    DinosaurSounds.Add(RaptorGrowl);
    
    FAudio_DinosaurSoundData RaptorFootstep;
    RaptorFootstep.Species = EAudio_DinosaurSpecies::Velociraptor;
    RaptorFootstep.SoundType = EAudio_DinosaurSoundType::Footstep;
    RaptorFootstep.BaseVolume = 0.4f;
    RaptorFootstep.BasePitch = 1.1f;
    RaptorFootstep.MaxDistance = 1500.0f;
    DinosaurSounds.Add(RaptorFootstep);
    
    // Initialize Brachiosaurus sounds
    FAudio_DinosaurSoundData BrachioRoar;
    BrachioRoar.Species = EAudio_DinosaurSpecies::Brachiosaurus;
    BrachioRoar.SoundType = EAudio_DinosaurSoundType::Roar;
    BrachioRoar.BaseVolume = 1.3f;
    BrachioRoar.BasePitch = 0.6f;
    BrachioRoar.MaxDistance = 12000.0f;
    DinosaurSounds.Add(BrachioRoar);
    
    FAudio_DinosaurSoundData BrachioFootstep;
    BrachioFootstep.Species = EAudio_DinosaurSpecies::Brachiosaurus;
    BrachioFootstep.SoundType = EAudio_DinosaurSoundType::Footstep;
    BrachioFootstep.BaseVolume = 1.4f;
    BrachioFootstep.BasePitch = 0.5f;
    BrachioFootstep.MaxDistance = 9000.0f;
    DinosaurSounds.Add(BrachioFootstep);
}

void AAudio_DinosaurAudioManager::PlayDinosaurSound(EAudio_DinosaurSpecies Species, EAudio_DinosaurSoundType SoundType, FVector Location, float VolumeMultiplier)
{
    FAudio_DinosaurSoundData* SoundData = FindSoundData(Species, SoundType);
    if (!SoundData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound data not found for species %d, sound type %d"), (int32)Species, (int32)SoundType);
        return;
    }
    
    UAudioComponent* AudioComp = CreateDinosaurAudioComponent(*SoundData, Location);
    if (AudioComp)
    {
        float FinalVolume = SoundData->BaseVolume * DinosaurVolumeMultiplier * VolumeMultiplier;
        AudioComp->SetVolumeMultiplier(FinalVolume);
        AudioComp->Play();
        
        ActiveDinosaurAudioComponents.Add(AudioComp);
    }
}

void AAudio_DinosaurAudioManager::PlayFootstepBySpecies(EAudio_DinosaurSpecies Species, FVector FootstepLocation, float Intensity)
{
    float SizeMultiplier = GetSpeciesSizeMultiplier(Species);
    float FinalIntensity = Intensity * SizeMultiplier * FootstepVolumeBySize;
    
    PlayDinosaurSound(Species, EAudio_DinosaurSoundType::Footstep, FootstepLocation, FinalIntensity);
    
    // Add ground shake effect for large dinosaurs
    if (SizeMultiplier > 1.0f && GetWorld())
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            float ShakeIntensity = FMath::Clamp(SizeMultiplier * 0.5f, 0.1f, 2.0f);
            PC->ClientStartCameraShake(nullptr, ShakeIntensity);
        }
    }
}

void AAudio_DinosaurAudioManager::PlayRoarWithEcho(EAudio_DinosaurSpecies Species, FVector RoarLocation)
{
    // Play main roar
    PlayDinosaurSound(Species, EAudio_DinosaurSoundType::Roar, RoarLocation, 1.0f);
    
    // Schedule echo roar
    FTimerHandle EchoTimer;
    GetWorld()->GetTimerManager().SetTimer(EchoTimer, [this, Species, RoarLocation]()
    {
        PlayDinosaurSound(Species, EAudio_DinosaurSoundType::Roar, RoarLocation, 0.3f);
    }, RoarEchoDelay, false);
}

void AAudio_DinosaurAudioManager::StopAllDinosaurSounds()
{
    for (UAudioComponent* AudioComp : ActiveDinosaurAudioComponents)
    {
        if (AudioComp && IsValid(AudioComp))
        {
            AudioComp->Stop();
            AudioComp->DestroyComponent();
        }
    }
    ActiveDinosaurAudioComponents.Empty();
}

void AAudio_DinosaurAudioManager::SetDinosaurVolumeMultiplier(float NewMultiplier)
{
    DinosaurVolumeMultiplier = FMath::Clamp(NewMultiplier, 0.0f, 2.0f);
}

float AAudio_DinosaurAudioManager::GetSpeciesSizeMultiplier(EAudio_DinosaurSpecies Species)
{
    switch (Species)
    {
        case EAudio_DinosaurSpecies::TRex:
            return 1.5f;
        case EAudio_DinosaurSpecies::Brachiosaurus:
            return 2.0f;
        case EAudio_DinosaurSpecies::Triceratops:
            return 1.3f;
        case EAudio_DinosaurSpecies::Ankylosaurus:
            return 1.2f;
        case EAudio_DinosaurSpecies::Parasaurolophus:
            return 1.1f;
        case EAudio_DinosaurSpecies::Pachycephalosaurus:
            return 0.9f;
        case EAudio_DinosaurSpecies::Protoceratops:
            return 0.7f;
        case EAudio_DinosaurSpecies::Tsintaosaurus:
            return 1.0f;
        case EAudio_DinosaurSpecies::Velociraptor:
            return 0.6f;
        default:
            return 1.0f;
    }
}

FAudio_DinosaurSoundData* AAudio_DinosaurAudioManager::FindSoundData(EAudio_DinosaurSpecies Species, EAudio_DinosaurSoundType SoundType)
{
    return DinosaurSounds.FindByPredicate([Species, SoundType](const FAudio_DinosaurSoundData& Data)
    {
        return Data.Species == Species && Data.SoundType == SoundType;
    });
}

UAudioComponent* AAudio_DinosaurAudioManager::CreateDinosaurAudioComponent(const FAudio_DinosaurSoundData& SoundData, FVector Location)
{
    UAudioComponent* NewComponent = NewObject<UAudioComponent>(this);
    if (NewComponent)
    {
        NewComponent->SetWorldLocation(Location);
        
        if (SoundData.SoundCue.IsValid())
        {
            NewComponent->SetSound(SoundData.SoundCue.Get());
        }
        
        NewComponent->SetVolumeMultiplier(SoundData.BaseVolume);
        NewComponent->SetPitchMultiplier(SoundData.BasePitch);
        NewComponent->bAutoActivate = false;
        
        if (SoundData.bIs3D)
        {
            NewComponent->AttenuationSettings = NewObject<USoundAttenuation>();
            if (NewComponent->AttenuationSettings)
            {
                NewComponent->AttenuationSettings->Attenuation.FalloffDistance = SoundData.MaxDistance;
            }
        }
        
        NewComponent->RegisterComponent();
    }
    
    return NewComponent;
}