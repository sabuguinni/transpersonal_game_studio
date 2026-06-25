// AudioSystem.cpp
// Agent #16 — Audio Agent
// Prehistoric survival game adaptive audio system
// PROD_CYCLE_AUTO_20260625_011

#include "AudioSystem.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ============================================================
// UAudio_AdaptiveManager — Implementation
// ============================================================

UAudio_AdaptiveManager::UAudio_AdaptiveManager()
{
    PrimaryComponentTick.bCanEverTick = true;

    CurrentTimeOfDay = EAudio_TimeOfDay::Day;
    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    CurrentBiome = EAudio_BiomeType::Jungle;
    DinosaurProximityMeters = 9999.0f;
    bPlayerInDanger = false;

    MasterVolume = 1.0f;
    AmbientVolume = 0.7f;
    VoiceVolume = 1.0f;
    MusicVolume = 0.5f;
    CrossfadeDuration = 2.0f;

    CurrentCrossfadeTime = 0.0f;
    PreviousThreatLevel = EAudio_ThreatLevel::Safe;
    PreviousTimeOfDay = EAudio_TimeOfDay::Day;
}

void UAudio_AdaptiveManager::BeginPlay()
{
    Super::BeginPlay();
    InitialiseDefaultLayers();
    InitialiseDefaultVoiceLines();
}

void UAudio_AdaptiveManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Crossfade timer
    if (CurrentCrossfadeTime > 0.0f)
    {
        CurrentCrossfadeTime = FMath::Max(0.0f, CurrentCrossfadeTime - DeltaTime);
    }

    // Auto-update danger state from proximity
    if (DinosaurProximityMeters < 30.0f)
    {
        SetThreatLevel(EAudio_ThreatLevel::Critical);
        bPlayerInDanger = true;
    }
    else if (DinosaurProximityMeters < 80.0f)
    {
        SetThreatLevel(EAudio_ThreatLevel::Danger);
        bPlayerInDanger = true;
    }
    else if (DinosaurProximityMeters < 200.0f)
    {
        SetThreatLevel(EAudio_ThreatLevel::Cautious);
        bPlayerInDanger = false;
    }
    else
    {
        SetThreatLevel(EAudio_ThreatLevel::Safe);
        bPlayerInDanger = false;
    }
}

void UAudio_AdaptiveManager::SetTimeOfDay(EAudio_TimeOfDay NewTime)
{
    if (NewTime != CurrentTimeOfDay)
    {
        PreviousTimeOfDay = CurrentTimeOfDay;
        CurrentTimeOfDay = NewTime;
        CurrentCrossfadeTime = CrossfadeDuration;
    }
}

void UAudio_AdaptiveManager::SetThreatLevel(EAudio_ThreatLevel NewThreat)
{
    if (NewThreat != CurrentThreatLevel)
    {
        PreviousThreatLevel = CurrentThreatLevel;
        CurrentThreatLevel = NewThreat;
        CurrentCrossfadeTime = CrossfadeDuration;
    }
}

void UAudio_AdaptiveManager::SetBiome(EAudio_BiomeType NewBiome)
{
    CurrentBiome = NewBiome;
    CurrentCrossfadeTime = CrossfadeDuration;
}

void UAudio_AdaptiveManager::UpdateDinosaurProximity(float DistanceMeters)
{
    DinosaurProximityMeters = DistanceMeters;
}

void UAudio_AdaptiveManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

float UAudio_AdaptiveManager::GetAmbientVolumeForCurrentState() const
{
    float BaseVol = AmbientVolume * MasterVolume;
    float ThreatMult = ComputeThreatMultiplier();
    return BaseVol * ThreatMult;
}

FString UAudio_AdaptiveManager::GetCurrentStateDescription() const
{
    FString TimeStr;
    switch (CurrentTimeOfDay)
    {
        case EAudio_TimeOfDay::Dawn:    TimeStr = TEXT("Dawn");    break;
        case EAudio_TimeOfDay::Day:     TimeStr = TEXT("Day");     break;
        case EAudio_TimeOfDay::Dusk:    TimeStr = TEXT("Dusk");    break;
        case EAudio_TimeOfDay::Night:   TimeStr = TEXT("Night");   break;
        default:                        TimeStr = TEXT("Unknown"); break;
    }

    FString ThreatStr;
    switch (CurrentThreatLevel)
    {
        case EAudio_ThreatLevel::Safe:      ThreatStr = TEXT("Safe");     break;
        case EAudio_ThreatLevel::Cautious:  ThreatStr = TEXT("Cautious"); break;
        case EAudio_ThreatLevel::Danger:    ThreatStr = TEXT("Danger");   break;
        case EAudio_ThreatLevel::Critical:  ThreatStr = TEXT("Critical"); break;
        default:                            ThreatStr = TEXT("Unknown");  break;
    }

    return FString::Printf(TEXT("Time:%s | Threat:%s | DinoProximity:%.0fm | Danger:%s"),
        *TimeStr, *ThreatStr, DinosaurProximityMeters,
        bPlayerInDanger ? TEXT("YES") : TEXT("NO"));
}

void UAudio_AdaptiveManager::RegisterVoiceLine(const FAudio_VoiceLine& VoiceLine)
{
    VoiceLines.Add(VoiceLine);
}

void UAudio_AdaptiveManager::RegisterAmbientLayer(const FAudio_AmbientLayer& Layer)
{
    AmbientLayers.Add(Layer);
}

void UAudio_AdaptiveManager::InitialiseDefaultLayers()
{
    // Jungle day layer
    FAudio_AmbientLayer JungleDay;
    JungleDay.LayerName = TEXT("Jungle_Day_Insects");
    JungleDay.Biome = EAudio_BiomeType::Jungle;
    JungleDay.TimeOfDay = EAudio_TimeOfDay::Day;
    JungleDay.BaseVolume = 0.7f;
    JungleDay.AttenuationRadius = 5000.0f;
    JungleDay.bLooping = true;
    AmbientLayers.Add(JungleDay);

    // Jungle night layer
    FAudio_AmbientLayer JungleNight;
    JungleNight.LayerName = TEXT("Jungle_Night_Frogs");
    JungleNight.Biome = EAudio_BiomeType::Jungle;
    JungleNight.TimeOfDay = EAudio_TimeOfDay::Night;
    JungleNight.BaseVolume = 0.5f;
    JungleNight.AttenuationRadius = 4000.0f;
    JungleNight.bLooping = true;
    AmbientLayers.Add(JungleNight);

    // Wind layer — plains
    FAudio_AmbientLayer WindPlains;
    WindPlains.LayerName = TEXT("Plains_Wind");
    WindPlains.Biome = EAudio_BiomeType::Plains;
    WindPlains.TimeOfDay = EAudio_TimeOfDay::Day;
    WindPlains.BaseVolume = 0.4f;
    WindPlains.AttenuationRadius = 8000.0f;
    WindPlains.bLooping = true;
    AmbientLayers.Add(WindPlains);

    // Campfire layer
    FAudio_AmbientLayer Campfire;
    Campfire.LayerName = TEXT("Campfire_Crackle");
    Campfire.Biome = EAudio_BiomeType::Jungle;
    Campfire.TimeOfDay = EAudio_TimeOfDay::Night;
    Campfire.BaseVolume = 0.8f;
    Campfire.AttenuationRadius = 800.0f;
    Campfire.bLooping = true;
    AmbientLayers.Add(Campfire);
}

void UAudio_AdaptiveManager::InitialiseDefaultVoiceLines()
{
    // Survival Narrator — T-Rex warning
    FAudio_VoiceLine TRexWarning;
    TRexWarning.CharacterName = TEXT("Survival_Narrator");
    TRexWarning.LineText = TEXT("Warning. T-Rex detected two hundred meters north. Do not run. Freeze.");
    TRexWarning.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782405948376_Survival_Narrator.mp3");
    TRexWarning.Duration = 12.0f;
    TRexWarning.TriggerThreat = EAudio_ThreatLevel::Danger;
    VoiceLines.Add(TRexWarning);

    // Elder Kael — night warning
    FAudio_VoiceLine NightWarning;
    NightWarning.CharacterName = TEXT("Elder_Kael");
    NightWarning.LineText = TEXT("Night is falling. The predators wake now. Stay close to the fire.");
    NightWarning.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782405953821_Elder_Kael_Night.mp3");
    NightWarning.Duration = 9.0f;
    NightWarning.TriggerThreat = EAudio_ThreatLevel::Safe;
    VoiceLines.Add(NightWarning);

    // Scout Mira — raptor tactics (from Agent #15)
    FAudio_VoiceLine RaptorTactics;
    RaptorTactics.CharacterName = TEXT("Scout_Mira");
    RaptorTactics.LineText = TEXT("Stay low. The pack hunts together. They will circle from the east.");
    RaptorTactics.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782405794311_Scout_Mira.mp3");
    RaptorTactics.Duration = 18.0f;
    RaptorTactics.TriggerThreat = EAudio_ThreatLevel::Cautious;
    VoiceLines.Add(RaptorTactics);

    // Hunter Brak — triceratops tactics (from Agent #15)
    FAudio_VoiceLine TriTactics;
    TriTactics.CharacterName = TEXT("Hunter_Brak");
    TriTactics.LineText = TEXT("I tracked the Triceratops herd for three days. Wait for the stragglers.");
    TriTactics.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782405823525_Hunter_Brak.mp3");
    TriTactics.Duration = 19.0f;
    TriTactics.TriggerThreat = EAudio_ThreatLevel::Cautious;
    VoiceLines.Add(TriTactics);
}

float UAudio_AdaptiveManager::ComputeThreatMultiplier() const
{
    switch (CurrentThreatLevel)
    {
        case EAudio_ThreatLevel::Safe:      return 1.0f;
        case EAudio_ThreatLevel::Cautious:  return 0.8f;
        case EAudio_ThreatLevel::Danger:    return 0.5f;
        case EAudio_ThreatLevel::Critical:  return 0.2f;
        default:                            return 1.0f;
    }
}

// ============================================================
// AAudio_ProximityTrigger — Implementation
// ============================================================

AAudio_ProximityTrigger::AAudio_ProximityTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    RootComponent = TriggerSphere;
    TriggerSphere->SetSphereRadius(500.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    TriggerRadius = 500.0f;
    FadeInDuration = 1.5f;
    FadeOutDuration = 2.0f;
    bPlayOnce = false;
    AssociatedThreat = EAudio_ThreatLevel::Safe;
    AssociatedVoiceURL = TEXT("");

    bPlayerPresent = false;
    bHasPlayed = false;
}

void AAudio_ProximityTrigger::BeginPlay()
{
    Super::BeginPlay();

    TriggerSphere->SetSphereRadius(TriggerRadius);
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_ProximityTrigger::OnPlayerEnterRange);
    TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_ProximityTrigger::OnPlayerExitRange);
}

void AAudio_ProximityTrigger::OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    bPlayerPresent = true;

    if (bPlayOnce && bHasPlayed) return;

    ActivateTrigger();
    bHasPlayed = true;
}

void AAudio_ProximityTrigger::OnPlayerExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    bPlayerPresent = false;

    if (!bPlayOnce)
    {
        DeactivateTrigger();
    }
}

void AAudio_ProximityTrigger::ActivateTrigger()
{
    if (AudioComponent && AudioComponent->Sound)
    {
        AudioComponent->FadeIn(FadeInDuration, 1.0f);
    }
}

void AAudio_ProximityTrigger::DeactivateTrigger()
{
    if (AudioComponent)
    {
        AudioComponent->FadeOut(FadeOutDuration, 0.0f);
    }
}

bool AAudio_ProximityTrigger::IsPlayerInRange() const
{
    return bPlayerPresent;
}
