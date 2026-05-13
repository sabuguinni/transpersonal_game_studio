#include "Audio_ScreenShakeSystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/Engine.h"

UAudio_ScreenShakeSystem::UAudio_ScreenShakeSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // T-Rex footstep shake profile - heavy, long duration
    TRexFootstepShake.Intensity = 2.5f;
    TRexFootstepShake.Duration = 1.2f;
    TRexFootstepShake.Frequency = 8.0f;
    TRexFootstepShake.Range = 2000.0f;
    TRexFootstepShake.bFalloffByDistance = true;

    // Dinosaur impact shake - medium intensity, quick
    DinosaurImpactShake.Intensity = 1.8f;
    DinosaurImpactShake.Duration = 0.6f;
    DinosaurImpactShake.Frequency = 15.0f;
    DinosaurImpactShake.Range = 1500.0f;
    DinosaurImpactShake.bFalloffByDistance = true;

    // Player damage shake - intense but brief
    PlayerDamageShake.Intensity = 3.0f;
    PlayerDamageShake.Duration = 0.3f;
    PlayerDamageShake.Frequency = 25.0f;
    PlayerDamageShake.Range = 0.0f; // No distance falloff for damage
    PlayerDamageShake.bFalloffByDistance = false;

    // Environmental shake - moderate, medium duration
    EnvironmentalShake.Intensity = 1.2f;
    EnvironmentalShake.Duration = 0.8f;
    EnvironmentalShake.Frequency = 12.0f;
    EnvironmentalShake.Range = 1200.0f;
    EnvironmentalShake.bFalloffByDistance = true;

    CachedPlayerController = nullptr;
    CachedPlayerPawn = nullptr;
}

void UAudio_ScreenShakeSystem::BeginPlay()
{
    Super::BeginPlay();
    CachePlayerReferences();
}

void UAudio_ScreenShakeSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Periodically refresh player references in case they change
    static float RefreshTimer = 0.0f;
    RefreshTimer += DeltaTime;
    if (RefreshTimer >= 2.0f)
    {
        CachePlayerReferences();
        RefreshTimer = 0.0f;
    }
}

void UAudio_ScreenShakeSystem::TriggerTRexFootstep(FVector Location)
{
    if (!IsPlayerValid()) return;

    // Apply screen shake with distance falloff
    ApplyScreenShake(TRexFootstepShake, Location);

    // Log for debugging
    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeSystem: T-Rex footstep shake triggered at %s"), *Location.ToString());
}

void UAudio_ScreenShakeSystem::TriggerDinosaurImpact(FVector Location, float ImpactForce)
{
    if (!IsPlayerValid()) return;

    // Scale shake intensity based on impact force
    FAudio_ShakeProfile ScaledProfile = DinosaurImpactShake;
    ScaledProfile.Intensity *= FMath::Clamp(ImpactForce / 100.0f, 0.5f, 3.0f);

    ApplyScreenShake(ScaledProfile, Location);

    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeSystem: Dinosaur impact shake (Force: %.1f) triggered at %s"), 
           ImpactForce, *Location.ToString());
}

void UAudio_ScreenShakeSystem::TriggerPlayerDamage(float DamageAmount)
{
    if (!IsPlayerValid()) return;

    // Scale shake intensity based on damage amount
    FAudio_ShakeProfile ScaledProfile = PlayerDamageShake;
    ScaledProfile.Intensity *= FMath::Clamp(DamageAmount / 50.0f, 0.3f, 2.0f);

    // Player damage shake has no location (affects camera directly)
    ApplyScreenShake(ScaledProfile);

    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeSystem: Player damage shake (Damage: %.1f) triggered"), DamageAmount);
}

void UAudio_ScreenShakeSystem::TriggerEnvironmentalShake(FVector Location, const FAudio_ShakeProfile& Profile)
{
    if (!IsPlayerValid()) return;

    ApplyScreenShake(Profile, Location);

    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeSystem: Environmental shake triggered at %s"), *Location.ToString());
}

void UAudio_ScreenShakeSystem::TriggerCustomShake(FVector Location, float Intensity, float Duration, float Range)
{
    if (!IsPlayerValid()) return;

    FAudio_ShakeProfile CustomProfile;
    CustomProfile.Intensity = Intensity;
    CustomProfile.Duration = Duration;
    CustomProfile.Range = Range;
    CustomProfile.Frequency = 15.0f;
    CustomProfile.bFalloffByDistance = (Range > 0.0f);

    ApplyScreenShake(CustomProfile, Location);

    UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeSystem: Custom shake (I:%.1f D:%.1f R:%.1f) triggered at %s"), 
           Intensity, Duration, Range, *Location.ToString());
}

void UAudio_ScreenShakeSystem::ApplyScreenShake(const FAudio_ShakeProfile& Profile, FVector Location)
{
    if (!IsPlayerValid()) return;

    float FinalIntensity = Profile.Intensity;

    // Apply distance falloff if enabled and location is specified
    if (Profile.bFalloffByDistance && Location != FVector::ZeroVector && Profile.Range > 0.0f)
    {
        float DistanceFalloff = CalculateDistanceFalloff(Location, Profile.Range);
        FinalIntensity *= DistanceFalloff;
        
        // Don't shake if too far away
        if (FinalIntensity < 0.1f)
        {
            return;
        }
    }

    // Create a simple camera shake using the engine's built-in system
    if (CachedPlayerController)
    {
        // Use a simple camera shake - in a real implementation you'd create a custom UCameraShakeBase subclass
        // For now, we'll use the basic shake functionality
        FVector2D ShakeScale(FinalIntensity, FinalIntensity);
        
        // Apply shake through the player controller
        // Note: In UE5, you would typically create a custom camera shake class
        // This is a simplified implementation for demonstration
        
        UE_LOG(LogTemp, Log, TEXT("Audio_ScreenShakeSystem: Applying screen shake - Intensity: %.2f, Duration: %.2f"), 
               FinalIntensity, Profile.Duration);
    }
}

float UAudio_ScreenShakeSystem::CalculateDistanceFalloff(FVector ShakeLocation, float MaxRange)
{
    if (!IsPlayerValid() || MaxRange <= 0.0f) return 1.0f;

    FVector PlayerLocation = CachedPlayerPawn->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, ShakeLocation);
    
    if (Distance >= MaxRange)
    {
        return 0.0f;
    }

    // Linear falloff - could be made more sophisticated with curves
    return 1.0f - (Distance / MaxRange);
}

void UAudio_ScreenShakeSystem::CachePlayerReferences()
{
    if (UWorld* World = GetWorld())
    {
        CachedPlayerController = UGameplayStatics::GetPlayerController(World, 0);
        if (CachedPlayerController)
        {
            CachedPlayerPawn = CachedPlayerController->GetPawn();
        }
    }
}

bool UAudio_ScreenShakeSystem::IsPlayerValid()
{
    return (CachedPlayerController != nullptr && CachedPlayerPawn != nullptr);
}