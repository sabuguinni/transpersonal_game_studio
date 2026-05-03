#include "AudioManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

AAudio_AudioManager::AAudio_AudioManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create Audio Components
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    RootComponent = AmbientAudioComponent;
    AmbientAudioComponent->bAutoActivate = false;

    ThreatAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ThreatAudio"));
    ThreatAudioComponent->SetupAttachment(RootComponent);
    ThreatAudioComponent->bAutoActivate = false;

    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudio"));
    MusicAudioComponent->SetupAttachment(RootComponent);
    MusicAudioComponent->bAutoActivate = false;

    // Initialize default values
    CurrentBiome = EAudio_BiomeType::Savana;
    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    MasterVolume = 1.0f;
    FadeTime = 2.0f;
    ThreatDetectionRadius = 5000.0f;
}

void AAudio_AudioManager::BeginPlay()
{
    Super::BeginPlay();

    InitializeBiomeConfigs();
    InitializeThreatConfigs();

    // Start biome update timer
    GetWorldTimerManager().SetTimer(BiomeUpdateTimer, this, &AAudio_AudioManager::CheckBiomeUpdate, 2.0f, true);

    // Set initial ambient audio
    UpdateAmbientAudio();

    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Sistema iniciado - Bioma: %d, Threat: %d"), 
           (int32)CurrentBiome, (int32)CurrentThreatLevel);
}

void AAudio_AudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update threat detection based on nearby actors
    float ThreatDistance = CalculateThreatDistance();
    
    EAudio_ThreatLevel NewThreatLevel = EAudio_ThreatLevel::Safe;
    if (ThreatDistance > 0.0f)
    {
        if (ThreatDistance < 1000.0f)
        {
            NewThreatLevel = EAudio_ThreatLevel::Critical;
        }
        else if (ThreatDistance < 2500.0f)
        {
            NewThreatLevel = EAudio_ThreatLevel::Danger;
        }
        else if (ThreatDistance < 5000.0f)
        {
            NewThreatLevel = EAudio_ThreatLevel::Caution;
        }
    }

    if (NewThreatLevel != CurrentThreatLevel)
    {
        SetThreatLevel(NewThreatLevel);
    }
}

void AAudio_AudioManager::InitializeBiomeConfigs()
{
    // Initialize biome configurations with default values
    FAudio_BiomeConfig PantanoConfig;
    PantanoConfig.BaseVolume = 0.6f;
    PantanoConfig.ReverbIntensity = 0.8f;
    PantanoConfig.WindIntensity = 0.3f;
    BiomeConfigs.Add(EAudio_BiomeType::Pantano, PantanoConfig);

    FAudio_BiomeConfig FlorestaConfig;
    FlorestaConfig.BaseVolume = 0.7f;
    FlorestaConfig.ReverbIntensity = 0.6f;
    FlorestaConfig.WindIntensity = 0.4f;
    BiomeConfigs.Add(EAudio_BiomeType::Floresta, FlorestaConfig);

    FAudio_BiomeConfig SavanaConfig;
    SavanaConfig.BaseVolume = 0.5f;
    SavanaConfig.ReverbIntensity = 0.2f;
    SavanaConfig.WindIntensity = 0.6f;
    BiomeConfigs.Add(EAudio_BiomeType::Savana, SavanaConfig);

    FAudio_BiomeConfig DesertoConfig;
    DesertoConfig.BaseVolume = 0.4f;
    DesertoConfig.ReverbIntensity = 0.1f;
    DesertoConfig.WindIntensity = 0.8f;
    BiomeConfigs.Add(EAudio_BiomeType::Deserto, DesertoConfig);

    FAudio_BiomeConfig MontanhaConfig;
    MontanhaConfig.BaseVolume = 0.8f;
    MontanhaConfig.ReverbIntensity = 0.9f;
    MontanhaConfig.WindIntensity = 0.9f;
    BiomeConfigs.Add(EAudio_BiomeType::Montanha, MontanhaConfig);

    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Configurações de bioma inicializadas"));
}

void AAudio_AudioManager::InitializeThreatConfigs()
{
    // Initialize threat configurations
    FAudio_ThreatConfig SafeConfig;
    SafeConfig.VolumeMultiplier = 0.0f;
    SafeConfig.PitchMultiplier = 1.0f;
    SafeConfig.bShouldLoop = false;
    ThreatConfigs.Add(EAudio_ThreatLevel::Safe, SafeConfig);

    FAudio_ThreatConfig CautionConfig;
    CautionConfig.VolumeMultiplier = 0.3f;
    CautionConfig.PitchMultiplier = 0.9f;
    CautionConfig.bShouldLoop = true;
    ThreatConfigs.Add(EAudio_ThreatLevel::Caution, CautionConfig);

    FAudio_ThreatConfig DangerConfig;
    DangerConfig.VolumeMultiplier = 0.6f;
    DangerConfig.PitchMultiplier = 0.8f;
    DangerConfig.bShouldLoop = true;
    ThreatConfigs.Add(EAudio_ThreatLevel::Danger, DangerConfig);

    FAudio_ThreatConfig CriticalConfig;
    CriticalConfig.VolumeMultiplier = 1.0f;
    CriticalConfig.PitchMultiplier = 0.7f;
    CriticalConfig.bShouldLoop = true;
    ThreatConfigs.Add(EAudio_ThreatLevel::Critical, CriticalConfig);

    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Configurações de ameaça inicializadas"));
}

void AAudio_AudioManager::SetBiome(EAudio_BiomeType NewBiome)
{
    if (NewBiome != CurrentBiome)
    {
        CurrentBiome = NewBiome;
        UpdateAmbientAudio();
        
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Bioma alterado para %d"), (int32)NewBiome);
    }
}

void AAudio_AudioManager::SetThreatLevel(EAudio_ThreatLevel NewThreatLevel)
{
    if (NewThreatLevel != CurrentThreatLevel)
    {
        CurrentThreatLevel = NewThreatLevel;
        
        if (NewThreatLevel == EAudio_ThreatLevel::Safe)
        {
            StopThreatSound();
        }
        else
        {
            PlayThreatSound(NewThreatLevel);
        }

        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Nível de ameaça alterado para %d"), (int32)NewThreatLevel);
    }
}

void AAudio_AudioManager::PlayThreatSound(EAudio_ThreatLevel ThreatLevel)
{
    if (ThreatConfigs.Contains(ThreatLevel))
    {
        const FAudio_ThreatConfig& Config = ThreatConfigs[ThreatLevel];
        
        if (ThreatAudioComponent && Config.ThreatSound)
        {
            ThreatAudioComponent->SetSound(Config.ThreatSound);
            ThreatAudioComponent->SetVolumeMultiplier(Config.VolumeMultiplier * MasterVolume);
            ThreatAudioComponent->SetPitchMultiplier(Config.PitchMultiplier);
            
            if (Config.bShouldLoop)
            {
                ThreatAudioComponent->Play();
            }
            else
            {
                ThreatAudioComponent->Play();
            }
        }
    }
}

void AAudio_AudioManager::StopThreatSound()
{
    if (ThreatAudioComponent && ThreatAudioComponent->IsPlaying())
    {
        ThreatAudioComponent->FadeOut(FadeTime, 0.0f);
    }
}

void AAudio_AudioManager::UpdateAudioBasedOnLocation(FVector PlayerLocation)
{
    EAudio_BiomeType DetectedBiome = DetectBiomeFromLocation(PlayerLocation);
    SetBiome(DetectedBiome);
}

EAudio_BiomeType AAudio_AudioManager::DetectBiomeFromLocation(FVector Location)
{
    // Biome detection based on world coordinates
    float X = Location.X;
    float Y = Location.Y;
    float Z = Location.Z;

    // Montanha Nevada (nordeste, altitude elevada)
    if (X > 15000.0f && Y > 20000.0f && Z > 400.0f)
    {
        return EAudio_BiomeType::Montanha;
    }
    
    // Deserto (leste)
    if (X > 25000.0f && Y > -30000.0f && Y < 30000.0f)
    {
        return EAudio_BiomeType::Deserto;
    }
    
    // Pantano (sudoeste)
    if (X < -25000.0f && Y < -15000.0f)
    {
        return EAudio_BiomeType::Pantano;
    }
    
    // Floresta (noroeste)
    if (X < -15000.0f && Y > 15000.0f)
    {
        return EAudio_BiomeType::Floresta;
    }
    
    // Savana (centro) - default
    return EAudio_BiomeType::Savana;
}

void AAudio_AudioManager::SetMasterVolume(float NewVolume)
{
    MasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    
    // Update all audio components
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(MasterVolume);
    }
    
    if (ThreatAudioComponent)
    {
        const FAudio_ThreatConfig* Config = ThreatConfigs.Find(CurrentThreatLevel);
        if (Config)
        {
            ThreatAudioComponent->SetVolumeMultiplier(Config->VolumeMultiplier * MasterVolume);
        }
    }
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MasterVolume);
    }
}

void AAudio_AudioManager::UpdateAmbientAudio()
{
    if (BiomeConfigs.Contains(CurrentBiome))
    {
        const FAudio_BiomeConfig& Config = BiomeConfigs[CurrentBiome];
        
        if (AmbientAudioComponent && Config.AmbientSound)
        {
            FadeToNewAmbient(Config.AmbientSound, Config.BaseVolume * MasterVolume);
        }
    }
}

void AAudio_AudioManager::FadeToNewAmbient(USoundBase* NewSound, float TargetVolume)
{
    if (AmbientAudioComponent)
    {
        if (AmbientAudioComponent->IsPlaying())
        {
            AmbientAudioComponent->FadeOut(FadeTime, 0.0f);
        }
        
        // Set new sound and fade in
        AmbientAudioComponent->SetSound(NewSound);
        AmbientAudioComponent->SetVolumeMultiplier(0.0f);
        AmbientAudioComponent->Play();
        AmbientAudioComponent->FadeIn(FadeTime, TargetVolume);
    }
}

void AAudio_AudioManager::CheckBiomeUpdate()
{
    // Get player location
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        UpdateAudioBasedOnLocation(PlayerLocation);
    }
}

float AAudio_AudioManager::CalculateThreatDistance()
{
    float ClosestThreatDistance = -1.0f;
    
    // Find all potential threat actors (dinosaurs)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return ClosestThreatDistance;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor != PlayerPawn)
        {
            FString ActorName = Actor->GetName().ToLower();
            
            // Check if actor is a threat (dinosaur)
            if (ActorName.Contains(TEXT("trex")) || 
                ActorName.Contains(TEXT("raptor")) || 
                ActorName.Contains(TEXT("dinosaur")))
            {
                float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
                
                if (Distance <= ThreatDetectionRadius)
                {
                    if (ClosestThreatDistance < 0.0f || Distance < ClosestThreatDistance)
                    {
                        ClosestThreatDistance = Distance;
                    }
                }
            }
        }
    }
    
    return ClosestThreatDistance;
}