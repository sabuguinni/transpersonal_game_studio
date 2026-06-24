#include "AudioSystemManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AAudio_SystemManager::AAudio_SystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 Hz — sufficient for audio zone detection

    // Pre-register the four core audio zones matching the MinPlayableMap layout
    FAudio_ZoneData TRexZone;
    TRexZone.Location   = FVector(2000.0f, 2500.0f, 300.0f);
    TRexZone.Radius     = 2000.0f;
    TRexZone.ZoneType   = EAudio_ZoneType::TRexTerritory;
    TRexZone.Volume     = 1.0f;
    RegisteredZones.Add(TRexZone);

    FAudio_ZoneData RaptorZone;
    RaptorZone.Location  = FVector(2400.0f, 2500.0f, 300.0f);
    RaptorZone.Radius    = 1500.0f;
    RaptorZone.ZoneType  = EAudio_ZoneType::RaptorPack;
    RaptorZone.Volume    = 0.9f;
    RegisteredZones.Add(RaptorZone);

    FAudio_ZoneData RiverZone;
    RiverZone.Location  = FVector(-500.0f, 1000.0f, 200.0f);
    RiverZone.Radius    = 1800.0f;
    RiverZone.ZoneType  = EAudio_ZoneType::RiverAmbience;
    RiverZone.Volume    = 0.8f;
    RegisteredZones.Add(RiverZone);

    FAudio_ZoneData ForestZone;
    ForestZone.Location = FVector(1000.0f, 1000.0f, 300.0f);
    ForestZone.Radius   = 2500.0f;
    ForestZone.ZoneType = EAudio_ZoneType::ForestCanopy;
    ForestZone.Volume   = 0.7f;
    RegisteredZones.Add(ForestZone);

    // Pre-register ElevenLabs TTS voice lines (generated in production cycles)
    FAudio_VoiceLine JungleSilenceHint;
    JungleSilenceHint.LineID    = "HINT_JUNGLE_SILENCE";
    JungleSilenceHint.Text      = "The jungle is never silent. Every sound tells you something.";
    JungleSilenceHint.AudioURL  = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782323776353_Narrator_Survival.mp3";
    JungleSilenceHint.Duration  = 13.0f;
    VoiceLines.Add(JungleSilenceHint);

    FAudio_VoiceLine DangerAlert;
    DangerAlert.LineID   = "ALERT_PREDATOR_NORTH";
    DangerAlert.Text     = "Danger. Large predator detected to the north.";
    DangerAlert.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782323807738_Narrator_Alert.mp3";
    DangerAlert.Duration = 9.0f;
    VoiceLines.Add(DangerAlert);
}

void AAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();
    CurrentDangerLevel = 0.0f;
    CurrentActiveZone  = EAudio_ZoneType::None;
    ZoneCheckTimer     = 0.0f;
    VoiceLineCooldown  = 0.0f;
}

void AAudio_SystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateDangerDecay(DeltaTime);

    ZoneCheckTimer += DeltaTime;
    if (ZoneCheckTimer >= ZoneCheckInterval)
    {
        ZoneCheckTimer = 0.0f;

        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC && PC->GetPawn())
        {
            FVector PlayerLoc = PC->GetPawn()->GetActorLocation();
            UpdateZoneDetection(PlayerLoc);
        }
    }

    if (VoiceLineCooldown > 0.0f)
    {
        VoiceLineCooldown -= DeltaTime;
    }
}

void AAudio_SystemManager::RegisterAudioZone(const FAudio_ZoneData& ZoneData)
{
    RegisteredZones.Add(ZoneData);
}

EAudio_ZoneType AAudio_SystemManager::GetActiveZoneForLocation(const FVector& PlayerLocation) const
{
    float ClosestDist = MAX_FLT;
    EAudio_ZoneType ClosestZone = EAudio_ZoneType::None;

    for (const FAudio_ZoneData& Zone : RegisteredZones)
    {
        if (!Zone.bIsActive) { continue; }
        float Dist = FVector::Dist(PlayerLocation, Zone.Location);
        if (Dist <= Zone.Radius && Dist < ClosestDist)
        {
            ClosestDist  = Dist;
            ClosestZone  = Zone.ZoneType;
        }
    }
    return ClosestZone;
}

void AAudio_SystemManager::SetZoneActive(EAudio_ZoneType ZoneType, bool bActive)
{
    for (FAudio_ZoneData& Zone : RegisteredZones)
    {
        if (Zone.ZoneType == ZoneType)
        {
            Zone.bIsActive = bActive;
        }
    }
}

void AAudio_SystemManager::RegisterVoiceLine(const FAudio_VoiceLine& VoiceLine)
{
    VoiceLines.Add(VoiceLine);
}

bool AAudio_SystemManager::TriggerVoiceLine(const FString& LineID)
{
    for (FAudio_VoiceLine& Line : VoiceLines)
    {
        if (Line.LineID == LineID && !Line.bHasBeenPlayed)
        {
            Line.bHasBeenPlayed = true;
            VoiceLineCooldown   = VoiceLineCooldownDuration;
            UE_LOG(LogTemp, Log, TEXT("AudioSystem: Playing voice line [%s] — %s"), *LineID, *Line.Text);
            return true;
        }
    }
    return false;
}

void AAudio_SystemManager::TriggerRandomSurvivalHint()
{
    if (VoiceLineCooldown > 0.0f) { return; }

    TArray<FAudio_VoiceLine*> Available;
    for (FAudio_VoiceLine& Line : VoiceLines)
    {
        if (!Line.bHasBeenPlayed)
        {
            Available.Add(&Line);
        }
    }

    if (Available.Num() == 0) { return; }

    int32 Idx = FMath::RandRange(0, Available.Num() - 1);
    Available[Idx]->bHasBeenPlayed = true;
    VoiceLineCooldown = VoiceLineCooldownDuration;

    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Random hint — %s"), *Available[Idx]->Text);
}

void AAudio_SystemManager::SetDangerLevel(float DangerLevel)
{
    CurrentDangerLevel = FMath::Clamp(DangerLevel, 0.0f, 1.0f);
}

void AAudio_SystemManager::OnDinosaurNearby(float DistanceMeters, bool bIsPredator)
{
    if (!bIsPredator) { return; }

    // Danger increases as predator gets closer (inverse square falloff)
    float MaxDangerDist = 500.0f;
    float NormalizedDist = FMath::Clamp(DistanceMeters / MaxDangerDist, 0.0f, 1.0f);
    float NewDanger = 1.0f - NormalizedDist;

    if (NewDanger > CurrentDangerLevel)
    {
        CurrentDangerLevel = NewDanger;
    }

    // Auto-trigger danger alert if danger is high and cooldown allows
    if (CurrentDangerLevel >= DangerThresholdHigh && VoiceLineCooldown <= 0.0f)
    {
        TriggerVoiceLine("ALERT_PREDATOR_NORTH");
    }
}

void AAudio_SystemManager::TriggerFootstepShake(float Intensity)
{
    // Intensity 0-1: maps to camera shake magnitude
    // Actual camera shake requires Blueprint or CameraShakeBase subclass
    // Log for now — Blueprint will hook into this
    UE_LOG(LogTemp, Log, TEXT("AudioSystem: FootstepShake intensity=%.2f"), Intensity);
}

void AAudio_SystemManager::TriggerDamageFlash()
{
    // Damage flash — red screen overlay
    // Triggered via Blueprint UI layer; this is the C++ notification
    UE_LOG(LogTemp, Log, TEXT("AudioSystem: DamageFlash triggered"));
}

void AAudio_SystemManager::UpdateZoneDetection(const FVector& PlayerLocation)
{
    EAudio_ZoneType NewZone = GetActiveZoneForLocation(PlayerLocation);
    if (NewZone != CurrentActiveZone)
    {
        UE_LOG(LogTemp, Log, TEXT("AudioSystem: Zone changed %d -> %d"),
            (int32)CurrentActiveZone, (int32)NewZone);
        CurrentActiveZone = NewZone;
    }
}

void AAudio_SystemManager::UpdateDangerDecay(float DeltaTime)
{
    if (CurrentDangerLevel > 0.0f)
    {
        CurrentDangerLevel = FMath::Max(0.0f, CurrentDangerLevel - DangerDecayRate * DeltaTime);
    }
}
