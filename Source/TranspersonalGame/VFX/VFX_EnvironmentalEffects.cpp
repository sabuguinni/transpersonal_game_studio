#include "VFX_EnvironmentalEffects.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"

AVFX_EnvironmentalEffects::AVFX_EnvironmentalEffects()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create primary particle system component
    PrimaryParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("PrimaryParticleComponent"));
    PrimaryParticleComponent->SetupAttachment(RootComponent);
    PrimaryParticleComponent->bAutoActivate = false;

    // Create secondary particle system component for layered effects
    SecondaryParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SecondaryParticleComponent"));
    SecondaryParticleComponent->SetupAttachment(RootComponent);
    SecondaryParticleComponent->bAutoActivate = false;

    // Create visual reference mesh component
    VisualReferenceComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualReferenceComponent"));
    VisualReferenceComponent->SetupAttachment(RootComponent);
    VisualReferenceComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    VisualReferenceComponent->SetVisibility(true);

    // Create audio component for environmental sounds
    EnvironmentalAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EnvironmentalAudioComponent"));
    EnvironmentalAudioComponent->SetupAttachment(RootComponent);
    EnvironmentalAudioComponent->bAutoActivate = false;

    // Initialize default settings
    EffectSettings.EffectType = EVFX_EnvironmentalType::DustCloud;
    EffectSettings.Intensity = 1.0f;
    EffectSettings.Duration = 5.0f;
    EffectSettings.bAutoActivate = true;
    EffectSettings.bLooping = false;

    bEffectActive = false;
}

void AVFX_EnvironmentalEffects::BeginPlay()
{
    Super::BeginPlay();

    // Setup initial particle system based on effect type
    SetupParticleSystemForType(EffectSettings.EffectType);

    // Auto-activate if enabled
    if (EffectSettings.bAutoActivate)
    {
        ActivateEnvironmentalEffect();
    }

    UE_LOG(LogTemp, Log, TEXT("VFX_EnvironmentalEffects: BeginPlay completed for effect type %d"), 
           static_cast<int32>(EffectSettings.EffectType));
}

void AVFX_EnvironmentalEffects::ActivateEnvironmentalEffect()
{
    if (bEffectActive)
    {
        return;
    }

    bEffectActive = true;

    // Activate particle systems
    if (PrimaryParticleComponent && PrimaryParticleComponent->Template)
    {
        PrimaryParticleComponent->Activate();
        UE_LOG(LogTemp, Log, TEXT("VFX_EnvironmentalEffects: Primary particle system activated"));
    }

    if (SecondaryParticleComponent && SecondaryParticleComponent->Template)
    {
        SecondaryParticleComponent->Activate();
        UE_LOG(LogTemp, Log, TEXT("VFX_EnvironmentalEffects: Secondary particle system activated"));
    }

    // Play environmental sound
    PlayEnvironmentalSound(EffectSettings.EffectType);

    // Set duration timer if not looping
    if (!EffectSettings.bLooping && EffectSettings.Duration > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            EffectDurationTimer,
            this,
            &AVFX_EnvironmentalEffects::OnEffectDurationComplete,
            EffectSettings.Duration,
            false
        );
    }

    UE_LOG(LogTemp, Log, TEXT("VFX_EnvironmentalEffects: Environmental effect activated"));
}

void AVFX_EnvironmentalEffects::DeactivateEnvironmentalEffect()
{
    if (!bEffectActive)
    {
        return;
    }

    StopAllEffects();
    bEffectActive = false;

    // Clear duration timer
    if (GetWorld() && EffectDurationTimer.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(EffectDurationTimer);
    }

    UE_LOG(LogTemp, Log, TEXT("VFX_EnvironmentalEffects: Environmental effect deactivated"));
}

void AVFX_EnvironmentalEffects::SetEffectType(EVFX_EnvironmentalType NewType)
{
    if (EffectSettings.EffectType != NewType)
    {
        bool bWasActive = bEffectActive;
        
        if (bWasActive)
        {
            DeactivateEnvironmentalEffect();
        }

        EffectSettings.EffectType = NewType;
        SetupParticleSystemForType(NewType);

        if (bWasActive)
        {
            ActivateEnvironmentalEffect();
        }

        UE_LOG(LogTemp, Log, TEXT("VFX_EnvironmentalEffects: Effect type changed to %d"), static_cast<int32>(NewType));
    }
}

void AVFX_EnvironmentalEffects::SetEffectIntensity(float NewIntensity)
{
    EffectSettings.Intensity = FMath::Clamp(NewIntensity, 0.1f, 10.0f);

    // Apply intensity to particle systems
    if (PrimaryParticleComponent)
    {
        PrimaryParticleComponent->SetFloatParameter(TEXT("Intensity"), EffectSettings.Intensity);
    }

    if (SecondaryParticleComponent)
    {
        SecondaryParticleComponent->SetFloatParameter(TEXT("Intensity"), EffectSettings.Intensity);
    }

    // Apply intensity to audio
    if (EnvironmentalAudioComponent)
    {
        EnvironmentalAudioComponent->SetVolumeMultiplier(EffectSettings.Intensity);
    }

    UE_LOG(LogTemp, Log, TEXT("VFX_EnvironmentalEffects: Intensity set to %f"), EffectSettings.Intensity);
}

void AVFX_EnvironmentalEffects::TriggerVolcanicEruption(FVector EruptionLocation, float EruptionPower)
{
    SetActorLocation(EruptionLocation);
    SetEffectType(EVFX_EnvironmentalType::VolcanicEruption);
    SetEffectIntensity(EruptionPower);
    ActivateEnvironmentalEffect();

    UE_LOG(LogTemp, Log, TEXT("VFX_EnvironmentalEffects: Volcanic eruption triggered at %s with power %f"), 
           *EruptionLocation.ToString(), EruptionPower);
}

void AVFX_EnvironmentalEffects::TriggerWaterSplash(FVector SplashLocation, float SplashSize)
{
    SetActorLocation(SplashLocation);
    SetEffectType(EVFX_EnvironmentalType::WaterSplash);
    SetEffectIntensity(SplashSize);
    ActivateEnvironmentalEffect();

    UE_LOG(LogTemp, Log, TEXT("VFX_EnvironmentalEffects: Water splash triggered at %s with size %f"), 
           *SplashLocation.ToString(), SplashSize);
}

void AVFX_EnvironmentalEffects::CreateDustCloud(FVector DustLocation, float CloudRadius)
{
    SetActorLocation(DustLocation);
    SetEffectType(EVFX_EnvironmentalType::DustCloud);
    
    // Scale intensity based on cloud radius
    float IntensityFromRadius = FMath::Clamp(CloudRadius / 500.0f, 0.1f, 10.0f);
    SetEffectIntensity(IntensityFromRadius);
    
    ActivateEnvironmentalEffect();

    UE_LOG(LogTemp, Log, TEXT("VFX_EnvironmentalEffects: Dust cloud created at %s with radius %f"), 
           *DustLocation.ToString(), CloudRadius);
}

void AVFX_EnvironmentalEffects::SetupParticleSystemForType(EVFX_EnvironmentalType Type)
{
    UParticleSystem* PrimarySystem = nullptr;
    UParticleSystem* SecondarySystem = nullptr;

    switch (Type)
    {
        case EVFX_EnvironmentalType::VolcanicEruption:
            if (VolcanicParticleSystems.Num() > 0)
            {
                PrimarySystem = VolcanicParticleSystems[0];
                if (VolcanicParticleSystems.Num() > 1)
                {
                    SecondarySystem = VolcanicParticleSystems[1];
                }
            }
            break;

        case EVFX_EnvironmentalType::WaterSplash:
            if (WaterSplashParticleSystems.Num() > 0)
            {
                PrimarySystem = WaterSplashParticleSystems[0];
                if (WaterSplashParticleSystems.Num() > 1)
                {
                    SecondarySystem = WaterSplashParticleSystems[1];
                }
            }
            break;

        case EVFX_EnvironmentalType::DustCloud:
            if (DustParticleSystems.Num() > 0)
            {
                PrimarySystem = DustParticleSystems[0];
                if (DustParticleSystems.Num() > 1)
                {
                    SecondarySystem = DustParticleSystems[1];
                }
            }
            break;

        default:
            break;
    }

    // Set particle systems
    if (PrimaryParticleComponent)
    {
        PrimaryParticleComponent->SetTemplate(PrimarySystem);
    }

    if (SecondaryParticleComponent)
    {
        SecondaryParticleComponent->SetTemplate(SecondarySystem);
    }

    UE_LOG(LogTemp, Log, TEXT("VFX_EnvironmentalEffects: Particle systems setup for type %d"), static_cast<int32>(Type));
}

void AVFX_EnvironmentalEffects::PlayEnvironmentalSound(EVFX_EnvironmentalType Type)
{
    if (EnvironmentalSounds.Num() > 0)
    {
        int32 SoundIndex = static_cast<int32>(Type) % EnvironmentalSounds.Num();
        USoundBase* SoundToPlay = EnvironmentalSounds[SoundIndex];

        if (SoundToPlay && EnvironmentalAudioComponent)
        {
            EnvironmentalAudioComponent->SetSound(SoundToPlay);
            EnvironmentalAudioComponent->Play();
            UE_LOG(LogTemp, Log, TEXT("VFX_EnvironmentalEffects: Environmental sound played for type %d"), static_cast<int32>(Type));
        }
    }
}

void AVFX_EnvironmentalEffects::StopAllEffects()
{
    if (PrimaryParticleComponent)
    {
        PrimaryParticleComponent->Deactivate();
    }

    if (SecondaryParticleComponent)
    {
        SecondaryParticleComponent->Deactivate();
    }

    if (EnvironmentalAudioComponent)
    {
        EnvironmentalAudioComponent->Stop();
    }

    UE_LOG(LogTemp, Log, TEXT("VFX_EnvironmentalEffects: All effects stopped"));
}

void AVFX_EnvironmentalEffects::OnEffectDurationComplete()
{
    DeactivateEnvironmentalEffect();
    UE_LOG(LogTemp, Log, TEXT("VFX_EnvironmentalEffects: Effect duration completed, deactivating"));
}