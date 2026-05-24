#include "Audio_PolishEffectsSystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

UAudio_PolishEffectsSystem::UAudio_PolishEffectsSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize default values
    MaxShakeIntensity = 2.0f;
    ShakeRadius = 1000.0f;
    FlashColor = FLinearColor::Red;
    FlashOpacity = 0.5f;
    CurrentTimeOfDay = 12.0f;
    DayNightSpeed = 1.0f;

    // Create audio component
    AudioFeedbackComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioFeedbackComponent"));
    if (AudioFeedbackComponent)
    {
        AudioFeedbackComponent->bAutoActivate = false;
    }
}

void UAudio_PolishEffectsSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeReferences();
    FindSunLight();
}

void UAudio_PolishEffectsSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update damage flash effect
    if (bDamageFlashActive)
    {
        UpdateDamageFlash(DeltaTime);
    }

    // Update day/night cycle
    CurrentTimeOfDay += DeltaTime * DayNightSpeed / 3600.0f; // Convert to hours
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay = 0.0f;
    }
    UpdateDayNightCycle(CurrentTimeOfDay);

    // Check for proximity-based screen shake
    CheckProximityShake();
}

void UAudio_PolishEffectsSystem::InitializeReferences()
{
    if (UWorld* World = GetWorld())
    {
        PlayerController = World->GetFirstPlayerController();
        if (PlayerController)
        {
            PlayerCharacter = Cast<ACharacter>(PlayerController->GetPawn());
        }
    }
}

void UAudio_PolishEffectsSystem::FindSunLight()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
        
        if (FoundActors.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundActors[0]);
            if (SunLight)
            {
                UE_LOG(LogTemp, Log, TEXT("Audio_PolishEffectsSystem: Found sun light for day/night cycle"));
            }
        }
    }
}

void UAudio_PolishEffectsSystem::TriggerScreenShake(float Intensity, float Duration, FVector SourceLocation)
{
    if (!PlayerController || !PlayerCharacter)
    {
        return;
    }

    // Calculate distance-based intensity
    float Distance = CalculateDistanceToPlayer(SourceLocation);
    float DistanceRatio = FMath::Clamp(1.0f - (Distance / ShakeRadius), 0.0f, 1.0f);
    float FinalIntensity = Intensity * DistanceRatio;

    if (FinalIntensity > 0.1f)
    {
        ApplyScreenShakeToCamera(FinalIntensity);
        
        // Log for debugging
        UE_LOG(LogTemp, Log, TEXT("Screen shake triggered: Intensity=%.2f, Distance=%.2f"), FinalIntensity, Distance);
    }
}

void UAudio_PolishEffectsSystem::TriggerDamageFlash(float Intensity, float Duration)
{
    bDamageFlashActive = true;
    DamageFlashTimer = 0.0f;
    DamageFlashDuration = Duration;
    
    UE_LOG(LogTemp, Log, TEXT("Damage flash triggered: Intensity=%.2f, Duration=%.2f"), Intensity, Duration);
}

void UAudio_PolishEffectsSystem::TriggerFootstepDust(FVector Location, float Scale)
{
    if (!FootstepDustEffect)
    {
        return;
    }

    if (UWorld* World = GetWorld())
    {
        // Spawn particle system at footstep location
        UGameplayStatics::SpawnEmitterAtLocation(
            World,
            FootstepDustEffect,
            Location,
            FRotator::ZeroRotator,
            FVector(Scale),
            true,
            EPSCPoolMethod::AutoRelease
        );
        
        UE_LOG(LogTemp, Log, TEXT("Footstep dust spawned at location: %s"), *Location.ToString());
    }
}

void UAudio_PolishEffectsSystem::TriggerAudioFeedback(const FString& EventName, float Volume)
{
    if (!AudioFeedbackComponent)
    {
        return;
    }

    // Play audio feedback based on event name
    if (EventName == TEXT("Footstep"))
    {
        // Play footstep sound
        AudioFeedbackComponent->SetVolumeMultiplier(Volume * 0.5f);
    }
    else if (EventName == TEXT("Damage"))
    {
        // Play damage sound
        AudioFeedbackComponent->SetVolumeMultiplier(Volume * 0.8f);
    }
    else if (EventName == TEXT("DinosaurNearby"))
    {
        // Play tension sound
        AudioFeedbackComponent->SetVolumeMultiplier(Volume * 0.6f);
    }

    if (!AudioFeedbackComponent->IsPlaying())
    {
        AudioFeedbackComponent->Play();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio feedback triggered: %s (Volume: %.2f)"), *EventName, Volume);
}

void UAudio_PolishEffectsSystem::UpdateDayNightCycle(float TimeOfDay)
{
    if (!SunLight || !SunLight->GetLightComponent())
    {
        return;
    }

    // Calculate sun angle based on time of day (0-24 hours)
    float SunAngle = (TimeOfDay - 6.0f) * 15.0f; // 6 AM = 0 degrees, 6 PM = 180 degrees
    
    // Set sun rotation
    FRotator SunRotation = FRotator(-SunAngle, 30.0f, 0.0f);
    SunLight->SetActorRotation(SunRotation);

    // Adjust light intensity based on time
    float LightIntensity = 1.0f;
    if (TimeOfDay < 6.0f || TimeOfDay > 18.0f)
    {
        // Night time - reduce intensity
        LightIntensity = 0.1f;
    }
    else if (TimeOfDay < 8.0f || TimeOfDay > 16.0f)
    {
        // Dawn/dusk - medium intensity
        LightIntensity = 0.5f;
    }

    SunLight->GetLightComponent()->SetIntensity(LightIntensity * 3.0f);
}

void UAudio_PolishEffectsSystem::CheckProximityShake()
{
    if (!PlayerCharacter)
    {
        return;
    }

    // Find nearby large dinosaurs (T-Rex, Brachiosaurus)
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetName().Contains(TEXT("TRex")))
            {
                float Distance = CalculateDistanceToPlayer(Actor->GetActorLocation());
                if (Distance < ShakeRadius)
                {
                    // Trigger screen shake based on proximity
                    float ShakeIntensity = FMath::Lerp(0.1f, MaxShakeIntensity, 1.0f - (Distance / ShakeRadius));
                    TriggerScreenShake(ShakeIntensity, 0.2f, Actor->GetActorLocation());
                    
                    // Trigger audio feedback
                    TriggerAudioFeedback(TEXT("DinosaurNearby"), 0.7f);
                }
            }
        }
    }
}

void UAudio_PolishEffectsSystem::UpdateDamageFlash(float DeltaTime)
{
    if (!bDamageFlashActive)
    {
        return;
    }

    DamageFlashTimer += DeltaTime;
    
    if (DamageFlashTimer >= DamageFlashDuration)
    {
        bDamageFlashActive = false;
        DamageFlashTimer = 0.0f;
    }
    else
    {
        // Calculate flash opacity based on timer
        float FlashProgress = DamageFlashTimer / DamageFlashDuration;
        float CurrentOpacity = FlashOpacity * (1.0f - FlashProgress);
        
        // Apply flash effect to screen (would need HUD implementation)
        // For now, just log the effect
        if (FMath::Fmod(DamageFlashTimer, 0.1f) < 0.05f)
        {
            UE_LOG(LogTemp, Log, TEXT("Damage flash active: Opacity=%.2f"), CurrentOpacity);
        }
    }
}

float UAudio_PolishEffectsSystem::CalculateDistanceToPlayer(FVector Location)
{
    if (!PlayerCharacter)
    {
        return 9999.0f;
    }

    return FVector::Dist(PlayerCharacter->GetActorLocation(), Location);
}

void UAudio_PolishEffectsSystem::ApplyScreenShakeToCamera(float Intensity)
{
    if (!PlayerController)
    {
        return;
    }

    // Create a simple camera shake effect
    // In a full implementation, this would use UE5's camera shake system
    if (PlayerCharacter)
    {
        UCameraComponent* CameraComp = PlayerCharacter->FindComponentByClass<UCameraComponent>();
        if (CameraComp)
        {
            // Apply a small random offset to simulate shake
            FVector RandomOffset = FVector(
                FMath::RandRange(-Intensity, Intensity),
                FMath::RandRange(-Intensity, Intensity),
                FMath::RandRange(-Intensity * 0.5f, Intensity * 0.5f)
            );
            
            // This is a simplified shake - in production would use proper camera shake classes
            UE_LOG(LogTemp, Log, TEXT("Camera shake applied: Offset=%s"), *RandomOffset.ToString());
        }
    }
}