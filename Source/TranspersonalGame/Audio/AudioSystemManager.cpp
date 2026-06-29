#include "AudioSystemManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ============================================================
// Audio Agent #16 — AudioSystemManager.cpp
// Adaptive audio system for prehistoric survival game
// ============================================================

AAudio_SystemManager::AAudio_SystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Tick every 0.5s — audio doesn't need per-frame updates

    // Create ambient audio component
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetRelativeLocation(FVector::ZeroVector);
    RootComponent = AmbientAudioComponent;

    // Pre-populate SFX registry with known IDs
    // Campfire sounds (Freesound IDs: 681366, 681367, 688992)
    FAudio_SFXEntry CampfireEntry;
    CampfireEntry.SFXId = FName("Campfire");
    CampfireEntry.VolumeMultiplier = 0.8f;
    CampfireEntry.PitchVariationMin = 0.95f;
    CampfireEntry.PitchVariationMax = 1.05f;
    CampfireEntry.bIs3D = true;
    SFXRegistry.Add(CampfireEntry);

    // Footstep — dirt
    FAudio_SFXEntry FootstepDirt;
    FootstepDirt.SFXId = FName("Footstep_Dirt");
    FootstepDirt.VolumeMultiplier = 0.6f;
    FootstepDirt.PitchVariationMin = 0.85f;
    FootstepDirt.PitchVariationMax = 1.15f;
    FootstepDirt.bIs3D = true;
    SFXRegistry.Add(FootstepDirt);

    // Footstep — stone
    FAudio_SFXEntry FootstepStone;
    FootstepStone.SFXId = FName("Footstep_Stone");
    FootstepStone.VolumeMultiplier = 0.7f;
    FootstepStone.PitchVariationMin = 0.88f;
    FootstepStone.PitchVariationMax = 1.12f;
    FootstepStone.bIs3D = true;
    SFXRegistry.Add(FootstepStone);

    // Crafting — flint knapping
    FAudio_SFXEntry CraftingFlint;
    CraftingFlint.SFXId = FName("Crafting_Flint");
    CraftingFlint.VolumeMultiplier = 0.9f;
    CraftingFlint.PitchVariationMin = 0.9f;
    CraftingFlint.PitchVariationMax = 1.1f;
    CraftingFlint.bIs3D = false;
    SFXRegistry.Add(CraftingFlint);

    // Pre-populate dialogue lines with TTS URLs from Agent #15
    FAudio_DialogueLine HunterLeaderMain;
    HunterLeaderMain.CharacterName = TEXT("HunterLeader");
    HunterLeaderMain.AudioURL = TTS_HunterLeader_MainQuest;
    HunterLeaderMain.Duration = 12.0f;
    HunterLeaderMain.bSubtitleEnabled = true;
    HunterLeaderMain.SubtitleText = TEXT("Three suns ago, our hunters went east — past the great river, into the valley of stone teeth. None have returned.");
    RegisteredDialogueLines.Add(HunterLeaderMain);

    FAudio_DialogueLine CampElderLore;
    CampElderLore.CharacterName = TEXT("CampElder");
    CampElderLore.AudioURL = TTS_CampElder_Lore;
    CampElderLore.Duration = 14.0f;
    CampElderLore.bSubtitleEnabled = true;
    CampElderLore.SubtitleText = TEXT("I remember when this valley was safe. My grandmother's grandmother gathered berries here without fear.");
    RegisteredDialogueLines.Add(CampElderLore);

    FAudio_DialogueLine ScoutWarning;
    ScoutWarning.CharacterName = TEXT("Scout_NPC");
    ScoutWarning.AudioURL = TTS_Scout_NPC_Warning;
    ScoutWarning.Duration = 11.0f;
    ScoutWarning.bSubtitleEnabled = true;
    ScoutWarning.SubtitleText = TEXT("The northern ridge — there is a narrow path between the two rock faces. Do not take it after dark.");
    RegisteredDialogueLines.Add(ScoutWarning);

    FAudio_DialogueLine TribalElderLore;
    TribalElderLore.CharacterName = TEXT("TribalElder");
    TribalElderLore.AudioURL = TTS_TribalElder_StoneValley;
    TribalElderLore.Duration = 13.0f;
    TribalElderLore.bSubtitleEnabled = true;
    TribalElderLore.SubtitleText = TEXT("The stone valley. Before the great shaking, there was a river there — wide enough to swim across.");
    RegisteredDialogueLines.Add(TribalElderLore);

    FAudio_DialogueLine NightNarrator;
    NightNarrator.CharacterName = TEXT("CampNarrator");
    NightNarrator.AudioURL = TTS_CampNarrator_NightWarning;
    NightNarrator.Duration = 14.0f;
    NightNarrator.bSubtitleEnabled = true;
    NightNarrator.SubtitleText = TEXT("Night is falling. The sounds you hear now — those are not the wind. Stay close to the fire.");
    RegisteredDialogueLines.Add(NightNarrator);

    FAudio_DialogueLine TRexWarning;
    TRexWarning.CharacterName = TEXT("HunterLeader");
    TRexWarning.AudioURL = TTS_HunterLeader_TRexWarning;
    TRexWarning.Duration = 12.0f;
    TRexWarning.bSubtitleEnabled = true;
    TRexWarning.SubtitleText = TEXT("The T-Rex territory begins at the ridge. When the birds go silent — all of them, at once — that is your last warning.");
    RegisteredDialogueLines.Add(TRexWarning);
}

void AAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] BeginPlay — Zone: %d, Danger: %d"),
        (int32)CurrentZone, (int32)CurrentDangerLevel);
}

void AAudio_SystemManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Stop();
    }
    Super::EndPlay(EndPlayReason);
}

void AAudio_SystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Smooth music volume transitions
    if (!FMath::IsNearlyEqual(CurrentMusicVolume, TargetMusicVolume, 0.01f))
    {
        CurrentMusicVolume = FMath::FInterpTo(CurrentMusicVolume, TargetMusicVolume, DeltaTime, MusicTransitionSpeed);
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->SetVolumeMultiplier(CurrentMusicVolume);
        }
    }
}

void AAudio_SystemManager::SetAudioZone(EAudio_ZoneType NewZone)
{
    if (NewZone == CurrentZone) return;

    PreviousZone = CurrentZone;
    CurrentZone = NewZone;

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Zone transition: %d -> %d"),
        (int32)PreviousZone, (int32)CurrentZone);

    TransitionAmbientLayer(PreviousZone, CurrentZone);
}

void AAudio_SystemManager::SetDangerLevel(EAudio_DangerLevel NewDanger)
{
    if (NewDanger == CurrentDangerLevel) return;

    PreviousDangerLevel = CurrentDangerLevel;
    CurrentDangerLevel = NewDanger;

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Danger level: %d -> %d"),
        (int32)PreviousDangerLevel, (int32)CurrentDangerLevel);

    ApplyDangerMusicIntensity(CurrentDangerLevel);
}

void AAudio_SystemManager::UpdateDangerFromProximity(float ClosestPredatorDistance)
{
    EAudio_DangerLevel NewLevel = EAudio_DangerLevel::None;

    if (ClosestPredatorDistance < 300.0f)
    {
        NewLevel = EAudio_DangerLevel::Critical;
    }
    else if (ClosestPredatorDistance < 700.0f)
    {
        NewLevel = EAudio_DangerLevel::High;
    }
    else if (ClosestPredatorDistance < 1200.0f)
    {
        NewLevel = EAudio_DangerLevel::Medium;
    }
    else if (ClosestPredatorDistance < DangerProximityRadius)
    {
        NewLevel = EAudio_DangerLevel::Low;
    }

    SetDangerLevel(NewLevel);
}

void AAudio_SystemManager::PlayDialogueLine(const FString& CharacterName)
{
    for (const FAudio_DialogueLine& Line : RegisteredDialogueLines)
    {
        if (Line.CharacterName == CharacterName)
        {
            UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Playing dialogue: %s | URL: %s"),
                *Line.CharacterName, *Line.AudioURL);

            if (Line.SoundCue && AmbientAudioComponent)
            {
                AmbientAudioComponent->SetSound(Line.SoundCue);
                AmbientAudioComponent->Play();
            }
            return;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("[AudioSystemManager] Dialogue not found for character: %s"), *CharacterName);
}

void AAudio_SystemManager::PlaySFX(FName SFXId, FVector WorldLocation)
{
    for (const FAudio_SFXEntry& Entry : SFXRegistry)
    {
        if (Entry.SFXId == SFXId && Entry.SoundCue)
        {
            float PitchVariation = FMath::RandRange(Entry.PitchVariationMin, Entry.PitchVariationMax);

            if (Entry.bIs3D)
            {
                UGameplayStatics::PlaySoundAtLocation(
                    this, Entry.SoundCue, WorldLocation,
                    Entry.VolumeMultiplier, PitchVariation);
            }
            else
            {
                UGameplayStatics::PlaySound2D(
                    this, Entry.SoundCue,
                    Entry.VolumeMultiplier, PitchVariation);
            }
            return;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("[AudioSystemManager] SFX not found: %s"), *SFXId.ToString());
}

void AAudio_SystemManager::PlayFootstepSound(bool bIsRunning, bool bIsOnStone)
{
    FName FootstepID = bIsOnStone ? FName("Footstep_Stone") : FName("Footstep_Dirt");
    FVector PlayerLoc = GetActorLocation();
    PlaySFX(FootstepID, PlayerLoc);
}

void AAudio_SystemManager::PlayCraftingSound(FName ToolType)
{
    FName CraftingID = FName(FString::Printf(TEXT("Crafting_%s"), *ToolType.ToString()));
    PlaySFX(CraftingID, GetActorLocation());
}

void AAudio_SystemManager::LogAudioState()
{
    UE_LOG(LogTemp, Log, TEXT("=== AudioSystemManager State ==="));
    UE_LOG(LogTemp, Log, TEXT("  Current Zone: %d"), (int32)CurrentZone);
    UE_LOG(LogTemp, Log, TEXT("  Danger Level: %d"), (int32)CurrentDangerLevel);
    UE_LOG(LogTemp, Log, TEXT("  Music Volume: %.2f / %.2f"), CurrentMusicVolume, TargetMusicVolume);
    UE_LOG(LogTemp, Log, TEXT("  Ambient Layers: %d"), AmbientLayers.Num());
    UE_LOG(LogTemp, Log, TEXT("  SFX Registry: %d entries"), SFXRegistry.Num());
    UE_LOG(LogTemp, Log, TEXT("  Dialogue Lines: %d registered"), RegisteredDialogueLines.Num());
    UE_LOG(LogTemp, Log, TEXT("  TTS URLs wired: 6 (from Agent #15 + Agent #16)"));
    UE_LOG(LogTemp, Log, TEXT("  Freesound Campfire IDs: 681366, 681367, 688992"));
    UE_LOG(LogTemp, Log, TEXT("  Freesound Thunder IDs: 802401, 743019"));
}

// ---- Private helpers ----

void AAudio_SystemManager::TransitionAmbientLayer(EAudio_ZoneType FromZone, EAudio_ZoneType ToZone)
{
    // Find matching layer for new zone and fade in
    for (const FAudio_AmbientLayer& Layer : AmbientLayers)
    {
        if (Layer.ZoneType == ToZone && Layer.SoundCue)
        {
            TargetMusicVolume = Layer.BaseVolume;
            UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Transitioning ambient to zone %d, target vol %.2f"),
                (int32)ToZone, TargetMusicVolume);
            break;
        }
    }
}

void AAudio_SystemManager::ApplyDangerMusicIntensity(EAudio_DangerLevel DangerLevel)
{
    // Scale music intensity based on danger — Walter Murch principle:
    // silence and restraint at low danger, full intensity only at critical
    switch (DangerLevel)
    {
        case EAudio_DangerLevel::None:
            TargetMusicVolume = 0.6f;
            break;
        case EAudio_DangerLevel::Low:
            TargetMusicVolume = 0.75f;
            break;
        case EAudio_DangerLevel::Medium:
            TargetMusicVolume = 0.9f;
            break;
        case EAudio_DangerLevel::High:
            TargetMusicVolume = 1.1f;
            break;
        case EAudio_DangerLevel::Critical:
            TargetMusicVolume = 1.3f;
            break;
        default:
            TargetMusicVolume = 1.0f;
            break;
    }
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Danger music intensity -> %.2f"), TargetMusicVolume);
}
