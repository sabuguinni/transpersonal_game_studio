#include "Audio_ScreenShakeController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraShakeBase.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"

AAudio_ScreenShakeController::AAudio_ScreenShakeController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize T-Rex shake settings
    TRexShakeSettings.Intensity = 2.0f;
    TRexShakeSettings.Duration = 1.0f;
    TRexShakeSettings.Falloff = 2000.0f;
    TRexShakeSettings.bOrientShakeTowardsEpicenter = true;

    // Initialize damage shake settings
    DamageShakeSettings.Intensity = 1.5f;
    DamageShakeSettings.Duration = 0.3f;
    DamageShakeSettings.Falloff = 500.0f;
    DamageShakeSettings.bOrientShakeTowardsEpicenter = false;

    // Initialize generic shake settings
    GenericShakeSettings.Intensity = 1.0f;
    GenericShakeSettings.Duration = 0.5f;
    GenericShakeSettings.Falloff = 1000.0f;
    GenericShakeSettings.bOrientShakeTowardsEpicenter = true;
}

void AAudio_ScreenShakeController::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeController: BeginPlay - Screen shake system initialized"));
}

void AAudio_ScreenShakeController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAudio_ScreenShakeController::TriggerProximityShake(FVector SourceLocation, float MaxDistance, float BaseIntensity)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("Audio_ScreenShakeController: No valid world found"));
        return;
    }

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->GetPawn())
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeController: No valid player controller or pawn found"));
        return;
    }

    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(SourceLocation, PlayerLocation);

    if (Distance <= MaxDistance)
    {
        float ShakeIntensity = CalculateShakeIntensity(SourceLocation, PlayerLocation, MaxDistance, BaseIntensity);
        
        FAudio_ShakeSettings CustomShake = GenericShakeSettings;
        CustomShake.Intensity = ShakeIntensity;
        
        ApplyCameraShake(CustomShake, SourceLocation);
        
        UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: Proximity shake triggered - Distance: %f, Intensity: %f"), Distance, ShakeIntensity);
    }
}

void AAudio_ScreenShakeController::TriggerTRexFootstep(FVector FootstepLocation)
{
    ApplyCameraShake(TRexShakeSettings, FootstepLocation);
    
    // Play T-Rex footstep sound if available
    if (TRexFootstepSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), TRexFootstepSound, FootstepLocation);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: T-Rex footstep shake triggered at location: %s"), *FootstepLocation.ToString());
}

void AAudio_ScreenShakeController::TriggerDamageShake(float DamageAmount)
{
    FAudio_ShakeSettings CustomDamageShake = DamageShakeSettings;
    CustomDamageShake.Intensity = FMath::Clamp(DamageAmount * 0.1f, 0.5f, 3.0f);
    
    ApplyCameraShake(CustomDamageShake, GetActorLocation());
    
    // Play damage impact sound if available
    if (DamageImpactSound)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), DamageImpactSound);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: Damage shake triggered - Damage: %f, Intensity: %f"), DamageAmount, CustomDamageShake.Intensity);
}

float AAudio_ScreenShakeController::CalculateShakeIntensity(FVector SourceLocation, FVector PlayerLocation, float MaxDistance, float BaseIntensity)
{
    float Distance = FVector::Dist(SourceLocation, PlayerLocation);
    float DistanceRatio = FMath::Clamp(1.0f - (Distance / MaxDistance), 0.0f, 1.0f);
    
    // Use exponential falloff for more realistic shake intensity
    float IntensityMultiplier = FMath::Pow(DistanceRatio, 2.0f);
    
    return BaseIntensity * IntensityMultiplier;
}

void AAudio_ScreenShakeController::ApplyCameraShake(const FAudio_ShakeSettings& ShakeSettings, FVector EpicenterLocation)
{
    if (!GetWorld())
    {
        return;
    }

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
    {
        return;
    }

    // Create a simple camera shake using legacy shake system
    // Note: In UE5, we would typically use UCameraShakeSourceComponent for more advanced shakes
    if (PlayerController->PlayerCameraManager)
    {
        // Apply a simple shake using the camera manager
        PlayerController->PlayerCameraManager->StartCameraShake(
            nullptr, // ShakeClass - using nullptr for simple shake
            ShakeSettings.Intensity,
            ECameraShakePlaySpace::CameraLocal,
            FRotator::ZeroRotator
        );
        
        UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: Camera shake applied - Intensity: %f, Duration: %f"), 
               ShakeSettings.Intensity, ShakeSettings.Duration);
    }
}