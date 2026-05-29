#include "Audio_ScreenShakeController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"

UAudio_ScreenShakeController::UAudio_ScreenShakeController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = CheckInterval;
    
    InitializeShakeParameters();
}

void UAudio_ScreenShakeController::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache player controller
    CachedPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    
    if (!CachedPlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeController: Could not find PlayerController"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController initialized"));
}

void UAudio_ScreenShakeController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bShakeEnabled || !CachedPlayerController)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastCheckTime >= CheckInterval)
    {
        CheckProximityShake();
        LastCheckTime = CurrentTime;
    }
}

void UAudio_ScreenShakeController::TriggerShake(EAudio_ShakeIntensity Intensity, FVector SourceLocation)
{
    if (!CanTriggerShake())
    {
        return;
    }
    
    FAudio_ShakeParameters ShakeParams = GetShakeParamsForIntensity(Intensity);
    
    // Calculate distance-based intensity
    if (CachedPlayerController && CachedPlayerController->GetPawn())
    {
        FVector PlayerLocation = CachedPlayerController->GetPawn()->GetActorLocation();
        float Distance = FVector::Dist(PlayerLocation, SourceLocation);
        float IntensityMultiplier = CalculateShakeIntensityByDistance(Distance, ShakeParams.FalloffDistance);
        
        if (IntensityMultiplier > 0.1f) // Only shake if close enough
        {
            ExecuteShake(ShakeParams, IntensityMultiplier);
        }
    }
}

void UAudio_ScreenShakeController::TriggerCustomShake(const FAudio_ShakeParameters& ShakeParams, FVector SourceLocation)
{
    if (!CanTriggerShake())
    {
        return;
    }
    
    if (CachedPlayerController && CachedPlayerController->GetPawn())
    {
        FVector PlayerLocation = CachedPlayerController->GetPawn()->GetActorLocation();
        float Distance = FVector::Dist(PlayerLocation, SourceLocation);
        float IntensityMultiplier = CalculateShakeIntensityByDistance(Distance, ShakeParams.FalloffDistance);
        
        if (IntensityMultiplier > 0.1f)
        {
            ExecuteShake(ShakeParams, IntensityMultiplier);
        }
    }
}

void UAudio_ScreenShakeController::CheckProximityShake()
{
    if (!CachedPlayerController || !CachedPlayerController->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = CachedPlayerController->GetPawn()->GetActorLocation();
    
    // Get all actors in detection radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);
    
    for (AActor* Actor : NearbyActors)
    {
        if (!Actor || Actor == CachedPlayerController->GetPawn())
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        
        if (Distance > DetectionRadius)
        {
            continue;
        }
        
        // Check for T-Rex (large actors with "TRex" in name)
        FString ActorName = Actor->GetName();
        if (ActorName.Contains(TEXT("TRex")) || ActorName.Contains(TEXT("T_Rex")))
        {
            if (Distance <= TRexShakeDistance)
            {
                // Calculate shake intensity based on distance
                EAudio_ShakeIntensity ShakeIntensity = EAudio_ShakeIntensity::Medium;
                
                if (Distance < TRexShakeDistance * 0.3f)
                {
                    ShakeIntensity = EAudio_ShakeIntensity::Extreme;
                }
                else if (Distance < TRexShakeDistance * 0.6f)
                {
                    ShakeIntensity = EAudio_ShakeIntensity::Heavy;
                }
                
                TriggerShake(ShakeIntensity, Actor->GetActorLocation());
                return; // Only one shake per tick
            }
        }
        // Check for other large dinosaurs
        else if (ActorName.Contains(TEXT("Brach")) || ActorName.Contains(TEXT("Sauropod")))
        {
            if (Distance <= LargeDinosaurShakeDistance)
            {
                EAudio_ShakeIntensity ShakeIntensity = Distance < LargeDinosaurShakeDistance * 0.5f ? 
                    EAudio_ShakeIntensity::Heavy : EAudio_ShakeIntensity::Medium;
                
                TriggerShake(ShakeIntensity, Actor->GetActorLocation());
                return;
            }
        }
    }
}

void UAudio_ScreenShakeController::SetShakeEnabled(bool bEnabled)
{
    bShakeEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController shake enabled: %s"), bEnabled ? TEXT("true") : TEXT("false"));
}

void UAudio_ScreenShakeController::InitializeShakeParameters()
{
    // Light shake parameters
    LightShakeParams.Duration = 0.3f;
    LightShakeParams.Amplitude = 0.5f;
    LightShakeParams.Frequency = 8.0f;
    LightShakeParams.FalloffDistance = 1000.0f;
    
    // Medium shake parameters
    MediumShakeParams.Duration = 0.6f;
    MediumShakeParams.Amplitude = 1.0f;
    MediumShakeParams.Frequency = 12.0f;
    MediumShakeParams.FalloffDistance = 1500.0f;
    
    // Heavy shake parameters
    HeavyShakeParams.Duration = 1.0f;
    HeavyShakeParams.Amplitude = 2.0f;
    HeavyShakeParams.Frequency = 15.0f;
    HeavyShakeParams.FalloffDistance = 2000.0f;
    
    // Extreme shake parameters
    ExtremeShakeParams.Duration = 1.5f;
    ExtremeShakeParams.Amplitude = 3.5f;
    ExtremeShakeParams.Frequency = 20.0f;
    ExtremeShakeParams.FalloffDistance = 2500.0f;
}

FAudio_ShakeParameters UAudio_ScreenShakeController::GetShakeParamsForIntensity(EAudio_ShakeIntensity Intensity) const
{
    switch (Intensity)
    {
        case EAudio_ShakeIntensity::Light:
            return LightShakeParams;
        case EAudio_ShakeIntensity::Medium:
            return MediumShakeParams;
        case EAudio_ShakeIntensity::Heavy:
            return HeavyShakeParams;
        case EAudio_ShakeIntensity::Extreme:
            return ExtremeShakeParams;
        default:
            return MediumShakeParams;
    }
}

float UAudio_ScreenShakeController::CalculateShakeIntensityByDistance(float Distance, float MaxDistance) const
{
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }
    
    // Linear falloff
    return 1.0f - (Distance / MaxDistance);
}

bool UAudio_ScreenShakeController::CanTriggerShake() const
{
    if (!bShakeEnabled || !CachedPlayerController)
    {
        return false;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastShakeTime) >= ShakeCooldown;
}

void UAudio_ScreenShakeController::ExecuteShake(const FAudio_ShakeParameters& Params, float IntensityMultiplier)
{
    if (!CachedPlayerController || !CachedPlayerController->PlayerCameraManager)
    {
        return;
    }
    
    // Create a simple camera shake using the legacy system
    // Note: In a full implementation, you'd create a custom UCameraShakeBase subclass
    
    float FinalAmplitude = Params.Amplitude * IntensityMultiplier;
    float FinalDuration = Params.Duration;
    
    // Use the built-in camera shake functionality
    CachedPlayerController->PlayerCameraManager->StartCameraShake(
        nullptr, // ShakeClass - using nullptr for simple shake
        FinalAmplitude, // Scale
        ECameraShakePlaySpace::CameraLocal,
        FRotator::ZeroRotator
    );
    
    LastShakeTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Screen shake triggered - Amplitude: %f, Duration: %f"), 
           FinalAmplitude, FinalDuration);
}