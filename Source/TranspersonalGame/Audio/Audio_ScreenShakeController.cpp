#include "Audio_ScreenShakeController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Camera/CameraShakeSourceComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

AAudio_ScreenShakeController::AAudio_ScreenShakeController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create camera shake source component
    CameraShakeSource = CreateDefaultSubobject<UCameraShakeSourceComponent>(TEXT("CameraShakeSource"));
    RootComponent = CameraShakeSource;

    // Create audio component for shake-related sounds
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    // Initialize default values
    GlobalShakeMultiplier = 1.0f;
    MaxShakeDistance = 5000.0f;
    bEnableScreenShake = true;
    TRexShakeRadius = 2000.0f;
    TRexShakeIntensity = 3.0f;
    TrackedActor = nullptr;
    LastProximityDistance = 0.0f;

    InitializeShakeProfiles();
}

void AAudio_ScreenShakeController::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: System initialized"));
}

void AAudio_ScreenShakeController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAudio_ScreenShakeController::TriggerScreenShake(EAudio_ShakeIntensity Intensity, FVector SourceLocation)
{
    if (!bEnableScreenShake)
        return;

    const FAudio_ShakeProfile* Profile = ShakeProfiles.Find(Intensity);
    if (Profile)
    {
        ApplyScreenShake(*Profile, SourceLocation);
        UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: Triggered shake intensity %d at location %s"), 
               (int32)Intensity, *SourceLocation.ToString());
    }
}

void AAudio_ScreenShakeController::TriggerCustomShake(const FAudio_ShakeProfile& ShakeProfile, FVector SourceLocation)
{
    if (!bEnableScreenShake)
        return;

    ApplyScreenShake(ShakeProfile, SourceLocation);
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: Triggered custom shake at location %s"), 
           *SourceLocation.ToString());
}

void AAudio_ScreenShakeController::TriggerTRexFootstep(FVector TRexLocation)
{
    if (!bEnableScreenShake)
        return;

    // Check if player is within T-Rex shake radius
    if (!IsPlayerNearby(TRexLocation, TRexShakeRadius))
        return;

    // Create custom T-Rex footstep shake profile
    FAudio_ShakeProfile TRexProfile;
    TRexProfile.Amplitude = TRexShakeIntensity * GlobalShakeMultiplier;
    TRexProfile.Frequency = 8.0f;
    TRexProfile.Duration = 0.8f;
    TRexProfile.FalloffRadius = TRexShakeRadius;
    TRexProfile.bUseFalloff = true;

    ApplyScreenShake(TRexProfile, TRexLocation);
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: T-Rex footstep shake at %s"), 
           *TRexLocation.ToString());
}

void AAudio_ScreenShakeController::TriggerDinosaurRoar(FVector DinosaurLocation, float DinosaurSize)
{
    if (!bEnableScreenShake)
        return;

    // Scale shake based on dinosaur size
    FAudio_ShakeProfile RoarProfile;
    RoarProfile.Amplitude = DinosaurSize * 0.8f * GlobalShakeMultiplier;
    RoarProfile.Frequency = 12.0f;
    RoarProfile.Duration = 1.2f;
    RoarProfile.FalloffRadius = DinosaurSize * 1500.0f;
    RoarProfile.bUseFalloff = true;

    ApplyScreenShake(RoarProfile, DinosaurLocation);
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: Dinosaur roar shake (size: %.2f) at %s"), 
           DinosaurSize, *DinosaurLocation.ToString());
}

void AAudio_ScreenShakeController::TriggerEnvironmentalShake(FVector SourceLocation, float Magnitude)
{
    if (!bEnableScreenShake)
        return;

    FAudio_ShakeProfile EnvProfile;
    EnvProfile.Amplitude = Magnitude * GlobalShakeMultiplier;
    EnvProfile.Frequency = 6.0f;
    EnvProfile.Duration = 2.0f;
    EnvProfile.FalloffRadius = MaxShakeDistance;
    EnvProfile.bUseFalloff = true;

    ApplyScreenShake(EnvProfile, SourceLocation);
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: Environmental shake (magnitude: %.2f) at %s"), 
           Magnitude, *SourceLocation.ToString());
}

void AAudio_ScreenShakeController::StartProximityShake(AActor* TargetActor, float UpdateInterval)
{
    if (!TargetActor || !bEnableScreenShake)
        return;

    TrackedActor = TargetActor;
    
    GetWorldTimerManager().SetTimer(
        ProximityTimerHandle,
        this,
        &AAudio_ScreenShakeController::UpdateProximityShake,
        UpdateInterval,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: Started proximity shake tracking for %s"), 
           *TargetActor->GetName());
}

void AAudio_ScreenShakeController::StopProximityShake()
{
    GetWorldTimerManager().ClearTimer(ProximityTimerHandle);
    TrackedActor = nullptr;
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: Stopped proximity shake tracking"));
}

void AAudio_ScreenShakeController::UpdateProximityShake()
{
    if (!TrackedActor || !bEnableScreenShake)
    {
        StopProximityShake();
        return;
    }

    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController || !PlayerController->GetPawn())
        return;

    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    FVector TrackedLocation = TrackedActor->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, TrackedLocation);

    // Only trigger shake if distance changed significantly
    if (FMath::Abs(Distance - LastProximityDistance) > 50.0f)
    {
        float ShakeIntensity = CalculateShakeIntensity(Distance, TRexShakeRadius);
        
        if (ShakeIntensity > 0.1f)
        {
            FAudio_ShakeProfile ProximityProfile;
            ProximityProfile.Amplitude = ShakeIntensity * GlobalShakeMultiplier;
            ProximityProfile.Frequency = 5.0f;
            ProximityProfile.Duration = 0.3f;
            ProximityProfile.FalloffRadius = TRexShakeRadius;
            ProximityProfile.bUseFalloff = false;

            ApplyScreenShake(ProximityProfile, TrackedLocation);
        }

        LastProximityDistance = Distance;
    }
}

float AAudio_ScreenShakeController::CalculateShakeIntensity(float Distance, float MaxDistance) const
{
    if (Distance >= MaxDistance)
        return 0.0f;

    return FMath::Clamp(1.0f - (Distance / MaxDistance), 0.0f, 1.0f);
}

void AAudio_ScreenShakeController::ApplyScreenShake(const FAudio_ShakeProfile& Profile, FVector Location)
{
    if (!CameraShakeSource)
        return;

    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController)
        return;

    // Calculate distance-based intensity if falloff is enabled
    float FinalAmplitude = Profile.Amplitude;
    
    if (Profile.bUseFalloff && PlayerController->GetPawn())
    {
        FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
        float Distance = FVector::Dist(PlayerLocation, Location);
        float IntensityMultiplier = CalculateShakeIntensity(Distance, Profile.FalloffRadius);
        FinalAmplitude *= IntensityMultiplier;
    }

    if (FinalAmplitude > 0.1f)
    {
        // Apply camera shake using the source component
        CameraShakeSource->Start();
        
        // Log the shake application
        UE_LOG(LogTemp, VeryVerbose, TEXT("Audio_ScreenShakeController: Applied shake - Amplitude: %.2f, Duration: %.2f"), 
               FinalAmplitude, Profile.Duration);
    }
}

void AAudio_ScreenShakeController::InitializeShakeProfiles()
{
    // Light shake
    FAudio_ShakeProfile LightProfile;
    LightProfile.Amplitude = 0.3f;
    LightProfile.Frequency = 15.0f;
    LightProfile.Duration = 0.5f;
    LightProfile.FalloffRadius = 800.0f;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Light, LightProfile);

    // Medium shake
    FAudio_ShakeProfile MediumProfile;
    MediumProfile.Amplitude = 0.8f;
    MediumProfile.Frequency = 12.0f;
    MediumProfile.Duration = 1.0f;
    MediumProfile.FalloffRadius = 1200.0f;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Medium, MediumProfile);

    // Heavy shake
    FAudio_ShakeProfile HeavyProfile;
    HeavyProfile.Amplitude = 1.5f;
    HeavyProfile.Frequency = 8.0f;
    HeavyProfile.Duration = 1.5f;
    HeavyProfile.FalloffRadius = 1800.0f;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Heavy, HeavyProfile);

    // Extreme shake
    FAudio_ShakeProfile ExtremeProfile;
    ExtremeProfile.Amplitude = 2.5f;
    ExtremeProfile.Frequency = 6.0f;
    ExtremeProfile.Duration = 2.0f;
    ExtremeProfile.FalloffRadius = 2500.0f;
    ShakeProfiles.Add(EAudio_ShakeIntensity::Extreme, ExtremeProfile);

    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: Shake profiles initialized"));
}

bool AAudio_ScreenShakeController::IsPlayerNearby(FVector Location, float Radius) const
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController || !PlayerController->GetPawn())
        return false;

    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, Location);
    
    return Distance <= Radius;
}