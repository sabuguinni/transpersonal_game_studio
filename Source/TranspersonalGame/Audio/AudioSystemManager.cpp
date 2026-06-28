// AudioSystemManager.cpp
// Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260628_001
// Implementation: adaptive ambient layers, dialogue ducking, danger-reactive music

#include "Audio/AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "EngineUtils.h"

// ─────────────────────────────────────────────────────────────────────────────
// AAudio_AmbientZone
// ─────────────────────────────────────────────────────────────────────────────

AAudio_AmbientZone::AAudio_AmbientZone()
{
    PrimaryActorTick.bCanEverTick = true;

    PrimaryAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("PrimaryAudioComponent"));
    PrimaryAudioComponent->SetupAttachment(RootComponent);
    PrimaryAudioComponent->bAutoActivate = false;
    PrimaryAudioComponent->SetVolumeMultiplier(0.0f);
}

void AAudio_AmbientZone::BeginPlay()
{
    Super::BeginPlay();
    // Start silent; AudioSystemManager will activate zones as player enters
    CurrentVolume = 0.0f;
}

void AAudio_AmbientZone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bZoneActive && CurrentVolume < 1.0f)
    {
        // Fade in
        CurrentVolume = FMath::Min(1.0f, CurrentVolume + DeltaTime / FMath::Max(0.1f, AmbientLayers.Num() > 0 ? AmbientLayers[0].FadeInTime : 2.0f));
        if (PrimaryAudioComponent)
        {
            PrimaryAudioComponent->SetVolumeMultiplier(CurrentVolume);
        }
    }
    else if (!bZoneActive && CurrentVolume > 0.0f)
    {
        // Fade out
        float FadeOut = (AmbientLayers.Num() > 0) ? AmbientLayers[0].FadeOutTime : 3.0f;
        CurrentVolume = FMath::Max(0.0f, CurrentVolume - DeltaTime / FMath::Max(0.1f, FadeOut));
        if (PrimaryAudioComponent)
        {
            PrimaryAudioComponent->SetVolumeMultiplier(CurrentVolume);
            if (CurrentVolume <= 0.0f)
            {
                PrimaryAudioComponent->Stop();
            }
        }
    }
}

void AAudio_AmbientZone::ActivateZone()
{
    if (bZoneActive) return;
    bZoneActive = true;

    if (PrimaryAudioComponent && AmbientLayers.Num() > 0 && AmbientLayers[0].SoundAsset)
    {
        PrimaryAudioComponent->SetSound(AmbientLayers[0].SoundAsset);
        PrimaryAudioComponent->Play();
    }
}

void AAudio_AmbientZone::DeactivateZone()
{
    bZoneActive = false;
    // Fade handled in Tick
}

bool AAudio_AmbientZone::IsPlayerInZone() const
{
    UWorld* World = GetWorld();
    if (!World) return false;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return false;

    float DistSq = FVector::DistSquared(GetActorLocation(), PlayerPawn->GetActorLocation());
    return DistSq <= (ZoneRadius * ZoneRadius);
}

// ─────────────────────────────────────────────────────────────────────────────
// AAudio_SystemManager
// ─────────────────────────────────────────────────────────────────────────────

AAudio_SystemManager::AAudio_SystemManager()
{
    PrimaryActorTick.bCanEverTick = true;

    MusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComponent"));
    MusicComponent->SetupAttachment(RootComponent);
    MusicComponent->bAutoActivate = false;
    MusicComponent->SetVolumeMultiplier(0.0f);
}

void AAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();
    DiscoverAmbientZones();
    CurrentMusicVolume = 0.0f;
    TargetMusicVolume = 0.8f;
}

void AAudio_SystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update ambient zone activation based on player proximity
    for (AAudio_AmbientZone* Zone : RegisteredZones)
    {
        if (!Zone) continue;
        if (Zone->IsPlayerInZone())
        {
            Zone->ActivateZone();
        }
        else
        {
            Zone->DeactivateZone();
        }
    }

    // Smooth music volume toward target
    UpdateMusicLayer(DeltaTime);
}

void AAudio_SystemManager::SetDangerLevel(EAudio_DangerLevel NewLevel)
{
    if (CurrentDangerLevel == NewLevel) return;
    CurrentDangerLevel = NewLevel;

    // Find matching music state and begin transition
    for (const FAudio_MusicState& State : MusicStates)
    {
        if (State.DangerLevel == NewLevel && State.MusicAsset)
        {
            TargetMusicVolume = State.Volume;
            if (MusicComponent)
            {
                MusicComponent->SetSound(State.MusicAsset);
                MusicComponent->Play();
            }
            break;
        }
    }
}

void AAudio_SystemManager::OnDialogueStart()
{
    if (bDialogueActive) return;
    bDialogueActive = true;
    ApplyDialogueDuck(true);
}

void AAudio_SystemManager::OnDialogueEnd()
{
    if (!bDialogueActive) return;
    bDialogueActive = false;
    ApplyDialogueDuck(false);
}

void AAudio_SystemManager::SetTimeOfDay(EAudio_TimeOfDay NewTime)
{
    CurrentTimeOfDay = NewTime;
    // Ambient zones will cross-fade based on time; future: filter layers by TimeOfDay
}

AAudio_SystemManager* AAudio_SystemManager::GetInstance(UObject* WorldContext)
{
    if (!WorldContext) return nullptr;
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return nullptr;

    for (TActorIterator<AAudio_SystemManager> It(World); It; ++It)
    {
        return *It; // Return first found
    }
    return nullptr;
}

void AAudio_SystemManager::DiscoverAmbientZones()
{
    RegisteredZones.Empty();
    UWorld* World = GetWorld();
    if (!World) return;

    for (TActorIterator<AAudio_AmbientZone> It(World); It; ++It)
    {
        RegisteredZones.Add(*It);
    }

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Discovered %d ambient zones"), RegisteredZones.Num());
}

void AAudio_SystemManager::UpdateMusicLayer(float DeltaTime)
{
    if (!MusicComponent) return;

    float Diff = TargetMusicVolume - CurrentMusicVolume;
    if (FMath::Abs(Diff) > 0.001f)
    {
        float Step = MusicTransitionSpeed * DeltaTime;
        CurrentMusicVolume = FMath::Clamp(CurrentMusicVolume + FMath::Sign(Diff) * Step, 0.0f, 1.0f);
        MusicComponent->SetVolumeMultiplier(CurrentMusicVolume);
    }
}

void AAudio_SystemManager::ApplyDialogueDuck(bool bDuck)
{
    float TargetVol = bDuck ? DuckConfig.DuckVolume : 1.0f;
    float FadeTime  = bDuck ? DuckConfig.DuckFadeTime : DuckConfig.RestoreFadeTime;

    // Apply to all registered ambient zones
    for (AAudio_AmbientZone* Zone : RegisteredZones)
    {
        if (!Zone) continue;
        UAudioComponent* Comp = Zone->PrimaryAudioComponent;
        if (Comp)
        {
            Comp->AdjustVolume(FadeTime, TargetVol);
        }
    }

    // Apply to music component
    if (MusicComponent)
    {
        MusicComponent->AdjustVolume(FadeTime, TargetVol);
    }
}
