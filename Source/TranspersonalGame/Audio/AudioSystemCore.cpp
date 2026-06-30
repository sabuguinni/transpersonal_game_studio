// AudioSystemCore.cpp — Audio Agent #16
// PROD_CYCLE_AUTO_20260630_011
// Adaptive music, ambient layers, voice line registry, Freesound SFX catalog

#include "AudioSystemCore.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

// ============================================================
// Constructor
// ============================================================

AAudioSystemCore::AAudioSystemCore()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz — audio state polling

    // Root
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("AudioRoot"));

    // Audio components
    MusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComponent"));
    MusicComponent->SetupAttachment(RootComponent);
    MusicComponent->bAutoActivate = false;

    AmbientComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientComponent"));
    AmbientComponent->SetupAttachment(RootComponent);
    AmbientComponent->bAutoActivate = false;

    VoiceComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceComponent"));
    VoiceComponent->SetupAttachment(RootComponent);
    VoiceComponent->bAutoActivate = false;
}

// ============================================================
// BeginPlay
// ============================================================

void AAudioSystemCore::BeginPlay()
{
    Super::BeginPlay();

    // Populate registries on start
    PopulateVoiceLineRegistry();
    PopulateFreesoundCatalog();

    // Set initial music state
    CurrentMusicState.TimeOfDay = EAudio_TimeOfDay::Day;
    CurrentMusicState.ThreatLevel = EAudio_ThreatLevel::None;
    CurrentMusicState.CurrentBiome = EAudio_BiomeType::OpenPlains;
    CurrentMusicState.PlayerHealthNormalized = 1.0f;
    CurrentMusicState.bPlayerInCombat = false;
    CurrentMusicState.bNearCampfire = false;

    ThreatIntensityTarget = 0.0f;
    ThreatMusicIntensity = 0.0f;
}

// ============================================================
// Tick
// ============================================================

void AAudioSystemCore::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TickAdaptiveMusic(DeltaTime);
}

// ============================================================
// Adaptive Music
// ============================================================

void AAudioSystemCore::TickAdaptiveMusic(float DeltaTime)
{
    // Smoothly interpolate threat intensity toward target
    float InterpSpeed = MusicTransitionSpeed * DeltaTime;
    ThreatMusicIntensity = FMath::FInterpTo(
        ThreatMusicIntensity,
        ThreatIntensityTarget,
        DeltaTime,
        MusicTransitionSpeed
    );

    // Apply transitions when intensity changes significantly
    TickAccumulator += DeltaTime;
    if (TickAccumulator >= 1.0f)
    {
        TickAccumulator = 0.0f;
        ApplyMusicTransition();
    }
}

void AAudioSystemCore::ApplyMusicTransition()
{
    // Music volume driven by threat intensity
    if (MusicComponent && MusicComponent->IsPlaying())
    {
        float TargetVolume = FMath::Lerp(0.3f, 1.0f, ThreatMusicIntensity);
        MusicComponent->SetVolumeMultiplier(TargetVolume);
    }
}

// ============================================================
// State setters
// ============================================================

void AAudioSystemCore::UpdateMusicState(const FAudio_MusicState& NewState)
{
    CurrentMusicState = NewState;

    // Recalculate threat intensity target
    switch (NewState.ThreatLevel)
    {
        case EAudio_ThreatLevel::None:        ThreatIntensityTarget = 0.0f; break;
        case EAudio_ThreatLevel::Distant:     ThreatIntensityTarget = 0.2f; break;
        case EAudio_ThreatLevel::Approaching: ThreatIntensityTarget = 0.5f; break;
        case EAudio_ThreatLevel::Immediate:   ThreatIntensityTarget = 0.8f; break;
        case EAudio_ThreatLevel::Combat:      ThreatIntensityTarget = 1.0f; break;
        default:                              ThreatIntensityTarget = 0.0f; break;
    }

    // Low health amplifies threat music
    if (NewState.PlayerHealthNormalized < 0.25f)
    {
        ThreatIntensityTarget = FMath::Min(ThreatIntensityTarget + 0.3f, 1.0f);
    }
}

void AAudioSystemCore::SetThreatLevel(EAudio_ThreatLevel NewThreat)
{
    CurrentMusicState.ThreatLevel = NewThreat;
    FAudio_MusicState Updated = CurrentMusicState;
    Updated.ThreatLevel = NewThreat;
    UpdateMusicState(Updated);
}

void AAudioSystemCore::SetTimeOfDay(EAudio_TimeOfDay NewTime)
{
    CurrentMusicState.TimeOfDay = NewTime;
}

void AAudioSystemCore::SetBiome(EAudio_BiomeType NewBiome)
{
    CurrentMusicState.CurrentBiome = NewBiome;
}

// ============================================================
// Voice Lines
// ============================================================

void AAudioSystemCore::PlayVoiceLine(EAudio_VoiceLineID LineID)
{
    for (FAudio_VoiceLine& Line : VoiceLineRegistry)
    {
        if (Line.LineID == LineID && !Line.bHasBeenPlayed)
        {
            // Mark as played
            Line.bHasBeenPlayed = true;
            UE_LOG(LogTemp, Log, TEXT("AudioSystemCore: Playing voice line [%s] — URL: %s"),
                *Line.TranscriptText.Left(40), *Line.AudioURL);
            // In production: load audio from URL and play via VoiceComponent
            return;
        }
    }
}

void AAudioSystemCore::RegisterVoiceLine(const FAudio_VoiceLine& VoiceLine)
{
    // Remove existing entry with same ID
    VoiceLineRegistry.RemoveAll([&](const FAudio_VoiceLine& L) {
        return L.LineID == VoiceLine.LineID;
    });
    VoiceLineRegistry.Add(VoiceLine);
}

// ============================================================
// Freesound SFX
// ============================================================

void AAudioSystemCore::RegisterFreesoundRef(const FAudio_FreesoundRef& SFXRef)
{
    FreesoundCatalog.RemoveAll([&](const FAudio_FreesoundRef& R) {
        return R.FreesoundID == SFXRef.FreesoundID;
    });
    FreesoundCatalog.Add(SFXRef);
}

// ============================================================
// Query
// ============================================================

float AAudioSystemCore::GetCurrentThreatIntensity() const
{
    return ThreatMusicIntensity;
}

FString AAudioSystemCore::GetCurrentAudioStateDebugString() const
{
    FString TimeStr;
    switch (CurrentMusicState.TimeOfDay)
    {
        case EAudio_TimeOfDay::Dawn:  TimeStr = TEXT("Dawn");  break;
        case EAudio_TimeOfDay::Day:   TimeStr = TEXT("Day");   break;
        case EAudio_TimeOfDay::Dusk:  TimeStr = TEXT("Dusk");  break;
        case EAudio_TimeOfDay::Night: TimeStr = TEXT("Night"); break;
        default:                      TimeStr = TEXT("Unknown"); break;
    }

    FString ThreatStr;
    switch (CurrentMusicState.ThreatLevel)
    {
        case EAudio_ThreatLevel::None:        ThreatStr = TEXT("None");        break;
        case EAudio_ThreatLevel::Distant:     ThreatStr = TEXT("Distant");     break;
        case EAudio_ThreatLevel::Approaching: ThreatStr = TEXT("Approaching"); break;
        case EAudio_ThreatLevel::Immediate:   ThreatStr = TEXT("Immediate");   break;
        case EAudio_ThreatLevel::Combat:      ThreatStr = TEXT("Combat");      break;
        default:                              ThreatStr = TEXT("Unknown");     break;
    }

    return FString::Printf(
        TEXT("AudioState[Time=%s | Threat=%s | Intensity=%.2f | Health=%.0f%% | Combat=%s | Campfire=%s | VoiceLines=%d | SFXRefs=%d]"),
        *TimeStr,
        *ThreatStr,
        ThreatMusicIntensity,
        CurrentMusicState.PlayerHealthNormalized * 100.0f,
        CurrentMusicState.bPlayerInCombat ? TEXT("YES") : TEXT("NO"),
        CurrentMusicState.bNearCampfire ? TEXT("YES") : TEXT("NO"),
        VoiceLineRegistry.Num(),
        FreesoundCatalog.Num()
    );
}

// ============================================================
// Registry population — ElevenLabs TTS URLs from this cycle
// ============================================================

void AAudioSystemCore::PopulateVoiceLineRegistry()
{
    VoiceLineRegistry.Empty();

    // --- TRex Warning (PROD_CYCLE_AUTO_20260630_011) ---
    {
        FAudio_VoiceLine Line;
        Line.LineID = EAudio_VoiceLineID::TRexWarning;
        Line.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782830155647_SurvivalNarrator_TRexWarning.mp3");
        Line.TranscriptText = TEXT("The T-Rex is close. You can hear it before you see it — the ground trembles, the birds go silent, and then there is that sound. Low. Deep. Like thunder that does not stop. Run. Do not look back. Run.");
        Line.DurationSeconds = 14.0f;
        Line.bHasBeenPlayed = false;
        VoiceLineRegistry.Add(Line);
    }

    // --- Dawn Ambience (PROD_CYCLE_AUTO_20260630_011) ---
    {
        FAudio_VoiceLine Line;
        Line.LineID = EAudio_VoiceLineID::DawnAmbience;
        Line.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782830174723_SurvivalNarrator_DawnAmbience.mp3");
        Line.TranscriptText = TEXT("Dawn. The jungle wakes in layers. First the insects stop. Then the birds start — one species at a time, each one a signal. By the time the sun clears the treeline, you know if last night was safe or not. Today it was not.");
        Line.DurationSeconds = 15.0f;
        Line.bHasBeenPlayed = false;
        VoiceLineRegistry.Add(Line);
    }

    // --- Previous cycle voice lines (PROD_CYCLE_AUTO_20260630_010) ---
    {
        FAudio_VoiceLine Line;
        Line.LineID = EAudio_VoiceLineID::HerdTracker;
        Line.TranscriptText = TEXT("Night is falling. The jungle goes quiet first — that is how you know something large is moving.");
        Line.DurationSeconds = 8.0f;
        Line.bHasBeenPlayed = false;
        VoiceLineRegistry.Add(Line);
    }

    UE_LOG(LogTemp, Log, TEXT("AudioSystemCore: Populated %d voice lines"), VoiceLineRegistry.Num());
}

// ============================================================
// Freesound catalog — campfire SFX from this cycle search
// ============================================================

void AAudioSystemCore::PopulateFreesoundCatalog()
{
    FreesoundCatalog.Empty();

    // Campfire (Position 1) — ID 681366
    {
        FAudio_FreesoundRef Ref;
        Ref.FreesoundID = 681366;
        Ref.SoundName = TEXT("Campfire (Position 1)");
        Ref.PreviewURL = TEXT("https://cdn.freesound.org/previews/681/681366_5752443-hq.mp3");
        Ref.DurationSeconds = 83.6f;
        Ref.UsageContext = TEXT("Campfire ambient loop — player shelter at night");
        FreesoundCatalog.Add(Ref);
    }

    // Campfire (Position 2) — ID 681367
    {
        FAudio_FreesoundRef Ref;
        Ref.FreesoundID = 681367;
        Ref.SoundName = TEXT("Campfire (Position 2)");
        Ref.PreviewURL = TEXT("https://cdn.freesound.org/previews/681/681367_5752443-hq.mp3");
        Ref.DurationSeconds = 22.1f;
        Ref.UsageContext = TEXT("Campfire short loop — crafting station");
        FreesoundCatalog.Add(Ref);
    }

    // Camp Fire Ambience — ID 708328
    {
        FAudio_FreesoundRef Ref;
        Ref.FreesoundID = 708328;
        Ref.SoundName = TEXT("Camp Fire Ambience");
        Ref.PreviewURL = TEXT("https://cdn.freesound.org/previews/708/708328_14714459-hq.mp3");
        Ref.DurationSeconds = 166.1f;
        Ref.UsageContext = TEXT("Extended campfire with wind and bird — open camp scene");
        FreesoundCatalog.Add(Ref);
    }

    // Spring Forest Campfire — ID 819666
    {
        FAudio_FreesoundRef Ref;
        Ref.FreesoundID = 819666;
        Ref.SoundName = TEXT("Spring Forest Campfire");
        Ref.PreviewURL = TEXT("https://cdn.freesound.org/previews/819/819666_12625353-hq.mp3");
        Ref.DurationSeconds = 213.0f;
        Ref.UsageContext = TEXT("Forest campfire with birdsong — dawn/dusk transition");
        FreesoundCatalog.Add(Ref);
    }

    UE_LOG(LogTemp, Log, TEXT("AudioSystemCore: Populated %d Freesound SFX references"), FreesoundCatalog.Num());
}
