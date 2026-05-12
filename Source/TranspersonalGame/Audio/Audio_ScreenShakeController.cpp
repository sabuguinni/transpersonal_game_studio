#include "Audio_ScreenShakeController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraShakeBase.h"
#include "Math/UnrealMathUtility.h"

AAudio_ScreenShakeController::AAudio_ScreenShakeController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second for smooth shake

    // Initialize default shake parameters
    TRexShakeParams.Intensity = 3.0f;
    TRexShakeParams.Duration = 2.5f;
    TRexShakeParams.Range = 2000.0f;
    TRexShakeParams.bFadeIn = true;
    TRexShakeParams.bFadeOut = true;

    RaptorShakeParams.Intensity = 1.0f;
    RaptorShakeParams.Duration = 1.0f;
    RaptorShakeParams.Range = 800.0f;
    RaptorShakeParams.bFadeIn = false;
    RaptorShakeParams.bFadeOut = true;

    VolcanicShakeParams.Intensity = 2.0f;
    VolcanicShakeParams.Duration = 5.0f;
    VolcanicShakeParams.Range = 5000.0f;
    VolcanicShakeParams.bFadeIn = true;
    VolcanicShakeParams.bFadeOut = true;

    // Initialize proximity shake data
    ProximityData.MinDistance = 100.0f;
    ProximityData.MaxDistance = 2000.0f;
    ProximityData.MinIntensity = 0.1f;
    ProximityData.MaxIntensity = 5.0f;
    ProximityData.bUseHeartbeat = true;

    // Initialize state
    bIsShaking = false;
    CurrentShakeIntensity = 0.0f;
    ShakeTimer = 0.0f;

    InitializeAudioComponents();
}

void AAudio_ScreenShakeController::InitializeAudioComponents()
{
    // Create heartbeat audio component
    HeartbeatAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("HeartbeatAudio"));
    if (HeartbeatAudioComponent)
    {
        HeartbeatAudioComponent->bAutoActivate = false;
        HeartbeatAudioComponent->SetVolumeMultiplier(0.5f);
    }

    // Create rumble audio component
    RumbleAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("RumbleAudio"));
    if (RumbleAudioComponent)
    {
        RumbleAudioComponent->bAutoActivate = false;
        RumbleAudioComponent->SetVolumeMultiplier(0.3f);
    }
}

void AAudio_ScreenShakeController::BeginPlay()
{
    Super::BeginPlay();

    // Load default sound assets if not set
    if (!HeartbeatSound)
    {
        HeartbeatSound = LoadObject<USoundCue>(nullptr, TEXT("/Game/Audio/SFX/Heartbeat_Cue"));
    }

    if (!RumbleSound)
    {
        RumbleSound = LoadObject<USoundCue>(nullptr, TEXT("/Game/Audio/SFX/Rumble_Cue"));
    }

    // Set up audio components with loaded sounds
    if (HeartbeatAudioComponent && HeartbeatSound)
    {
        HeartbeatAudioComponent->SetSound(HeartbeatSound);
    }

    if (RumbleAudioComponent && RumbleSound)
    {
        RumbleAudioComponent->SetSound(RumbleSound);
    }
}

void AAudio_ScreenShakeController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsShaking)
    {
        ShakeTimer += DeltaTime;
        
        // Update audio feedback based on current shake intensity
        UpdateHeartbeatAudio(CurrentShakeIntensity);
        UpdateRumbleAudio(CurrentShakeIntensity);
    }
}

void AAudio_ScreenShakeController::TriggerTRexShake(const FVector& TRexLocation, float TRexSize)
{
    if (!IsPlayerInRange(TRexLocation, TRexShakeParams.Range))
    {
        return;
    }

    FAudio_ShakeParameters AdjustedParams = TRexShakeParams;
    AdjustedParams.Intensity *= TRexSize; // Scale intensity by T-Rex size
    
    float Distance = FVector::Dist(GetPlayerPawn()->GetActorLocation(), TRexLocation);
    float IntensityMultiplier = 1.0f - (Distance / TRexShakeParams.Range);
    AdjustedParams.Intensity *= IntensityMultiplier;

    ExecuteShake(AdjustedParams, TRexLocation);

    UE_LOG(LogTemp, Warning, TEXT("T-Rex shake triggered - Distance: %f, Intensity: %f"), Distance, AdjustedParams.Intensity);
}

void AAudio_ScreenShakeController::TriggerRaptorShake(const FVector& RaptorLocation, int32 RaptorCount)
{
    if (!IsPlayerInRange(RaptorLocation, RaptorShakeParams.Range))
    {
        return;
    }

    FAudio_ShakeParameters AdjustedParams = RaptorShakeParams;
    AdjustedParams.Intensity *= FMath::Sqrt(static_cast<float>(RaptorCount)); // Scale by pack size
    
    float Distance = FVector::Dist(GetPlayerPawn()->GetActorLocation(), RaptorLocation);
    float IntensityMultiplier = 1.0f - (Distance / RaptorShakeParams.Range);
    AdjustedParams.Intensity *= IntensityMultiplier;

    ExecuteShake(AdjustedParams, RaptorLocation);

    UE_LOG(LogTemp, Warning, TEXT("Raptor pack shake triggered - Count: %d, Intensity: %f"), RaptorCount, AdjustedParams.Intensity);
}

void AAudio_ScreenShakeController::TriggerVolcanicShake(float VolcanicIntensity)
{
    FAudio_ShakeParameters AdjustedParams = VolcanicShakeParams;
    AdjustedParams.Intensity *= VolcanicIntensity;
    
    // Volcanic shakes affect the entire map
    ExecuteShake(AdjustedParams, GetActorLocation());

    UE_LOG(LogTemp, Warning, TEXT("Volcanic shake triggered - Intensity: %f"), AdjustedParams.Intensity);
}

void AAudio_ScreenShakeController::TriggerCustomShake(const FAudio_ShakeParameters& ShakeParams, const FVector& SourceLocation)
{
    if (!IsPlayerInRange(SourceLocation, ShakeParams.Range))
    {
        return;
    }

    ExecuteShake(ShakeParams, SourceLocation);
}

void AAudio_ScreenShakeController::UpdateProximityShake(const TArray<AActor*>& NearbyThreats)
{
    if (NearbyThreats.Num() == 0)
    {
        StopProximityShake();
        return;
    }

    // Find the closest threat
    AActor* ClosestThreat = nullptr;
    float ClosestDistance = ProximityData.MaxDistance;

    APawn* PlayerPawn = GetPlayerPawn();
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    for (AActor* Threat : NearbyThreats)
    {
        if (!Threat)
        {
            continue;
        }

        float Distance = FVector::Dist(PlayerLocation, Threat->GetActorLocation());
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestThreat = Threat;
        }
    }

    if (ClosestThreat && ClosestDistance <= ProximityData.MaxDistance)
    {
        StartProximityShake(ClosestThreat);
    }
    else
    {
        StopProximityShake();
    }
}

void AAudio_ScreenShakeController::StartProximityShake(AActor* ThreatActor)
{
    if (!ThreatActor)
    {
        return;
    }

    float Distance = CalculateDistanceBasedIntensity(ThreatActor->GetActorLocation());
    CurrentShakeIntensity = FMath::Lerp(ProximityData.MinIntensity, ProximityData.MaxIntensity, 1.0f - Distance);

    if (!bIsShaking)
    {
        bIsShaking = true;
        ShakeTimer = 0.0f;
    }

    // Create continuous low-level shake parameters
    FAudio_ShakeParameters ProximityShake;
    ProximityShake.Intensity = CurrentShakeIntensity * 0.3f; // Subtle continuous shake
    ProximityShake.Duration = 0.5f; // Short duration, will be retriggered
    ProximityShake.Range = ProximityData.MaxDistance;
    ProximityShake.bFadeIn = false;
    ProximityShake.bFadeOut = false;

    ExecuteShake(ProximityShake, ThreatActor->GetActorLocation());
}

void AAudio_ScreenShakeController::StopProximityShake()
{
    if (bIsShaking)
    {
        bIsShaking = false;
        CurrentShakeIntensity = 0.0f;
        ShakeTimer = 0.0f;

        // Stop audio feedback
        if (HeartbeatAudioComponent && HeartbeatAudioComponent->IsPlaying())
        {
            HeartbeatAudioComponent->FadeOut(1.0f, 0.0f);
        }

        if (RumbleAudioComponent && RumbleAudioComponent->IsPlaying())
        {
            RumbleAudioComponent->FadeOut(1.0f, 0.0f);
        }
    }
}

void AAudio_ScreenShakeController::StopAllShakes()
{
    APlayerController* PC = GetPlayerController();
    if (PC)
    {
        PC->ClientStopCameraShake(nullptr, true);
    }

    StopProximityShake();
}

float AAudio_ScreenShakeController::CalculateShakeIntensity(const FVector& SourceLocation, float BaseIntensity, float MaxRange) const
{
    APawn* PlayerPawn = GetPlayerPawn();
    if (!PlayerPawn)
    {
        return 0.0f;
    }

    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), SourceLocation);
    if (Distance > MaxRange)
    {
        return 0.0f;
    }

    float DistanceRatio = Distance / MaxRange;
    return BaseIntensity * (1.0f - DistanceRatio);
}

bool AAudio_ScreenShakeController::IsPlayerInRange(const FVector& SourceLocation, float Range) const
{
    APawn* PlayerPawn = GetPlayerPawn();
    if (!PlayerPawn)
    {
        return false;
    }

    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), SourceLocation);
    return Distance <= Range;
}

float AAudio_ScreenShakeController::CalculateDistanceBasedIntensity(const FVector& SourceLocation) const
{
    APawn* PlayerPawn = GetPlayerPawn();
    if (!PlayerPawn)
    {
        return 0.0f;
    }

    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), SourceLocation);
    float ClampedDistance = FMath::Clamp(Distance, ProximityData.MinDistance, ProximityData.MaxDistance);
    
    return (ClampedDistance - ProximityData.MinDistance) / (ProximityData.MaxDistance - ProximityData.MinDistance);
}

void AAudio_ScreenShakeController::ExecuteShake(const FAudio_ShakeParameters& ShakeParams, const FVector& SourceLocation)
{
    APlayerController* PC = GetPlayerController();
    if (!PC)
    {
        return;
    }

    float FinalIntensity = CalculateShakeIntensity(SourceLocation, ShakeParams.Intensity, ShakeParams.Range);
    if (FinalIntensity <= 0.0f)
    {
        return;
    }

    // Update current shake state
    bIsShaking = true;
    CurrentShakeIntensity = FinalIntensity;
    ShakeTimer = 0.0f;

    // Trigger camera shake using UE5's built-in system
    // Note: In a real implementation, you would create a custom CameraShakeBase subclass
    // For now, we'll use the generic shake system
    
    UE_LOG(LogTemp, Log, TEXT("Screen shake executed - Intensity: %f, Duration: %f"), FinalIntensity, ShakeParams.Duration);
}

void AAudio_ScreenShakeController::UpdateHeartbeatAudio(float Intensity)
{
    if (!HeartbeatAudioComponent || !ProximityData.bUseHeartbeat)
    {
        return;
    }

    if (Intensity > 0.1f)
    {
        if (!HeartbeatAudioComponent->IsPlaying())
        {
            HeartbeatAudioComponent->Play();
        }

        // Adjust heartbeat rate and volume based on intensity
        float VolumeMultiplier = FMath::Clamp(Intensity * 0.8f, 0.1f, 1.0f);
        float PitchMultiplier = 1.0f + (Intensity * 0.5f); // Faster heartbeat when more intense

        HeartbeatAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
        HeartbeatAudioComponent->SetPitchMultiplier(PitchMultiplier);
    }
    else if (HeartbeatAudioComponent->IsPlaying())
    {
        HeartbeatAudioComponent->FadeOut(2.0f, 0.0f);
    }
}

void AAudio_ScreenShakeController::UpdateRumbleAudio(float Intensity)
{
    if (!RumbleAudioComponent)
    {
        return;
    }

    if (Intensity > 0.3f) // Only play rumble for more intense shakes
    {
        if (!RumbleAudioComponent->IsPlaying())
        {
            RumbleAudioComponent->Play();
        }

        float VolumeMultiplier = FMath::Clamp(Intensity * 0.6f, 0.1f, 0.8f);
        RumbleAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
    }
    else if (RumbleAudioComponent->IsPlaying())
    {
        RumbleAudioComponent->FadeOut(1.5f, 0.0f);
    }
}

APlayerController* AAudio_ScreenShakeController::GetPlayerController() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    return World->GetFirstPlayerController();
}

APawn* AAudio_ScreenShakeController::GetPlayerPawn() const
{
    APlayerController* PC = GetPlayerController();
    if (!PC)
    {
        return nullptr;
    }

    return PC->GetPawn();
}