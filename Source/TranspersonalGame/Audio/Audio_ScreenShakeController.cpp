#include "Audio_ScreenShakeController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/CameraShakeSourceComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UAudio_ScreenShakeController::UAudio_ScreenShakeController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // Initialize default footstep shake parameters
    DefaultFootstepShake.Intensity = 0.3f;
    DefaultFootstepShake.Duration = 0.2f;
    DefaultFootstepShake.Frequency = 15.0f;
    DefaultFootstepShake.FalloffRadius = 800.0f;
    DefaultFootstepShake.bUseDistanceAttenuation = true;

    // Initialize T-Rex approach shake parameters
    TRexApproachShake.Intensity = 1.5f;
    TRexApproachShake.Duration = 1.0f;
    TRexApproachShake.Frequency = 8.0f;
    TRexApproachShake.FalloffRadius = 1500.0f;
    TRexApproachShake.bUseDistanceAttenuation = true;

    MaxShakeDistance = 2000.0f;
    bEnableScreenShake = true;
    ShakeCooldownTimer = 0.0f;
}

void UAudio_ScreenShakeController::BeginPlay()
{
    Super::BeginPlay();

    // Cache player controller reference
    if (UWorld* World = GetWorld())
    {
        CachedPlayerController = UGameplayStatics::GetPlayerController(World, 0);
        if (CachedPlayerController)
        {
            UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: Player controller cached successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeController: Failed to cache player controller"));
        }
    }
}

void UAudio_ScreenShakeController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update shake cooldown timer
    if (ShakeCooldownTimer > 0.0f)
    {
        ShakeCooldownTimer -= DeltaTime;
    }

    // Update active shake durations
    for (int32 i = ActiveShakeDurations.Num() - 1; i >= 0; i--)
    {
        ActiveShakeDurations[i] -= DeltaTime;
        if (ActiveShakeDurations[i] <= 0.0f)
        {
            ActiveShakeDurations.RemoveAt(i);
        }
    }
}

void UAudio_ScreenShakeController::TriggerDinosaurFootstepShake(const FVector& FootstepLocation, float DinosaurSize)
{
    if (!bEnableScreenShake || ShakeCooldownTimer > 0.0f)
    {
        return;
    }

    // Scale shake parameters based on dinosaur size
    FAudio_ShakeParameters ScaledShake = DefaultFootstepShake;
    ScaledShake.Intensity *= DinosaurSize;
    ScaledShake.Duration *= FMath::Clamp(DinosaurSize, 0.5f, 2.0f);
    ScaledShake.FalloffRadius *= DinosaurSize;

    ApplyScreenShake(ScaledShake, FootstepLocation);
    
    // Set cooldown to prevent shake spam
    ShakeCooldownTimer = SHAKE_COOLDOWN;

    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: Dinosaur footstep shake triggered - Size: %f"), DinosaurSize);
}

void UAudio_ScreenShakeController::TriggerTRexApproachShake(const FVector& TRexLocation)
{
    if (!bEnableScreenShake)
    {
        return;
    }

    ApplyScreenShake(TRexApproachShake, TRexLocation);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: T-Rex approach shake triggered"));
}

void UAudio_ScreenShakeController::TriggerCustomShake(const FAudio_ShakeParameters& ShakeParams, const FVector& SourceLocation)
{
    if (!bEnableScreenShake)
    {
        return;
    }

    ApplyScreenShake(ShakeParams, SourceLocation);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: Custom shake triggered - Intensity: %f"), ShakeParams.Intensity);
}

void UAudio_ScreenShakeController::StopAllShakes()
{
    if (CachedPlayerController)
    {
        CachedPlayerController->ClientStopCameraShake(nullptr, true);
        ActiveShakeDurations.Empty();
        UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: All screen shakes stopped"));
    }
}

void UAudio_ScreenShakeController::ApplyScreenShake(const FAudio_ShakeParameters& ShakeParams, const FVector& SourceLocation)
{
    if (!CachedPlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeController: No player controller available for screen shake"));
        return;
    }

    // Calculate distance-based intensity
    float FinalIntensity = ShakeParams.Intensity;
    if (ShakeParams.bUseDistanceAttenuation)
    {
        FinalIntensity = CalculateShakeIntensity(SourceLocation, ShakeParams.Intensity, ShakeParams.FalloffRadius);
    }

    // Don't apply shake if intensity is too low
    if (FinalIntensity < 0.01f)
    {
        return;
    }

    // Create and apply camera shake
    // Note: Using legacy camera shake approach for compatibility
    if (UWorld* World = GetWorld())
    {
        // Create a simple camera shake effect using player controller
        FVector PlayerLocation = CachedPlayerController->GetPawn() ? 
            CachedPlayerController->GetPawn()->GetActorLocation() : 
            FVector::ZeroVector;

        // Apply shake through player controller's camera manager
        if (CachedPlayerController->PlayerCameraManager)
        {
            // Use a simple shake implementation
            float ShakeScale = FMath::Clamp(FinalIntensity, 0.0f, 2.0f);
            
            // Apply the shake (simplified approach)
            CachedPlayerController->PlayerCameraManager->StartCameraShake(
                nullptr, // Use default shake class
                ShakeScale
            );

            // Track active shake
            ActiveShakeDurations.Add(ShakeParams.Duration);

            UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: Screen shake applied - Intensity: %f, Duration: %f"), 
                FinalIntensity, ShakeParams.Duration);
        }
    }
}

float UAudio_ScreenShakeController::CalculateShakeIntensity(const FVector& SourceLocation, float BaseIntensity, float FalloffRadius)
{
    if (!CachedPlayerController || !CachedPlayerController->GetPawn())
    {
        return 0.0f;
    }

    FVector PlayerLocation = CachedPlayerController->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, SourceLocation);

    // Return 0 if beyond max shake distance
    if (Distance > MaxShakeDistance)
    {
        return 0.0f;
    }

    // Calculate falloff
    if (Distance <= FalloffRadius)
    {
        return BaseIntensity;
    }

    // Linear falloff beyond falloff radius
    float FalloffFactor = 1.0f - ((Distance - FalloffRadius) / (MaxShakeDistance - FalloffRadius));
    return BaseIntensity * FMath::Clamp(FalloffFactor, 0.0f, 1.0f);
}