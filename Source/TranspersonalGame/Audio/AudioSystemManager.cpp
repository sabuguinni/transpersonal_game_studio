#include "AudioSystemManager.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ============================================================
// AAudio_ZoneActor — Implementation
// ============================================================

AAudio_ZoneActor::AAudio_ZoneActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Trigger sphere for player overlap detection
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(2000.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    // Audio component for spatial ambient playback
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetVolumeMultiplier(0.0f);
}

void AAudio_ZoneActor::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_ZoneActor::OnPlayerEnterZone);
    TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_ZoneActor::OnPlayerExitZone);

    // Set sphere radius from config
    TriggerSphere->SetSphereRadius(ZoneRadius);

    // Start ambient if sound asset is assigned
    if (PrimaryAmbientSound && AmbientAudioComponent)
    {
        AmbientAudioComponent->SetSound(PrimaryAmbientSound);
        AmbientAudioComponent->Play();
    }
}

void AAudio_ZoneActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Smooth volume fade
    if (bFading && AmbientAudioComponent)
    {
        float CurrentVol = AmbientAudioComponent->VolumeMultiplier;
        float NewVol = FMath::FInterpTo(CurrentVol, FadeTargetVolume, DeltaTime, FadeSpeed);
        AmbientAudioComponent->SetVolumeMultiplier(NewVol);
        CurrentVolume = NewVol;

        if (FMath::IsNearlyEqual(NewVol, FadeTargetVolume, 0.01f))
        {
            AmbientAudioComponent->SetVolumeMultiplier(FadeTargetVolume);
            CurrentVolume = FadeTargetVolume;
            bFading = false;
        }
    }

    // Voice line cooldown timer
    if (bPlayerInZone && VoiceLines.Num() > 0)
    {
        VoiceLineTimer -= DeltaTime;
        if (VoiceLineTimer <= 0.0f)
        {
            TriggerNextVoiceLine();
            // Reset timer: 30-60 seconds between lines
            VoiceLineTimer = FMath::RandRange(30.0f, 60.0f);
        }
    }
}

void AAudio_ZoneActor::SetDangerLevel(EAudio_DangerLevel NewLevel)
{
    CurrentDangerLevel = NewLevel;

    // Adjust ambient volume based on danger
    float TargetVol = AmbientLayer.BaseVolume;
    if (NewLevel == EAudio_DangerLevel::Danger || NewLevel == EAudio_DangerLevel::Critical)
    {
        TargetVol *= AmbientLayer.DangerVolumeMultiplier;
    }

    FadeInAmbient(TargetVol, AmbientLayer.FadeInTime);

    // Play danger stinger if available
    if (DangerStingerSound && (NewLevel == EAudio_DangerLevel::Danger || NewLevel == EAudio_DangerLevel::Critical))
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), DangerStingerSound, GetActorLocation());
    }
}

void AAudio_ZoneActor::TriggerNextVoiceLine()
{
    if (VoiceLines.Num() == 0) return;

    // Find next unplayed line
    for (int32 i = 0; i < VoiceLines.Num(); i++)
    {
        int32 Idx = (CurrentVoiceLineIndex + i) % VoiceLines.Num();
        if (!VoiceLines[Idx].bHasPlayed)
        {
            CurrentVoiceLineIndex = Idx;
            VoiceLines[Idx].bHasPlayed = true;
            UE_LOG(LogTemp, Log, TEXT("[AudioZone] Playing voice line: %s — %s"),
                *VoiceLines[Idx].SpeakerName, *VoiceLines[Idx].LineText);
            return;
        }
    }

    // All played — reset and cycle
    for (auto& Line : VoiceLines)
    {
        Line.bHasPlayed = false;
    }
    CurrentVoiceLineIndex = 0;
}

void AAudio_ZoneActor::FadeInAmbient(float TargetVolume, float FadeTime)
{
    FadeTargetVolume = TargetVolume;
    FadeSpeed = (FadeTime > 0.0f) ? (1.0f / FadeTime) : 10.0f;
    bFading = true;

    if (AmbientAudioComponent && !AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Play();
    }
}

void AAudio_ZoneActor::FadeOutAmbient(float FadeTime)
{
    FadeTargetVolume = 0.0f;
    FadeSpeed = (FadeTime > 0.0f) ? (1.0f / FadeTime) : 10.0f;
    bFading = true;
}

void AAudio_ZoneActor::OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    // Check if it's a player character
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (PlayerChar && PlayerChar->IsPlayerControlled())
    {
        bPlayerInZone = true;
        VoiceLineTimer = 5.0f; // First line after 5 seconds
        FadeInAmbient(AmbientLayer.BaseVolume, AmbientLayer.FadeInTime);
        UE_LOG(LogTemp, Log, TEXT("[AudioZone] Player entered zone: %s"), *GetActorLabel());
    }
}

void AAudio_ZoneActor::OnPlayerExitZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (PlayerChar && PlayerChar->IsPlayerControlled())
    {
        bPlayerInZone = false;
        FadeOutAmbient(AmbientLayer.FadeOutTime);
        UE_LOG(LogTemp, Log, TEXT("[AudioZone] Player exited zone: %s"), *GetActorLabel());
    }
}

// ============================================================
// UAudio_SystemManager — Implementation
// ============================================================

UAudio_SystemManager::UAudio_SystemManager()
{
    // Pre-populate Freesound asset IDs discovered this cycle
    FreesoundAssetIDs.Add(TEXT("Campfire_Crackle"), 157187);
    FreesoundAssetIDs.Add(TEXT("Campfire_Long"), 394952);
    FreesoundAssetIDs.Add(TEXT("River_Stream_Gentle"), 446019);
    FreesoundAssetIDs.Add(TEXT("River_Spring_Water"), 684315);
}

void UAudio_SystemManager::RegisterVoiceLineURL(const FString& LineID, const FString& URL)
{
    VoiceLineURLs.Add(LineID, URL);
    UE_LOG(LogTemp, Log, TEXT("[AudioSystem] Registered voice line: %s"), *LineID);
}

void UAudio_SystemManager::SetGlobalDangerLevel(EAudio_DangerLevel NewLevel)
{
    GlobalDangerLevel = NewLevel;

    // Propagate to all registered zones
    for (AAudio_ZoneActor* Zone : ActiveZones)
    {
        if (Zone)
        {
            Zone->SetDangerLevel(NewLevel);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[AudioSystem] Global danger level set to: %d"),
        static_cast<int32>(NewLevel));
}

void UAudio_SystemManager::RegisterZone(AAudio_ZoneActor* Zone)
{
    if (Zone && !ActiveZones.Contains(Zone))
    {
        ActiveZones.Add(Zone);
        UE_LOG(LogTemp, Log, TEXT("[AudioSystem] Zone registered: %s"), *Zone->GetActorLabel());
    }
}

void UAudio_SystemManager::UnregisterZone(AAudio_ZoneActor* Zone)
{
    ActiveZones.Remove(Zone);
}

FString UAudio_SystemManager::GetVoiceLineURL(const FString& LineID) const
{
    const FString* URL = VoiceLineURLs.Find(LineID);
    return URL ? *URL : FString();
}

void UAudio_SystemManager::InitializeDefaultVoiceLines()
{
    // TTS voice lines generated this cycle (Agent #16 — PROD_CYCLE_AUTO_20260626_003)
    RegisterVoiceLineURL(TEXT("TRex_Approach_Warning"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782448450103_SurvivalNarrator.mp3"));

    RegisterVoiceLineURL(TEXT("Raptor_Pack_Warning"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782448457485_SurvivalNarrator.mp3"));

    // TTS voice lines from Agent #15 (PROD_CYCLE_AUTO_20260626_003)
    RegisterVoiceLineURL(TEXT("Elder_Kael_Fire_Defence"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782448341913_Elder_Kael.mp3"));

    RegisterVoiceLineURL(TEXT("Scout_Mira_Eastern_Ridge"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782448344688_Scout_Mira.mp3"));

    RegisterVoiceLineURL(TEXT("Hunter_Brak_Birds_Quiet"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782448369738_Hunter_Brak.mp3"));

    RegisterVoiceLineURL(TEXT("Scout_Mira_River_Fork"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782448371935_Scout_Mira.mp3"));

    // Previous cycle TTS lines (Agent #16 — PROD_CYCLE_AUTO_20260626_002)
    RegisterVoiceLineURL(TEXT("Narrator_TRex_Sight"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782448450103_SurvivalNarrator.mp3"));

    UE_LOG(LogTemp, Log, TEXT("[AudioSystem] InitializeDefaultVoiceLines: %d lines registered"),
        VoiceLineURLs.Num());
}
