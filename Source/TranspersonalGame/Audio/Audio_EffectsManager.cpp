#include "Audio_EffectsManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/CameraShakeSourceComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "GameFramework/HUD.h"

AAudio_EffectsManager::AAudio_EffectsManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    RootComponent = AudioComponent;

    // Initialize default shake settings
    TRexFootstepShake.Intensity = 2.0f;
    TRexFootstepShake.Duration = 0.8f;
    TRexFootstepShake.Frequency = 8.0f;
    TRexFootstepShake.Range = 3000.0f;

    RaptorFootstepShake.Intensity = 0.5f;
    RaptorFootstepShake.Duration = 0.3f;
    RaptorFootstepShake.Frequency = 15.0f;
    RaptorFootstepShake.Range = 1500.0f;

    DamageFlash.FlashColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.7f);
    DamageFlash.FlashDuration = 0.3f;
    DamageFlash.FlashIntensity = 0.8f;
}

void AAudio_EffectsManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_EffectsManager initialized"));
}

void AAudio_EffectsManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAudio_EffectsManager::TriggerTRexFootstep(FVector Location)
{
    // Play footstep sound
    if (TRexFootstepSound)
    {
        PlaySoundAtLocation(TRexFootstepSound, Location, 1.0f);
    }

    // Trigger screen shake
    TriggerScreenShake(TRexFootstepShake, Location);

    UE_LOG(LogTemp, Warning, TEXT("T-Rex footstep triggered at location: %s"), *Location.ToString());
}

void AAudio_EffectsManager::TriggerRaptorFootstep(FVector Location)
{
    // Play footstep sound
    if (RaptorFootstepSound)
    {
        PlaySoundAtLocation(RaptorFootstepSound, Location, 0.7f);
    }

    // Trigger screen shake
    TriggerScreenShake(RaptorFootstepShake, Location);

    UE_LOG(LogTemp, Warning, TEXT("Raptor footstep triggered at location: %s"), *Location.ToString());
}

void AAudio_EffectsManager::TriggerDamageFlash(float DamageAmount)
{
    // Scale flash intensity based on damage amount
    FAudio_DamageFlashSettings ScaledFlash = DamageFlash;
    ScaledFlash.FlashIntensity = FMath::Clamp(DamageAmount / 100.0f, 0.2f, 1.0f);

    ApplyDamageFlash(ScaledFlash);

    // Play damage sound
    if (DamageSound)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), DamageSound);
    }

    UE_LOG(LogTemp, Warning, TEXT("Damage flash triggered with intensity: %f"), ScaledFlash.FlashIntensity);
}

void AAudio_EffectsManager::TriggerScreenShake(FAudio_ScreenShakeSettings ShakeSettings, FVector EpicenterLocation)
{
    ApplyScreenShake(ShakeSettings.Intensity, ShakeSettings.Duration, ShakeSettings.Frequency, EpicenterLocation, ShakeSettings.Range);
}

void AAudio_EffectsManager::PlaySoundAtLocation(USoundCue* SoundCue, FVector Location, float VolumeMultiplier)
{
    if (SoundCue && GetWorld())
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundCue, Location, VolumeMultiplier);
    }
}

void AAudio_EffectsManager::ApplyScreenShake(float Intensity, float Duration, float Frequency, FVector EpicenterLocation, float Range)
{
    APlayerController* PlayerController = GetPlayerController();
    if (!PlayerController)
    {
        return;
    }

    float DistanceToPlayer = GetDistanceToPlayer(EpicenterLocation);
    if (DistanceToPlayer > Range)
    {
        return; // Player is too far away
    }

    // Calculate falloff based on distance
    float DistanceFalloff = 1.0f - (DistanceToPlayer / Range);
    float AdjustedIntensity = Intensity * DistanceFalloff;

    // Apply camera shake using legacy method for compatibility
    if (AdjustedIntensity > 0.1f)
    {
        PlayerController->ClientStartCameraShake(nullptr, AdjustedIntensity);
        
        UE_LOG(LogTemp, Warning, TEXT("Screen shake applied: Intensity=%f, Distance=%f, Falloff=%f"), 
               AdjustedIntensity, DistanceToPlayer, DistanceFalloff);
    }
}

void AAudio_EffectsManager::ApplyDamageFlash(FAudio_DamageFlashSettings FlashSettings)
{
    APlayerController* PlayerController = GetPlayerController();
    if (!PlayerController)
    {
        return;
    }

    // Apply damage flash using HUD overlay
    AHUD* PlayerHUD = PlayerController->GetHUD();
    if (PlayerHUD)
    {
        // Note: In a full implementation, this would use a custom HUD class
        // For now, we log the effect
        UE_LOG(LogTemp, Warning, TEXT("Damage flash: Color=(%f,%f,%f,%f), Duration=%f"), 
               FlashSettings.FlashColor.R, FlashSettings.FlashColor.G, 
               FlashSettings.FlashColor.B, FlashSettings.FlashIntensity,
               FlashSettings.FlashDuration);
    }
}

float AAudio_EffectsManager::GetDistanceToPlayer(FVector Location)
{
    APlayerController* PlayerController = GetPlayerController();
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return 99999.0f; // Very large distance if no player
    }

    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    return FVector::Dist(Location, PlayerLocation);
}

APlayerController* AAudio_EffectsManager::GetPlayerController()
{
    if (GetWorld())
    {
        return GetWorld()->GetFirstPlayerController();
    }
    return nullptr;
}