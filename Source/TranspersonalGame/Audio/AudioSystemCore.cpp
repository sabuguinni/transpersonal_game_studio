#include "AudioSystemCore.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../Dinosaurs/Dinosaur.h"
#include "../Player/PlayerCharacter.h"

UAudioSystemCore::UAudioSystemCore()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS tick for audio updates

    // Criar componentes de áudio
    MusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComponent"));
    AmbientComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientComponent"));
    StingerComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("StingerComponent"));

    // Configurar componentes
    if (MusicComponent)
    {
        MusicComponent->bAutoActivate = false;
        MusicComponent->VolumeMultiplier = 0.7f;
    }

    if (AmbientComponent)
    {
        AmbientComponent->bAutoActivate = false;
        AmbientComponent->VolumeMultiplier = 0.8f;
    }

    if (StingerComponent)
    {
        StingerComponent->bAutoActivate = false;
        StingerComponent->VolumeMultiplier = 1.0f;
    }
}

void UAudioSystemCore::BeginPlay()
{
    Super::BeginPlay();

    // Inicializar sistema de música adaptativa
    if (AdaptiveMusicSystem && MusicComponent)
    {
        MusicComponent->SetSound(AdaptiveMusicSystem);
        MusicComponent->Play();
    }

    // Inicializar soundscape ambiente
    if (AmbientSoundscape && AmbientComponent)
    {
        AmbientComponent->SetSound(AmbientSoundscape);
        AmbientComponent->Play();
    }

    // Configurar timer para verificação de ameaças
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ThreatCheckTimer,
            this,
            &UAudioSystemCore::CheckNearbyThreats,
            2.0f, // Check every 2 seconds
            true
        );
    }

    // Estado inicial
    CurrentAudioState.CurrentZone = EAudioZoneType::SafeZone;
    CurrentAudioState.ThreatLevel = EThreatLevel::None;
    CurrentAudioState.StressLevel = 0.0f;

    UpdateMusicParameters();
    UpdateAmbientParameters();
}

void UAudioSystemCore::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    LastMusicUpdate += DeltaTime;
    
    if (LastMusicUpdate >= MusicUpdateInterval)
    {
        UpdateMusicParameters();
        UpdateAmbientParameters();
        LastMusicUpdate = 0.0f;
    }
}

void UAudioSystemCore::UpdateAudioZone(EAudioZoneType NewZone)
{
    if (CurrentAudioState.CurrentZone != NewZone)
    {
        CurrentAudioState.CurrentZone = NewZone;
        
        // Log da mudança de zona
        UE_LOG(LogTemp, Log, TEXT("Audio Zone Changed: %d"), (int32)NewZone);
        
        UpdateMusicParameters();
        UpdateAmbientParameters();
    }
}

void UAudioSystemCore::UpdateThreatLevel(EThreatLevel NewThreatLevel)
{
    if (CurrentAudioState.ThreatLevel != NewThreatLevel)
    {
        EThreatLevel OldLevel = CurrentAudioState.ThreatLevel;
        CurrentAudioState.ThreatLevel = NewThreatLevel;
        
        // Se a ameaça aumentou significativamente, tocar stinger
        if ((int32)NewThreatLevel > (int32)OldLevel + 1)
        {
            // TODO: Implementar stinger baseado no nível de ameaça
        }
        
        UpdateMusicParameters();
    }
}

void UAudioSystemCore::UpdateStressLevel(float NewStressLevel)
{
    NewStressLevel = FMath::Clamp(NewStressLevel, 0.0f, 1.0f);
    
    if (FMath::Abs(CurrentAudioState.StressLevel - NewStressLevel) > 0.05f)
    {
        CurrentAudioState.StressLevel = NewStressLevel;
        UpdateMusicParameters();
    }
}

void UAudioSystemCore::PlayStinger(USoundCue* StingerSound)
{
    if (StingerSound && StingerComponent)
    {
        StingerComponent->SetSound(StingerSound);
        StingerComponent->Play();
    }
}

void UAudioSystemCore::OnDinosaurSpotted(ADinosaur* Dinosaur)
{
    if (Dinosaur && !CurrentAudioState.NearbyDinosaurs.Contains(Dinosaur))
    {
        CurrentAudioState.NearbyDinosaurs.Add(Dinosaur);
        
        // Determinar nível de ameaça baseado no dinossauro
        // TODO: Implementar sistema de classificação de ameaça dos dinossauros
        
        UpdateThreatLevel(EThreatLevel::Medium); // Placeholder
    }
}

void UAudioSystemCore::OnDinosaurLost(ADinosaur* Dinosaur)
{
    if (Dinosaur)
    {
        CurrentAudioState.NearbyDinosaurs.Remove(Dinosaur);
        
        // Recalcular nível de ameaça
        CheckNearbyThreats();
    }
}

void UAudioSystemCore::UpdateMusicParameters()
{
    if (!MusicComponent || !AdaptiveMusicSystem)
        return;

    // Calcular parâmetros baseados no estado atual
    float Tension = 0.0f;
    float Danger = 0.0f;
    float Exploration = 0.0f;

    switch (CurrentAudioState.CurrentZone)
    {
        case EAudioZoneType::SafeZone:
            Tension = 0.1f;
            Danger = 0.0f;
            Exploration = 0.3f;
            break;
            
        case EAudioZoneType::ExplorationZone:
            Tension = 0.3f;
            Danger = 0.2f;
            Exploration = 0.8f;
            break;
            
        case EAudioZoneType::DangerZone:
            Tension = 0.7f;
            Danger = 0.6f;
            Exploration = 0.4f;
            break;
            
        case EAudioZoneType::HuntingZone:
            Tension = 1.0f;
            Danger = 1.0f;
            Exploration = 0.0f;
            break;
    }

    // Ajustar baseado no nível de ameaça
    float ThreatMultiplier = (float)CurrentAudioState.ThreatLevel / 4.0f;
    Tension = FMath::Clamp(Tension + (ThreatMultiplier * 0.3f), 0.0f, 1.0f);
    Danger = FMath::Clamp(Danger + (ThreatMultiplier * 0.4f), 0.0f, 1.0f);

    // Aplicar stress level
    Tension = FMath::Clamp(Tension + (CurrentAudioState.StressLevel * 0.2f), 0.0f, 1.0f);

    // Enviar parâmetros para o MetaSound
    MusicComponent->SetFloatParameter(FName("Tension"), Tension);
    MusicComponent->SetFloatParameter(FName("Danger"), Danger);
    MusicComponent->SetFloatParameter(FName("Exploration"), Exploration);
    MusicComponent->SetFloatParameter(FName("TimeOfDay"), CurrentAudioState.TimeOfDay / 24.0f);
    MusicComponent->SetBoolParameter(FName("IsRaining"), CurrentAudioState.bIsRaining);
}

void UAudioSystemCore::UpdateAmbientParameters()
{
    if (!AmbientComponent || !AmbientSoundscape)
        return;

    // Parâmetros para o soundscape ambiente
    float ForestDensity = 0.8f; // Base forest density
    float WildlifeActivity = 1.0f - (CurrentAudioState.StressLevel * 0.7f); // Animals get quiet when danger is near
    float WeatherIntensity = CurrentAudioState.bIsRaining ? 0.8f : 0.2f;

    // Reduzir atividade da vida selvagem baseado em ameaças próximas
    if (CurrentAudioState.ThreatLevel >= EThreatLevel::Medium)
    {
        WildlifeActivity *= 0.3f;
    }

    // Enviar parâmetros para o ambiente
    AmbientComponent->SetFloatParameter(FName("ForestDensity"), ForestDensity);
    AmbientComponent->SetFloatParameter(FName("WildlifeActivity"), WildlifeActivity);
    AmbientComponent->SetFloatParameter(FName("WeatherIntensity"), WeatherIntensity);
    AmbientComponent->SetFloatParameter(FName("TimeOfDay"), CurrentAudioState.TimeOfDay / 24.0f);
}

void UAudioSystemCore::CheckNearbyThreats()
{
    // Limpar dinossauros que não existem mais
    CurrentAudioState.NearbyDinosaurs.RemoveAll([](ADinosaur* Dino) {
        return !IsValid(Dino);
    });

    // Determinar nível de ameaça baseado nos dinossauros próximos
    EThreatLevel NewThreatLevel = EThreatLevel::None;
    
    for (ADinosaur* Dinosaur : CurrentAudioState.NearbyDinosaurs)
    {
        if (IsValid(Dinosaur))
        {
            // TODO: Implementar sistema de classificação de ameaça
            // Por agora, assumir que qualquer dinossauro próximo é ameaça média
            NewThreatLevel = FMath::Max(NewThreatLevel, EThreatLevel::Medium);
        }
    }

    UpdateThreatLevel(NewThreatLevel);
}