#include "AudioFeedbackSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

AAudioFeedbackSystem::AAudioFeedbackSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Check every 100ms for performance

    // Default T-Rex shake config — strong shake at close range
    TRexShakeConfig.ShakeIntensity = 2.5f;
    TRexShakeConfig.ShakeDuration = 0.8f;
    TRexShakeConfig.TriggerRadius = 4000.0f;
    TRexShakeConfig.bFalloffWithDistance = true;

    ShakeCooldown = 2.0f;
    DinoProximityCheckInterval = 0.5f;
    TimeSinceLastProximityCheck = 0.0f;
    LastShakeTime = -999.0f;
    bIsLowHealthWarningActive = false;
}

void AAudioFeedbackSystem::BeginPlay()
{
    Super::BeginPlay();

    // Create persistent audio components for looping sounds
    if (LowHealthHeartbeatSound)
    {
        HeartbeatAudioComponent = UGameplayStatics::SpawnSoundAttached(
            LowHealthHeartbeatSound,
            GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            EAttachLocation::KeepRelativeOffset,
            false,
            0.0f // Start silent
        );
        if (HeartbeatAudioComponent)
        {
            HeartbeatAudioComponent->SetVolumeMultiplier(0.0f);
        }
    }

    if (CampfireCrackleSound)
    {
        CampfireAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
            GetWorld(),
            CampfireCrackleSound,
            FVector(500.0f, 500.0f, 100.0f), // Campfire base position
            FRotator::ZeroRotator,
            1.0f,
            1.0f,
            0.0f
        );
    }
}

void AAudioFeedbackSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastProximityCheck += DeltaTime;
    if (TimeSinceLastProximityCheck >= DinoProximityCheckInterval)
    {
        TimeSinceLastProximityCheck = 0.0f;
        UpdateDinoProximityAudio(DeltaTime);
    }
}

void AAudioFeedbackSystem::TriggerTRexFootstepShake(float DistanceToTRex)
{
    if (!GetWorld()) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastShakeTime < ShakeCooldown) return;

    if (DistanceToTRex > TRexShakeConfig.TriggerRadius) return;

    float Intensity = GetShakeIntensityByDistance(DistanceToTRex, TRexShakeConfig.TriggerRadius);
    Intensity *= TRexShakeConfig.ShakeIntensity;

    ApplyCameraShake(Intensity, TRexShakeConfig.ShakeDuration);
    LastShakeTime = CurrentTime;

    // Play footstep sound at T-Rex location
    if (TRexFootstepSound && DistanceToTRex < TRexShakeConfig.TriggerRadius)
    {
        float SoundVolume = FMath::Clamp(1.0f - (DistanceToTRex / TRexShakeConfig.TriggerRadius), 0.1f, 1.0f);
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            TRexFootstepSound,
            GetActorLocation(), // Approximate — actual T-Rex location passed by caller
            SoundVolume,
            FMath::RandRange(0.85f, 1.0f) // Slight pitch variation per step
        );
    }
}

void AAudioFeedbackSystem::TriggerStampedeShake(float Intensity)
{
    if (!GetWorld()) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastShakeTime < ShakeCooldown * 0.5f) return;

    ApplyCameraShake(Intensity * 3.0f, 2.0f);
    LastShakeTime = CurrentTime;

    if (StampedeRumbleSound)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), StampedeRumbleSound, Intensity, 1.0f);
    }
}

void AAudioFeedbackSystem::PlayDamageFeedback(float DamageAmount, float CurrentHealth, float MaxHealth)
{
    if (!GetWorld()) return;

    // Play hit sound
    if (DamageHitSound)
    {
        float HitVolume = FMath::Clamp(DamageAmount / 50.0f, 0.3f, 1.0f);
        UGameplayStatics::PlaySound2D(GetWorld(), DamageHitSound, HitVolume, 1.0f);
    }

    // Apply damage camera shake — more intense for heavier hits
    float ShakeIntensity = FMath::Clamp(DamageAmount / 30.0f, 0.3f, 2.0f);
    ApplyCameraShake(ShakeIntensity, 0.3f);

    // Activate heartbeat if health is low (below 30%)
    float HealthPercent = (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
    bool bShouldHeartbeat = HealthPercent < 0.3f;

    if (bShouldHeartbeat != bIsLowHealthWarningActive)
    {
        bIsLowHealthWarningActive = bShouldHeartbeat;
        if (HeartbeatAudioComponent)
        {
            float TargetVolume = bShouldHeartbeat ? 1.0f : 0.0f;
            HeartbeatAudioComponent->SetVolumeMultiplier(TargetVolume);
            // Increase heartbeat rate as health drops further
            float HeartbeatPitch = FMath::Lerp(1.0f, 1.8f, 1.0f - HealthPercent / 0.3f);
            HeartbeatAudioComponent->SetPitchMultiplier(HeartbeatPitch);
        }
    }
}

void AAudioFeedbackSystem::PlayDeathAudio()
{
    if (!GetWorld()) return;

    // Stop heartbeat
    if (HeartbeatAudioComponent)
    {
        HeartbeatAudioComponent->SetVolumeMultiplier(0.0f);
        HeartbeatAudioComponent->Stop();
    }

    // Strong death shake
    ApplyCameraShake(3.0f, 1.5f);

    // Slow down all audio (time dilation effect)
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.3f);

    // Reset time dilation after 1.5 seconds
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
    {
        if (GetWorld())
        {
            UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
        }
    }, 1.5f, false);
}

void AAudioFeedbackSystem::TriggerFeedbackEvent(FAudio_FeedbackEvent Event)
{
    if (!GetWorld()) return;

    switch (Event.FeedbackType)
    {
        case EAudio_FeedbackType::TRexRoar:
            TriggerTRexFootstepShake(Event.WorldLocation.Size());
            break;

        case EAudio_FeedbackType::HerdStampede:
            TriggerStampedeShake(Event.Volume);
            break;

        case EAudio_FeedbackType::PlayerDamage:
            PlayDamageFeedback(Event.Volume * 50.0f, 50.0f, 100.0f);
            break;

        case EAudio_FeedbackType::PlayerDeath:
            PlayDeathAudio();
            break;

        case EAudio_FeedbackType::FireLit:
            if (CampfireAudioComponent)
            {
                CampfireAudioComponent->SetVolumeMultiplier(1.0f);
            }
            break;

        case EAudio_FeedbackType::DinoNearby:
            // Subtle tension — reduce music, increase ambient
            ApplyCameraShake(0.2f, 0.1f);
            break;

        default:
            break;
    }
}

void AAudioFeedbackSystem::UpdateDinoProximityAudio(float DeltaTime)
{
    if (!GetWorld()) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn()) return;

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();

    // T-Rex reference position (from MinPlayableMap spawn)
    FVector TRexLocation(2000.0f, 2500.0f, 400.0f);
    float DistToTRex = FVector::Dist(PlayerLocation, TRexLocation);

    if (DistToTRex < TRexShakeConfig.TriggerRadius)
    {
        // Trigger periodic footstep shake as if T-Rex is walking
        float CurrentTime = GetWorld()->GetTimeSeconds();
        float FootstepInterval = FMath::Lerp(1.5f, 4.0f, DistToTRex / TRexShakeConfig.TriggerRadius);
        if (CurrentTime - LastShakeTime >= FootstepInterval)
        {
            TriggerTRexFootstepShake(DistToTRex);
        }
    }

    // Raptor territory check
    FVector RaptorLocation(2400.0f, 2500.0f, 400.0f);
    float DistToRaptors = FVector::Dist(PlayerLocation, RaptorLocation);
    if (DistToRaptors < 2000.0f && RaptorScreechSound)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastShakeTime > 8.0f) // Occasional screech
        {
            float ScreechVolume = FMath::Clamp(1.0f - (DistToRaptors / 2000.0f), 0.1f, 0.8f);
            UGameplayStatics::PlaySoundAtLocation(
                GetWorld(),
                RaptorScreechSound,
                RaptorLocation,
                ScreechVolume,
                FMath::RandRange(0.9f, 1.1f)
            );
        }
    }
}

float AAudioFeedbackSystem::GetShakeIntensityByDistance(float Distance, float MaxRadius) const
{
    if (MaxRadius <= 0.0f) return 0.0f;
    float NormalizedDist = FMath::Clamp(Distance / MaxRadius, 0.0f, 1.0f);
    // Exponential falloff — very strong close, drops quickly with distance
    return FMath::Pow(1.0f - NormalizedDist, 2.0f);
}

void AAudioFeedbackSystem::ApplyCameraShake(float Intensity, float Duration)
{
    if (!GetWorld()) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    // Use UE5's built-in camera shake system
    // The shake class would be set in Blueprint — here we use the base class
    // In production, assign a UCameraShakeBase subclass via Blueprint
    PC->ClientStartCameraShake(UCameraShakeBase::StaticClass(), Intensity);
}
