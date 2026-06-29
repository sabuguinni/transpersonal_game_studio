#include "AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// UAudio_ProximityComponent
// ============================================================

UAudio_ProximityComponent::UAudio_ProximityComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UAudio_ProximityComponent::BeginPlay()
{
    Super::BeginPlay();

    if (ProximitySound)
    {
        AudioComp = UGameplayStatics::SpawnSoundAttached(
            ProximitySound,
            GetOwner()->GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            EAttachLocation::KeepRelativeOffset,
            false,
            0.0f  // Start silent — volume driven by proximity
        );

        if (AudioComp && ProximitySettings.bLooping)
        {
            AudioComp->SetVolumeMultiplier(0.0f);
        }
    }
}

void UAudio_ProximityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!AudioComp) return;

    // Find player pawn
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
    UpdateProximityVolume(Distance);
}

void UAudio_ProximityComponent::UpdateProximityVolume(float DistanceToPlayer)
{
    if (!AudioComp) return;

    float TargetVolume = 0.0f;

    if (DistanceToPlayer <= ProximitySettings.TriggerRadius)
    {
        // Linear falloff from max volume at 0 distance to 0 at trigger radius
        float Alpha = 1.0f - (DistanceToPlayer / ProximitySettings.TriggerRadius);
        TargetVolume = Alpha * ProximitySettings.MaxVolume;
        bPlayerInRange = true;
    }
    else
    {
        bPlayerInRange = false;
    }

    // Smooth volume transition
    float FadeSpeed = bPlayerInRange ? (1.0f / ProximitySettings.FadeInTime) : (1.0f / ProximitySettings.FadeOutTime);
    CurrentVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, GetWorld()->GetDeltaSeconds(), FadeSpeed);
    AudioComp->SetVolumeMultiplier(CurrentVolume);
}

void UAudio_ProximityComponent::SetProximitySound(USoundBase* NewSound)
{
    ProximitySound = NewSound;
    if (AudioComp)
    {
        AudioComp->SetSound(NewSound);
    }
}

// ============================================================
// AAudio_SystemManager
// ============================================================

AAudio_SystemManager::AAudio_SystemManager()
{
    PrimaryActorTick.bCanEverTick = true;

    MusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComponent"));
    MusicComponent->SetupAttachment(RootComponent);
    MusicComponent->bAutoActivate = false;

    AmbientComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientComponent"));
    AmbientComponent->SetupAttachment(RootComponent);
    AmbientComponent->bAutoActivate = false;
}

void AAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultVoiceLines();
    InitializeDefaultAmbientLayers();

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Initialized. ThreatLevel=%d, TimeOfDay=%d, Biome=%d"),
        (int32)CurrentThreatLevel, (int32)CurrentTimeOfDay, (int32)CurrentBiome);
}

void AAudio_SystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateMusicState(DeltaTime);
}

void AAudio_SystemManager::InitializeDefaultVoiceLines()
{
    // Narrator_Survival — "The valley is silent. That is when you run."
    FAudio_VoiceLine NarratorLine;
    NarratorLine.CharacterName = TEXT("Narrator_Survival");
    NarratorLine.DialogueText = TEXT("The valley is silent. That is when you run. When the insects stop, when the birds scatter — something is hunting. You have thirty seconds before it finds you.");
    NarratorLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782705423321_Narrator_Survival.mp3");
    NarratorLine.TriggerCondition = TEXT("OnDinoDetectedNearby");
    NarratorLine.PlaybackVolume = 1.0f;
    NarratorLine.bSubtitlesEnabled = true;
    RegisteredVoiceLines.Add(NarratorLine);

    // Elder_Voss — "Fire keeps them back."
    FAudio_VoiceLine ElderLine;
    ElderLine.CharacterName = TEXT("Elder_Voss");
    ElderLine.DialogueText = TEXT("Fire keeps them back. Not all of them — but enough. Build it high, keep it burning, and never let it die before dawn.");
    ElderLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782705429713_Elder_Voss.mp3");
    ElderLine.TriggerCondition = TEXT("OnFirstNightfall");
    ElderLine.PlaybackVolume = 1.0f;
    ElderLine.bSubtitlesEnabled = true;
    RegisteredVoiceLines.Add(ElderLine);

    // Hunter_Kael — "The raptors hunt in threes."
    FAudio_VoiceLine KaelLine;
    KaelLine.CharacterName = TEXT("Hunter_Kael");
    KaelLine.DialogueText = TEXT("The raptors hunt in threes — one drives you toward the others. If you see one, stop moving. Look for the other two.");
    KaelLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782705276063_Hunter_Kael.mp3");
    KaelLine.TriggerCondition = TEXT("OnEnterRaptorTerritory");
    KaelLine.PlaybackVolume = 1.0f;
    KaelLine.bSubtitlesEnabled = true;
    RegisteredVoiceLines.Add(KaelLine);

    // Scout_Mira — "The river bends east."
    FAudio_VoiceLine MiraLine;
    MiraLine.CharacterName = TEXT("Scout_Mira");
    MiraLine.DialogueText = TEXT("The river bends east past the dead forest. Follow it two days and you reach the stone ridge. Our camp is there.");
    MiraLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782705278191_Scout_Mira.mp3");
    MiraLine.TriggerCondition = TEXT("OnQuestWaypointReached");
    MiraLine.PlaybackVolume = 1.0f;
    MiraLine.bSubtitlesEnabled = true;
    RegisteredVoiceLines.Add(MiraLine);

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Registered %d voice lines"), RegisteredVoiceLines.Num());
}

void AAudio_SystemManager::InitializeDefaultAmbientLayers()
{
    // Day — Open Plains
    FAudio_AmbientLayer DayPlains;
    DayPlains.Biome = EAudio_BiomeType::OpenPlains;
    DayPlains.TimeOfDay = EAudio_TimeOfDay::Day;
    DayPlains.BaseVolume = 0.5f;
    DayPlains.CrossfadeDuration = 3.0f;
    DayPlains.FreesoundPreviewURL = TEXT("https://cdn.freesound.org/previews/688/688994_13721094-hq.mp3");
    DayPlains.SoundAssetPath = TEXT("/Game/Audio/Ambient/Plains_Day");
    AmbientLayers.Add(DayPlains);

    // Night — Campfire ambience (Freesound 681366)
    FAudio_AmbientLayer NightCampfire;
    NightCampfire.Biome = EAudio_BiomeType::OpenPlains;
    NightCampfire.TimeOfDay = EAudio_TimeOfDay::Night;
    NightCampfire.BaseVolume = 0.7f;
    NightCampfire.CrossfadeDuration = 5.0f;
    NightCampfire.FreesoundPreviewURL = TEXT("https://cdn.freesound.org/previews/681/681366_5752443-hq.mp3");
    NightCampfire.SoundAssetPath = TEXT("/Game/Audio/Ambient/Campfire_Night");
    AmbientLayers.Add(NightCampfire);

    // Dense Forest — Day
    FAudio_AmbientLayer ForestDay;
    ForestDay.Biome = EAudio_BiomeType::DenseForest;
    ForestDay.TimeOfDay = EAudio_TimeOfDay::Day;
    ForestDay.BaseVolume = 0.6f;
    ForestDay.CrossfadeDuration = 4.0f;
    ForestDay.FreesoundPreviewURL = TEXT("https://cdn.freesound.org/previews/501/501731_9573665-hq.mp3");
    ForestDay.SoundAssetPath = TEXT("/Game/Audio/Ambient/Forest_Day");
    AmbientLayers.Add(ForestDay);

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Registered %d ambient layers"), AmbientLayers.Num());
}

void AAudio_SystemManager::SetThreatLevel(EAudio_ThreatLevel NewLevel)
{
    if (NewLevel == CurrentThreatLevel) return;

    PendingThreatLevel = NewLevel;
    bTransitioningThreat = true;
    ThreatTransitionTimer = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: ThreatLevel transition %d -> %d"),
        (int32)CurrentThreatLevel, (int32)NewLevel);
}

void AAudio_SystemManager::SetTimeOfDay(EAudio_TimeOfDay NewTime)
{
    if (NewTime == CurrentTimeOfDay) return;
    CurrentTimeOfDay = NewTime;
    CrossfadeToAmbientLayer(CurrentBiome, CurrentTimeOfDay);
}

void AAudio_SystemManager::SetBiome(EAudio_BiomeType NewBiome)
{
    if (NewBiome == CurrentBiome) return;
    CurrentBiome = NewBiome;
    CrossfadeToAmbientLayer(CurrentBiome, CurrentTimeOfDay);
}

void AAudio_SystemManager::PlayVoiceLine(const FString& CharacterName, const FString& TriggerCondition)
{
    for (const FAudio_VoiceLine& Line : RegisteredVoiceLines)
    {
        if (Line.CharacterName == CharacterName && Line.TriggerCondition == TriggerCondition)
        {
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing voice line — %s: \"%s\""),
                *Line.CharacterName, *Line.DialogueText);
            // In production: load from Line.AudioURL and play via AudioComponent
            // For now: log the subtitle text
            if (Line.bSubtitlesEnabled)
            {
                UE_LOG(LogTemp, Log, TEXT("SUBTITLE [%s]: %s"), *Line.CharacterName, *Line.DialogueText);
            }
            return;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: No voice line found for %s / %s"), *CharacterName, *TriggerCondition);
}

void AAudio_SystemManager::RegisterVoiceLine(const FAudio_VoiceLine& VoiceLine)
{
    RegisteredVoiceLines.Add(VoiceLine);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Registered new voice line for %s"), *VoiceLine.CharacterName);
}

void AAudio_SystemManager::CrossfadeToAmbientLayer(EAudio_BiomeType Biome, EAudio_TimeOfDay Time)
{
    for (const FAudio_AmbientLayer& Layer : AmbientLayers)
    {
        if (Layer.Biome == Biome && Layer.TimeOfDay == Time)
        {
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Crossfading to ambient layer — Biome=%d, Time=%d, Volume=%.2f"),
                (int32)Biome, (int32)Time, Layer.BaseVolume);

            if (AmbientComponent)
            {
                AmbientComponent->SetVolumeMultiplier(Layer.BaseVolume);
            }
            return;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: No ambient layer for Biome=%d, Time=%d"), (int32)Biome, (int32)Time);
}

void AAudio_SystemManager::UpdateMusicState(float DeltaTime)
{
    if (!bTransitioningThreat) return;

    ThreatTransitionTimer += DeltaTime;

    if (ThreatTransitionTimer >= ThreatEscalationDelay)
    {
        CurrentThreatLevel = PendingThreatLevel;
        bTransitioningThreat = false;

        // Drive music intensity based on threat level
        float MusicVolume = 0.0f;
        float MusicPitch = 1.0f;

        switch (CurrentThreatLevel)
        {
        case EAudio_ThreatLevel::Calm:   MusicVolume = 0.3f; MusicPitch = 1.0f;  break;
        case EAudio_ThreatLevel::Aware:  MusicVolume = 0.5f; MusicPitch = 1.05f; break;
        case EAudio_ThreatLevel::Tense:  MusicVolume = 0.7f; MusicPitch = 1.1f;  break;
        case EAudio_ThreatLevel::Chase:  MusicVolume = 0.9f; MusicPitch = 1.2f;  break;
        case EAudio_ThreatLevel::Combat: MusicVolume = 1.0f; MusicPitch = 1.3f;  break;
        case EAudio_ThreatLevel::Safe:   MusicVolume = 0.2f; MusicPitch = 0.95f; break;
        }

        if (MusicComponent)
        {
            MusicComponent->SetVolumeMultiplier(MusicVolume);
            MusicComponent->SetPitchMultiplier(MusicPitch);
        }

        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Music updated — ThreatLevel=%d, Vol=%.2f, Pitch=%.2f"),
            (int32)CurrentThreatLevel, MusicVolume, MusicPitch);
    }
}

void AAudio_SystemManager::LogAudioState() const
{
    UE_LOG(LogTemp, Log, TEXT("=== AudioSystemManager State ==="));
    UE_LOG(LogTemp, Log, TEXT("  ThreatLevel: %d"), (int32)CurrentThreatLevel);
    UE_LOG(LogTemp, Log, TEXT("  TimeOfDay:   %d"), (int32)CurrentTimeOfDay);
    UE_LOG(LogTemp, Log, TEXT("  Biome:       %d"), (int32)CurrentBiome);
    UE_LOG(LogTemp, Log, TEXT("  VoiceLines:  %d registered"), RegisteredVoiceLines.Num());
    UE_LOG(LogTemp, Log, TEXT("  AmbientLayers: %d registered"), AmbientLayers.Num());
    UE_LOG(LogTemp, Log, TEXT("================================"));
}
