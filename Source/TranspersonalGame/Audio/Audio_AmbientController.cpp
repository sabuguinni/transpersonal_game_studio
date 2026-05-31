#include "Audio_AmbientController.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"

AAudio_AmbientController::AAudio_AmbientController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create trigger sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    RootComponent = TriggerSphere;
    TriggerSphere->SetSphereRadius(5000.0f);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create ambient audio component
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;

    // Initialize default values
    AmbientZone.BiomeType = EAudio_BiomeType::Savanna;
    AmbientZone.Radius = 5000.0f;
    AmbientZone.Volume = 0.8f;
    AmbientZone.bActive = true;

    bDayNightVariation = true;
    bWeatherVariation = true;
    bPlayerInZone = false;
    CurrentVolume = 0.0f;
    TargetVolume = 0.0f;
    bIsNightTime = false;
    bIsRaining = false;

    // Bind overlap events
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_AmbientController::OnTriggerBeginOverlap);
    TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_AmbientController::OnTriggerEndOverlap);
}

void AAudio_AmbientController::BeginPlay()
{
    Super::BeginPlay();
    
    // Set trigger sphere radius from ambient zone settings
    TriggerSphere->SetSphereRadius(AmbientZone.Radius);
    
    // Initialize audio component
    if (AmbientZone.bActive)
    {
        UpdateAmbientSound();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_AmbientController: BeginPlay - Ambient controller initialized for biome"));
}

void AAudio_AmbientController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Fade audio volume smoothly
    FadeAudioVolume(DeltaTime);
}

void AAudio_AmbientController::ActivateAmbientZone()
{
    AmbientZone.bActive = true;
    UpdateAmbientSound();
    UE_LOG(LogTemp, Log, TEXT("Audio_AmbientController: Ambient zone activated"));
}

void AAudio_AmbientController::DeactivateAmbientZone()
{
    AmbientZone.bActive = false;
    TargetVolume = 0.0f;
    UE_LOG(LogTemp, Log, TEXT("Audio_AmbientController: Ambient zone deactivated"));
}

void AAudio_AmbientController::SetAmbientVolume(float NewVolume)
{
    AmbientZone.Volume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    if (bPlayerInZone && AmbientZone.bActive)
    {
        TargetVolume = AmbientZone.Volume;
    }
    UE_LOG(LogTemp, Log, TEXT("Audio_AmbientController: Ambient volume set to %f"), NewVolume);
}

void AAudio_AmbientController::SwitchToNightAmbient()
{
    bIsNightTime = true;
    UpdateAmbientSound();
    UE_LOG(LogTemp, Log, TEXT("Audio_AmbientController: Switched to night ambient"));
}

void AAudio_AmbientController::SwitchToDayAmbient()
{
    bIsNightTime = false;
    UpdateAmbientSound();
    UE_LOG(LogTemp, Log, TEXT("Audio_AmbientController: Switched to day ambient"));
}

void AAudio_AmbientController::EnableWeatherAmbient(bool bRaining)
{
    bIsRaining = bRaining;
    UpdateAmbientSound();
    UE_LOG(LogTemp, Log, TEXT("Audio_AmbientController: Weather ambient %s"), bRaining ? TEXT("enabled") : TEXT("disabled"));
}

void AAudio_AmbientController::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        bPlayerInZone = true;
        if (AmbientZone.bActive)
        {
            TargetVolume = AmbientZone.Volume;
            UpdateAmbientSound();
        }
        UE_LOG(LogTemp, Log, TEXT("Audio_AmbientController: Player entered ambient zone"));
    }
}

void AAudio_AmbientController::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        bPlayerInZone = false;
        TargetVolume = 0.0f;
        UE_LOG(LogTemp, Log, TEXT("Audio_AmbientController: Player left ambient zone"));
    }
}

void AAudio_AmbientController::UpdateAmbientSound()
{
    if (!AmbientAudioComponent)
    {
        return;
    }

    USoundCue* CurrentSound = GetCurrentAmbientSound();
    if (CurrentSound)
    {
        if (AmbientAudioComponent->Sound != CurrentSound)
        {
            AmbientAudioComponent->SetSound(CurrentSound);
        }
        
        if (!AmbientAudioComponent->IsPlaying())
        {
            AmbientAudioComponent->Play();
        }
    }
}

void AAudio_AmbientController::FadeAudioVolume(float DeltaTime)
{
    if (!AmbientAudioComponent)
    {
        return;
    }

    // Smooth volume transition
    float FadeSpeed = 2.0f; // Volume units per second
    
    if (FMath::Abs(CurrentVolume - TargetVolume) > 0.01f)
    {
        if (CurrentVolume < TargetVolume)
        {
            CurrentVolume = FMath::Min(CurrentVolume + FadeSpeed * DeltaTime, TargetVolume);
        }
        else
        {
            CurrentVolume = FMath::Max(CurrentVolume - FadeSpeed * DeltaTime, TargetVolume);
        }
        
        AmbientAudioComponent->SetVolumeMultiplier(CurrentVolume);
    }

    // Stop audio if volume reaches zero
    if (CurrentVolume <= 0.01f && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Stop();
    }
}

USoundCue* AAudio_AmbientController::GetCurrentAmbientSound()
{
    // Priority: Weather > Night/Day > Default
    if (bIsRaining && bWeatherVariation && RainAmbientSoundCue.IsValid())
    {
        return RainAmbientSoundCue.Get();
    }
    
    if (bIsNightTime && bDayNightVariation && NightAmbientSoundCue.IsValid())
    {
        return NightAmbientSoundCue.Get();
    }
    
    if (AmbientSoundCue.IsValid())
    {
        return AmbientSoundCue.Get();
    }
    
    return nullptr;
}