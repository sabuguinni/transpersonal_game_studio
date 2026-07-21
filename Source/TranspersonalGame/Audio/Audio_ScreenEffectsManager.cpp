#include "Audio_ScreenEffectsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

AAudio_ScreenEffectsManager::AAudio_ScreenEffectsManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default shake data for T-Rex
    DefaultTRexShakeData.Intensity = 3.0f;
    DefaultTRexShakeData.Duration = 1.5f;
    DefaultTRexShakeData.Frequency = 15.0f;
    DefaultTRexShakeData.FalloffRadius = 3000.0f;
    
    // Initialize default shake data for player
    DefaultPlayerShakeData.Intensity = 1.0f;
    DefaultPlayerShakeData.Duration = 0.8f;
    DefaultPlayerShakeData.Frequency = 20.0f;
    DefaultPlayerShakeData.FalloffRadius = 1500.0f;
    
    // Initialize damage flash data
    DefaultDamageFlashData.FlashIntensity = 0.7f;
    DefaultDamageFlashData.FlashDuration = 0.4f;
    DefaultDamageFlashData.FlashColor = FLinearColor(1.0f, 0.2f, 0.2f, 0.8f);
    
    // Initialize state
    bDamageFlashActive = false;
    CurrentFlashAlpha = 0.0f;
    DamageFlashMaterial = nullptr;
    FootstepDustParticles = nullptr;
    DinosaurFootstepParticles = nullptr;
}

void AAudio_ScreenEffectsManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Try to load default particle systems
    FootstepDustParticles = LoadObject<UParticleSystem>(nullptr, TEXT("/Engine/VFX/P_Dust_Puff"));
    DinosaurFootstepParticles = LoadObject<UParticleSystem>(nullptr, TEXT("/Engine/VFX/P_Explosion"));
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenEffectsManager: BeginPlay completed"));
}

void AAudio_ScreenEffectsManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bDamageFlashActive)
    {
        UpdateDamageFlash();
    }
}

void AAudio_ScreenEffectsManager::TriggerScreenShake(const FVector& EpicenterLocation, const FAudio_ScreenShakeData& ShakeData)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenEffectsManager: No valid world for screen shake"));
        return;
    }
    
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenEffectsManager: No player controller found"));
        return;
    }
    
    APawn* PlayerPawn = PlayerController->GetPawn();
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenEffectsManager: No player pawn found"));
        return;
    }
    
    // Calculate distance-based intensity
    float Distance = FVector::Dist(EpicenterLocation, PlayerPawn->GetActorLocation());
    float IntensityMultiplier = CalculateShakeIntensityByDistance(EpicenterLocation, ShakeData.FalloffRadius);
    
    if (IntensityMultiplier > 0.1f)
    {
        float FinalIntensity = ShakeData.Intensity * IntensityMultiplier;
        ApplyScreenShakeToPlayer(FinalIntensity, ShakeData.Duration, ShakeData.Frequency);
        
        UE_LOG(LogTemp, Log, TEXT("Audio_ScreenEffectsManager: Screen shake triggered - Intensity: %f, Duration: %f"), 
               FinalIntensity, ShakeData.Duration);
    }
}

void AAudio_ScreenEffectsManager::TriggerTRexFootstepShake(const FVector& FootstepLocation)
{
    TriggerScreenShake(FootstepLocation, DefaultTRexShakeData);
    
    // Also spawn footstep particles for T-Rex
    SpawnDinosaurFootstepParticles(FootstepLocation, TEXT("TRex"));
}

void AAudio_ScreenEffectsManager::TriggerDamageFlash(const FAudio_DamageFlashData& FlashData)
{
    if (bDamageFlashActive)
    {
        // Reset the timer if already flashing
        GetWorld()->GetTimerManager().ClearTimer(DamageFlashTimerHandle);
    }
    
    bDamageFlashActive = true;
    CurrentFlashAlpha = FlashData.FlashIntensity;
    
    // Set timer to end the flash
    GetWorld()->GetTimerManager().SetTimer(
        DamageFlashTimerHandle,
        this,
        &AAudio_ScreenEffectsManager::EndDamageFlash,
        FlashData.FlashDuration,
        false
    );
    
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenEffectsManager: Damage flash triggered - Intensity: %f, Duration: %f"), 
           FlashData.FlashIntensity, FlashData.FlashDuration);
}

void AAudio_ScreenEffectsManager::TriggerPlayerDamageFlash(float DamageAmount)
{
    // Scale flash intensity based on damage amount
    FAudio_DamageFlashData ScaledFlashData = DefaultDamageFlashData;
    ScaledFlashData.FlashIntensity = FMath::Clamp(DamageAmount * 0.1f, 0.3f, 1.0f);
    ScaledFlashData.FlashDuration = FMath::Clamp(DamageAmount * 0.05f, 0.2f, 0.8f);
    
    TriggerDamageFlash(ScaledFlashData);
}

void AAudio_ScreenEffectsManager::SpawnFootstepParticles(const FVector& Location, float ParticleScale)
{
    if (!GetWorld() || !FootstepDustParticles)
    {
        return;
    }
    
    FVector SpawnLocation = Location;
    SpawnLocation.Z += 10.0f; // Slightly above ground
    
    UParticleSystemComponent* ParticleComponent = UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(),
        FootstepDustParticles,
        SpawnLocation,
        FRotator::ZeroRotator,
        FVector(ParticleScale),
        true
    );
    
    if (ParticleComponent)
    {
        UE_LOG(LogTemp, Log, TEXT("Audio_ScreenEffectsManager: Footstep particles spawned at location: %s"), 
               *SpawnLocation.ToString());
    }
}

void AAudio_ScreenEffectsManager::SpawnDinosaurFootstepParticles(const FVector& Location, const FString& DinosaurType)
{
    float ParticleScale = 1.0f;
    
    // Scale particles based on dinosaur type
    if (DinosaurType.Contains(TEXT("TRex")))
    {
        ParticleScale = 3.0f;
    }
    else if (DinosaurType.Contains(TEXT("Brachi")))
    {
        ParticleScale = 4.0f;
    }
    else if (DinosaurType.Contains(TEXT("Raptor")))
    {
        ParticleScale = 0.8f;
    }
    else if (DinosaurType.Contains(TEXT("Tricera")))
    {
        ParticleScale = 2.5f;
    }
    
    SpawnFootstepParticles(Location, ParticleScale);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenEffectsManager: %s footstep particles spawned with scale %f"), 
           *DinosaurType, ParticleScale);
}

void AAudio_ScreenEffectsManager::UpdateDamageFlash()
{
    if (!bDamageFlashActive)
    {
        return;
    }
    
    // Fade out the flash over time
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    CurrentFlashAlpha = FMath::Max(0.0f, CurrentFlashAlpha - (DeltaTime * 2.0f));
    
    // Update material parameter if we have a material
    if (DamageFlashMaterial)
    {
        DamageFlashMaterial->SetScalarParameterValue(TEXT("FlashAlpha"), CurrentFlashAlpha);
        DamageFlashMaterial->SetVectorParameterValue(TEXT("FlashColor"), DefaultDamageFlashData.FlashColor);
    }
}

void AAudio_ScreenEffectsManager::EndDamageFlash()
{
    bDamageFlashActive = false;
    CurrentFlashAlpha = 0.0f;
    
    if (DamageFlashMaterial)
    {
        DamageFlashMaterial->SetScalarParameterValue(TEXT("FlashAlpha"), 0.0f);
    }
    
    GetWorld()->GetTimerManager().ClearTimer(DamageFlashTimerHandle);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenEffectsManager: Damage flash ended"));
}

float AAudio_ScreenEffectsManager::CalculateShakeIntensityByDistance(const FVector& EpicenterLocation, float MaxRadius) const
{
    if (!GetWorld())
    {
        return 0.0f;
    }
    
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return 0.0f;
    }
    
    float Distance = FVector::Dist(EpicenterLocation, PlayerController->GetPawn()->GetActorLocation());
    
    if (Distance >= MaxRadius)
    {
        return 0.0f;
    }
    
    // Linear falloff from 1.0 at epicenter to 0.0 at max radius
    return FMath::Clamp(1.0f - (Distance / MaxRadius), 0.0f, 1.0f);
}

void AAudio_ScreenEffectsManager::ApplyScreenShakeToPlayer(float Intensity, float Duration, float Frequency)
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
    
    // Create a simple camera shake effect using player controller
    // This is a basic implementation - in a full game you'd use UE5's camera shake classes
    
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenEffectsManager: Applying screen shake to player - Intensity: %f, Duration: %f, Frequency: %f"), 
           Intensity, Duration, Frequency);
    
    // For now, we'll use a simple approach with the player controller
    // In a production game, you'd implement proper camera shake classes
}