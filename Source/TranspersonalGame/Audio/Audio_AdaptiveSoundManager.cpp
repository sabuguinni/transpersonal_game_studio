#include "Audio_AdaptiveSoundManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Sound/SoundCue.h"

AAudio_AdaptiveSoundManager::AAudio_AdaptiveSoundManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Criar componentes de áudio
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    RootComponent = AmbientAudioComponent;
    AmbientAudioComponent->bAutoActivate = true;
    AmbientAudioComponent->VolumeMultiplier = 0.7f;

    ThreatAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ThreatAudio"));
    ThreatAudioComponent->SetupAttachment(RootComponent);
    ThreatAudioComponent->bAutoActivate = false;
    ThreatAudioComponent->VolumeMultiplier = 0.8f;

    WeatherAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WeatherAudio"));
    WeatherAudioComponent->SetupAttachment(RootComponent);
    WeatherAudioComponent->bAutoActivate = false;
    WeatherAudioComponent->VolumeMultiplier = 0.6f;

    // Inicializar estado
    CurrentBiome = EAudio_BiomeType::Savanna;
    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    CurrentWeatherIntensity = 0.3f;
    BiomeTransitionTimer = 0.0f;
    ThreatFadeTimer = 0.0f;
    RandomSoundTimer = 0.0f;
    RandomSoundInterval = 15.0f;
}

void AAudio_AdaptiveSoundManager::BeginPlay()
{
    Super::BeginPlay();

    InitializeBiomeSettings();
    InitializeThreatSettings();

    // Activar áudio ambiente inicial
    UpdateBiomeAudio(CurrentBiome);
}

void AAudio_AdaptiveSoundManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateAudioComponents(DeltaTime);
    HandleBiomeTransition(DeltaTime);
    HandleThreatTransition(DeltaTime);

    // Timer para sons aleatórios
    RandomSoundTimer += DeltaTime;
    if (RandomSoundTimer >= RandomSoundInterval)
    {
        PlayRandomBiomeSound();
        RandomSoundTimer = 0.0f;
        // Variar o intervalo
        RandomSoundInterval = FMath::RandRange(10.0f, 25.0f);
    }

    // Detectar bioma actual baseado na posição do jogador
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        EAudio_BiomeType DetectedBiome = DetectCurrentBiome(PlayerLocation);
        
        if (DetectedBiome != CurrentBiome)
        {
            UpdateBiomeAudio(DetectedBiome);
        }

        // Calcular nível de ameaça
        EAudio_ThreatLevel NewThreatLevel = CalculateThreatLevel();
        if (NewThreatLevel != CurrentThreatLevel)
        {
            UpdateThreatLevel(NewThreatLevel);
        }
    }
}

void AAudio_AdaptiveSoundManager::UpdateBiomeAudio(EAudio_BiomeType NewBiome)
{
    if (NewBiome == CurrentBiome)
    {
        return;
    }

    CurrentBiome = NewBiome;
    BiomeTransitionTimer = 0.0f;

    // Obter configurações do bioma
    if (BiomeAudioSettings.Contains(NewBiome))
    {
        const FAudio_BiomeAudioSettings& Settings = BiomeAudioSettings[NewBiome];
        
        if (Settings.AmbientSound.IsValid())
        {
            USoundCue* SoundCue = Settings.AmbientSound.LoadSynchronous();
            if (SoundCue && AmbientAudioComponent)
            {
                AmbientAudioComponent->SetSound(SoundCue);
                AmbientAudioComponent->SetVolumeMultiplier(Settings.BaseVolume);
                AmbientAudioComponent->Play();
            }
        }

        // Ajustar intensidade do vento
        UpdateWeatherIntensity(Settings.WindIntensity);
    }

    UE_LOG(LogTemp, Warning, TEXT("Bioma alterado para: %d"), (int32)NewBiome);
}

void AAudio_AdaptiveSoundManager::UpdateThreatLevel(EAudio_ThreatLevel NewThreatLevel)
{
    if (NewThreatLevel == CurrentThreatLevel)
    {
        return;
    }

    CurrentThreatLevel = NewThreatLevel;
    ThreatFadeTimer = 0.0f;

    // Configurar áudio de ameaça
    if (ThreatAudioSettings.Contains(NewThreatLevel))
    {
        const FAudio_ThreatAudioData& ThreatData = ThreatAudioSettings[NewThreatLevel];
        
        if (NewThreatLevel == EAudio_ThreatLevel::Safe)
        {
            // Parar sons de ameaça
            if (ThreatAudioComponent && ThreatAudioComponent->IsPlaying())
            {
                ThreatAudioComponent->FadeOut(2.0f, 0.0f);
            }
        }
        else
        {
            // Activar sons de ameaça
            if (ThreatData.ThreatSound.IsValid())
            {
                USoundCue* ThreatSound = ThreatData.ThreatSound.LoadSynchronous();
                if (ThreatSound && ThreatAudioComponent)
                {
                    ThreatAudioComponent->SetSound(ThreatSound);
                    ThreatAudioComponent->SetVolumeMultiplier(ThreatData.VolumeMultiplier);
                    ThreatAudioComponent->FadeIn(1.5f);
                }
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Nível de ameaça alterado para: %d"), (int32)NewThreatLevel);
}

void AAudio_AdaptiveSoundManager::UpdateWeatherIntensity(float WeatherIntensity)
{
    CurrentWeatherIntensity = FMath::Clamp(WeatherIntensity, 0.0f, 1.0f);

    if (WeatherAudioComponent)
    {
        WeatherAudioComponent->SetVolumeMultiplier(CurrentWeatherIntensity * 0.6f);
        
        if (CurrentWeatherIntensity > 0.1f && !WeatherAudioComponent->IsPlaying())
        {
            WeatherAudioComponent->Play();
        }
        else if (CurrentWeatherIntensity <= 0.1f && WeatherAudioComponent->IsPlaying())
        {
            WeatherAudioComponent->FadeOut(3.0f, 0.0f);
        }
    }
}

void AAudio_AdaptiveSoundManager::PlayRandomBiomeSound()
{
    if (!BiomeAudioSettings.Contains(CurrentBiome))
    {
        return;
    }

    const FAudio_BiomeAudioSettings& Settings = BiomeAudioSettings[CurrentBiome];
    
    if (Settings.RandomSounds.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, Settings.RandomSounds.Num() - 1);
        TSoftObjectPtr<USoundCue> RandomSoundPtr = Settings.RandomSounds[RandomIndex];
        
        if (RandomSoundPtr.IsValid())
        {
            USoundCue* RandomSound = RandomSoundPtr.LoadSynchronous();
            if (RandomSound)
            {
                UGameplayStatics::PlaySoundAtLocation(
                    GetWorld(),
                    RandomSound,
                    GetActorLocation(),
                    FMath::RandRange(0.6f, 1.0f)
                );
            }
        }
    }
}

EAudio_BiomeType AAudio_AdaptiveSoundManager::DetectCurrentBiome(FVector PlayerLocation)
{
    // Coordenadas dos biomas baseadas na memória do sistema
    float X = PlayerLocation.X;
    float Y = PlayerLocation.Y;

    // Pantano (sudoeste)
    if (X >= -77500 && X <= -25000 && Y >= -76500 && Y <= -15000)
    {
        return EAudio_BiomeType::Swamp;
    }
    // Floresta (noroeste)
    else if (X >= -77500 && X <= -15000 && Y >= 15000 && Y <= 76500)
    {
        return EAudio_BiomeType::Forest;
    }
    // Deserto (leste)
    else if (X >= 25000 && X <= 79500 && Y >= -30000 && Y <= 30000)
    {
        return EAudio_BiomeType::Desert;
    }
    // Montanha Nevada (nordeste)
    else if (X >= 15000 && X <= 79500 && Y >= 20000 && Y <= 76500)
    {
        return EAudio_BiomeType::Mountain;
    }
    // Savana (centro) - default
    else
    {
        return EAudio_BiomeType::Savanna;
    }
}

EAudio_ThreatLevel AAudio_AdaptiveSoundManager::CalculateThreatLevel()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return EAudio_ThreatLevel::Safe;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Procurar dinossauros próximos
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);

    float ClosestThreatDistance = FLT_MAX;
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == PlayerPawn)
        {
            continue;
        }

        // Verificar se é um dinossauro (assumindo que têm "Dinosaur" no nome)
        FString ActorName = Actor->GetName();
        if (ActorName.Contains(TEXT("Dinosaur")) || ActorName.Contains(TEXT("TRex")) || 
            ActorName.Contains(TEXT("Raptor")) || ActorName.Contains(TEXT("Brach")))
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            ClosestThreatDistance = FMath::Min(ClosestThreatDistance, Distance);
        }
    }

    // Determinar nível de ameaça baseado na distância
    if (ClosestThreatDistance < 500.0f)
    {
        return EAudio_ThreatLevel::Critical;
    }
    else if (ClosestThreatDistance < 1500.0f)
    {
        return EAudio_ThreatLevel::Danger;
    }
    else if (ClosestThreatDistance < 3000.0f)
    {
        return EAudio_ThreatLevel::Caution;
    }
    else
    {
        return EAudio_ThreatLevel::Safe;
    }
}

void AAudio_AdaptiveSoundManager::InitializeBiomeSettings()
{
    // Configurações básicas para cada bioma
    // Nota: Os SoundCues serão configurados no Blueprint ou carregados dinamicamente
    
    FAudio_BiomeAudioSettings SwampSettings;
    SwampSettings.BaseVolume = 0.8f;
    SwampSettings.WindIntensity = 0.3f;
    BiomeAudioSettings.Add(EAudio_BiomeType::Swamp, SwampSettings);

    FAudio_BiomeAudioSettings ForestSettings;
    ForestSettings.BaseVolume = 0.7f;
    ForestSettings.WindIntensity = 0.6f;
    BiomeAudioSettings.Add(EAudio_BiomeType::Forest, ForestSettings);

    FAudio_BiomeAudioSettings SavannaSettings;
    SavannaSettings.BaseVolume = 0.6f;
    SavannaSettings.WindIntensity = 0.4f;
    BiomeAudioSettings.Add(EAudio_BiomeType::Savanna, SavannaSettings);

    FAudio_BiomeAudioSettings DesertSettings;
    DesertSettings.BaseVolume = 0.5f;
    DesertSettings.WindIntensity = 0.8f;
    BiomeAudioSettings.Add(EAudio_BiomeType::Desert, DesertSettings);

    FAudio_BiomeAudioSettings MountainSettings;
    MountainSettings.BaseVolume = 0.4f;
    MountainSettings.WindIntensity = 0.9f;
    BiomeAudioSettings.Add(EAudio_BiomeType::Mountain, MountainSettings);
}

void AAudio_AdaptiveSoundManager::InitializeThreatSettings()
{
    FAudio_ThreatAudioData SafeData;
    SafeData.VolumeMultiplier = 0.0f;
    ThreatAudioSettings.Add(EAudio_ThreatLevel::Safe, SafeData);

    FAudio_ThreatAudioData CautionData;
    CautionData.VolumeMultiplier = 0.3f;
    CautionData.TriggerDistance = 3000.0f;
    ThreatAudioSettings.Add(EAudio_ThreatLevel::Caution, CautionData);

    FAudio_ThreatAudioData DangerData;
    DangerData.VolumeMultiplier = 0.6f;
    DangerData.TriggerDistance = 1500.0f;
    ThreatAudioSettings.Add(EAudio_ThreatLevel::Danger, DangerData);

    FAudio_ThreatAudioData CriticalData;
    CriticalData.VolumeMultiplier = 1.0f;
    CriticalData.TriggerDistance = 500.0f;
    ThreatAudioSettings.Add(EAudio_ThreatLevel::Critical, CriticalData);
}

void AAudio_AdaptiveSoundManager::UpdateAudioComponents(float DeltaTime)
{
    // Ajustar volumes baseado no estado actual
    if (AmbientAudioComponent && BiomeAudioSettings.Contains(CurrentBiome))
    {
        const FAudio_BiomeAudioSettings& Settings = BiomeAudioSettings[CurrentBiome];
        float TargetVolume = Settings.BaseVolume;
        
        // Reduzir volume ambiente quando há ameaça
        if (CurrentThreatLevel != EAudio_ThreatLevel::Safe)
        {
            TargetVolume *= 0.7f;
        }
        
        float CurrentVolume = AmbientAudioComponent->VolumeMultiplier;
        float NewVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, 2.0f);
        AmbientAudioComponent->SetVolumeMultiplier(NewVolume);
    }
}

void AAudio_AdaptiveSoundManager::HandleBiomeTransition(float DeltaTime)
{
    BiomeTransitionTimer += DeltaTime;
    
    // Transição suave entre biomas (fade in/out)
    if (BiomeTransitionTimer < 3.0f)
    {
        float TransitionAlpha = BiomeTransitionTimer / 3.0f;
        
        if (AmbientAudioComponent)
        {
            float BaseVolume = BiomeAudioSettings.Contains(CurrentBiome) ? 
                BiomeAudioSettings[CurrentBiome].BaseVolume : 0.7f;
            float TransitionVolume = FMath::Lerp(0.0f, BaseVolume, TransitionAlpha);
            AmbientAudioComponent->SetVolumeMultiplier(TransitionVolume);
        }
    }
}

void AAudio_AdaptiveSoundManager::HandleThreatTransition(float DeltaTime)
{
    ThreatFadeTimer += DeltaTime;
    
    // Transição de áudio de ameaça
    if (ThreatFadeTimer < 2.0f && ThreatAudioComponent)
    {
        float FadeAlpha = ThreatFadeTimer / 2.0f;
        
        if (CurrentThreatLevel == EAudio_ThreatLevel::Safe)
        {
            // Fade out
            float Volume = FMath::Lerp(1.0f, 0.0f, FadeAlpha);
            ThreatAudioComponent->SetVolumeMultiplier(Volume);
        }
        else
        {
            // Fade in
            float TargetVolume = ThreatAudioSettings.Contains(CurrentThreatLevel) ?
                ThreatAudioSettings[CurrentThreatLevel].VolumeMultiplier : 0.5f;
            float Volume = FMath::Lerp(0.0f, TargetVolume, FadeAlpha);
            ThreatAudioComponent->SetVolumeMultiplier(Volume);
        }
    }
}