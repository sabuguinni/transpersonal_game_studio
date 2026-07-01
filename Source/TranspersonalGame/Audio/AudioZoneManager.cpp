#include "AudioZoneManager.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

AAudioZoneManager::AAudioZoneManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root + Trigger Sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(800.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    // Ambient Audio Component
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;

    // Default narrator lines — wired to TTS URLs from this cycle
    TRexWarningURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782866205135_SurvivalNarrator.mp3");
    FireSurvivalURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782866223368_SurvivalNarrator.mp3");
}

void AAudioZoneManager::BeginPlay()
{
    Super::BeginPlay();
    BuildDefaultNarratorLines();
    unreal::log(TEXT("AudioZoneManager: BeginPlay — zone active"));
}

void AAudioZoneManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Stop();
    }
    Super::EndPlay(EndPlayReason);
}

void AAudioZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateThreatBasedAudio(DeltaTime);

    LastThreatScanTime += DeltaTime;
    if (LastThreatScanTime >= ThreatScanInterval)
    {
        LastThreatScanTime = 0.0f;
        ScanForNearbyDinosaurs();
    }
}

void AAudioZoneManager::SetThreatLevel(EAudio_ThreatLevel NewLevel)
{
    if (CurrentThreatLevel == NewLevel) return;

    CurrentThreatLevel = NewLevel;

    switch (NewLevel)
    {
        case EAudio_ThreatLevel::Safe:
            MusicTensionBlend = 0.0f;
            break;
        case EAudio_ThreatLevel::Cautious:
            MusicTensionBlend = 0.33f;
            break;
        case EAudio_ThreatLevel::Danger:
            MusicTensionBlend = 0.66f;
            break;
        case EAudio_ThreatLevel::Critical:
            MusicTensionBlend = 1.0f;
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: ThreatLevel -> %d | Tension: %.2f"),
        (int32)NewLevel, MusicTensionBlend);
}

void AAudioZoneManager::TriggerNarratorLine(const FString& LineID)
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    for (FAudio_NarratorLine& Line : NarratorLines)
    {
        if (Line.LineID == LineID)
        {
            if (Line.bHasPlayed && (CurrentTime - LastNarratorPlayTime) < Line.CooldownSeconds)
            {
                UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Narrator line '%s' on cooldown"), *LineID);
                return;
            }

            Line.bHasPlayed = true;
            LastNarratorPlayTime = CurrentTime;
            UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Playing narrator line '%s' — URL: %s"),
                *LineID, *Line.AudioURL);
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("AudioZoneManager: Narrator line '%s' not found"), *LineID);
}

void AAudioZoneManager::SetMusicTension(float TensionValue)
{
    MusicTensionBlend = FMath::Clamp(TensionValue, 0.0f, 1.0f);

    if (MusicTensionBlend < 0.25f)
        CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    else if (MusicTensionBlend < 0.5f)
        CurrentThreatLevel = EAudio_ThreatLevel::Cautious;
    else if (MusicTensionBlend < 0.75f)
        CurrentThreatLevel = EAudio_ThreatLevel::Danger;
    else
        CurrentThreatLevel = EAudio_ThreatLevel::Critical;
}

void AAudioZoneManager::RegisterDialogueZone(EAudio_ZoneType ZoneType, FVector Location)
{
    ActiveZoneType = ZoneType;
    SetActorLocation(Location);
    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Registered zone type %d at %s"),
        (int32)ZoneType, *Location.ToString());
}

void AAudioZoneManager::DebugPrintAudioState()
{
    UE_LOG(LogTemp, Log, TEXT("=== AudioZoneManager Debug ==="));
    UE_LOG(LogTemp, Log, TEXT("Zone: %d | Threat: %d | Tension: %.2f"),
        (int32)ActiveZoneType, (int32)CurrentThreatLevel, MusicTensionBlend);
    UE_LOG(LogTemp, Log, TEXT("Narrator lines registered: %d"), NarratorLines.Num());
    UE_LOG(LogTemp, Log, TEXT("Campfire SFX ID: %d | River SFX ID: %d"),
        CampfireSoundID, RiverSoundID);
    UE_LOG(LogTemp, Log, TEXT("TRex Warning URL: %s"), *TRexWarningURL);
    UE_LOG(LogTemp, Log, TEXT("Fire Survival URL: %s"), *FireSurvivalURL);
}

void AAudioZoneManager::UpdateThreatBasedAudio(float DeltaTime)
{
    // Smooth tension blend over time
    float TargetTension = 0.0f;
    switch (CurrentThreatLevel)
    {
        case EAudio_ThreatLevel::Safe:     TargetTension = 0.0f;  break;
        case EAudio_ThreatLevel::Cautious: TargetTension = 0.33f; break;
        case EAudio_ThreatLevel::Danger:   TargetTension = 0.66f; break;
        case EAudio_ThreatLevel::Critical: TargetTension = 1.0f;  break;
    }

    MusicTensionBlend = FMath::FInterpTo(MusicTensionBlend, TargetTension, DeltaTime, 0.5f);
}

void AAudioZoneManager::ScanForNearbyDinosaurs()
{
    if (!GetWorld()) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    float ClosestDinoDistance = ThreatDetectionRadius + 1.0f;

    for (AActor* Actor : AllActors)
    {
        FString ActorName = Actor->GetName();
        if (ActorName.Contains(TEXT("TRex")) || ActorName.Contains(TEXT("Raptor")) ||
            ActorName.Contains(TEXT("Dino")) || ActorName.Contains(TEXT("Dinosaur")))
        {
            float Dist = FVector::Dist(PlayerLoc, Actor->GetActorLocation());
            if (Dist < ClosestDinoDistance)
            {
                ClosestDinoDistance = Dist;
            }
        }
    }

    // Map distance to threat level
    if (ClosestDinoDistance < 300.0f)
        SetThreatLevel(EAudio_ThreatLevel::Critical);
    else if (ClosestDinoDistance < 600.0f)
        SetThreatLevel(EAudio_ThreatLevel::Danger);
    else if (ClosestDinoDistance < ThreatDetectionRadius)
        SetThreatLevel(EAudio_ThreatLevel::Cautious);
    else
        SetThreatLevel(EAudio_ThreatLevel::Safe);
}

void AAudioZoneManager::BuildDefaultNarratorLines()
{
    NarratorLines.Empty();

    // Line 1 — T-Rex Warning (from this cycle's TTS)
    FAudio_NarratorLine TRexLine;
    TRexLine.LineID = TEXT("TREX_WARNING");
    TRexLine.AudioURL = TRexWarningURL;
    TRexLine.ThreatContext = EAudio_ThreatLevel::Critical;
    TRexLine.CooldownSeconds = 180.0f;
    NarratorLines.Add(TRexLine);

    // Line 2 — Fire Survival (from this cycle's TTS)
    FAudio_NarratorLine FireLine;
    FireLine.LineID = TEXT("FIRE_SURVIVAL");
    FireLine.AudioURL = FireSurvivalURL;
    FireLine.ThreatContext = EAudio_ThreatLevel::Safe;
    FireLine.CooldownSeconds = 120.0f;
    NarratorLines.Add(FireLine);

    // Line 3 — Raptor Warning (from Agent #15 TTS)
    FAudio_NarratorLine RaptorLine;
    RaptorLine.LineID = TEXT("RAPTOR_HUNT_PATTERN");
    RaptorLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782866048595_TribalElder.mp3");
    RaptorLine.ThreatContext = EAudio_ThreatLevel::Danger;
    RaptorLine.CooldownSeconds = 150.0f;
    NarratorLines.Add(RaptorLine);

    // Line 4 — Herd Tracker (from Agent #15 TTS)
    FAudio_NarratorLine HerdLine;
    HerdLine.LineID = TEXT("HERD_OBSERVATION");
    HerdLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782866064626_HerdTracker.mp3");
    HerdLine.ThreatContext = EAudio_ThreatLevel::Cautious;
    HerdLine.CooldownSeconds = 120.0f;
    NarratorLines.Add(HerdLine);

    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Built %d default narrator lines"), NarratorLines.Num());
}
