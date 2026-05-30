#include "Audio_ScreenEffectsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/PostProcessComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Camera/CameraShakeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

AAudio_ScreenEffectsManager::AAudio_ScreenEffectsManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create Post Process Component
    PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
    RootComponent = PostProcessComponent;

    // Initialize Damage Flash Settings
    DamageFlashSettings.FlashIntensity = 0.8f;
    DamageFlashSettings.FlashDuration = 0.3f;
    DamageFlashSettings.FlashColor = FLinearColor::Red;
    DamageFlashSettings.FadeOutSpeed = 2.0f;

    // Initialize T-Rex Shake Settings
    TRexShakeSettings.ShakeIntensity = 2.0f;
    TRexShakeSettings.ShakeDuration = 0.8f;
    TRexShakeSettings.ShakeRadius = 3000.0f;
    TRexShakeSettings.FalloffExponent = 1.5f;

    // Initialize Raptor Shake Settings
    RaptorShakeSettings.ShakeIntensity = 0.5f;
    RaptorShakeSettings.ShakeDuration = 0.3f;
    RaptorShakeSettings.ShakeRadius = 1500.0f;
    RaptorShakeSettings.FalloffExponent = 2.0f;

    // Initialize Brachiosaurus Shake Settings
    BrachiosaurusShakeSettings.ShakeIntensity = 1.5f;
    BrachiosaurusShakeSettings.ShakeDuration = 1.0f;
    BrachiosaurusShakeSettings.ShakeRadius = 4000.0f;
    BrachiosaurusShakeSettings.FalloffExponent = 1.0f;

    // Set default post process settings
    PostProcessComponent->Settings.bOverride_ColorSaturation = true;
    PostProcessComponent->Settings.ColorSaturation = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
    PostProcessComponent->bUnbound = true;
    PostProcessComponent->Priority = 100.0f;
}

void AAudio_ScreenEffectsManager::BeginPlay()
{
    Super::BeginPlay();

    // Get player references
    PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (PlayerController)
    {
        PlayerPawn = PlayerController->GetPawn();
    }

    // Apply initial post process settings
    ApplyPostProcessEffects();
}

void AAudio_ScreenEffectsManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update damage flash effect
    UpdateDamageFlash(DeltaTime);

    // Check for nearby dinosaurs for proximity effects
    CheckDinosaurProximity();
}

void AAudio_ScreenEffectsManager::TriggerDamageFlash(float Intensity, FLinearColor Color)
{
    bDamageFlashActive = true;
    CurrentFlashIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    FlashTimer = DamageFlashSettings.FlashDuration;
    
    // Apply immediate flash effect
    PostProcessComponent->Settings.bOverride_ColorSaturation = true;
    FVector4 FlashSaturation = FVector4(
        1.0f + (Color.R * CurrentFlashIntensity),
        1.0f - (CurrentFlashIntensity * 0.5f),
        1.0f - (CurrentFlashIntensity * 0.5f),
        1.0f
    );
    PostProcessComponent->Settings.ColorSaturation = FlashSaturation;

    UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenEffectsManager: Damage flash triggered with intensity %f"), CurrentFlashIntensity);
}

void AAudio_ScreenEffectsManager::StopDamageFlash()
{
    bDamageFlashActive = false;
    CurrentFlashIntensity = 0.0f;
    FlashTimer = 0.0f;
    
    // Reset post process settings
    PostProcessComponent->Settings.ColorSaturation = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
}

void AAudio_ScreenEffectsManager::TriggerTRexFootstepShake(FVector ShakeLocation)
{
    if (!PlayerController || !PlayerPawn) return;

    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), ShakeLocation);
    float AdjustedIntensity = CalculateShakeIntensityByDistance(Distance, TRexShakeSettings.ShakeRadius);

    if (AdjustedIntensity > 0.1f && CameraShakeClass)
    {
        PlayerController->ClientStartCameraShake(CameraShakeClass, AdjustedIntensity);
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenEffectsManager: T-Rex footstep shake triggered - Distance: %f, Intensity: %f"), Distance, AdjustedIntensity);
    }
}

void AAudio_ScreenEffectsManager::TriggerRaptorMovementShake(FVector ShakeLocation)
{
    if (!PlayerController || !PlayerPawn) return;

    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), ShakeLocation);
    float AdjustedIntensity = CalculateShakeIntensityByDistance(Distance, RaptorShakeSettings.ShakeRadius);

    if (AdjustedIntensity > 0.05f && CameraShakeClass)
    {
        PlayerController->ClientStartCameraShake(CameraShakeClass, AdjustedIntensity * 0.5f);
        UE_LOG(LogTemp, Log, TEXT("Audio_ScreenEffectsManager: Raptor movement shake triggered - Distance: %f, Intensity: %f"), Distance, AdjustedIntensity);
    }
}

void AAudio_ScreenEffectsManager::TriggerBrachiosaurusShake(FVector ShakeLocation)
{
    if (!PlayerController || !PlayerPawn) return;

    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), ShakeLocation);
    float AdjustedIntensity = CalculateShakeIntensityByDistance(Distance, BrachiosaurusShakeSettings.ShakeRadius);

    if (AdjustedIntensity > 0.1f && CameraShakeClass)
    {
        PlayerController->ClientStartCameraShake(CameraShakeClass, AdjustedIntensity);
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenEffectsManager: Brachiosaurus shake triggered - Distance: %f, Intensity: %f"), Distance, AdjustedIntensity);
    }
}

void AAudio_ScreenEffectsManager::TriggerCustomShake(FVector ShakeLocation, EAudio_EffectIntensity Intensity)
{
    if (!PlayerController || !CameraShakeClass) return;

    float ShakeValue = 0.0f;
    switch (Intensity)
    {
        case EAudio_EffectIntensity::Light:
            ShakeValue = 0.3f;
            break;
        case EAudio_EffectIntensity::Medium:
            ShakeValue = 0.6f;
            break;
        case EAudio_EffectIntensity::Heavy:
            ShakeValue = 1.0f;
            break;
        case EAudio_EffectIntensity::Extreme:
            ShakeValue = 1.5f;
            break;
        default:
            return;
    }

    PlayerController->ClientStartCameraShake(CameraShakeClass, ShakeValue);
}

void AAudio_ScreenEffectsManager::CheckDinosaurProximity()
{
    if (!PlayerPawn) return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    UWorld* World = GetWorld();
    if (!World) return;

    // Find all actors with "TRex", "Raptor", or "Brachiosaurus" in their name
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor) continue;

        FString ActorName = Actor->GetName();
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());

        // Check for T-Rex proximity
        if (ActorName.Contains(TEXT("TRex")) || ActorName.Contains(TEXT("Tyrannosaurus")))
        {
            if (Distance < TRexShakeSettings.ShakeRadius)
            {
                // Trigger periodic footstep shakes for nearby T-Rex
                static float TRexShakeTimer = 0.0f;
                TRexShakeTimer += GetWorld()->GetDeltaSeconds();
                if (TRexShakeTimer >= 2.0f) // Every 2 seconds
                {
                    TriggerTRexFootstepShake(Actor->GetActorLocation());
                    TRexShakeTimer = 0.0f;
                }
            }
        }
        // Check for Raptor proximity
        else if (ActorName.Contains(TEXT("Raptor")) || ActorName.Contains(TEXT("Velociraptor")))
        {
            if (Distance < RaptorShakeSettings.ShakeRadius)
            {
                // Trigger subtle movement shakes for nearby Raptors
                static float RaptorShakeTimer = 0.0f;
                RaptorShakeTimer += GetWorld()->GetDeltaSeconds();
                if (RaptorShakeTimer >= 1.5f) // Every 1.5 seconds
                {
                    TriggerRaptorMovementShake(Actor->GetActorLocation());
                    RaptorShakeTimer = 0.0f;
                }
            }
        }
        // Check for Brachiosaurus proximity
        else if (ActorName.Contains(TEXT("Brachiosaurus")) || ActorName.Contains(TEXT("Brachio")))
        {
            if (Distance < BrachiosaurusShakeSettings.ShakeRadius)
            {
                // Trigger heavy footstep shakes for nearby Brachiosaurus
                static float BrachioShakeTimer = 0.0f;
                BrachioShakeTimer += GetWorld()->GetDeltaSeconds();
                if (BrachioShakeTimer >= 3.0f) // Every 3 seconds
                {
                    TriggerBrachiosaurusShake(Actor->GetActorLocation());
                    BrachioShakeTimer = 0.0f;
                }
            }
        }
    }
}

void AAudio_ScreenEffectsManager::OnPlayerDamaged(float DamageAmount, AActor* DamageSource)
{
    // Calculate flash intensity based on damage amount
    float FlashIntensity = FMath::Clamp(DamageAmount / 100.0f, 0.2f, 1.0f);
    
    // Determine flash color based on damage source
    FLinearColor FlashColor = FLinearColor::Red;
    if (DamageSource)
    {
        FString SourceName = DamageSource->GetName();
        if (SourceName.Contains(TEXT("Fire")) || SourceName.Contains(TEXT("Lava")))
        {
            FlashColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange for fire damage
        }
        else if (SourceName.Contains(TEXT("Ice")) || SourceName.Contains(TEXT("Cold")))
        {
            FlashColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f); // Blue for cold damage
        }
    }

    TriggerDamageFlash(FlashIntensity, FlashColor);
}

void AAudio_ScreenEffectsManager::UpdateDamageFlash(float DeltaTime)
{
    if (!bDamageFlashActive) return;

    FlashTimer -= DeltaTime;
    
    if (FlashTimer <= 0.0f)
    {
        // Fade out the flash effect
        CurrentFlashIntensity = FMath::FInterpTo(CurrentFlashIntensity, 0.0f, DeltaTime, DamageFlashSettings.FadeOutSpeed);
        
        if (CurrentFlashIntensity <= 0.01f)
        {
            StopDamageFlash();
            return;
        }
    }

    // Update post process effect intensity
    ApplyPostProcessEffects();
}

void AAudio_ScreenEffectsManager::ApplyPostProcessEffects()
{
    if (!PostProcessComponent) return;

    if (bDamageFlashActive && CurrentFlashIntensity > 0.0f)
    {
        // Apply damage flash effect
        FVector4 FlashSaturation = FVector4(
            1.0f + (DamageFlashSettings.FlashColor.R * CurrentFlashIntensity),
            1.0f - (CurrentFlashIntensity * 0.5f),
            1.0f - (CurrentFlashIntensity * 0.5f),
            1.0f
        );
        PostProcessComponent->Settings.ColorSaturation = FlashSaturation;
    }
    else
    {
        // Reset to normal
        PostProcessComponent->Settings.ColorSaturation = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

float AAudio_ScreenEffectsManager::CalculateShakeIntensityByDistance(float Distance, float MaxDistance)
{
    if (Distance >= MaxDistance) return 0.0f;
    
    float NormalizedDistance = Distance / MaxDistance;
    return FMath::Pow(1.0f - NormalizedDistance, 2.0f); // Quadratic falloff
}