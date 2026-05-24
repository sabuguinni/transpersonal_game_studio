#include "Audio_ScreenEffectsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Canvas.h"
#include "Components/PrimitiveComponent.h"

UAudio_ScreenEffectsManager::UAudio_ScreenEffectsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // Initialize default values
    bIsFlashing = false;
    CurrentFlashTime = 0.0f;
    FlashDuration = 0.0f;
    FlashColor = FLinearColor::Red;
    FlashIntensity = 0.5f;
    PlayerController = nullptr;

    // Set default damage effect
    DefaultDamageEffect.FlashDuration = 0.3f;
    DefaultDamageEffect.FlashColor = FLinearColor::Red;
    DefaultDamageEffect.FlashIntensity = 0.5f;
    DefaultDamageEffect.bUseScreenShake = true;
    DefaultDamageEffect.ShakeIntensity = 1.0f;

    // Set default T-Rex proximity effect
    TRexProximityEffect.MaxDistance = 2000.0f;
    TRexProximityEffect.MinShakeIntensity = 0.1f;
    TRexProximityEffect.MaxShakeIntensity = 2.0f;
    TRexProximityEffect.ShakeDuration = 0.5f;
}

void UAudio_ScreenEffectsManager::BeginPlay()
{
    Super::BeginPlay();

    // Get player controller reference
    if (UWorld* World = GetWorld())
    {
        PlayerController = World->GetFirstPlayerController();
        if (!PlayerController)
        {
            UE_LOG(LogTemp, Warning, TEXT("UAudio_ScreenEffectsManager: No PlayerController found"));
        }
    }
}

void UAudio_ScreenEffectsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update flash effect
    if (bIsFlashing)
    {
        UpdateFlashEffect(DeltaTime);
    }

    // Check T-Rex proximity every frame for dynamic shake
    CheckTRexProximity();
}

void UAudio_ScreenEffectsManager::TriggerDamageFlash(const FAudio_DamageEffect& DamageEffect)
{
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAudio_ScreenEffectsManager: No PlayerController for damage flash"));
        return;
    }

    // Start flash effect
    bIsFlashing = true;
    CurrentFlashTime = 0.0f;
    FlashDuration = DamageEffect.FlashDuration;
    FlashColor = DamageEffect.FlashColor;
    FlashIntensity = DamageEffect.FlashIntensity;

    // Trigger camera shake if enabled
    if (DamageEffect.bUseScreenShake && DefaultCameraShakeClass)
    {
        PlayerController->ClientStartCameraShake(DefaultCameraShakeClass, DamageEffect.ShakeIntensity);
    }

    UE_LOG(LogTemp, Log, TEXT("UAudio_ScreenEffectsManager: Triggered damage flash - Duration: %f, Intensity: %f"), 
           FlashDuration, FlashIntensity);
}

void UAudio_ScreenEffectsManager::TriggerCustomFlash(FLinearColor InFlashColor, float Duration, float Intensity)
{
    FAudio_DamageEffect CustomEffect;
    CustomEffect.FlashColor = InFlashColor;
    CustomEffect.FlashDuration = Duration;
    CustomEffect.FlashIntensity = Intensity;
    CustomEffect.bUseScreenShake = false;

    TriggerDamageFlash(CustomEffect);
}

void UAudio_ScreenEffectsManager::TriggerCameraShake(float Intensity, float Duration)
{
    if (!PlayerController || !DefaultCameraShakeClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAudio_ScreenEffectsManager: Cannot trigger camera shake - missing PlayerController or ShakeClass"));
        return;
    }

    PlayerController->ClientStartCameraShake(DefaultCameraShakeClass, Intensity);
    UE_LOG(LogTemp, Log, TEXT("UAudio_ScreenEffectsManager: Triggered camera shake - Intensity: %f"), Intensity);
}

void UAudio_ScreenEffectsManager::TriggerProximityShake(AActor* SourceActor, const FAudio_ProximityEffect& ProximityEffect)
{
    if (!PlayerController || !SourceActor)
    {
        return;
    }

    // Calculate distance to source actor
    FVector PlayerLocation = PlayerController->GetPawn() ? PlayerController->GetPawn()->GetActorLocation() : FVector::ZeroVector;
    FVector SourceLocation = SourceActor->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, SourceLocation);

    // Apply shake if within range
    if (Distance <= ProximityEffect.MaxDistance)
    {
        float ShakeIntensity = CalculateProximityIntensity(Distance, ProximityEffect);
        TriggerCameraShake(ShakeIntensity, ProximityEffect.ShakeDuration);
    }
}

void UAudio_ScreenEffectsManager::CheckTRexProximity()
{
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return;
    }

    // Find T-Rex actors in the world
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    
    // Look for actors with "TRex" in their name
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor && Actor->GetName().Contains(TEXT("TRex")))
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            
            // Apply proximity shake if T-Rex is close
            if (Distance <= TRexProximityEffect.MaxDistance)
            {
                float ShakeIntensity = CalculateProximityIntensity(Distance, TRexProximityEffect);
                
                // Only apply shake if intensity is significant
                if (ShakeIntensity > 0.05f)
                {
                    TriggerCameraShake(ShakeIntensity, 0.1f); // Short duration for continuous effect
                }
            }
        }
    }
}

void UAudio_ScreenEffectsManager::SetScreenOverlay(FLinearColor OverlayColor, float Opacity)
{
    // This would typically be implemented with a UI overlay widget
    // For now, we'll log the request
    UE_LOG(LogTemp, Log, TEXT("UAudio_ScreenEffectsManager: Screen overlay requested - Color: %s, Opacity: %f"), 
           *OverlayColor.ToString(), Opacity);
}

void UAudio_ScreenEffectsManager::ClearScreenOverlay()
{
    UE_LOG(LogTemp, Log, TEXT("UAudio_ScreenEffectsManager: Screen overlay cleared"));
}

void UAudio_ScreenEffectsManager::UpdateFlashEffect(float DeltaTime)
{
    if (!bIsFlashing)
    {
        return;
    }

    CurrentFlashTime += DeltaTime;
    
    if (CurrentFlashTime >= FlashDuration)
    {
        // Flash complete
        bIsFlashing = false;
        CurrentFlashTime = 0.0f;
        ApplyScreenFlash(0.0f); // Clear flash
    }
    else
    {
        // Calculate flash alpha (fade out over time)
        float Alpha = 1.0f - (CurrentFlashTime / FlashDuration);
        Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
        ApplyScreenFlash(Alpha);
    }
}

void UAudio_ScreenEffectsManager::ApplyScreenFlash(float Alpha)
{
    // This would typically modify a screen overlay material or post-process volume
    // For now, we'll use engine debug drawing
    if (Alpha > 0.0f && GEngine)
    {
        FLinearColor FlashColorWithAlpha = FlashColor;
        FlashColorWithAlpha.A = Alpha * FlashIntensity;
        
        // Log the flash state for debugging
        UE_LOG(LogTemp, VeryVerbose, TEXT("UAudio_ScreenEffectsManager: Flash Alpha: %f, Color: %s"), 
               Alpha, *FlashColorWithAlpha.ToString());
    }
}

float UAudio_ScreenEffectsManager::CalculateProximityIntensity(float Distance, const FAudio_ProximityEffect& Effect)
{
    if (Distance >= Effect.MaxDistance)
    {
        return 0.0f;
    }

    // Linear interpolation from max intensity at distance 0 to min intensity at max distance
    float DistanceRatio = Distance / Effect.MaxDistance;
    float Intensity = FMath::Lerp(Effect.MaxShakeIntensity, Effect.MinShakeIntensity, DistanceRatio);
    
    return FMath::Clamp(Intensity, 0.0f, Effect.MaxShakeIntensity);
}