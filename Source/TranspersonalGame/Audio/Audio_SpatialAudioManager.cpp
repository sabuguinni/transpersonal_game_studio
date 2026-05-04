#include "Audio_SpatialAudioManager.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Components/SceneComponent.h"
#include "Sound/SoundCue.h"

AAudio_SpatialAudioManager::AAudio_SpatialAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Criar componente raiz
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Criar componentes de áudio
    MasterAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MasterAudioComponent"));
    MasterAudioComponent->SetupAttachment(RootComponent);
    MasterAudioComponent->bAutoActivate = false;

    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;

    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));
    MusicAudioComponent->SetupAttachment(RootComponent);
    MusicAudioComponent->bAutoActivate = false;

    EffectsAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EffectsAudioComponent"));
    EffectsAudioComponent->SetupAttachment(RootComponent);
    EffectsAudioComponent->bAutoActivate = false;

    // Inicializar variáveis
    PlayerPawn = nullptr;
    CurrentBiome = TEXT("Savana");
    CurrentThreatLevel = 0.0f;
    RandomSoundTimer = 0.0f;
}

void AAudio_SpatialAudioManager::BeginPlay()
{
    Super::BeginPlay();

    // Encontrar referência do jogador
    UpdatePlayerReference();

    // Inicializar configurações de áudio dos biomas
    InitializeBiomeAudioSettings();

    // Configurar componentes de áudio
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(0.7f);
        AmbientAudioComponent->bAutoActivate = true;
    }

    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(0.5f);
        MusicAudioComponent->bAutoActivate = true;
    }

    if (EffectsAudioComponent)
    {
        EffectsAudioComponent->SetVolumeMultiplier(1.0f);
        EffectsAudioComponent->bAutoActivate = true;
    }

    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: Sistema de áudio espacial inicializado"));
}

void AAudio_SpatialAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (PlayerPawn)
    {
        // Actualizar áudio espacial baseado na posição do jogador
        UpdateSpatialAudio();

        // Actualizar áudio do bioma
        UpdateBiomeAudio();

        // Processar sons aleatórios
        ProcessRandomSounds(DeltaTime);

        // Limpar sons terminados
        CleanupFinishedSounds();
    }
}

void AAudio_SpatialAudioManager::PlaySpatialSound(const FAudio_SpatialSoundData& SoundData)
{
    if (!SoundData.SoundCue.IsNull())
    {
        // Adicionar à lista de sons espaciais activos
        ActiveSpatialSounds.Add(SoundData);

        // Calcular volume baseado na distância
        if (PlayerPawn)
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            float Distance = FVector::Dist(PlayerLocation, SoundData.WorldLocation);
            float Volume = CalculateVolumeFromDistance(SoundData.WorldLocation, PlayerLocation, SoundData.MaxAudibleDistance);

            if (Volume > 0.01f)
            {
                // Tocar o som com volume calculado
                if (EffectsAudioComponent && !EffectsAudioComponent->IsPlaying())
                {
                    USoundCue* LoadedSound = SoundData.SoundCue.LoadSynchronous();
                    if (LoadedSound)
                    {
                        EffectsAudioComponent->SetSound(LoadedSound);
                        EffectsAudioComponent->SetVolumeMultiplier(Volume * SoundData.VolumeMultiplier);
                        EffectsAudioComponent->SetWorldLocation(SoundData.WorldLocation);
                        EffectsAudioComponent->Play();

                        UE_LOG(LogTemp, Log, TEXT("Audio_SpatialAudioManager: Som espacial reproduzido em %s com volume %f"), 
                               *SoundData.WorldLocation.ToString(), Volume);
                    }
                }
            }
        }
    }
}

void AAudio_SpatialAudioManager::StopSpatialSound(FVector Location, float Tolerance)
{
    for (int32 i = ActiveSpatialSounds.Num() - 1; i >= 0; i--)
    {
        if (FVector::Dist(ActiveSpatialSounds[i].WorldLocation, Location) <= Tolerance)
        {
            ActiveSpatialSounds.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Audio_SpatialAudioManager: Som espacial removido em %s"), *Location.ToString());
        }
    }
}

void AAudio_SpatialAudioManager::SetCurrentBiome(const FString& BiomeName)
{
    if (CurrentBiome != BiomeName)
    {
        CurrentBiome = BiomeName;
        UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: Bioma alterado para %s"), *BiomeName);

        // Actualizar áudio do bioma imediatamente
        UpdateBiomeAudio();
    }
}

void AAudio_SpatialAudioManager::SetThreatLevel(float ThreatLevel)
{
    CurrentThreatLevel = FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
    
    // Ajustar volumes baseado no nível de ameaça
    if (AmbientAudioComponent)
    {
        float AmbientVolume = FMath::Lerp(0.7f, 0.3f, CurrentThreatLevel);
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume);
    }

    if (MusicAudioComponent)
    {
        float MusicVolume = FMath::Lerp(0.5f, 0.8f, CurrentThreatLevel);
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume);
    }

    UE_LOG(LogTemp, Log, TEXT("Audio_SpatialAudioManager: Nível de ameaça definido para %f"), CurrentThreatLevel);
}

void AAudio_SpatialAudioManager::UpdatePlayerReference()
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerPawn = PC->GetPawn();
            if (PlayerPawn)
            {
                UE_LOG(LogTemp, Log, TEXT("Audio_SpatialAudioManager: Referência do jogador actualizada"));
            }
        }
    }
}

void AAudio_SpatialAudioManager::InitializeBiomeAudioSettings()
{
    // Configurações para Savana
    FAudio_BiomeAudioSettings SavanaSettings;
    SavanaSettings.AmbientVolume = 0.6f;
    SavanaSettings.MusicVolume = 0.4f;
    SavanaSettings.RandomSoundFrequency = 20.0f;
    BiomeAudioSettings.Add(TEXT("Savana"), SavanaSettings);

    // Configurações para Floresta
    FAudio_BiomeAudioSettings FlorestaSettings;
    FlorestaSettings.AmbientVolume = 0.8f;
    FlorestaSettings.MusicVolume = 0.3f;
    FlorestaSettings.RandomSoundFrequency = 12.0f;
    BiomeAudioSettings.Add(TEXT("Floresta"), FlorestaSettings);

    // Configurações para Pântano
    FAudio_BiomeAudioSettings PantanoSettings;
    PantanoSettings.AmbientVolume = 0.7f;
    PantanoSettings.MusicVolume = 0.6f;
    PantanoSettings.RandomSoundFrequency = 25.0f;
    BiomeAudioSettings.Add(TEXT("Pantano"), PantanoSettings);

    // Configurações para Deserto
    FAudio_BiomeAudioSettings DesertoSettings;
    DesertoSettings.AmbientVolume = 0.4f;
    DesertoSettings.MusicVolume = 0.5f;
    DesertoSettings.RandomSoundFrequency = 30.0f;
    BiomeAudioSettings.Add(TEXT("Deserto"), DesertoSettings);

    // Configurações para Montanha Nevada
    FAudio_BiomeAudioSettings MontanhaSettings;
    MontanhaSettings.AmbientVolume = 0.5f;
    MontanhaSettings.MusicVolume = 0.4f;
    MontanhaSettings.RandomSoundFrequency = 35.0f;
    BiomeAudioSettings.Add(TEXT("Montanha"), MontanhaSettings);

    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: Configurações de áudio dos biomas inicializadas"));
}

FString AAudio_SpatialAudioManager::DetectCurrentBiome(FVector PlayerLocation)
{
    // Coordenadas dos biomas baseadas na memória do Hugo
    float X = PlayerLocation.X;
    float Y = PlayerLocation.Y;

    // Pântano (sudoeste): X(-77500 a -25000), Y(-76500 a -15000)
    if (X >= -77500 && X <= -25000 && Y >= -76500 && Y <= -15000)
    {
        return TEXT("Pantano");
    }
    // Floresta (noroeste): X(-77500 a -15000), Y(15000 a 76500)
    else if (X >= -77500 && X <= -15000 && Y >= 15000 && Y <= 76500)
    {
        return TEXT("Floresta");
    }
    // Savana (centro): X(-20000 a 20000), Y(-20000 a 20000)
    else if (X >= -20000 && X <= 20000 && Y >= -20000 && Y <= 20000)
    {
        return TEXT("Savana");
    }
    // Deserto (leste): X(25000 a 79500), Y(-30000 a 30000)
    else if (X >= 25000 && X <= 79500 && Y >= -30000 && Y <= 30000)
    {
        return TEXT("Deserto");
    }
    // Montanha Nevada (nordeste): X(15000 a 79500), Y(20000 a 76500)
    else if (X >= 15000 && X <= 79500 && Y >= 20000 && Y <= 76500)
    {
        return TEXT("Montanha");
    }

    // Default para Savana se não detectar bioma específico
    return TEXT("Savana");
}

void AAudio_SpatialAudioManager::UpdateSpatialAudio()
{
    if (!PlayerPawn) return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Detectar bioma actual
    FString DetectedBiome = DetectCurrentBiome(PlayerLocation);
    if (DetectedBiome != CurrentBiome)
    {
        SetCurrentBiome(DetectedBiome);
    }

    // Actualizar volume dos sons espaciais baseado na distância
    for (FAudio_SpatialSoundData& SpatialSound : ActiveSpatialSounds)
    {
        float Volume = CalculateVolumeFromDistance(SpatialSound.WorldLocation, PlayerLocation, SpatialSound.MaxAudibleDistance);
        SpatialSound.VolumeMultiplier = Volume;
    }
}

void AAudio_SpatialAudioManager::UpdateBiomeAudio()
{
    if (BiomeAudioSettings.Contains(CurrentBiome))
    {
        const FAudio_BiomeAudioSettings& Settings = BiomeAudioSettings[CurrentBiome];

        // Actualizar volumes dos componentes de áudio
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->SetVolumeMultiplier(Settings.AmbientVolume * (1.0f - CurrentThreatLevel * 0.5f));
        }

        if (MusicAudioComponent)
        {
            MusicAudioComponent->SetVolumeMultiplier(Settings.MusicVolume * (1.0f + CurrentThreatLevel * 0.5f));
        }
    }
}

void AAudio_SpatialAudioManager::ProcessRandomSounds(float DeltaTime)
{
    RandomSoundTimer += DeltaTime;

    if (BiomeAudioSettings.Contains(CurrentBiome))
    {
        const FAudio_BiomeAudioSettings& Settings = BiomeAudioSettings[CurrentBiome];

        if (RandomSoundTimer >= Settings.RandomSoundFrequency)
        {
            RandomSoundTimer = 0.0f;

            // Reproduzir som aleatório do bioma (implementação futura com assets)
            UE_LOG(LogTemp, Log, TEXT("Audio_SpatialAudioManager: Tempo para som aleatório do bioma %s"), *CurrentBiome);
        }
    }
}

void AAudio_SpatialAudioManager::CleanupFinishedSounds()
{
    // Remover sons espaciais que já terminaram (implementação futura)
    for (int32 i = ActiveSpatialSounds.Num() - 1; i >= 0; i--)
    {
        // Lógica para verificar se o som terminou
        // Por agora, manter todos os sons activos
    }
}

float AAudio_SpatialAudioManager::CalculateVolumeFromDistance(FVector SoundLocation, FVector ListenerLocation, float MaxDistance)
{
    float Distance = FVector::Dist(SoundLocation, ListenerLocation);
    
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }

    // Curva de atenuação logarítmica
    float DistanceRatio = Distance / MaxDistance;
    float Volume = 1.0f - (DistanceRatio * DistanceRatio);
    
    return FMath::Clamp(Volume, 0.0f, 1.0f);
}