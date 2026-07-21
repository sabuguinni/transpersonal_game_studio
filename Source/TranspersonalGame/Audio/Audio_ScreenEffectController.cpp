#include "Audio_ScreenEffectController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalGame/TranspersonalCharacter.h"

DEFINE_LOG_CATEGORY_STATIC(LogAudioScreenEffects, Log, All);

UAudio_ScreenEffectController::UAudio_ScreenEffectController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for performance
    
    // Screen shake parameters
    ShakeIntensity = 1.0f;
    ShakeRadius = 2000.0f;
    ShakeFalloff = 1.0f;
    
    // Damage flash parameters
    FlashDuration = 0.5f;
    FlashIntensity = 0.8f;
    FlashColor = FLinearColor::Red;
    
    // Proximity detection
    ProximityCheckRadius = 1500.0f;
    MinShakeDistance = 500.0f;
    MaxShakeDistance = 2000.0f;
    
    // State
    bIsFlashing = false;
    FlashTimer = 0.0f;
    LastShakeTime = 0.0f;
    ShakeCooldown = 1.0f;
}

void UAudio_ScreenEffectController::BeginPlay()
{
    Super::BeginPlay();
    
    // Get player controller
    if (UWorld* World = GetWorld())
    {
        PlayerController = World->GetFirstPlayerController();
        if (PlayerController)
        {
            UE_LOG(LogAudioScreenEffects, Log, TEXT("ScreenEffectController initialized with PlayerController"));
        }
    }
    
    // Find post process volume for damage flash
    FindPostProcessVolume();
}

void UAudio_ScreenEffectController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!PlayerController)
        return;
    
    // Update damage flash
    UpdateDamageFlash(DeltaTime);
    
    // Check for proximity shake triggers
    CheckProximityShake();
}

void UAudio_ScreenEffectController::TriggerScreenShake(float Intensity, FVector Location, float Radius)
{
    if (!PlayerController)
        return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastShakeTime < ShakeCooldown)
        return;
    
    // Calculate distance-based intensity
    if (APawn* PlayerPawn = PlayerController->GetPawn())
    {
        float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Location);
        float DistanceRatio = FMath::Clamp((MaxShakeDistance - Distance) / (MaxShakeDistance - MinShakeDistance), 0.0f, 1.0f);
        float FinalIntensity = Intensity * DistanceRatio;
        
        if (FinalIntensity > 0.1f)
        {
            // Create camera shake
            TriggerCameraShake(FinalIntensity);
            LastShakeTime = CurrentTime;
            
            UE_LOG(LogAudioScreenEffects, Log, TEXT("Screen shake triggered: Intensity=%.2f, Distance=%.0f"), FinalIntensity, Distance);
        }
    }
}

void UAudio_ScreenEffectController::TriggerDamageFlash(float Intensity, FLinearColor Color, float Duration)
{
    if (bIsFlashing)
        return;
    
    bIsFlashing = true;
    FlashTimer = 0.0f;
    FlashDuration = Duration;
    FlashIntensity = Intensity;
    FlashColor = Color;
    
    // Apply flash effect
    ApplyDamageFlashEffect(Intensity);
    
    UE_LOG(LogAudioScreenEffects, Log, TEXT("Damage flash triggered: Intensity=%.2f, Duration=%.2f"), Intensity, Duration);
}

void UAudio_ScreenEffectController::TriggerCameraShake(float Intensity)
{
    if (!PlayerController)
        return;
    
    // Use built-in camera shake
    FVector ShakeLocation = PlayerController->GetPawn() ? PlayerController->GetPawn()->GetActorLocation() : FVector::ZeroVector;
    
    // Create shake parameters
    float ShakeScale = FMath::Clamp(Intensity * ShakeIntensity, 0.1f, 2.0f);
    
    // Apply camera shake using UE5 camera shake system
    if (PlayerController->PlayerCameraManager)
    {
        PlayerController->PlayerCameraManager->StartCameraShake(
            nullptr, // Use default camera shake class
            ShakeScale,
            ECameraShakePlaySpace::CameraLocal,
            FRotator::ZeroRotator
        );
    }
}

void UAudio_ScreenEffectController::CheckProximityShake()
{
    if (!PlayerController || !PlayerController->GetPawn())
        return;
    
    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    
    // Find nearby large dinosaurs (T-Rex, Brachiosaurus)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (!Actor || Actor == PlayerController->GetPawn())
            continue;
        
        FString ActorName = Actor->GetName().ToLower();
        bool bIsLargeDinosaur = ActorName.Contains(TEXT("trex")) || 
                               ActorName.Contains(TEXT("tyrannosaurus")) ||
                               ActorName.Contains(TEXT("brachio")) ||
                               ActorName.Contains(TEXT("brachiosaurus"));
        
        if (bIsLargeDinosaur)
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            if (Distance <= ProximityCheckRadius)
            {
                // Trigger proximity shake based on movement
                if (APawn* DinosaurPawn = Cast<APawn>(Actor))
                {
                    FVector Velocity = DinosaurPawn->GetVelocity();
                    float Speed = Velocity.Size();
                    
                    if (Speed > 100.0f) // Moving
                    {
                        float ProximityIntensity = FMath::Clamp((ProximityCheckRadius - Distance) / ProximityCheckRadius, 0.0f, 1.0f);
                        TriggerScreenShake(ProximityIntensity * 0.5f, Actor->GetActorLocation(), ProximityCheckRadius);
                    }
                }
            }
        }
    }
}

void UAudio_ScreenEffectController::UpdateDamageFlash(float DeltaTime)
{
    if (!bIsFlashing)
        return;
    
    FlashTimer += DeltaTime;
    
    if (FlashTimer >= FlashDuration)
    {
        // End flash
        bIsFlashing = false;
        ApplyDamageFlashEffect(0.0f);
        UE_LOG(LogAudioScreenEffects, Log, TEXT("Damage flash ended"));
    }
    else
    {
        // Update flash intensity (fade out)
        float FlashProgress = FlashTimer / FlashDuration;
        float CurrentIntensity = FlashIntensity * (1.0f - FlashProgress);
        ApplyDamageFlashEffect(CurrentIntensity);
    }
}

void UAudio_ScreenEffectController::ApplyDamageFlashEffect(float Intensity)
{
    // Apply damage flash using post-process volume or material parameter collection
    if (PostProcessVolume)
    {
        // Modify post-process settings for damage flash
        FPostProcessSettings& Settings = PostProcessVolume->Settings;
        Settings.bOverride_ColorSaturation = true;
        Settings.ColorSaturation = FVector4(1.0f - Intensity * 0.5f, 1.0f - Intensity * 0.8f, 1.0f - Intensity * 0.8f, 1.0f);
        
        Settings.bOverride_ColorContrast = true;
        Settings.ColorContrast = FVector4(1.0f + Intensity * 0.3f, 1.0f + Intensity * 0.3f, 1.0f + Intensity * 0.3f, 1.0f);
        
        Settings.bOverride_VignetteIntensity = true;
        Settings.VignetteIntensity = Intensity * 0.5f;
    }
}

void UAudio_ScreenEffectController::FindPostProcessVolume()
{
    // Find post-process volume in the level
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APostProcessVolume::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(FoundActors[0]);
        UE_LOG(LogAudioScreenEffects, Log, TEXT("Found PostProcessVolume for damage flash effects"));
    }
    else
    {
        UE_LOG(LogAudioScreenEffects, Warning, TEXT("No PostProcessVolume found for damage flash effects"));
    }
}

void UAudio_ScreenEffectController::OnDamageTaken(float DamageAmount, FVector HitLocation)
{
    // Calculate flash intensity based on damage
    float FlashIntensityFromDamage = FMath::Clamp(DamageAmount / 100.0f, 0.2f, 1.0f);
    
    // Trigger damage flash
    TriggerDamageFlash(FlashIntensityFromDamage, FlashColor, FlashDuration);
}

void UAudio_ScreenEffectController::OnLargeDinosaurNearby(AActor* DinosaurActor, float Distance)
{
    if (!DinosaurActor)
        return;
    
    // Calculate shake intensity based on distance and dinosaur size
    float ProximityRatio = FMath::Clamp((ProximityCheckRadius - Distance) / ProximityCheckRadius, 0.0f, 1.0f);
    float ShakeIntensityFromProximity = ProximityRatio * 0.7f;
    
    // Trigger proximity shake
    TriggerScreenShake(ShakeIntensityFromProximity, DinosaurActor->GetActorLocation(), ProximityCheckRadius);
}