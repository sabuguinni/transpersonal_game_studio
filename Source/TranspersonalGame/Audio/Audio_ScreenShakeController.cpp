#include "Audio_ScreenShakeController.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UAudio_ScreenShakeController::UAudio_ScreenShakeController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    MaxShakeDistance = 2000.0f;
    bEnableShake = true;
    bIsShaking = false;
    CurrentShakeTime = 0.0f;
    CurrentDistanceFactor = 1.0f;
    PlayerCamera = nullptr;
    PlayerController = nullptr;
    
    InitializeDefaultPatterns();
}

void UAudio_ScreenShakeController::BeginPlay()
{
    Super::BeginPlay();
    
    // Get player controller and camera
    PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController && PlayerController->GetPawn())
    {
        PlayerCamera = PlayerController->GetPawn()->FindComponentByClass<UCameraComponent>();
    }
}

void UAudio_ScreenShakeController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsShaking && PlayerCamera)
    {
        CurrentShakeTime += DeltaTime;
        
        if (CurrentShakeTime >= CurrentPattern.Duration)
        {
            // Shake finished
            bIsShaking = false;
            CurrentShakeTime = 0.0f;
            
            // Reset camera rotation
            if (PlayerController && PlayerController->GetPawn())
            {
                PlayerController->GetPawn()->GetRootComponent()->SetRelativeRotation(FRotator::ZeroRotator);
            }
        }
        else
        {
            // Apply shake effect
            ApplyShakeToCamera(CurrentPattern, CurrentDistanceFactor);
        }
    }
}

void UAudio_ScreenShakeController::TriggerShakeFromDistance(FVector SourceLocation, float MaxDistance, EAudio_ShakeIntensity Intensity)
{
    if (!bEnableShake || !PlayerController || !PlayerController->GetPawn())
    {
        return;
    }
    
    float DistanceFactor = CalculateDistanceFactor(SourceLocation, MaxDistance);
    if (DistanceFactor <= 0.0f)
    {
        return; // Too far away
    }
    
    if (ShakePatterns.Contains(Intensity))
    {
        CurrentPattern = ShakePatterns[Intensity];
        CurrentDistanceFactor = DistanceFactor;
        bIsShaking = true;
        CurrentShakeTime = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Screen shake triggered: Intensity=%d, Distance Factor=%.2f"), 
               (int32)Intensity, DistanceFactor);
    }
}

void UAudio_ScreenShakeController::TriggerCustomShake(const FAudio_ShakePattern& Pattern)
{
    if (!bEnableShake || !PlayerCamera)
    {
        return;
    }
    
    CurrentPattern = Pattern;
    CurrentDistanceFactor = 1.0f;
    bIsShaking = true;
    CurrentShakeTime = 0.0f;
}

void UAudio_ScreenShakeController::TriggerTRexFootstep(FVector TRexLocation)
{
    // T-Rex footsteps create heavy shake with large radius
    TriggerShakeFromDistance(TRexLocation, 3000.0f, EAudio_ShakeIntensity::Heavy);
}

void UAudio_ScreenShakeController::StopAllShakes()
{
    bIsShaking = false;
    CurrentShakeTime = 0.0f;
    
    // Reset camera rotation
    if (PlayerController && PlayerController->GetPawn())
    {
        PlayerController->GetPawn()->GetRootComponent()->SetRelativeRotation(FRotator::ZeroRotator);
    }
}

void UAudio_ScreenShakeController::InitializeDefaultPatterns()
{
    // Light shake (small creatures, distant impacts)
    FAudio_ShakePattern LightShake;
    LightShake.Duration = 0.3f;
    LightShake.Magnitude = 1.0f;
    LightShake.Frequency = 15.0f;
    LightShake.bFadeOut = true;
    ShakePatterns.Add(EAudio_ShakeIntensity::Light, LightShake);
    
    // Medium shake (medium dinosaurs, close impacts)
    FAudio_ShakePattern MediumShake;
    MediumShake.Duration = 0.6f;
    MediumShake.Magnitude = 3.0f;
    MediumShake.Frequency = 12.0f;
    MediumShake.bFadeOut = true;
    ShakePatterns.Add(EAudio_ShakeIntensity::Medium, MediumShake);
    
    // Heavy shake (T-Rex footsteps, large impacts)
    FAudio_ShakePattern HeavyShake;
    HeavyShake.Duration = 1.0f;
    HeavyShake.Magnitude = 8.0f;
    HeavyShake.Frequency = 8.0f;
    HeavyShake.bFadeOut = true;
    ShakePatterns.Add(EAudio_ShakeIntensity::Heavy, HeavyShake);
    
    // Extreme shake (T-Rex roar, explosions)
    FAudio_ShakePattern ExtremeShake;
    ExtremeShake.Duration = 1.5f;
    ExtremeShake.Magnitude = 15.0f;
    ExtremeShake.Frequency = 5.0f;
    ExtremeShake.bFadeOut = true;
    ShakePatterns.Add(EAudio_ShakeIntensity::Extreme, ExtremeShake);
}

void UAudio_ScreenShakeController::ApplyShakeToCamera(const FAudio_ShakePattern& Pattern, float DistanceFactor)
{
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return;
    }
    
    // Calculate shake intensity based on time and distance
    float TimeProgress = CurrentShakeTime / Pattern.Duration;
    float Intensity = Pattern.Magnitude * DistanceFactor;
    
    if (Pattern.bFadeOut)
    {
        Intensity *= (1.0f - TimeProgress); // Fade out over time
    }
    
    // Generate shake offset using sine waves
    float ShakeX = FMath::Sin(CurrentShakeTime * Pattern.Frequency * 2.0f) * Intensity;
    float ShakeY = FMath::Cos(CurrentShakeTime * Pattern.Frequency * 1.7f) * Intensity * 0.7f;
    float ShakeZ = FMath::Sin(CurrentShakeTime * Pattern.Frequency * 1.3f) * Intensity * 0.5f;
    
    // Apply shake as rotation offset
    FRotator ShakeRotation(ShakeY * 0.1f, ShakeX * 0.1f, ShakeZ * 0.05f);
    
    if (PlayerCamera)
    {
        PlayerCamera->SetRelativeRotation(ShakeRotation);
    }
    else if (PlayerController->GetPawn())
    {
        // Fallback: shake the pawn itself
        FRotator CurrentRotation = PlayerController->GetPawn()->GetActorRotation();
        PlayerController->GetPawn()->SetActorRotation(CurrentRotation + ShakeRotation);
    }
}

float UAudio_ScreenShakeController::CalculateDistanceFactor(FVector SourceLocation, float MaxDistance)
{
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return 0.0f;
    }
    
    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, SourceLocation);
    
    if (Distance >= MaxDistance)
    {
        return 0.0f; // Too far away
    }
    
    // Linear falloff from 1.0 at distance 0 to 0.0 at MaxDistance
    return 1.0f - (Distance / MaxDistance);
}