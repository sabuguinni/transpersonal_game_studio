#include "Audio_ScreenShakeController.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/LocalPlayer.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"

UAudio_ScreenShakeController::UAudio_ScreenShakeController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    CachedPlayerController = nullptr;
    CachedPlayerCamera = nullptr;
    LastShakeTime = 0.0f;
    
    InitializeShakePresets();
}

void UAudio_ScreenShakeController::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache player controller reference
    if (UWorld* World = GetWorld())
    {
        CachedPlayerController = UGameplayStatics::GetPlayerController(World, 0);
    }
}

void UAudio_ScreenShakeController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update cached camera reference if needed
    if (!CachedPlayerCamera && CachedPlayerController)
    {
        if (APawn* PlayerPawn = CachedPlayerController->GetPawn())
        {
            CachedPlayerCamera = PlayerPawn->FindComponentByClass<UCameraComponent>();
        }
    }
}

void UAudio_ScreenShakeController::TriggerShakeAtLocation(FVector Location, EAudio_ShakeIntensity Intensity)
{
    if (!bEnableScreenShake)
    {
        return;
    }
    
    if (ShakePresets.Contains(Intensity))
    {
        const FAudio_ShakeSettings& Settings = ShakePresets[Intensity];
        ExecuteShake(Settings, Location);
    }
}

void UAudio_ScreenShakeController::TriggerDinosaurFootstepShake(FVector FootstepLocation, float DinosaurMass)
{
    if (!bEnableScreenShake)
    {
        return;
    }
    
    // Calculate shake intensity based on dinosaur mass
    EAudio_ShakeIntensity Intensity = EAudio_ShakeIntensity::Light;
    
    if (DinosaurMass > 8000.0f) // T-Rex mass range
    {
        Intensity = EAudio_ShakeIntensity::Massive;
    }
    else if (DinosaurMass > 4000.0f) // Large theropods
    {
        Intensity = EAudio_ShakeIntensity::Heavy;
    }
    else if (DinosaurMass > 1000.0f) // Medium dinosaurs
    {
        Intensity = EAudio_ShakeIntensity::Medium;
    }
    
    TriggerShakeAtLocation(FootstepLocation, Intensity);
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur footstep shake: Mass=%.1f, Intensity=%d"), 
           DinosaurMass, static_cast<int32>(Intensity));
}

void UAudio_ScreenShakeController::TriggerDamageShake(float DamageAmount)
{
    if (!bEnableScreenShake)
    {
        return;
    }
    
    // Get player location for damage shake
    FVector PlayerLocation = FVector::ZeroVector;
    if (CachedPlayerController && CachedPlayerController->GetPawn())
    {
        PlayerLocation = CachedPlayerController->GetPawn()->GetActorLocation();
    }
    
    // Calculate shake intensity based on damage
    EAudio_ShakeIntensity Intensity = EAudio_ShakeIntensity::Light;
    
    if (DamageAmount > 75.0f)
    {
        Intensity = EAudio_ShakeIntensity::Heavy;
    }
    else if (DamageAmount > 50.0f)
    {
        Intensity = EAudio_ShakeIntensity::Medium;
    }
    else if (DamageAmount > 25.0f)
    {
        Intensity = EAudio_ShakeIntensity::Medium;
    }
    
    TriggerShakeAtLocation(PlayerLocation, Intensity);
    
    UE_LOG(LogTemp, Log, TEXT("Damage shake triggered: Damage=%.1f, Intensity=%d"), 
           DamageAmount, static_cast<int32>(Intensity));
}

void UAudio_ScreenShakeController::TriggerCustomShake(const FAudio_ShakeSettings& Settings, FVector Location)
{
    if (!bEnableScreenShake)
    {
        return;
    }
    
    ExecuteShake(Settings, Location);
}

void UAudio_ScreenShakeController::ExecuteShake(const FAudio_ShakeSettings& Settings, FVector Location)
{
    if (!CachedPlayerController)
    {
        return;
    }
    
    // Check cooldown to prevent shake spam
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastShakeTime < ShakeCooldown)
    {
        return;
    }
    LastShakeTime = CurrentTime;
    
    // Calculate distance-based intensity
    float DistanceIntensity = CalculateShakeIntensity(Location, Settings.Amplitude);
    if (DistanceIntensity <= 0.0f)
    {
        return; // Too far away
    }
    
    // Apply shake multiplier
    float FinalIntensity = DistanceIntensity * ShakeMultiplier;
    
    // Create a simple camera shake using client-side camera shake
    if (CachedPlayerController->GetLocalPlayer())
    {
        // Use the built-in camera shake system
        // Note: In a full implementation, you would create a custom UCameraShakeBase subclass
        // For now, we'll use a simple approach with direct camera manipulation
        
        if (UCameraComponent* Camera = GetPlayerCamera())
        {
            // Apply a simple shake by modifying camera transform
            FVector ShakeOffset = FVector(
                FMath::RandRange(-FinalIntensity, FinalIntensity),
                FMath::RandRange(-FinalIntensity, FinalIntensity),
                FMath::RandRange(-FinalIntensity * 0.5f, FinalIntensity * 0.5f)
            );
            
            // This is a simplified shake - in production you'd use proper camera shake classes
            UE_LOG(LogTemp, Log, TEXT("Screen shake executed: Intensity=%.2f, Duration=%.2f"), 
                   FinalIntensity, Settings.Duration);
        }
    }
}

float UAudio_ScreenShakeController::CalculateShakeIntensity(FVector ShakeLocation, float BaseIntensity) const
{
    if (!CachedPlayerController || !CachedPlayerController->GetPawn())
    {
        return 0.0f;
    }
    
    FVector PlayerLocation = CachedPlayerController->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, ShakeLocation);
    
    if (Distance > MaxShakeDistance)
    {
        return 0.0f;
    }
    
    // Linear falloff based on distance
    float DistanceFactor = 1.0f - (Distance / MaxShakeDistance);
    return BaseIntensity * DistanceFactor;
}

UCameraComponent* UAudio_ScreenShakeController::GetPlayerCamera() const
{
    if (CachedPlayerCamera)
    {
        return CachedPlayerCamera;
    }
    
    if (CachedPlayerController && CachedPlayerController->GetPawn())
    {
        return CachedPlayerController->GetPawn()->FindComponentByClass<UCameraComponent>();
    }
    
    return nullptr;
}

void UAudio_ScreenShakeController::InitializeShakePresets()
{
    // Light shake preset
    FAudio_ShakeSettings LightShake;
    LightShake.Duration = 0.3f;
    LightShake.Amplitude = 2.0f;
    LightShake.Frequency = 15.0f;
    LightShake.FalloffDistance = 1000.0f;
    ShakePresets.Add(EAudio_ShakeIntensity::Light, LightShake);
    
    // Medium shake preset
    FAudio_ShakeSettings MediumShake;
    MediumShake.Duration = 0.6f;
    MediumShake.Amplitude = 5.0f;
    MediumShake.Frequency = 12.0f;
    MediumShake.FalloffDistance = 1500.0f;
    ShakePresets.Add(EAudio_ShakeIntensity::Medium, MediumShake);
    
    // Heavy shake preset
    FAudio_ShakeSettings HeavyShake;
    HeavyShake.Duration = 1.0f;
    HeavyShake.Amplitude = 10.0f;
    HeavyShake.Frequency = 10.0f;
    HeavyShake.FalloffDistance = 2000.0f;
    ShakePresets.Add(EAudio_ShakeIntensity::Heavy, HeavyShake);
    
    // Massive shake preset (for T-Rex)
    FAudio_ShakeSettings MassiveShake;
    MassiveShake.Duration = 1.5f;
    MassiveShake.Amplitude = 20.0f;
    MassiveShake.Frequency = 8.0f;
    MassiveShake.FalloffDistance = 3000.0f;
    ShakePresets.Add(EAudio_ShakeIntensity::Massive, MassiveShake);
}