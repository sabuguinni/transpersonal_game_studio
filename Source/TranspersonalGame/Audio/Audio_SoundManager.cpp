#include "Audio_SoundManager.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"

AAudio_SoundManager::AAudio_SoundManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create master audio component
    MasterAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MasterAudioComponent"));
    RootComponent = MasterAudioComponent;

    // Initialize default values
    MasterVolume = 1.0f;
    EnvironmentVolume = 0.8f;
    DinosaurVolume = 1.0f;
    PlayerVolume = 0.7f;

    CurrentBiome = EAudio_BiomeType::Savana;
    BiomeTransitionTimer = 0.0f;
    bIsTransitioningBiome = false;
    LastProximityCheck = 0.0f;
}

void AAudio_SoundManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeAudio();
    InitializeDinosaurAudio();
    UpdateAudioLevels();
}

void AAudio_SoundManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update biome transition
    if (bIsTransitioningBiome)
    {
        BiomeTransitionTimer += DeltaTime;
        if (BiomeTransitionTimer >= 2.0f) // 2 second transition
        {
            bIsTransitioningBiome = false;
            BiomeTransitionTimer = 0.0f;
        }
    }

    // Update proximity audio every 0.5 seconds
    LastProximityCheck += DeltaTime;
    if (LastProximityCheck >= 0.5f)
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        {
            UpdatePlayerProximityAudio(PlayerPawn->GetActorLocation());
        }
        LastProximityCheck = 0.0f;
    }
}

void AAudio_SoundManager::InitializeBiomeAudio()
{
    // Initialize biome audio profiles with default settings
    for (int32 i = 0; i < 5; ++i)
    {
        EAudio_BiomeType BiomeType = static_cast<EAudio_BiomeType>(i);
        FAudio_SoundProfile Profile;
        
        // Set biome-specific parameters
        switch (BiomeType)
        {
            case EAudio_BiomeType::Savana:
                Profile.VolumeMultiplier = 0.8f;
                Profile.AttenuationRadius = 3000.0f;
                break;
            case EAudio_BiomeType::Pantano:
                Profile.VolumeMultiplier = 0.9f;
                Profile.AttenuationRadius = 2500.0f;
                break;
            case EAudio_BiomeType::Floresta:
                Profile.VolumeMultiplier = 1.0f;
                Profile.AttenuationRadius = 2000.0f;
                break;
            case EAudio_BiomeType::Deserto:
                Profile.VolumeMultiplier = 0.7f;
                Profile.AttenuationRadius = 4000.0f;
                break;
            case EAudio_BiomeType::Montanha:
                Profile.VolumeMultiplier = 0.6f;
                Profile.AttenuationRadius = 5000.0f;
                break;
        }
        
        BiomeAudioProfiles.Add(BiomeType, Profile);
    }

    // Create audio components for each biome
    for (int32 i = 0; i < 5; ++i)
    {
        UAudioComponent* BiomeAudio = CreateDefaultSubobject<UAudioComponent>(*FString::Printf(TEXT("BiomeAudio_%d"), i));
        BiomeAudio->SetupAttachment(RootComponent);
        BiomeAudio->SetVolumeMultiplier(0.0f); // Start silent
        BiomeAudioComponents.Add(BiomeAudio);
    }
}

void AAudio_SoundManager::InitializeDinosaurAudio()
{
    // Initialize dinosaur audio profiles
    TArray<EAudio_DinosaurType> DinosaurTypes = {
        EAudio_DinosaurType::TRex,
        EAudio_DinosaurType::Velociraptor,
        EAudio_DinosaurType::Triceratops,
        EAudio_DinosaurType::Brachiosaurus,
        EAudio_DinosaurType::Ankylosaurus
    };

    for (EAudio_DinosaurType DinoType : DinosaurTypes)
    {
        FAudio_SoundProfile Profile;
        
        // Set dinosaur-specific parameters
        switch (DinoType)
        {
            case EAudio_DinosaurType::TRex:
                Profile.VolumeMultiplier = 1.5f;
                Profile.AttenuationRadius = 5000.0f;
                break;
            case EAudio_DinosaurType::Velociraptor:
                Profile.VolumeMultiplier = 0.8f;
                Profile.AttenuationRadius = 1500.0f;
                break;
            case EAudio_DinosaurType::Triceratops:
                Profile.VolumeMultiplier = 1.2f;
                Profile.AttenuationRadius = 3000.0f;
                break;
            case EAudio_DinosaurType::Brachiosaurus:
                Profile.VolumeMultiplier = 1.3f;
                Profile.AttenuationRadius = 4000.0f;
                break;
            case EAudio_DinosaurType::Ankylosaurus:
                Profile.VolumeMultiplier = 1.0f;
                Profile.AttenuationRadius = 2500.0f;
                break;
        }
        
        DinosaurAudioProfiles.Add(DinoType, Profile);
    }
}

void AAudio_SoundManager::PlayBiomeAmbient(EAudio_BiomeType BiomeType, FVector Location)
{
    if (BiomeAudioProfiles.Contains(BiomeType))
    {
        const FAudio_SoundProfile& Profile = BiomeAudioProfiles[BiomeType];
        
        // Start biome transition if different from current
        if (BiomeType != CurrentBiome)
        {
            CurrentBiome = BiomeType;
            bIsTransitioningBiome = true;
            BiomeTransitionTimer = 0.0f;
        }

        // Play ambient sound if available
        if (Profile.AmbientSound.IsValid())
        {
            int32 BiomeIndex = static_cast<int32>(BiomeType);
            if (BiomeAudioComponents.IsValidIndex(BiomeIndex))
            {
                UAudioComponent* BiomeAudio = BiomeAudioComponents[BiomeIndex];
                BiomeAudio->SetSound(Profile.AmbientSound.Get());
                BiomeAudio->SetVolumeMultiplier(Profile.VolumeMultiplier * EnvironmentVolume * MasterVolume);
                BiomeAudio->Play();
            }
        }
    }
}

void AAudio_SoundManager::PlayDinosaurSound(EAudio_DinosaurType DinosaurType, FVector Location, bool bIsFootstep)
{
    if (DinosaurAudioProfiles.Contains(DinosaurType))
    {
        const FAudio_SoundProfile& Profile = DinosaurAudioProfiles[DinosaurType];
        
        // Determine which sound to play
        TSoftObjectPtr<USoundCue> SoundToPlay = bIsFootstep ? Profile.FootstepSound : Profile.VocalizationSound;
        
        if (SoundToPlay.IsValid())
        {
            // Play sound at location
            UGameplayStatics::PlaySoundAtLocation(
                GetWorld(),
                SoundToPlay.Get(),
                Location,
                Profile.VolumeMultiplier * DinosaurVolume * MasterVolume
            );
        }
    }
}

void AAudio_SoundManager::SetMasterVolume(float NewVolume)
{
    MasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    UpdateAudioLevels();
}

void AAudio_SoundManager::SetEnvironmentVolume(float NewVolume)
{
    EnvironmentVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    UpdateAudioLevels();
}

void AAudio_SoundManager::SetDinosaurVolume(float NewVolume)
{
    DinosaurVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    UpdateAudioLevels();
}

void AAudio_SoundManager::StopAllAmbientSounds()
{
    for (UAudioComponent* BiomeAudio : BiomeAudioComponents)
    {
        if (BiomeAudio && BiomeAudio->IsPlaying())
        {
            BiomeAudio->Stop();
        }
    }
}

void AAudio_SoundManager::UpdatePlayerProximityAudio(FVector PlayerLocation)
{
    // Find nearby dinosaurs and adjust audio accordingly
    NearbyDinosaurs.Empty();
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetActorLabel().Contains(TEXT("Dino")))
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            if (Distance <= 3000.0f) // 30 meter proximity
            {
                NearbyDinosaurs.Add(Actor);
            }
        }
    }

    // Determine current biome based on player location
    EAudio_BiomeType NewBiome = EAudio_BiomeType::Savana;
    
    if (PlayerLocation.X < -30000 && PlayerLocation.Y < -30000)
        NewBiome = EAudio_BiomeType::Pantano;
    else if (PlayerLocation.X < -30000 && PlayerLocation.Y > 30000)
        NewBiome = EAudio_BiomeType::Floresta;
    else if (PlayerLocation.X > 30000 && FMath::Abs(PlayerLocation.Y) < 30000)
        NewBiome = EAudio_BiomeType::Deserto;
    else if (PlayerLocation.X > 30000 && PlayerLocation.Y > 30000)
        NewBiome = EAudio_BiomeType::Montanha;

    // Update biome audio if changed
    if (NewBiome != CurrentBiome)
    {
        PlayBiomeAmbient(NewBiome, PlayerLocation);
    }
}

void AAudio_SoundManager::UpdateAudioLevels()
{
    // Update master audio component
    if (MasterAudioComponent)
    {
        MasterAudioComponent->SetVolumeMultiplier(MasterVolume);
    }

    // Update biome audio components
    for (int32 i = 0; i < BiomeAudioComponents.Num(); ++i)
    {
        if (BiomeAudioComponents[i])
        {
            EAudio_BiomeType BiomeType = static_cast<EAudio_BiomeType>(i);
            if (BiomeAudioProfiles.Contains(BiomeType))
            {
                const FAudio_SoundProfile& Profile = BiomeAudioProfiles[BiomeType];
                float FinalVolume = Profile.VolumeMultiplier * EnvironmentVolume * MasterVolume;
                BiomeAudioComponents[i]->SetVolumeMultiplier(FinalVolume);
            }
        }
    }
}

void AAudio_SoundManager::FadeAudioComponent(UAudioComponent* AudioComp, float TargetVolume, float FadeTime)
{
    if (AudioComp)
    {
        // Simple fade implementation - could be enhanced with timeline
        AudioComp->SetVolumeMultiplier(TargetVolume);
    }
}