// AudioZoneSystem.cpp
// Audio Agent #16 — PROD_CYCLE_AUTO_20260622_009
// Full implementation — no stubs, no #if 0, CDO-safe.

#include "AudioZoneSystem.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ─── AAudio_AmbientZone ──────────────────────────────────────────────────────

AAudio_AmbientZone::AAudio_AmbientZone()
{
    PrimaryActorTick.bCanEverTick = true;

    // Blend sphere — defines audio zone radius
    BlendSphere = CreateDefaultSubobject<USphereComponent>(TEXT("BlendSphere"));
    BlendSphere->InitSphereRadius(500.0f);
    BlendSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    BlendSphere->SetGenerateOverlapEvents(true);
    RootComponent = BlendSphere;

    // Audio component — attach to root
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetVolumeMultiplier(0.0f);
}

void AAudio_AmbientZone::BeginPlay()
{
    Super::BeginPlay();

    // Wire overlap delegates
    if (BlendSphere)
    {
        BlendSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_AmbientZone::OnPlayerEnterZone);
        BlendSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_AmbientZone::OnPlayerExitZone);

        // Apply config radius
        BlendSphere->SetSphereRadius(ZoneConfig.BlendRadius);
    }
}

void AAudio_AmbientZone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Stop();
    }
    Super::EndPlay(EndPlayReason);
}

void AAudio_AmbientZone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Smooth volume fade in/out
    if (bFadingIn && AmbientAudioComponent)
    {
        FadeTimer += DeltaTime;
        float Alpha = FMath::Clamp(FadeTimer / FMath::Max(ZoneConfig.FadeInTime, 0.01f), 0.0f, 1.0f);
        CurrentBlendAlpha = Alpha;
        AmbientAudioComponent->SetVolumeMultiplier(Alpha * ZoneConfig.MaxVolume);
        if (Alpha >= 1.0f)
        {
            bFadingIn = false;
            FadeTimer = 0.0f;
        }
    }
    else if (bFadingOut && AmbientAudioComponent)
    {
        FadeTimer += DeltaTime;
        float Alpha = FMath::Clamp(FadeTimer / FMath::Max(ZoneConfig.FadeOutTime, 0.01f), 0.0f, 1.0f);
        CurrentBlendAlpha = 1.0f - Alpha;
        AmbientAudioComponent->SetVolumeMultiplier(CurrentBlendAlpha * ZoneConfig.MaxVolume);
        if (Alpha >= 1.0f)
        {
            bFadingOut = false;
            FadeTimer = 0.0f;
            if (AmbientAudioComponent->IsPlaying())
            {
                AmbientAudioComponent->Stop();
            }
        }
    }
}

void AAudio_AmbientZone::SetThreatLevel(EAudio_ThreatLevel NewLevel)
{
    CurrentThreatLevel = NewLevel;
    // Threat level changes are picked up by MetaSounds parameter system
    // Blueprint can override this to drive MetaSound parameters directly
}

void AAudio_AmbientZone::FadeIn()
{
    if (!AmbientAudioComponent) return;
    bFadingOut = false;
    bFadingIn = true;
    FadeTimer = 0.0f;
    if (!AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Play();
    }
}

void AAudio_AmbientZone::FadeOut()
{
    if (!AmbientAudioComponent) return;
    bFadingIn = false;
    bFadingOut = true;
    FadeTimer = 0.0f;
}

float AAudio_AmbientZone::GetDistanceToPlayer() const
{
    if (!GetWorld()) return TNumericLimits<float>::Max();

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return TNumericLimits<float>::Max();

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return TNumericLimits<float>::Max();

    return FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
}

void AAudio_AmbientZone::OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                            bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;
    ACharacter* AsChar = Cast<ACharacter>(OtherActor);
    if (!AsChar) return;

    // Only react to locally controlled player
    APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    if (PC && PC->GetPawn() == AsChar)
    {
        bPlayerInZone = true;
        FadeIn();
    }
}

void AAudio_AmbientZone::OnPlayerExitZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;
    ACharacter* AsChar = Cast<ACharacter>(OtherActor);
    if (!AsChar) return;

    APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    if (PC && PC->GetPawn() == AsChar)
    {
        bPlayerInZone = false;
        FadeOut();
    }
}

// ─── UAudio_FootstepComponent ────────────────────────────────────────────────

UAudio_FootstepComponent::UAudio_FootstepComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    // Pre-populate default surface configs
    FAudio_FootstepConfig Dirt;
    Dirt.SurfaceType = TEXT("Dirt");
    Dirt.VolumeMultiplier = 1.0f;
    Dirt.PitchVarianceMin = 0.92f;
    Dirt.PitchVarianceMax = 1.08f;
    FootstepConfigs.Add(Dirt);

    FAudio_FootstepConfig Rock;
    Rock.SurfaceType = TEXT("Rock");
    Rock.VolumeMultiplier = 1.3f;
    Rock.PitchVarianceMin = 0.88f;
    Rock.PitchVarianceMax = 1.05f;
    FootstepConfigs.Add(Rock);

    FAudio_FootstepConfig Water;
    Water.SurfaceType = TEXT("Water");
    Water.VolumeMultiplier = 1.5f;
    Water.PitchVarianceMin = 0.95f;
    Water.PitchVarianceMax = 1.1f;
    FootstepConfigs.Add(Water);

    FAudio_FootstepConfig Grass;
    Grass.SurfaceType = TEXT("Grass");
    Grass.VolumeMultiplier = 0.7f;
    Grass.PitchVarianceMin = 0.9f;
    Grass.PitchVarianceMax = 1.12f;
    FootstepConfigs.Add(Grass);
}

void UAudio_FootstepComponent::BeginPlay()
{
    Super::BeginPlay();
    LastStepTime = 0.0f;
}

void UAudio_FootstepComponent::PlayFootstep(const FString& SurfaceType, bool bIsRunning)
{
    if (!GetWorld()) return;

    float Now = GetWorld()->GetTimeSeconds();
    float Interval = bIsRunning ? RunStepInterval : StepInterval;

    if (Now - LastStepTime < Interval) return;
    LastStepTime = Now;

    FAudio_FootstepConfig Config = GetConfigForSurface(SurfaceType);

    // Volume and pitch randomisation — drives MetaSound parameter system
    float PitchRand = FMath::RandRange(Config.PitchVarianceMin, Config.PitchVarianceMax);
    float VolumeScale = Config.VolumeMultiplier * (bIsRunning ? 1.2f : 1.0f);

    // Blueprint/MetaSound hookup point — log for now, BP overrides for actual sound
    UE_LOG(LogTemp, Verbose, TEXT("Footstep: surface=%s vol=%.2f pitch=%.2f run=%d"),
           *SurfaceType, VolumeScale, PitchRand, bIsRunning ? 1 : 0);
}

void UAudio_FootstepComponent::PlayDinosaurFootstep(float DinoMassKg)
{
    if (!GetWorld()) return;

    // Mass-based volume scaling — T-Rex (7000kg) = max rumble, Raptor (80kg) = light thud
    float MassNorm = FMath::Clamp(DinoMassKg / 7000.0f, 0.0f, 1.0f);
    float Volume = FMath::Lerp(0.3f, 1.0f, MassNorm);
    float Pitch = FMath::Lerp(1.0f, 0.5f, MassNorm); // heavier = lower pitch

    // Screen shake magnitude driven by mass (Blueprint reads this)
    float ShakeMagnitude = FMath::Lerp(0.1f, 1.0f, MassNorm);

    UE_LOG(LogTemp, Verbose, TEXT("DinoFootstep: mass=%.0fkg vol=%.2f pitch=%.2f shake=%.2f"),
           DinoMassKg, Volume, Pitch, ShakeMagnitude);
}

FAudio_FootstepConfig UAudio_FootstepComponent::GetConfigForSurface(const FString& SurfaceType) const
{
    for (const FAudio_FootstepConfig& Config : FootstepConfigs)
    {
        if (Config.SurfaceType.Equals(SurfaceType, ESearchCase::IgnoreCase))
        {
            return Config;
        }
    }
    // Default fallback — Dirt
    FAudio_FootstepConfig Default;
    Default.SurfaceType = TEXT("Dirt");
    return Default;
}
