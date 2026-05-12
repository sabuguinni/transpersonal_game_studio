#include "Audio_ScreenShakeController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

UAudio_ScreenShakeController::UAudio_ScreenShakeController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize T-Rex shake settings
    TRexShakeSettings.Intensity = 2.0f;
    TRexShakeSettings.Duration = 1.0f;
    TRexShakeSettings.Frequency = 8.0f;
    TRexShakeSettings.MaxDistance = 3000.0f;

    // Initialize damage shake settings
    DamageShakeSettings.Intensity = 1.5f;
    DamageShakeSettings.Duration = 0.3f;
    DamageShakeSettings.Frequency = 15.0f;
    DamageShakeSettings.MaxDistance = 100.0f;

    // Initialize footstep shake settings
    FootstepShakeSettings.Intensity = 0.8f;
    FootstepShakeSettings.Duration = 0.2f;
    FootstepShakeSettings.Frequency = 12.0f;
    FootstepShakeSettings.MaxDistance = 1500.0f;

    TRexDetectionRadius = 3000.0f;
    ShakeUpdateInterval = 0.1f;
    bAutoDetectTRex = true;
}

void UAudio_ScreenShakeController::BeginPlay()
{
    Super::BeginPlay();

    // Start T-Rex detection timer if auto-detection is enabled
    if (bAutoDetectTRex)
    {
        GetWorld()->GetTimerManager().SetTimer(
            TRexDetectionTimer,
            this,
            &UAudio_ScreenShakeController::UpdateTRexProximityShake,
            ShakeUpdateInterval,
            true
        );
    }
}

void UAudio_ScreenShakeController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Additional tick-based logic if needed
}

void UAudio_ScreenShakeController::TriggerTRexProximityShake(FVector TRexLocation, float TRexMass)
{
    if (!GetWorld() || !GetOwner())
    {
        return;
    }

    FVector PlayerLocation = GetOwner()->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, TRexLocation);

    if (Distance <= TRexShakeSettings.MaxDistance)
    {
        // Calculate intensity based on distance and mass
        float IntensityMultiplier = CalculateShakeIntensity(Distance, TRexShakeSettings.MaxDistance, TRexMass);
        
        FAudio_ShakeSettings ModifiedSettings = TRexShakeSettings;
        ModifiedSettings.Intensity *= IntensityMultiplier;

        ExecuteScreenShake(ModifiedSettings, TRexLocation);

        // Update tracking variables
        LastTRexDistance = Distance;
        LastTRexLocation = TRexLocation;
    }
}

void UAudio_ScreenShakeController::TriggerDamageShake(float DamageAmount)
{
    if (!GetWorld() || !GetOwner())
    {
        return;
    }

    // Scale shake intensity based on damage amount
    FAudio_ShakeSettings ModifiedSettings = DamageShakeSettings;
    ModifiedSettings.Intensity *= FMath::Clamp(DamageAmount / 100.0f, 0.1f, 3.0f);

    ExecuteScreenShake(ModifiedSettings, GetOwner()->GetActorLocation());
}

void UAudio_ScreenShakeController::TriggerFootstepShake(FVector FootstepLocation, float CreatureMass)
{
    if (!GetWorld() || !GetOwner())
    {
        return;
    }

    FVector PlayerLocation = GetOwner()->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, FootstepLocation);

    if (Distance <= FootstepShakeSettings.MaxDistance)
    {
        float IntensityMultiplier = CalculateShakeIntensity(Distance, FootstepShakeSettings.MaxDistance, CreatureMass);
        
        FAudio_ShakeSettings ModifiedSettings = FootstepShakeSettings;
        ModifiedSettings.Intensity *= IntensityMultiplier;

        ExecuteScreenShake(ModifiedSettings, FootstepLocation);
    }
}

void UAudio_ScreenShakeController::TriggerCustomShake(const FAudio_ShakeSettings& ShakeSettings, FVector SourceLocation)
{
    ExecuteScreenShake(ShakeSettings, SourceLocation);
}

void UAudio_ScreenShakeController::UpdateTRexProximityShake()
{
    if (!GetWorld() || !GetOwner())
    {
        return;
    }

    TArray<AActor*> TRexActors = FindTRexActors();
    
    for (AActor* TRexActor : TRexActors)
    {
        if (TRexActor)
        {
            FVector TRexLocation = TRexActor->GetActorLocation();
            FVector PlayerLocation = GetOwner()->GetActorLocation();
            float Distance = FVector::Dist(PlayerLocation, TRexLocation);

            // Only trigger shake if T-Rex is moving or if distance changed significantly
            bool bShouldShake = false;
            
            if (FVector::Dist(TRexLocation, LastTRexLocation) > 50.0f) // T-Rex is moving
            {
                bShouldShake = true;
            }
            else if (FMath::Abs(Distance - LastTRexDistance) > 100.0f) // Distance changed significantly
            {
                bShouldShake = true;
            }

            if (bShouldShake && Distance <= TRexDetectionRadius)
            {
                TriggerTRexProximityShake(TRexLocation, 7000.0f); // Average T-Rex mass
            }
        }
    }
}

float UAudio_ScreenShakeController::CalculateShakeIntensity(float Distance, float MaxDistance, float BaseMass) const
{
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }

    // Distance falloff (closer = stronger)
    float DistanceFactor = 1.0f - (Distance / MaxDistance);
    DistanceFactor = FMath::Pow(DistanceFactor, 2.0f); // Quadratic falloff

    // Mass factor (heavier creatures = stronger shake)
    float MassFactor = FMath::Clamp(BaseMass / 1000.0f, 0.1f, 10.0f);

    return DistanceFactor * MassFactor;
}

void UAudio_ScreenShakeController::ExecuteScreenShake(const FAudio_ShakeSettings& Settings, FVector SourceLocation)
{
    if (!GetWorld())
    {
        return;
    }

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->PlayerCameraManager)
    {
        return;
    }

    // Create a simple camera shake using the legacy system
    // Note: In UE5, you might want to use the new camera shake system
    PlayerController->PlayerCameraManager->StartCameraShake(
        nullptr, // Camera shake class - using default
        Settings.Intensity,
        ECameraShakePlaySpace::CameraLocal,
        FRotator::ZeroRotator
    );

    bIsShaking = true;

    // Set timer to stop shaking
    FTimerHandle ShakeTimer;
    GetWorld()->GetTimerManager().SetTimer(
        ShakeTimer,
        [this]() { bIsShaking = false; },
        Settings.Duration,
        false
    );
}

TArray<AActor*> UAudio_ScreenShakeController::FindTRexActors() const
{
    TArray<AActor*> TRexActors;
    
    if (!GetWorld())
    {
        return TRexActors;
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            FString ActorName = Actor->GetName().ToLower();
            FString ActorLabel = Actor->GetActorLabel().ToLower();
            
            // Check for T-Rex identifiers
            if (ActorName.Contains(TEXT("trex")) || 
                ActorName.Contains(TEXT("tyrannosaurus")) ||
                ActorLabel.Contains(TEXT("trex")) ||
                ActorLabel.Contains(TEXT("tyrannosaurus")))
            {
                TRexActors.Add(Actor);
            }
        }
    }

    return TRexActors;
}