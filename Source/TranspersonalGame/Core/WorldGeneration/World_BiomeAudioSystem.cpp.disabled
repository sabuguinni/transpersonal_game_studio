#include "World_BiomeAudioSystem.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogBiomeAudio);

UWorld_BiomeAudioSystem::UWorld_BiomeAudioSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second
    
    // Initialize biome audio settings
    InitializeBiomeAudioSettings();
}

void UWorld_BiomeAudioSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Create audio components for each biome
    CreateBiomeAudioComponents();
    
    // Start with default biome (Savanna)
    CurrentBiome = EWorld_BiomeType::Savanna;
    TransitionToBiome(CurrentBiome, 0.0f);
    
    UE_LOG(LogBiomeAudio, Log, TEXT("BiomeAudioSystem initialized"));
}

void UWorld_BiomeAudioSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update audio transitions
    UpdateAudioTransitions(DeltaTime);
    
    // Check for biome changes based on player location
    CheckBiomeTransition();
}

void UWorld_BiomeAudioSystem::InitializeBiomeAudioSettings()
{
    // Swamp biome audio
    FWorld_BiomeAudioData SwampAudio;
    SwampAudio.BiomeType = EWorld_BiomeType::Swamp;
    SwampAudio.AmbientVolume = 0.7f;
    SwampAudio.WindIntensity = 0.2f;
    SwampAudio.WaterSounds = true;
    SwampAudio.InsectSounds = true;
    SwampAudio.BirdSounds = false;
    SwampAudio.ReverbPreset = EWorld_AudioReverbType::Cave;
    BiomeAudioSettings.Add(EWorld_BiomeType::Swamp, SwampAudio);
    
    // Forest biome audio
    FWorld_BiomeAudioData ForestAudio;
    ForestAudio.BiomeType = EWorld_BiomeType::Forest;
    ForestAudio.AmbientVolume = 0.8f;
    ForestAudio.WindIntensity = 0.4f;
    ForestAudio.WaterSounds = false;
    ForestAudio.InsectSounds = true;
    ForestAudio.BirdSounds = true;
    ForestAudio.ReverbPreset = EWorld_AudioReverbType::Forest;
    BiomeAudioSettings.Add(EWorld_BiomeType::Forest, ForestAudio);
    
    // Savanna biome audio
    FWorld_BiomeAudioData SavannaAudio;
    SavannaAudio.BiomeType = EWorld_BiomeType::Savanna;
    SavannaAudio.AmbientVolume = 0.6f;
    SavannaAudio.WindIntensity = 0.5f;
    SavannaAudio.WaterSounds = false;
    SavannaAudio.InsectSounds = false;
    SavannaAudio.BirdSounds = true;
    SavannaAudio.ReverbPreset = EWorld_AudioReverbType::Plains;
    BiomeAudioSettings.Add(EWorld_BiomeType::Savanna, SavannaAudio);
    
    // Desert biome audio
    FWorld_BiomeAudioData DesertAudio;
    DesertAudio.BiomeType = EWorld_BiomeType::Desert;
    DesertAudio.AmbientVolume = 0.4f;
    DesertAudio.WindIntensity = 0.8f;
    DesertAudio.WaterSounds = false;
    DesertAudio.InsectSounds = false;
    DesertAudio.BirdSounds = false;
    DesertAudio.ReverbPreset = EWorld_AudioReverbType::Desert;
    BiomeAudioSettings.Add(EWorld_BiomeType::Desert, DesertAudio);
    
    // Mountain biome audio
    FWorld_BiomeAudioData MountainAudio;
    MountainAudio.BiomeType = EWorld_BiomeType::Mountain;
    MountainAudio.AmbientVolume = 0.5f;
    MountainAudio.WindIntensity = 0.9f;
    MountainAudio.WaterSounds = false;
    MountainAudio.InsectSounds = false;
    MountainAudio.BirdSounds = false;
    MountainAudio.ReverbPreset = EWorld_AudioReverbType::Mountain;
    BiomeAudioSettings.Add(EWorld_BiomeType::Mountain, MountainAudio);
    
    UE_LOG(LogBiomeAudio, Log, TEXT("Biome audio settings initialized for %d biomes"), BiomeAudioSettings.Num());
}

void UWorld_BiomeAudioSystem::CreateBiomeAudioComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogBiomeAudio, Error, TEXT("No owner actor for BiomeAudioSystem"));
        return;
    }
    
    // Create ambient audio component
    AmbientAudioComponent = Owner->CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->bAutoActivate = false;
        AmbientAudioComponent->SetVolumeMultiplier(0.0f);
        UE_LOG(LogBiomeAudio, Log, TEXT("Ambient audio component created"));
    }
    
    // Create wind audio component
    WindAudioComponent = Owner->CreateDefaultSubobject<UAudioComponent>(TEXT("WindAudio"));
    if (WindAudioComponent)
    {
        WindAudioComponent->bAutoActivate = false;
        WindAudioComponent->SetVolumeMultiplier(0.0f);
        UE_LOG(LogBiomeAudio, Log, TEXT("Wind audio component created"));
    }
    
    // Create water audio component
    WaterAudioComponent = Owner->CreateDefaultSubobject<UAudioComponent>(TEXT("WaterAudio"));
    if (WaterAudioComponent)
    {
        WaterAudioComponent->bAutoActivate = false;
        WaterAudioComponent->SetVolumeMultiplier(0.0f);
        UE_LOG(LogBiomeAudio, Log, TEXT("Water audio component created"));
    }
    
    // Create wildlife audio component
    WildlifeAudioComponent = Owner->CreateDefaultSubobject<UAudioComponent>(TEXT("WildlifeAudio"));
    if (WildlifeAudioComponent)
    {
        WildlifeAudioComponent->bAutoActivate = false;
        WildlifeAudioComponent->SetVolumeMultiplier(0.0f);
        UE_LOG(LogBiomeAudio, Log, TEXT("Wildlife audio component created"));
    }
}

void UWorld_BiomeAudioSystem::TransitionToBiome(EWorld_BiomeType NewBiome, float TransitionDuration)
{
    if (NewBiome == CurrentBiome && !bIsTransitioning)
    {
        return; // Already in this biome
    }
    
    UE_LOG(LogBiomeAudio, Log, TEXT("Transitioning to biome: %d"), (int32)NewBiome);
    
    TargetBiome = NewBiome;
    bIsTransitioning = true;
    TransitionTimer = 0.0f;
    TransitionDuration = FMath::Max(TransitionDuration, 0.1f); // Minimum transition time
    
    // Get target audio settings
    if (BiomeAudioSettings.Contains(TargetBiome))
    {
        TargetAudioData = BiomeAudioSettings[TargetBiome];
    }
    else
    {
        UE_LOG(LogBiomeAudio, Warning, TEXT("No audio settings found for biome: %d"), (int32)TargetBiome);
        bIsTransitioning = false;
        return;
    }
    
    // Store current settings for interpolation
    if (BiomeAudioSettings.Contains(CurrentBiome))
    {
        CurrentAudioData = BiomeAudioSettings[CurrentBiome];
    }
}

void UWorld_BiomeAudioSystem::UpdateAudioTransitions(float DeltaTime)
{
    if (!bIsTransitioning)
    {
        return;
    }
    
    TransitionTimer += DeltaTime;
    float TransitionProgress = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);
    
    // Interpolate audio settings
    FWorld_BiomeAudioData InterpolatedData;
    InterpolatedData.BiomeType = TargetBiome;
    InterpolatedData.AmbientVolume = FMath::Lerp(CurrentAudioData.AmbientVolume, TargetAudioData.AmbientVolume, TransitionProgress);
    InterpolatedData.WindIntensity = FMath::Lerp(CurrentAudioData.WindIntensity, TargetAudioData.WindIntensity, TransitionProgress);
    InterpolatedData.WaterSounds = TransitionProgress > 0.5f ? TargetAudioData.WaterSounds : CurrentAudioData.WaterSounds;
    InterpolatedData.InsectSounds = TransitionProgress > 0.5f ? TargetAudioData.InsectSounds : CurrentAudioData.InsectSounds;
    InterpolatedData.BirdSounds = TransitionProgress > 0.5f ? TargetAudioData.BirdSounds : CurrentAudioData.BirdSounds;
    InterpolatedData.ReverbPreset = TransitionProgress > 0.5f ? TargetAudioData.ReverbPreset : CurrentAudioData.ReverbPreset;
    
    // Apply interpolated settings
    ApplyAudioSettings(InterpolatedData);
    
    // Check if transition is complete
    if (TransitionProgress >= 1.0f)
    {
        bIsTransitioning = false;
        CurrentBiome = TargetBiome;
        CurrentAudioData = TargetAudioData;
        UE_LOG(LogBiomeAudio, Log, TEXT("Biome transition complete to: %d"), (int32)CurrentBiome);
    }
}

void UWorld_BiomeAudioSystem::ApplyAudioSettings(const FWorld_BiomeAudioData& AudioData)
{
    // Apply ambient volume
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(AudioData.AmbientVolume);
        if (AudioData.AmbientVolume > 0.0f && !AmbientAudioComponent->IsPlaying())
        {
            AmbientAudioComponent->Play();
        }
        else if (AudioData.AmbientVolume <= 0.0f && AmbientAudioComponent->IsPlaying())
        {
            AmbientAudioComponent->Stop();
        }
    }
    
    // Apply wind intensity
    if (WindAudioComponent)
    {
        WindAudioComponent->SetVolumeMultiplier(AudioData.WindIntensity);
        if (AudioData.WindIntensity > 0.0f && !WindAudioComponent->IsPlaying())
        {
            WindAudioComponent->Play();
        }
        else if (AudioData.WindIntensity <= 0.0f && WindAudioComponent->IsPlaying())
        {
            WindAudioComponent->Stop();
        }
    }
    
    // Apply water sounds
    if (WaterAudioComponent)
    {
        if (AudioData.WaterSounds && !WaterAudioComponent->IsPlaying())
        {
            WaterAudioComponent->SetVolumeMultiplier(0.6f);
            WaterAudioComponent->Play();
        }
        else if (!AudioData.WaterSounds && WaterAudioComponent->IsPlaying())
        {
            WaterAudioComponent->Stop();
        }
    }
    
    // Apply wildlife sounds
    if (WildlifeAudioComponent)
    {
        float WildlifeVolume = 0.0f;
        if (AudioData.InsectSounds) WildlifeVolume += 0.3f;
        if (AudioData.BirdSounds) WildlifeVolume += 0.4f;
        
        WildlifeAudioComponent->SetVolumeMultiplier(WildlifeVolume);
        if (WildlifeVolume > 0.0f && !WildlifeAudioComponent->IsPlaying())
        {
            WildlifeAudioComponent->Play();
        }
        else if (WildlifeVolume <= 0.0f && WildlifeAudioComponent->IsPlaying())
        {
            WildlifeAudioComponent->Stop();
        }
    }
}

void UWorld_BiomeAudioSystem::CheckBiomeTransition()
{
    // Get player location
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    EWorld_BiomeType DetectedBiome = GetBiomeAtLocation(PlayerLocation);
    
    if (DetectedBiome != CurrentBiome)
    {
        TransitionToBiome(DetectedBiome, 3.0f); // 3 second transition
    }
}

EWorld_BiomeType UWorld_BiomeAudioSystem::GetBiomeAtLocation(const FVector& Location) const
{
    // Biome detection based on coordinates from brain memory
    float X = Location.X;
    float Y = Location.Y;
    
    // Swamp (southwest): X(-77500 to -25000), Y(-76500 to -15000)
    if (X >= -77500 && X <= -25000 && Y >= -76500 && Y <= -15000)
    {
        return EWorld_BiomeType::Swamp;
    }
    
    // Forest (northwest): X(-77500 to -15000), Y(15000 to 76500)
    if (X >= -77500 && X <= -15000 && Y >= 15000 && Y <= 76500)
    {
        return EWorld_BiomeType::Forest;
    }
    
    // Desert (east): X(25000 to 79500), Y(-30000 to 30000)
    if (X >= 25000 && X <= 79500 && Y >= -30000 && Y <= 30000)
    {
        return EWorld_BiomeType::Desert;
    }
    
    // Mountain (northeast): X(15000 to 79500), Y(20000 to 76500)
    if (X >= 15000 && X <= 79500 && Y >= 20000 && Y <= 76500)
    {
        return EWorld_BiomeType::Mountain;
    }
    
    // Default to Savanna (center): X(-20000 to 20000), Y(-20000 to 20000)
    return EWorld_BiomeType::Savanna;
}

void UWorld_BiomeAudioSystem::SetGlobalAudioVolume(float Volume)
{
    GlobalAudioVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Apply to all audio components
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(CurrentAudioData.AmbientVolume * GlobalAudioVolume);
    }
    if (WindAudioComponent)
    {
        WindAudioComponent->SetVolumeMultiplier(CurrentAudioData.WindIntensity * GlobalAudioVolume);
    }
    if (WaterAudioComponent && CurrentAudioData.WaterSounds)
    {
        WaterAudioComponent->SetVolumeMultiplier(0.6f * GlobalAudioVolume);
    }
    if (WildlifeAudioComponent)
    {
        float WildlifeVolume = 0.0f;
        if (CurrentAudioData.InsectSounds) WildlifeVolume += 0.3f;
        if (CurrentAudioData.BirdSounds) WildlifeVolume += 0.4f;
        WildlifeAudioComponent->SetVolumeMultiplier(WildlifeVolume * GlobalAudioVolume);
    }
    
    UE_LOG(LogBiomeAudio, Log, TEXT("Global audio volume set to: %f"), GlobalAudioVolume);
}

void UWorld_BiomeAudioSystem::EnableWeatherAudio(bool bEnable)
{
    bWeatherAudioEnabled = bEnable;
    UE_LOG(LogBiomeAudio, Log, TEXT("Weather audio %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}