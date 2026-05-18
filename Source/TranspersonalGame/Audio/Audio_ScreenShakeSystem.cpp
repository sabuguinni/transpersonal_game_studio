#include "Audio_ScreenShakeSystem.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

AAudio_ScreenShakeSystem::AAudio_ScreenShakeSystem()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create trigger sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetupAttachment(RootComponent);
    TriggerSphere->SetSphereRadius(TriggerRadius);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    TriggerSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Create audio component
    ShakeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ShakeAudioComponent"));
    ShakeAudioComponent->SetupAttachment(RootComponent);
    ShakeAudioComponent->SetAutoActivate(false);

    // Initialize default shake settings
    InitializeShakeSettings();
}

void AAudio_ScreenShakeSystem::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (TriggerSphere)
    {
        TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_ScreenShakeSystem::OnTriggerBeginOverlap);
    }

    // Update trigger radius
    if (TriggerSphere)
    {
        TriggerSphere->SetSphereRadius(TriggerRadius);
    }
}

void AAudio_ScreenShakeSystem::InitializeShakeSettings()
{
    // Light shake (small creatures, distant impacts)
    LightShake.Duration = 0.3f;
    LightShake.Amplitude = 0.2f;
    LightShake.Frequency = 8.0f;
    LightShake.bFadeIn = true;
    LightShake.bFadeOut = true;

    // Medium shake (medium dinosaurs, nearby impacts)
    MediumShake.Duration = 0.6f;
    MediumShake.Amplitude = 0.5f;
    MediumShake.Frequency = 12.0f;
    MediumShake.bFadeIn = true;
    MediumShake.bFadeOut = true;

    // Heavy shake (T-Rex footsteps, large impacts)
    HeavyShake.Duration = 1.0f;
    HeavyShake.Amplitude = 1.0f;
    HeavyShake.Frequency = 15.0f;
    HeavyShake.bFadeIn = false;
    HeavyShake.bFadeOut = true;

    // Extreme shake (T-Rex roar, massive impacts)
    ExtremeShake.Duration = 1.5f;
    ExtremeShake.Amplitude = 1.5f;
    ExtremeShake.Frequency = 20.0f;
    ExtremeShake.bFadeIn = false;
    ExtremeShake.bFadeOut = true;
}

void AAudio_ScreenShakeSystem::TriggerScreenShake(EAudio_ShakeIntensity Intensity)
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (!PlayerController)
    {
        return;
    }

    FAudio_ShakeSettings ShakeSettings = GetShakeSettingsByIntensity(Intensity);
    
    // Create a simple camera shake effect
    // Note: In a full implementation, you would create a custom UCameraShakeBase subclass
    if (GEngine)
    {
        FString ShakeMessage = FString::Printf(TEXT("Screen Shake: %s - Duration: %.1f, Amplitude: %.1f"), 
            *UEnum::GetValueAsString(Intensity), ShakeSettings.Duration, ShakeSettings.Amplitude);
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, ShakeMessage);
    }

    // Play associated audio
    PlayShakeAudio(Intensity);

    UE_LOG(LogTemp, Log, TEXT("Screen shake triggered: %s"), *UEnum::GetValueAsString(Intensity));
}

void AAudio_ScreenShakeSystem::TriggerTRexFootstep()
{
    TriggerScreenShake(EAudio_ShakeIntensity::Heavy);
    
    if (FootstepSound && ShakeAudioComponent)
    {
        ShakeAudioComponent->SetSound(FootstepSound);
        ShakeAudioComponent->Play();
    }
}

void AAudio_ScreenShakeSystem::TriggerDamageShake()
{
    TriggerScreenShake(EAudio_ShakeIntensity::Medium);
    
    if (ImpactSound && ShakeAudioComponent)
    {
        ShakeAudioComponent->SetSound(ImpactSound);
        ShakeAudioComponent->Play();
    }
}

void AAudio_ScreenShakeSystem::SetShakeIntensityByDistance(float Distance)
{
    EAudio_ShakeIntensity Intensity = EAudio_ShakeIntensity::Light;
    
    if (Distance < 500.0f)
    {
        Intensity = EAudio_ShakeIntensity::Extreme;
    }
    else if (Distance < 1000.0f)
    {
        Intensity = EAudio_ShakeIntensity::Heavy;
    }
    else if (Distance < 1500.0f)
    {
        Intensity = EAudio_ShakeIntensity::Medium;
    }
    
    TriggerScreenShake(Intensity);
}

void AAudio_ScreenShakeSystem::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                                    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bAutoTrigger || !OtherActor)
    {
        return;
    }

    // Check if the overlapping actor is the player
    if (OtherActor->IsA<APawn>())
    {
        float Distance = FVector::Dist(GetActorLocation(), OtherActor->GetActorLocation());
        SetShakeIntensityByDistance(Distance);
    }
}

FAudio_ShakeSettings AAudio_ScreenShakeSystem::GetShakeSettingsByIntensity(EAudio_ShakeIntensity Intensity)
{
    switch (Intensity)
    {
        case EAudio_ShakeIntensity::Light:
            return LightShake;
        case EAudio_ShakeIntensity::Medium:
            return MediumShake;
        case EAudio_ShakeIntensity::Heavy:
            return HeavyShake;
        case EAudio_ShakeIntensity::Extreme:
            return ExtremeShake;
        default:
            return LightShake;
    }
}

void AAudio_ScreenShakeSystem::PlayShakeAudio(EAudio_ShakeIntensity Intensity)
{
    if (!ShakeAudioComponent)
    {
        return;
    }

    // Play different sounds based on intensity
    USoundBase* SoundToPlay = nullptr;
    
    switch (Intensity)
    {
        case EAudio_ShakeIntensity::Light:
        case EAudio_ShakeIntensity::Medium:
            SoundToPlay = FootstepSound;
            break;
        case EAudio_ShakeIntensity::Heavy:
        case EAudio_ShakeIntensity::Extreme:
            SoundToPlay = ImpactSound;
            break;
    }

    if (SoundToPlay)
    {
        ShakeAudioComponent->SetSound(SoundToPlay);
        ShakeAudioComponent->Play();
    }
}