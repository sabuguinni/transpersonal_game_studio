// DinosaurAudioManager.cpp
// Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260629_004
// Adaptive audio system for prehistoric survival game.
// Walter Murch principle: the silence before the roar is more powerful than the roar itself.

#include "DinosaurAudioManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Components/AudioComponent.h"

ADinosaurAudioManager::ADinosaurAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;

    PrimaryAmbientComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("PrimaryAmbientComponent"));
    PrimaryAmbientComponent->SetupAttachment(RootComponent);
    PrimaryAmbientComponent->bAutoActivate = false;

    DangerStingerComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DangerStingerComponent"));
    DangerStingerComponent->SetupAttachment(RootComponent);
    DangerStingerComponent->bAutoActivate = false;

    // Default dinosaur profiles
    FAudio_DinosaurSoundProfile TRexProfile;
    TRexProfile.SpeciesName = FName("TRex");
    TRexProfile.IdleVocalFrequency = 20.0f;
    TRexProfile.AlertVocalFrequency = 8.0f;
    TRexProfile.FootstepWeight = 3.0f;
    TRexProfile.RoarRadius = 5000.0f;
    TRexProfile.bCausesScreenShake = true;
    DinosaurProfiles.Add(TRexProfile);

    FAudio_DinosaurSoundProfile RaptorProfile;
    RaptorProfile.SpeciesName = FName("Raptor");
    RaptorProfile.IdleVocalFrequency = 10.0f;
    RaptorProfile.AlertVocalFrequency = 3.0f;
    RaptorProfile.FootstepWeight = 0.5f;
    RaptorProfile.RoarRadius = 1500.0f;
    RaptorProfile.bCausesScreenShake = false;
    DinosaurProfiles.Add(RaptorProfile);

    FAudio_DinosaurSoundProfile BrachioProfile;
    BrachioProfile.SpeciesName = FName("Brachiosaurus");
    BrachioProfile.IdleVocalFrequency = 30.0f;
    BrachioProfile.AlertVocalFrequency = 15.0f;
    BrachioProfile.FootstepWeight = 2.5f;
    BrachioProfile.RoarRadius = 4000.0f;
    BrachioProfile.bCausesScreenShake = true;
    DinosaurProfiles.Add(BrachioProfile);

    // Default ambient zone configs
    FAudio_AmbientZoneConfig PlainsConfig;
    PlainsConfig.BiomeZone = EAudio_BiomeZone::OpenPlains;
    PlainsConfig.DayVolume = 0.6f;
    PlainsConfig.NightVolume = 0.5f;
    PlainsConfig.DangerMultiplier = 0.2f;
    AmbientZoneConfigs.Add(PlainsConfig);

    FAudio_AmbientZoneConfig ForestConfig;
    ForestConfig.BiomeZone = EAudio_BiomeZone::DenseForest;
    ForestConfig.DayVolume = 0.9f;
    ForestConfig.NightVolume = 1.0f;
    ForestConfig.DangerMultiplier = 0.4f;
    AmbientZoneConfigs.Add(ForestConfig);

    FAudio_AmbientZoneConfig RiverConfig;
    RiverConfig.BiomeZone = EAudio_BiomeZone::Riverbank;
    RiverConfig.DayVolume = 0.8f;
    RiverConfig.NightVolume = 0.7f;
    RiverConfig.DangerMultiplier = 0.3f;
    AmbientZoneConfigs.Add(RiverConfig);
}

void ADinosaurAudioManager::BeginPlay()
{
    Super::BeginPlay();

    if (PrimaryAmbientComponent)
    {
        PrimaryAmbientComponent->SetVolumeMultiplier(0.7f);
        PrimaryAmbientComponent->Play();
    }
}

void ADinosaurAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateAmbientVolumes(DeltaTime);
    TickSilenceEvent(DeltaTime);
}

void ADinosaurAudioManager::TriggerDinosaurRoar(AActor* DinosaurActor, FName SpeciesName, float Intensity)
{
    if (!DinosaurActor) return;

    FAudio_DinosaurSoundProfile* Profile = FindProfileForSpecies(SpeciesName);
    if (!Profile) return;

    // Trigger silence event before the roar — Walter Murch principle
    // The silence makes the roar hit harder
    TriggerSilenceEvent(0.8f, Profile->RoarRadius * 0.5f);

    // Screen shake for heavy dinosaurs
    if (Profile->bCausesScreenShake)
    {
        TriggerFootstepScreenShake(Intensity * Profile->FootstepWeight, DinosaurActor->GetActorLocation());
    }

    // Increase danger level on roar
    CurrentDangerLevel = FMath::Clamp(CurrentDangerLevel + (Intensity * 0.4f), 0.0f, 1.0f);

    UE_LOG(LogTemp, Log, TEXT("DinosaurAudioManager: %s roar triggered at intensity %.2f, danger now %.2f"),
        *SpeciesName.ToString(), Intensity, CurrentDangerLevel);
}

void ADinosaurAudioManager::UpdateDinosaurState(AActor* DinosaurActor, EAudio_DinosaurState NewState)
{
    if (!DinosaurActor) return;

    switch (NewState)
    {
        case EAudio_DinosaurState::Hunting:
            // Danger level spikes when a dinosaur starts hunting
            CurrentDangerLevel = FMath::Clamp(CurrentDangerLevel + 0.3f, 0.0f, 1.0f);
            break;

        case EAudio_DinosaurState::Idle:
            // Danger slowly decreases when dinosaur is idle
            CurrentDangerLevel = FMath::Clamp(CurrentDangerLevel - 0.1f, 0.0f, 1.0f);
            break;

        case EAudio_DinosaurState::Fleeing:
            // Fleeing dinosaur = something bigger nearby — danger spikes
            CurrentDangerLevel = FMath::Clamp(CurrentDangerLevel + 0.5f, 0.0f, 1.0f);
            break;

        case EAudio_DinosaurState::Feeding:
            // Feeding = distracted, danger drops slightly
            CurrentDangerLevel = FMath::Clamp(CurrentDangerLevel - 0.05f, 0.0f, 1.0f);
            break;

        default:
            break;
    }
}

void ADinosaurAudioManager::PlayFootstepImpact(AActor* DinosaurActor, float FootWeight, FVector ImpactLocation)
{
    if (!DinosaurActor) return;

    // Heavy footsteps trigger screen shake proportional to weight and distance to player
    if (FootWeight > 1.5f)
    {
        TriggerFootstepScreenShake(FootWeight, ImpactLocation);
    }
}

void ADinosaurAudioManager::TriggerSilenceEvent(float SilenceDuration, float RadiusMeters)
{
    // Silence event: mute ambient sounds to create tension before a major audio event
    bInSilenceEvent = true;
    SilenceTimer = SilenceDuration;

    if (PrimaryAmbientComponent && PrimaryAmbientComponent->IsPlaying())
    {
        PrimaryAmbientComponent->SetVolumeMultiplier(0.0f);
    }

    UE_LOG(LogTemp, Log, TEXT("DinosaurAudioManager: Silence event triggered for %.1fs radius %.0fm"),
        SilenceDuration, RadiusMeters);
}

void ADinosaurAudioManager::SetPlayerBiomeZone(EAudio_BiomeZone NewZone)
{
    if (CurrentBiomeZone == NewZone) return;

    CurrentBiomeZone = NewZone;

    UE_LOG(LogTemp, Log, TEXT("DinosaurAudioManager: Player entered biome zone %d"),
        static_cast<int32>(NewZone));
}

void ADinosaurAudioManager::SetDangerLevel(float DangerLevel01)
{
    CurrentDangerLevel = FMath::Clamp(DangerLevel01, 0.0f, 1.0f);
}

void ADinosaurAudioManager::SetTimeOfDay(float TimeNormalized01)
{
    CurrentTimeOfDay = FMath::Clamp(TimeNormalized01, 0.0f, 1.0f);
}

void ADinosaurAudioManager::TriggerFootstepScreenShake(float Intensity, FVector SourceLocation)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    // Scale shake by distance from player to source
    APawn* PlayerPawn = PC->GetPawn();
    if (PlayerPawn)
    {
        float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), SourceLocation);
        float DistanceFactor = FMath::Clamp(1.0f - (Distance / MaxRoarScreenShakeRadius), 0.0f, 1.0f);
        float FinalIntensity = Intensity * DistanceFactor * GlobalAudioScale;

        if (FinalIntensity > 0.1f)
        {
            PC->ClientStartCameraShake(UCameraShakeBase::StaticClass(), FinalIntensity);
            UE_LOG(LogTemp, Log, TEXT("DinosaurAudioManager: Screen shake intensity %.2f at distance %.0fm"),
                FinalIntensity, Distance);
        }
    }
}

void ADinosaurAudioManager::TriggerDamageAudioFeedback(float DamageAmount)
{
    // Damage audio feedback: brief silence then pain sound
    // The silence is the moment of shock before the pain registers
    float SilenceDuration = FMath::Clamp(DamageAmount / 100.0f * 0.3f, 0.05f, 0.3f);
    TriggerSilenceEvent(SilenceDuration, 500.0f);

    UE_LOG(LogTemp, Log, TEXT("DinosaurAudioManager: Damage audio feedback for %.1f damage"),
        DamageAmount);
}

void ADinosaurAudioManager::UpdateAmbientVolumes(float DeltaTime)
{
    if (bInSilenceEvent) return;

    // Find config for current biome
    FAudio_AmbientZoneConfig* ZoneConfig = nullptr;
    for (FAudio_AmbientZoneConfig& Config : AmbientZoneConfigs)
    {
        if (Config.BiomeZone == CurrentBiomeZone)
        {
            ZoneConfig = &Config;
            break;
        }
    }

    if (!ZoneConfig) return;

    // Blend between day and night volume based on time of day
    // CurrentTimeOfDay: 0=midnight, 0.5=noon, 1=midnight
    float DayFactor = FMath::Sin(CurrentTimeOfDay * PI);
    float BaseVolume = FMath::Lerp(ZoneConfig->NightVolume, ZoneConfig->DayVolume, DayFactor);

    // Danger reduces ambient volume (world goes quiet when predators are near)
    float DangerReduction = CurrentDangerLevel * ZoneConfig->DangerMultiplier;
    float FinalVolume = FMath::Clamp((BaseVolume - DangerReduction) * GlobalAudioScale, 0.0f, 1.0f);

    if (PrimaryAmbientComponent)
    {
        float CurrentVol = PrimaryAmbientComponent->VolumeMultiplier;
        float NewVol = FMath::FInterpTo(CurrentVol, FinalVolume, DeltaTime, ZoneConfig->TensionFadeSpeed);
        PrimaryAmbientComponent->SetVolumeMultiplier(NewVol);
    }
}

void ADinosaurAudioManager::TickSilenceEvent(float DeltaTime)
{
    if (!bInSilenceEvent) return;

    SilenceTimer -= DeltaTime;
    if (SilenceTimer <= 0.0f)
    {
        bInSilenceEvent = false;
        SilenceTimer = 0.0f;
        UE_LOG(LogTemp, Log, TEXT("DinosaurAudioManager: Silence event ended — ambient audio restoring"));
    }
}

FAudio_DinosaurSoundProfile* ADinosaurAudioManager::FindProfileForSpecies(FName SpeciesName)
{
    for (FAudio_DinosaurSoundProfile& Profile : DinosaurProfiles)
    {
        if (Profile.SpeciesName == SpeciesName)
        {
            return &Profile;
        }
    }
    return nullptr;
}
