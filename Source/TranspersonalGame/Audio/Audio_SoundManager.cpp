#include "Audio_SoundManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "TimerManager.h"

AAudio_SoundManager::AAudio_SoundManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create Audio Components
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    RootComponent = AmbientAudioComponent;
    AmbientAudioComponent->bAutoActivate = false;

    EffectsAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EffectsAudioComponent"));
    EffectsAudioComponent->SetupAttachment(RootComponent);
    EffectsAudioComponent->bAutoActivate = false;

    NarrationAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NarrationAudioComponent"));
    NarrationAudioComponent->SetupAttachment(RootComponent);
    NarrationAudioComponent->bAutoActivate = false;

    // Initialize default values
    CurrentBiome = EAudio_BiomeType::Savana;
    bIsAmbientPlaying = false;
    MasterVolume = 1.0f;
    AmbientVolume = 0.8f;
    EffectsVolume = 1.0f;
    NarrationVolume = 0.9f;
}

void AAudio_SoundManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize configurations
    InitializeBiomeConfigs();
    InitializeFootstepConfigs();

    // Start biome detection timer
    GetWorldTimerManager().SetTimer(BiomeCheckTimer, this, &AAudio_SoundManager::CheckBiomeChange, 2.0f, true);

    // Start with default biome audio
    PlayAmbientAudio(CurrentBiome);

    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: BeginPlay completed"));
}

void AAudio_SoundManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAudio_SoundManager::SetCurrentBiome(EAudio_BiomeType NewBiome)
{
    if (CurrentBiome != NewBiome)
    {
        CurrentBiome = NewBiome;
        PlayAmbientAudio(NewBiome);
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Biome changed to %d"), (int32)NewBiome);
    }
}

void AAudio_SoundManager::PlayAmbientAudio(EAudio_BiomeType BiomeType)
{
    if (!BiomeAudioConfigs.Contains(BiomeType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: No audio config for biome %d"), (int32)BiomeType);
        return;
    }

    const FAudio_BiomeAudioConfig& Config = BiomeAudioConfigs[BiomeType];
    
    // Stop current ambient audio
    StopAmbientAudio();

    // Play new ambient loop if available
    if (Config.AmbientLoop && AmbientAudioComponent)
    {
        AmbientAudioComponent->SetSound(Config.AmbientLoop);
        AmbientAudioComponent->SetVolumeMultiplier(Config.AmbientVolume * AmbientVolume * MasterVolume);
        AmbientAudioComponent->Play();
        bIsAmbientPlaying = true;

        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Playing ambient audio for biome %d"), (int32)BiomeType);
    }

    // Setup random sound timer
    if (Config.RandomSounds.Num() > 0)
    {
        GetWorldTimerManager().ClearTimer(RandomSoundTimer);
        GetWorldTimerManager().SetTimer(RandomSoundTimer, this, &AAudio_SoundManager::PlayRandomBiomeSound, Config.RandomSoundInterval, true);
    }
}

void AAudio_SoundManager::StopAmbientAudio()
{
    if (AmbientAudioComponent && bIsAmbientPlaying)
    {
        AmbientAudioComponent->Stop();
        bIsAmbientPlaying = false;
    }

    GetWorldTimerManager().ClearTimer(RandomSoundTimer);
}

void AAudio_SoundManager::PlayFootstepSound(const FString& SurfaceType, const FVector& Location)
{
    if (!FootstepConfigs.Contains(SurfaceType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: No footstep config for surface %s"), *SurfaceType);
        return;
    }

    const FAudio_FootstepConfig& Config = FootstepConfigs[SurfaceType];
    
    if (Config.FootstepSound && EffectsAudioComponent)
    {
        EffectsAudioComponent->SetSound(Config.FootstepSound);
        EffectsAudioComponent->SetVolumeMultiplier(Config.VolumeMultiplier * EffectsVolume * MasterVolume);
        
        // Add pitch variation
        float PitchVariation = FMath::RandRange(-Config.PitchVariation, Config.PitchVariation);
        EffectsAudioComponent->SetPitchMultiplier(1.0f + PitchVariation);
        
        EffectsAudioComponent->Play();
        
        UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Playing footstep sound for surface %s"), *SurfaceType);
    }
}

void AAudio_SoundManager::PlayCreatureSound(TObjectPtr<USoundCue> CreatureSound, const FVector& Location, float Volume)
{
    if (CreatureSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, CreatureSound, Location, Volume * EffectsVolume * MasterVolume);
        UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Playing creature sound at location"));
    }
}

void AAudio_SoundManager::PlayNarration(TObjectPtr<USoundCue> NarrationSound)
{
    if (NarrationSound && NarrationAudioComponent)
    {
        NarrationAudioComponent->SetSound(NarrationSound);
        NarrationAudioComponent->SetVolumeMultiplier(NarrationVolume * MasterVolume);
        NarrationAudioComponent->Play();
        
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Playing narration"));
    }
}

void AAudio_SoundManager::StopNarration()
{
    if (NarrationAudioComponent)
    {
        NarrationAudioComponent->Stop();
    }
}

EAudio_BiomeType AAudio_SoundManager::DetectCurrentBiome(const FVector& PlayerLocation)
{
    // Biome detection based on coordinates from memory ID 709
    // Savana (0,0), Pantano (-50000,-45000), Floresta (-45000,40000), Deserto (55000,0), Montanha (40000,50000)
    
    float X = PlayerLocation.X;
    float Y = PlayerLocation.Y;
    
    // Calculate distances to each biome center
    float DistToSavana = FVector::Dist2D(PlayerLocation, FVector(0, 0, 0));
    float DistToPantano = FVector::Dist2D(PlayerLocation, FVector(-50000, -45000, 0));
    float DistToFloresta = FVector::Dist2D(PlayerLocation, FVector(-45000, 40000, 0));
    float DistToDeserto = FVector::Dist2D(PlayerLocation, FVector(55000, 0, 0));
    float DistToMontanha = FVector::Dist2D(PlayerLocation, FVector(40000, 50000, 0));
    
    // Find closest biome
    float MinDistance = DistToSavana;
    EAudio_BiomeType ClosestBiome = EAudio_BiomeType::Savana;
    
    if (DistToPantano < MinDistance)
    {
        MinDistance = DistToPantano;
        ClosestBiome = EAudio_BiomeType::Pantano;
    }
    
    if (DistToFloresta < MinDistance)
    {
        MinDistance = DistToFloresta;
        ClosestBiome = EAudio_BiomeType::Floresta;
    }
    
    if (DistToDeserto < MinDistance)
    {
        MinDistance = DistToDeserto;
        ClosestBiome = EAudio_BiomeType::Deserto;
    }
    
    if (DistToMontanha < MinDistance)
    {
        MinDistance = DistToMontanha;
        ClosestBiome = EAudio_BiomeType::Montanha;
    }
    
    return ClosestBiome;
}

void AAudio_SoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all active audio components
    if (AmbientAudioComponent && bIsAmbientPlaying)
    {
        const FAudio_BiomeAudioConfig& Config = BiomeAudioConfigs[CurrentBiome];
        AmbientAudioComponent->SetVolumeMultiplier(Config.AmbientVolume * AmbientVolume * MasterVolume);
    }
}

void AAudio_SoundManager::SetAmbientVolume(float Volume)
{
    AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (AmbientAudioComponent && bIsAmbientPlaying)
    {
        const FAudio_BiomeAudioConfig& Config = BiomeAudioConfigs[CurrentBiome];
        AmbientAudioComponent->SetVolumeMultiplier(Config.AmbientVolume * AmbientVolume * MasterVolume);
    }
}

void AAudio_SoundManager::SetEffectsVolume(float Volume)
{
    EffectsVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void AAudio_SoundManager::PlayRandomBiomeSound()
{
    if (!BiomeAudioConfigs.Contains(CurrentBiome))
    {
        return;
    }

    const FAudio_BiomeAudioConfig& Config = BiomeAudioConfigs[CurrentBiome];
    
    if (Config.RandomSounds.Num() > 0 && FMath::RandRange(0.0f, 1.0f) < Config.RandomSoundChance)
    {
        int32 RandomIndex = FMath::RandRange(0, Config.RandomSounds.Num() - 1);
        TObjectPtr<USoundCue> RandomSound = Config.RandomSounds[RandomIndex];
        
        if (RandomSound)
        {
            UGameplayStatics::PlaySound2D(this, RandomSound, EffectsVolume * MasterVolume);
            UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Playing random biome sound"));
        }
    }
}

void AAudio_SoundManager::CheckBiomeChange()
{
    // Get player location
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (PlayerPawn)
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        EAudio_BiomeType DetectedBiome = DetectCurrentBiome(PlayerLocation);
        
        if (DetectedBiome != CurrentBiome)
        {
            SetCurrentBiome(DetectedBiome);
        }
    }
}

void AAudio_SoundManager::InitializeBiomeConfigs()
{
    // Initialize default biome configurations
    // Note: Sound assets will need to be assigned in Blueprint or loaded from content
    
    FAudio_BiomeAudioConfig SavanaConfig;
    SavanaConfig.AmbientVolume = 0.7f;
    SavanaConfig.RandomSoundInterval = 20.0f;
    SavanaConfig.RandomSoundChance = 0.25f;
    BiomeAudioConfigs.Add(EAudio_BiomeType::Savana, SavanaConfig);
    
    FAudio_BiomeAudioConfig PantanoConfig;
    PantanoConfig.AmbientVolume = 0.8f;
    PantanoConfig.RandomSoundInterval = 15.0f;
    PantanoConfig.RandomSoundChance = 0.4f;
    BiomeAudioConfigs.Add(EAudio_BiomeType::Pantano, PantanoConfig);
    
    FAudio_BiomeAudioConfig FlorestaConfig;
    FlorestaConfig.AmbientVolume = 0.9f;
    FlorestaConfig.RandomSoundInterval = 12.0f;
    FlorestaConfig.RandomSoundChance = 0.5f;
    BiomeAudioConfigs.Add(EAudio_BiomeType::Floresta, FlorestaConfig);
    
    FAudio_BiomeAudioConfig DesertoConfig;
    DesertoConfig.AmbientVolume = 0.5f;
    DesertoConfig.RandomSoundInterval = 30.0f;
    DesertoConfig.RandomSoundChance = 0.15f;
    BiomeAudioConfigs.Add(EAudio_BiomeType::Deserto, DesertoConfig);
    
    FAudio_BiomeAudioConfig MontanhaConfig;
    MontanhaConfig.AmbientVolume = 0.6f;
    MontanhaConfig.RandomSoundInterval = 25.0f;
    MontanhaConfig.RandomSoundChance = 0.2f;
    BiomeAudioConfigs.Add(EAudio_BiomeType::Montanha, MontanhaConfig);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Biome configs initialized"));
}

void AAudio_SoundManager::InitializeFootstepConfigs()
{
    // Initialize footstep configurations for different surface types
    
    FAudio_FootstepConfig GrassConfig;
    GrassConfig.VolumeMultiplier = 0.8f;
    GrassConfig.PitchVariation = 0.15f;
    FootstepConfigs.Add(TEXT("Grass"), GrassConfig);
    
    FAudio_FootstepConfig DirtConfig;
    DirtConfig.VolumeMultiplier = 1.0f;
    DirtConfig.PitchVariation = 0.2f;
    FootstepConfigs.Add(TEXT("Dirt"), DirtConfig);
    
    FAudio_FootstepConfig StoneConfig;
    StoneConfig.VolumeMultiplier = 1.2f;
    StoneConfig.PitchVariation = 0.1f;
    FootstepConfigs.Add(TEXT("Stone"), StoneConfig);
    
    FAudio_FootstepConfig WaterConfig;
    WaterConfig.VolumeMultiplier = 0.9f;
    WaterConfig.PitchVariation = 0.25f;
    FootstepConfigs.Add(TEXT("Water"), WaterConfig);
    
    FAudio_FootstepConfig SandConfig;
    SandConfig.VolumeMultiplier = 0.7f;
    SandConfig.PitchVariation = 0.3f;
    FootstepConfigs.Add(TEXT("Sand"), SandConfig);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Footstep configs initialized"));
}