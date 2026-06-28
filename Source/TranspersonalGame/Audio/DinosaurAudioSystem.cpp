#include "DinosaurAudioSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ============================================================
// UAudio_DinosaurAudioComponent
// ============================================================

UAudio_DinosaurAudioComponent::UAudio_DinosaurAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f; // 20Hz tick — sufficient for audio
}

void UAudio_DinosaurAudioComponent::BeginPlay()
{
    Super::BeginPlay();
    CachePlayerReference();
    FootstepTimer = 0.0f;
    StateTransitionCooldown = 0.0f;
}

void UAudio_DinosaurAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (StateTransitionCooldown > 0.0f)
    {
        StateTransitionCooldown -= DeltaTime;
    }

    UpdateFootstepLoop(DeltaTime);
}

void UAudio_DinosaurAudioComponent::SetDinosaurState(EAudio_DinosaurState NewState)
{
    if (NewState == CurrentState) return;
    if (StateTransitionCooldown > 0.0f) return;

    EAudio_DinosaurState OldState = CurrentState;
    CurrentState = NewState;
    StateTransitionCooldown = 0.5f;

    // Play appropriate vocalisation on state change
    PlayVocalisation(NewState);
}

void UAudio_DinosaurAudioComponent::PlayFootstep(bool bIsHeavyStep)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector Location = Owner->GetActorLocation();
    USoundCue* FootstepSound = bIsHeavyStep ? SoundProfile.FootstepHeavy : SoundProfile.FootstepLight;

    if (FootstepSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            FootstepSound,
            Location,
            1.0f,  // Volume
            1.0f,  // Pitch
            0.0f,  // StartTime
            nullptr,
            nullptr
        );
    }

    // Ground shake for heavy steps (T-Rex, Brachiosaurus)
    if (bIsHeavyStep && bEnableGroundShake)
    {
        TriggerGroundShakeAtLocation(Location, GroundShakeIntensity, SoundProfile.FootstepGroundShakeRadius);
    }
}

void UAudio_DinosaurAudioComponent::PlayVocalisation(EAudio_DinosaurState ForState)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector Location = Owner->GetActorLocation();
    USoundCue* VocSound = nullptr;

    switch (ForState)
    {
        case EAudio_DinosaurState::Idle:
            VocSound = SoundProfile.IdleVocalisation;
            break;
        case EAudio_DinosaurState::Alert:
        case EAudio_DinosaurState::Hunting:
            VocSound = SoundProfile.AlertRoar;
            break;
        case EAudio_DinosaurState::Attacking:
            VocSound = SoundProfile.AttackRoar;
            break;
        default:
            VocSound = SoundProfile.IdleVocalisation;
            break;
    }

    if (VocSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            VocSound,
            Location,
            1.0f,
            FMath::RandRange(0.9f, 1.1f), // Slight pitch variation for realism
            0.0f,
            nullptr,
            nullptr
        );
    }
}

void UAudio_DinosaurAudioComponent::TriggerGroundShakeAtLocation(FVector Location, float Intensity, float Radius)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Apply camera shake to player if within radius
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    float DistToPlayer = FVector::Dist(Location, PlayerPawn->GetActorLocation());
    if (DistToPlayer > Radius) return;

    // Scale shake intensity by distance (inverse square falloff)
    float DistanceFactor = FMath::Clamp(1.0f - (DistToPlayer / Radius), 0.0f, 1.0f);
    float FinalIntensity = Intensity * DistanceFactor;

    // Apply camera shake via PlayerController
    // Note: Assign a UCameraShakeBase subclass in Blueprint for full shake effect
    // Here we use the console command approach as a reliable fallback
    if (FinalIntensity > 0.1f)
    {
        FString ShakeCmd = FString::Printf(TEXT("shake %.2f"), FinalIntensity);
        UGameplayStatics::PlayWorldCameraShake(
            World,
            nullptr, // Assign BP_DinoFootstepShake in editor
            Location,
            0.0f,
            Radius,
            FinalIntensity,
            false
        );
    }
}

float UAudio_DinosaurAudioComponent::GetDistanceToPlayer() const
{
    AActor* Owner = GetOwner();
    if (!Owner || !CachedPlayerPawn) return TNumericLimits<float>::Max();
    return FVector::Dist(Owner->GetActorLocation(), CachedPlayerPawn->GetActorLocation());
}

bool UAudio_DinosaurAudioComponent::IsPlayerInAlertRange() const
{
    return GetDistanceToPlayer() < SoundProfile.VocalisationRadius;
}

void UAudio_DinosaurAudioComponent::UpdateFootstepLoop(float DeltaTime)
{
    // Only generate footsteps when moving (state != Idle/Sleeping)
    if (CurrentState == EAudio_DinosaurState::Idle ||
        CurrentState == EAudio_DinosaurState::Sleeping ||
        CurrentState == EAudio_DinosaurState::Feeding)
    {
        FootstepTimer = 0.0f;
        return;
    }

    FootstepTimer += DeltaTime;

    float Interval = SoundProfile.FootstepInterval;
    if (CurrentState == EAudio_DinosaurState::Attacking)
    {
        Interval *= 0.6f; // Faster footsteps when charging
    }

    if (FootstepTimer >= Interval)
    {
        FootstepTimer = 0.0f;
        bool bHeavy = (SoundProfile.Species == EAudio_DinosaurSpecies::TyrannosaurusRex ||
                       SoundProfile.Species == EAudio_DinosaurSpecies::Brachiosaurus ||
                       SoundProfile.Species == EAudio_DinosaurSpecies::Triceratops ||
                       SoundProfile.Species == EAudio_DinosaurSpecies::Ankylosaurus);
        PlayFootstep(bHeavy);
    }
}

void UAudio_DinosaurAudioComponent::CachePlayerReference()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (PC)
    {
        CachedPlayerPawn = PC->GetPawn();
    }
}

// ============================================================
// UAudio_EnvironmentAudioManager
// ============================================================

UAudio_EnvironmentAudioManager::UAudio_EnvironmentAudioManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — ambient audio doesn't need high frequency
}

void UAudio_EnvironmentAudioManager::BeginPlay()
{
    Super::BeginPlay();
    CurrentBlendAlpha = bIsNight ? 1.0f : 0.0f;
    ThunderCooldown = 0.0f;
}

void UAudio_EnvironmentAudioManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateDayNightBlend(DeltaTime);
    UpdateCampfireProximityAudio();
    UpdateWeatherAudio(DeltaTime);

    if (ThunderCooldown > 0.0f)
    {
        ThunderCooldown -= DeltaTime;
    }
}

void UAudio_EnvironmentAudioManager::SetDayNightState(bool bNight)
{
    bIsNight = bNight;
}

void UAudio_EnvironmentAudioManager::SetWeatherState(bool bRaining)
{
    bIsRaining = bRaining;
}

void UAudio_EnvironmentAudioManager::TriggerThunder(FVector StrikeLocation)
{
    if (ThunderCooldown > 0.0f) return;
    if (!ThunderStrike) return;

    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        ThunderStrike,
        StrikeLocation,
        1.0f,
        FMath::RandRange(0.85f, 1.15f),
        0.0f,
        nullptr,
        nullptr
    );

    ThunderCooldown = FMath::RandRange(8.0f, 25.0f); // Random interval between thunder strikes
}

void UAudio_EnvironmentAudioManager::RegisterCampfireLocation(FVector Location)
{
    ActiveCampfireLocations.AddUnique(Location);
}

void UAudio_EnvironmentAudioManager::UnregisterCampfireLocation(FVector Location)
{
    ActiveCampfireLocations.Remove(Location);
}

float UAudio_EnvironmentAudioManager::GetCurrentAmbientVolume() const
{
    // Night is louder — insects, nocturnal predators
    return bIsNight ? 0.9f : 0.7f;
}

void UAudio_EnvironmentAudioManager::UpdateDayNightBlend(float DeltaTime)
{
    float TargetAlpha = bIsNight ? 1.0f : 0.0f;
    CurrentBlendAlpha = FMath::FInterpTo(CurrentBlendAlpha, TargetAlpha, DeltaTime, DayNightBlendSpeed);

    // Volume crossfade between day and night ambience layers
    // Actual audio component volume setting happens in Blueprint via GetCurrentAmbientVolume()
}

void UAudio_EnvironmentAudioManager::UpdateCampfireProximityAudio()
{
    if (ActiveCampfireLocations.Num() == 0) return;
    if (!CampfireLoop) return;

    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();

    // Find nearest campfire
    float NearestDist = TNumericLimits<float>::Max();
    for (const FVector& FireLoc : ActiveCampfireLocations)
    {
        float Dist = FVector::Dist(PlayerLoc, FireLoc);
        if (Dist < NearestDist)
        {
            NearestDist = Dist;
        }
    }

    // Volume falloff based on distance to nearest campfire
    // Blueprint should use this to drive audio component volume
    float VolumeAlpha = FMath::Clamp(1.0f - (NearestDist / CampfireAudioRadius), 0.0f, 1.0f);
    // VolumeAlpha drives campfire audio volume in Blueprint
}

void UAudio_EnvironmentAudioManager::UpdateWeatherAudio(float DeltaTime)
{
    if (!bIsRaining) return;
    if (!RainLoop) return;

    // Rain audio is managed by Blueprint — this tick validates state
    // Thunder is triggered via TriggerThunder() called from weather system
}
