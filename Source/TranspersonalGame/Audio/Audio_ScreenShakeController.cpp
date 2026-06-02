#include "Audio_ScreenShakeController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/CameraComponent.h"
#include "Components/PrimitiveComponent.h"

UAudio_ScreenShakeController::UAudio_ScreenShakeController()
{
    PrimaryComponentTick.bCanEverTick = false;

    // T-Rex footstep shake - powerful and long-lasting
    TRexFootstepShake.Intensity = 3.0f;
    TRexFootstepShake.Duration = 1.2f;
    TRexFootstepShake.Frequency = 8.0f;
    TRexFootstepShake.Distance = 2500.0f;

    // Player footstep shake - subtle
    PlayerFootstepShake.Intensity = 0.3f;
    PlayerFootstepShake.Duration = 0.2f;
    PlayerFootstepShake.Frequency = 15.0f;
    PlayerFootstepShake.Distance = 100.0f;

    // Damage shake - intense but brief
    DamageShake.Intensity = 2.5f;
    DamageShake.Duration = 0.8f;
    DamageShake.Frequency = 20.0f;
    DamageShake.Distance = 0.0f; // Always full intensity for damage

    TRexDetectionRadius = 2000.0f;
    ShakeUpdateInterval = 0.1f;
    PlayerController = nullptr;
}

void UAudio_ScreenShakeController::BeginPlay()
{
    Super::BeginPlay();

    // Get player controller reference
    PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeController: Could not find PlayerController"));
    }
}

void UAudio_ScreenShakeController::TriggerTRexFootstepShake(const FVector& TRexLocation)
{
    if (!PlayerController)
    {
        return;
    }

    ApplyScreenShake(TRexFootstepShake, TRexLocation);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: T-Rex footstep shake triggered at location %s"), 
           *TRexLocation.ToString());
}

void UAudio_ScreenShakeController::TriggerPlayerFootstepShake()
{
    if (!PlayerController)
    {
        return;
    }

    ApplyScreenShake(PlayerFootstepShake);
}

void UAudio_ScreenShakeController::TriggerDamageShake(float DamageAmount)
{
    if (!PlayerController)
    {
        return;
    }

    // Scale shake intensity based on damage amount
    FAudio_ShakeParams ScaledDamageShake = DamageShake;
    ScaledDamageShake.Intensity *= FMath::Clamp(DamageAmount / 100.0f, 0.5f, 2.0f);

    ApplyScreenShake(ScaledDamageShake);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: Damage shake triggered with intensity %.2f"), 
           ScaledDamageShake.Intensity);
}

void UAudio_ScreenShakeController::StartTRexProximityShaking()
{
    if (!GetWorld())
    {
        return;
    }

    // Start timer to check T-Rex proximity periodically
    GetWorld()->GetTimerManager().SetTimer(
        ShakeTimerHandle,
        this,
        &UAudio_ScreenShakeController::CheckTRexProximity,
        ShakeUpdateInterval,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: Started T-Rex proximity monitoring"));
}

void UAudio_ScreenShakeController::StopTRexProximityShaking()
{
    if (!GetWorld())
    {
        return;
    }

    GetWorld()->GetTimerManager().ClearTimer(ShakeTimerHandle);
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeController: Stopped T-Rex proximity monitoring"));
}

void UAudio_ScreenShakeController::CheckTRexProximity()
{
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return;
    }

    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    UWorld* World = GetWorld();

    if (!World)
    {
        return;
    }

    // Find all actors with "TRex" in their name
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }

        FString ActorName = Actor->GetName().ToLower();
        if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("t_rex")) || ActorName.Contains(TEXT("tyrannosaurus")))
        {
            FVector TRexLocation = Actor->GetActorLocation();
            float Distance = FVector::Dist(PlayerLocation, TRexLocation);

            if (Distance <= TRexDetectionRadius)
            {
                // Calculate shake intensity based on distance
                float IntensityMultiplier = 1.0f - (Distance / TRexDetectionRadius);
                FAudio_ShakeParams ProximityShake = TRexFootstepShake;
                ProximityShake.Intensity *= IntensityMultiplier * 0.5f; // Reduce intensity for proximity
                ProximityShake.Duration = 0.3f; // Shorter duration for proximity

                ApplyScreenShake(ProximityShake, TRexLocation);
                break; // Only shake for the closest T-Rex
            }
        }
    }
}

void UAudio_ScreenShakeController::ApplyScreenShake(const FAudio_ShakeParams& ShakeParams, const FVector& SourceLocation)
{
    if (!PlayerController)
    {
        return;
    }

    float FinalIntensity = ShakeParams.Intensity;

    // If source location is provided, calculate distance-based intensity
    if (SourceLocation != FVector::ZeroVector && PlayerController->GetPawn())
    {
        FinalIntensity = CalculateShakeIntensity(SourceLocation, ShakeParams.Distance);
    }

    // Apply camera shake using legacy method (compatible with UE5)
    if (FinalIntensity > 0.1f)
    {
        // Create a simple camera shake effect
        FVector ShakeOffset = FVector(
            FMath::RandRange(-FinalIntensity, FinalIntensity),
            FMath::RandRange(-FinalIntensity, FinalIntensity),
            FMath::RandRange(-FinalIntensity * 0.5f, FinalIntensity * 0.5f)
        );

        // Apply shake through player controller
        PlayerController->ClientStartCameraShake(nullptr, FinalIntensity);
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Audio_ScreenShakeController: Applied shake with intensity %.2f"), FinalIntensity);
    }
}

float UAudio_ScreenShakeController::CalculateShakeIntensity(const FVector& SourceLocation, float MaxDistance) const
{
    if (!PlayerController || !PlayerController->GetPawn() || MaxDistance <= 0.0f)
    {
        return 0.0f;
    }

    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, SourceLocation);

    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }

    // Linear falloff based on distance
    float DistanceRatio = 1.0f - (Distance / MaxDistance);
    return DistanceRatio;
}