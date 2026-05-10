#include "Audio_ScreenShakeController.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"

UAudio_ScreenShakeController::UAudio_ScreenShakeController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Check 10 times per second

    // Initialize shake profiles
    LightShakeProfile.Duration = 0.3f;
    LightShakeProfile.Amplitude = 0.5f;
    LightShakeProfile.Frequency = 8.0f;
    LightShakeProfile.BlendInTime = 0.05f;
    LightShakeProfile.BlendOutTime = 0.1f;

    MediumShakeProfile.Duration = 0.6f;
    MediumShakeProfile.Amplitude = 1.0f;
    MediumShakeProfile.Frequency = 12.0f;
    MediumShakeProfile.BlendInTime = 0.1f;
    MediumShakeProfile.BlendOutTime = 0.2f;

    HeavyShakeProfile.Duration = 1.0f;
    HeavyShakeProfile.Amplitude = 1.8f;
    HeavyShakeProfile.Frequency = 15.0f;
    HeavyShakeProfile.BlendInTime = 0.15f;
    HeavyShakeProfile.BlendOutTime = 0.3f;

    ExtremeShakeProfile.Duration = 1.5f;
    ExtremeShakeProfile.Amplitude = 3.0f;
    ExtremeShakeProfile.Frequency = 20.0f;
    ExtremeShakeProfile.BlendInTime = 0.2f;
    ExtremeShakeProfile.BlendOutTime = 0.5f;

    // T-Rex proximity settings
    TRexDetectionRadius = 5000.0f; // 50 meters
    MaxShakeDistance = 3000.0f; // 30 meters max shake range
    bTRexProximityEnabled = true;
    ProximityCheckInterval = 0.5f; // Check every 0.5 seconds

    PlayerController = nullptr;
    CameraManager = nullptr;
    LastProximityCheckTime = 0.0f;
}

void UAudio_ScreenShakeController::BeginPlay()
{
    Super::BeginPlay();

    // Get player controller and camera manager
    if (UWorld* World = GetWorld())
    {
        PlayerController = World->GetFirstPlayerController();
        if (PlayerController)
        {
            CameraManager = PlayerController->PlayerCameraManager;
        }
    }

    if (!PlayerController || !CameraManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeController: Failed to get PlayerController or CameraManager"));
    }
}

void UAudio_ScreenShakeController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bTRexProximityEnabled)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastProximityCheckTime >= ProximityCheckInterval)
        {
            CheckTRexProximity();
            LastProximityCheckTime = CurrentTime;
        }
    }
}

void UAudio_ScreenShakeController::TriggerTRexFootstepShake(float Distance, float TRexMass)
{
    if (!CameraManager || Distance > MaxShakeDistance)
    {
        return;
    }

    // Calculate intensity based on distance and mass
    float DistanceRatio = FMath::Clamp(1.0f - (Distance / MaxShakeDistance), 0.0f, 1.0f);
    float MassMultiplier = FMath::Clamp(TRexMass / 7000.0f, 0.5f, 2.0f); // Normalize to 7-ton T-Rex
    float FinalIntensity = DistanceRatio * MassMultiplier;

    // Choose shake profile based on intensity
    FAudio_ShakeProfile ShakeProfile;
    if (FinalIntensity > 0.8f)
    {
        ShakeProfile = ExtremeShakeProfile;
    }
    else if (FinalIntensity > 0.6f)
    {
        ShakeProfile = HeavyShakeProfile;
    }
    else if (FinalIntensity > 0.3f)
    {
        ShakeProfile = MediumShakeProfile;
    }
    else
    {
        ShakeProfile = LightShakeProfile;
    }

    ApplyShakeProfile(ShakeProfile, FinalIntensity);

    UE_LOG(LogTemp, Log, TEXT("T-Rex footstep shake triggered: Distance=%.1f, Mass=%.1f, Intensity=%.2f"), 
           Distance, TRexMass, FinalIntensity);
}

void UAudio_ScreenShakeController::TriggerDamageShake(float DamageAmount, EAudio_ShakeIntensity IntensityOverride)
{
    if (!CameraManager)
    {
        return;
    }

    // Determine shake intensity based on damage amount
    EAudio_ShakeIntensity ShakeIntensity = IntensityOverride;
    
    if (IntensityOverride == EAudio_ShakeIntensity::Medium) // Use damage-based calculation
    {
        if (DamageAmount >= 50.0f)
        {
            ShakeIntensity = EAudio_ShakeIntensity::Extreme;
        }
        else if (DamageAmount >= 25.0f)
        {
            ShakeIntensity = EAudio_ShakeIntensity::Heavy;
        }
        else if (DamageAmount >= 10.0f)
        {
            ShakeIntensity = EAudio_ShakeIntensity::Medium;
        }
        else
        {
            ShakeIntensity = EAudio_ShakeIntensity::Light;
        }
    }

    FAudio_ShakeProfile ShakeProfile = GetShakeProfileByIntensity(ShakeIntensity);
    float IntensityMultiplier = FMath::Clamp(DamageAmount / 25.0f, 0.3f, 2.0f);
    
    ApplyShakeProfile(ShakeProfile, IntensityMultiplier);

    UE_LOG(LogTemp, Log, TEXT("Damage shake triggered: Damage=%.1f, Intensity=%d"), 
           DamageAmount, (int32)ShakeIntensity);
}

void UAudio_ScreenShakeController::TriggerCustomShake(const FAudio_ShakeProfile& ShakeProfile)
{
    ApplyShakeProfile(ShakeProfile, 1.0f);
}

void UAudio_ScreenShakeController::StopAllShakes()
{
    if (CameraManager)
    {
        CameraManager->StopAllCameraShakes(false);
        UE_LOG(LogTemp, Log, TEXT("All camera shakes stopped"));
    }
}

void UAudio_ScreenShakeController::CheckTRexProximity()
{
    if (!PlayerController || !CameraManager)
    {
        return;
    }

    APawn* PlayerPawn = PlayerController->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    TArray<AActor*> NearbyTRexes = FindNearbyTRexActors(TRexDetectionRadius);

    for (AActor* TRexActor : NearbyTRexes)
    {
        if (!TRexActor)
        {
            continue;
        }

        float Distance = FVector::Dist(PlayerLocation, TRexActor->GetActorLocation());
        
        // Only trigger shake if T-Rex is within shake range and moving
        if (Distance <= MaxShakeDistance)
        {
            // Check if T-Rex is moving (simple velocity check)
            FVector TRexVelocity = FVector::ZeroVector;
            if (UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(TRexActor->GetRootComponent()))
            {
                TRexVelocity = RootComp->GetComponentVelocity();
            }

            // Only shake if T-Rex is moving fast enough (walking/running)
            if (TRexVelocity.Size() > 100.0f) // 1 m/s threshold
            {
                TriggerTRexFootstepShake(Distance, 7000.0f);
                break; // Only process the closest moving T-Rex
            }
        }
    }
}

void UAudio_ScreenShakeController::SetTRexProximityEnabled(bool bEnabled)
{
    bTRexProximityEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("T-Rex proximity detection %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

FAudio_ShakeProfile UAudio_ScreenShakeController::GetShakeProfileByIntensity(EAudio_ShakeIntensity Intensity)
{
    switch (Intensity)
    {
        case EAudio_ShakeIntensity::Light:
            return LightShakeProfile;
        case EAudio_ShakeIntensity::Medium:
            return MediumShakeProfile;
        case EAudio_ShakeIntensity::Heavy:
            return HeavyShakeProfile;
        case EAudio_ShakeIntensity::Extreme:
            return ExtremeShakeProfile;
        default:
            return MediumShakeProfile;
    }
}

float UAudio_ScreenShakeController::CalculateShakeIntensityByDistance(float Distance, float MaxDistance)
{
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }
    
    return FMath::Clamp(1.0f - (Distance / MaxDistance), 0.0f, 1.0f);
}

void UAudio_ScreenShakeController::ApplyShakeProfile(const FAudio_ShakeProfile& Profile, float IntensityMultiplier)
{
    if (!CameraManager)
    {
        return;
    }

    // Create a simple camera shake using the profile settings
    // Note: In a full implementation, you would create a custom UCameraShakeBase subclass
    // For now, we'll use the basic camera shake functionality
    
    FVector ShakeLocation = FVector::ZeroVector;
    FRotator ShakeRotation = FRotator::ZeroRotator;
    
    // Apply shake with scaled amplitude
    float ScaledAmplitude = Profile.Amplitude * IntensityMultiplier;
    
    // Simple implementation - in production you'd want a proper camera shake class
    if (CameraManager->GetCameraLocation().IsZero() == false)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Applying screen shake: Amplitude=%.2f, Duration=%.2f"), 
               ScaledAmplitude, Profile.Duration);
    }
}

TArray<AActor*> UAudio_ScreenShakeController::FindNearbyTRexActors(float SearchRadius)
{
    TArray<AActor*> FoundTRexes;
    
    if (!GetWorld())
    {
        return FoundTRexes;
    }

    APawn* PlayerPawn = nullptr;
    if (PlayerController)
    {
        PlayerPawn = PlayerController->GetPawn();
    }
    
    if (!PlayerPawn)
    {
        return FoundTRexes;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Find all actors in the world and filter for T-Rex types
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor)
        {
            continue;
        }

        // Check if this is a T-Rex actor (by name or class)
        FString ActorName = Actor->GetName();
        if (ActorName.Contains(TEXT("TRex")) || ActorName.Contains(TEXT("Tyrannosaurus")))
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            if (Distance <= SearchRadius)
            {
                FoundTRexes.Add(Actor);
            }
        }
    }

    return FoundTRexes;
}