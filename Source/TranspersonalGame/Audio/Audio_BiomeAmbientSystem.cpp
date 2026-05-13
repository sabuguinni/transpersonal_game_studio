#include "Audio_BiomeAmbientSystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Math/UnrealMathUtility.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

AAudio_BiomeAmbientSystem::AAudio_BiomeAmbientSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Update twice per second

    // Create audio components
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    RootComponent = AmbientAudioComponent;
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetVolumeMultiplier(0.7f);

    RandomAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("RandomAudio"));
    RandomAudioComponent->SetupAttachment(RootComponent);
    RandomAudioComponent->bAutoActivate = false;

    // Initialize default settings
    BiomeType = EAudio_BiomeType::Forest;
    MaxAudibleDistance = 5000.0f;
    bUseTimeOfDayVariation = true;
    NightVolumeMultiplier = 0.6f;
    
    RandomSoundTimer = 0.0f;
    NextRandomSoundTime = FMath::RandRange(10.0f, 25.0f);
    bIsPlayerInRange = false;
    CurrentVolumeMultiplier = 1.0f;
}

void AAudio_BiomeAmbientSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeProfiles();
    LoadAudioAssets();
    UpdateAmbientAudio();
}

void AAudio_BiomeAmbientSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Get player location for distance calculations
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && PC->GetPawn())
    {
        FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
        UpdateAudioBasedOnPlayerDistance(PlayerLocation);
    }

    // Update random sound timer
    UpdateRandomSounds(DeltaTime);
}

void AAudio_BiomeAmbientSystem::InitializeBiomeProfiles()
{
    // Initialize Forest profile
    FAudio_BiomeAudioProfile ForestProfile;
    ForestProfile.BaseVolume = 0.8f;
    ForestProfile.FadeDistance = 2500.0f;
    ForestProfile.RandomSoundInterval = 20.0f;
    ForestProfile.RandomSoundVariance = 15.0f;
    BiomeAudioProfiles.Add(EAudio_BiomeType::Forest, ForestProfile);

    // Initialize Swamp profile
    FAudio_BiomeAudioProfile SwampProfile;
    SwampProfile.BaseVolume = 0.7f;
    SwampProfile.FadeDistance = 2000.0f;
    SwampProfile.RandomSoundInterval = 25.0f;
    SwampProfile.RandomSoundVariance = 12.0f;
    BiomeAudioProfiles.Add(EAudio_BiomeType::Swamp, SwampProfile);

    // Initialize Savanna profile
    FAudio_BiomeAudioProfile SavannaProfile;
    SavannaProfile.BaseVolume = 0.6f;
    SavannaProfile.FadeDistance = 3000.0f;
    SavannaProfile.RandomSoundInterval = 30.0f;
    SavannaProfile.RandomSoundVariance = 20.0f;
    BiomeAudioProfiles.Add(EAudio_BiomeType::Savanna, SavannaProfile);

    // Initialize Desert profile
    FAudio_BiomeAudioProfile DesertProfile;
    DesertProfile.BaseVolume = 0.5f;
    DesertProfile.FadeDistance = 4000.0f;
    DesertProfile.RandomSoundInterval = 45.0f;
    DesertProfile.RandomSoundVariance = 30.0f;
    BiomeAudioProfiles.Add(EAudio_BiomeType::Desert, DesertProfile);

    // Initialize Mountain profile
    FAudio_BiomeAudioProfile MountainProfile;
    MountainProfile.BaseVolume = 0.6f;
    MountainProfile.FadeDistance = 3500.0f;
    MountainProfile.RandomSoundInterval = 35.0f;
    MountainProfile.RandomSoundVariance = 25.0f;
    BiomeAudioProfiles.Add(EAudio_BiomeType::Mountain, MountainProfile);

    // Initialize Cave profile
    FAudio_BiomeAudioProfile CaveProfile;
    CaveProfile.BaseVolume = 0.4f;
    CaveProfile.FadeDistance = 1500.0f;
    CaveProfile.RandomSoundInterval = 40.0f;
    CaveProfile.RandomSoundVariance = 20.0f;
    BiomeAudioProfiles.Add(EAudio_BiomeType::Cave, CaveProfile);

    // Initialize River profile
    FAudio_BiomeAudioProfile RiverProfile;
    RiverProfile.BaseVolume = 0.9f;
    RiverProfile.FadeDistance = 1800.0f;
    RiverProfile.RandomSoundInterval = 15.0f;
    RiverProfile.RandomSoundVariance = 8.0f;
    BiomeAudioProfiles.Add(EAudio_BiomeType::River, RiverProfile);
}

void AAudio_BiomeAmbientSystem::LoadAudioAssets()
{
    // In a full implementation, you would load actual sound assets here
    // For now, we'll set up the structure for when assets are available
    
    // Example of how to load sound cues:
    // USoundCue* ForestAmbient = LoadObject<USoundCue>(nullptr, TEXT("/Game/Audio/Ambient/Forest_Loop"));
    // if (ForestAmbient && BiomeAudioProfiles.Contains(EAudio_BiomeType::Forest))
    // {
    //     BiomeAudioProfiles[EAudio_BiomeType::Forest].AmbientLoop = ForestAmbient;
    // }
}

void AAudio_BiomeAmbientSystem::SetBiomeType(EAudio_BiomeType NewBiomeType)
{
    if (BiomeType != NewBiomeType)
    {
        BiomeType = NewBiomeType;
        UpdateAmbientAudio();
        
        // Reset random sound timer
        if (BiomeAudioProfiles.Contains(BiomeType))
        {
            const FAudio_BiomeAudioProfile& Profile = BiomeAudioProfiles[BiomeType];
            NextRandomSoundTime = FMath::RandRange(
                Profile.RandomSoundInterval - Profile.RandomSoundVariance,
                Profile.RandomSoundInterval + Profile.RandomSoundVariance
            );
        }
        RandomSoundTimer = 0.0f;
    }
}

void AAudio_BiomeAmbientSystem::UpdateAudioBasedOnPlayerDistance(FVector PlayerLocation)
{
    float DistanceVolume = CalculateDistanceVolume(PlayerLocation);
    bool bWasInRange = bIsPlayerInRange;
    bIsPlayerInRange = (DistanceVolume > 0.0f);

    if (bIsPlayerInRange != bWasInRange)
    {
        if (bIsPlayerInRange)
        {
            // Player entered range - start ambient audio
            UpdateAmbientAudio();
        }
        else
        {
            // Player left range - stop ambient audio
            if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
            {
                AmbientAudioComponent->FadeOut(2.0f, 0.0f);
            }
        }
    }

    if (bIsPlayerInRange)
    {
        // Update volume based on distance and time of day
        CurrentVolumeMultiplier = DistanceVolume;
        
        if (BiomeAudioProfiles.Contains(BiomeType))
        {
            const FAudio_BiomeAudioProfile& Profile = BiomeAudioProfiles[BiomeType];
            float FinalVolume = Profile.BaseVolume * CurrentVolumeMultiplier;
            
            if (AmbientAudioComponent)
            {
                AmbientAudioComponent->SetVolumeMultiplier(FinalVolume);
            }
        }
    }
}

void AAudio_BiomeAmbientSystem::PlayRandomBiomeSound()
{
    if (!bIsPlayerInRange || !BiomeAudioProfiles.Contains(BiomeType))
    {
        return;
    }

    const FAudio_BiomeAudioProfile& Profile = BiomeAudioProfiles[BiomeType];
    
    if (Profile.RandomSounds.Num() > 0 && RandomAudioComponent)
    {
        int32 RandomIndex = FMath::RandRange(0, Profile.RandomSounds.Num() - 1);
        TSoftObjectPtr<USoundCue> RandomSound = Profile.RandomSounds[RandomIndex];
        
        if (RandomSound.IsValid())
        {
            USoundCue* SoundCue = RandomSound.Get();
            if (SoundCue)
            {
                float RandomVolume = Profile.BaseVolume * CurrentVolumeMultiplier * FMath::RandRange(0.7f, 1.0f);
                RandomAudioComponent->SetVolumeMultiplier(RandomVolume);
                RandomAudioComponent->SetSound(SoundCue);
                RandomAudioComponent->Play();
            }
        }
    }
}

void AAudio_BiomeAmbientSystem::SetTimeOfDayMultiplier(float TimeMultiplier)
{
    if (bUseTimeOfDayVariation)
    {
        CurrentVolumeMultiplier = FMath::Clamp(TimeMultiplier, 0.1f, 1.0f);
        
        if (AmbientAudioComponent && BiomeAudioProfiles.Contains(BiomeType))
        {
            const FAudio_BiomeAudioProfile& Profile = BiomeAudioProfiles[BiomeType];
            float FinalVolume = Profile.BaseVolume * CurrentVolumeMultiplier;
            AmbientAudioComponent->SetVolumeMultiplier(FinalVolume);
        }
    }
}

void AAudio_BiomeAmbientSystem::UpdateAmbientAudio()
{
    if (!BiomeAudioProfiles.Contains(BiomeType) || !AmbientAudioComponent)
    {
        return;
    }

    const FAudio_BiomeAudioProfile& Profile = BiomeAudioProfiles[BiomeType];
    
    if (Profile.AmbientLoop.IsValid())
    {
        USoundCue* AmbientCue = Profile.AmbientLoop.Get();
        if (AmbientCue)
        {
            AmbientAudioComponent->SetSound(AmbientCue);
            AmbientAudioComponent->SetVolumeMultiplier(Profile.BaseVolume * CurrentVolumeMultiplier);
            
            if (bIsPlayerInRange && !AmbientAudioComponent->IsPlaying())
            {
                AmbientAudioComponent->FadeIn(1.0f, Profile.BaseVolume * CurrentVolumeMultiplier);
            }
        }
    }
}

void AAudio_BiomeAmbientSystem::UpdateRandomSounds(float DeltaTime)
{
    if (!bIsPlayerInRange || !BiomeAudioProfiles.Contains(BiomeType))
    {
        return;
    }

    RandomSoundTimer += DeltaTime;
    
    if (RandomSoundTimer >= NextRandomSoundTime)
    {
        PlayRandomBiomeSound();
        
        // Set next random sound time
        const FAudio_BiomeAudioProfile& Profile = BiomeAudioProfiles[BiomeType];
        NextRandomSoundTime = FMath::RandRange(
            Profile.RandomSoundInterval - Profile.RandomSoundVariance,
            Profile.RandomSoundInterval + Profile.RandomSoundVariance
        );
        RandomSoundTimer = 0.0f;
    }
}

float AAudio_BiomeAmbientSystem::CalculateDistanceVolume(FVector PlayerLocation)
{
    float Distance = FVector::Dist(GetActorLocation(), PlayerLocation);
    
    if (Distance >= MaxAudibleDistance)
    {
        return 0.0f;
    }

    if (!BiomeAudioProfiles.Contains(BiomeType))
    {
        return 1.0f;
    }

    const FAudio_BiomeAudioProfile& Profile = BiomeAudioProfiles[BiomeType];
    
    if (Distance <= Profile.FadeDistance)
    {
        return 1.0f;
    }

    // Linear falloff between fade distance and max distance
    float FalloffRange = MaxAudibleDistance - Profile.FadeDistance;
    float FalloffDistance = Distance - Profile.FadeDistance;
    float VolumeMultiplier = 1.0f - (FalloffDistance / FalloffRange);
    
    return FMath::Clamp(VolumeMultiplier, 0.0f, 1.0f);
}