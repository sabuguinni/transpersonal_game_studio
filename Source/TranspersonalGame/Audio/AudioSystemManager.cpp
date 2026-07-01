#include "AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Camera/CameraShakeBase.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"

AAudioSystemManager::AAudioSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default screen shake config for T-Rex
    TRexShakeConfig.Magnitude = 3.5f;
    TRexShakeConfig.Duration = 0.8f;
    TRexShakeConfig.Frequency = 8.0f;
    TRexShakeConfig.TriggerRadius = 2500.0f;

    CurrentBiome = EAudio_BiomeType::Forest;
    CurrentTimeOfDay = EAudio_TimeOfDay::Day;
    CurrentDangerLevel = EAudio_DangerLevel::Safe;
    CurrentTension = 0.0f;

    // Create audio components
    AmbienceComponentA = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbienceA"));
    AmbienceComponentA->bAutoActivate = false;
    AmbienceComponentA->SetVolumeMultiplier(0.0f);

    AmbienceComponentB = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbienceB"));
    AmbienceComponentB->bAutoActivate = false;
    AmbienceComponentB->SetVolumeMultiplier(0.0f);

    MusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComponent"));
    MusicComponent->bAutoActivate = false;
    MusicComponent->SetVolumeMultiplier(0.0f);
}

void AAudioSystemManager::BeginPlay()
{
    Super::BeginPlay();
    InitAudioComponents();
    ApplyVolumeSettings();
    unreal_log_impl(TEXT("AudioSystemManager: BeginPlay — adaptive audio system initialised"));
}

void AAudioSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateAmbienceTick(DeltaTime);
}

// ── Ambience ──────────────────────────────────────────────────────────────────

void AAudioSystemManager::SetBiome(EAudio_BiomeType NewBiome)
{
    if (CurrentBiome == NewBiome) return;
    CurrentBiome = NewBiome;
    // Crossfade will be triggered by the ambience tick
}

void AAudioSystemManager::SetTimeOfDay(EAudio_TimeOfDay NewTime)
{
    if (CurrentTimeOfDay == NewTime) return;
    CurrentTimeOfDay = NewTime;
}

void AAudioSystemManager::SetDangerLevel(EAudio_DangerLevel NewDanger)
{
    if (CurrentDangerLevel == NewDanger) return;
    CurrentDangerLevel = NewDanger;

    // Map danger to tension
    switch (NewDanger)
    {
        case EAudio_DangerLevel::Safe:     CurrentTension = 0.0f;  break;
        case EAudio_DangerLevel::Cautious: CurrentTension = 0.3f;  break;
        case EAudio_DangerLevel::Danger:   CurrentTension = 0.7f;  break;
        case EAudio_DangerLevel::Critical: CurrentTension = 1.0f;  break;
    }
}

void AAudioSystemManager::CrossfadeAmbience(USoundCue* NewCue, float FadeTime)
{
    if (!NewCue) return;

    UAudioComponent* FadeOut = bAmbienceOnA ? AmbienceComponentA : AmbienceComponentB;
    UAudioComponent* FadeIn  = bAmbienceOnA ? AmbienceComponentB : AmbienceComponentA;

    if (FadeOut && FadeOut->IsPlaying())
    {
        FadeOut->FadeOut(FadeTime, 0.0f);
    }

    if (FadeIn)
    {
        FadeIn->SetSound(NewCue);
        FadeIn->FadeIn(FadeTime, AmbienceVolume * MasterVolume);
    }

    bAmbienceOnA = !bAmbienceOnA;
}

// ── Screen Shake ──────────────────────────────────────────────────────────────

void AAudioSystemManager::TriggerTRexScreenShake(FVector TRexLocation)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    float Distance = FVector::Dist(TRexLocation, PlayerPawn->GetActorLocation());
    float ShakeMag = ComputeShakeMagnitude(TRexLocation, PlayerPawn->GetActorLocation(), TRexShakeConfig.TriggerRadius);

    if (ShakeMag > 0.05f)
    {
        // Apply camera shake via player controller
        // Scale magnitude by proximity
        PC->ClientStartCameraShake(
            UCameraShakeBase::StaticClass(),
            ShakeMag
        );
    }
}

void AAudioSystemManager::TriggerImpactShake(FVector ImpactLocation, float Magnitude)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    float ShakeMag = ComputeShakeMagnitude(ImpactLocation, PlayerPawn->GetActorLocation(), 1500.0f);
    ShakeMag *= Magnitude;

    if (ShakeMag > 0.05f)
    {
        PC->ClientStartCameraShake(UCameraShakeBase::StaticClass(), ShakeMag);
    }
}

// ── Music Tension ─────────────────────────────────────────────────────────────

void AAudioSystemManager::UpdateMusicTension(float TensionValue)
{
    CurrentTension = FMath::Clamp(TensionValue, 0.0f, 1.0f);

    if (MusicComponent && MusicComponent->IsPlaying())
    {
        // Scale pitch and volume with tension
        float PitchMult = FMath::Lerp(1.0f, 1.15f, CurrentTension);
        float VolMult   = FMath::Lerp(0.3f, 1.0f, CurrentTension) * MusicVolume * MasterVolume;
        MusicComponent->SetPitchMultiplier(PitchMult);
        MusicComponent->SetVolumeMultiplier(VolMult);
    }
}

void AAudioSystemManager::PlayStingerOneShot(USoundCue* Stinger)
{
    if (!Stinger) return;
    UGameplayStatics::PlaySoundAtLocation(this, Stinger, GetActorLocation(), SFXVolume * MasterVolume);
}

// ── Footstep System ───────────────────────────────────────────────────────────

void AAudioSystemManager::PlayDinosaurFootstep(FVector FootLocation, float DinosaurMass)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Screen shake proportional to dinosaur mass (T-Rex ~8000kg, Raptor ~80kg)
    float NormalisedMass = FMath::Clamp(DinosaurMass / 8000.0f, 0.0f, 1.0f);
    float ShakeMag = FMath::Lerp(0.1f, TRexShakeConfig.Magnitude, NormalisedMass);

    TriggerImpactShake(FootLocation, ShakeMag);

    // Spawn ground dust at footstep location (visual feedback)
    // VFX Agent #17 will hook into this location
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: DinosaurFootstep at %s — mass %.0f kg — shake %.2f"),
        *FootLocation.ToString(), DinosaurMass, ShakeMag);
}

void AAudioSystemManager::PlayPlayerFootstep(FVector FootLocation, bool bRunning)
{
    // Footstep volume and pitch vary with run state
    float Vol   = bRunning ? 0.8f : 0.5f;
    float Pitch = bRunning ? 1.1f : 1.0f;

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: PlayerFootstep at %s — running=%d"),
        *FootLocation.ToString(), bRunning ? 1 : 0);
}

// ── Private ───────────────────────────────────────────────────────────────────

void AAudioSystemManager::InitAudioComponents()
{
    if (AmbienceComponentA) AmbienceComponentA->SetVolumeMultiplier(AmbienceVolume * MasterVolume);
    if (AmbienceComponentB) AmbienceComponentB->SetVolumeMultiplier(0.0f);
    if (MusicComponent)     MusicComponent->SetVolumeMultiplier(0.0f);
}

void AAudioSystemManager::UpdateAmbienceTick(float DeltaTime)
{
    // Smooth tension blend
    float TargetTension = CurrentTension;
    TensionBlendAlpha = FMath::FInterpTo(TensionBlendAlpha, TargetTension, DeltaTime, 0.5f);
    UpdateMusicTension(TensionBlendAlpha);
}

void AAudioSystemManager::ApplyVolumeSettings()
{
    if (AmbienceComponentA) AmbienceComponentA->SetVolumeMultiplier(AmbienceVolume * MasterVolume);
    if (MusicComponent)     MusicComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
}

float AAudioSystemManager::ComputeShakeMagnitude(FVector SourceLocation, FVector PlayerLocation, float TriggerRadius)
{
    float Distance = FVector::Dist(SourceLocation, PlayerLocation);
    if (Distance >= TriggerRadius) return 0.0f;

    // Inverse square falloff
    float Alpha = 1.0f - (Distance / TriggerRadius);
    return Alpha * Alpha;
}
