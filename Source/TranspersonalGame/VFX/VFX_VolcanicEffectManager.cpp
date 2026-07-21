#include "VFX_VolcanicEffectManager.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AVFX_VolcanicEffectManager::AVFX_VolcanicEffectManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second for performance

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create Niagara components
    AshCloudComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AshCloudComponent"));
    AshCloudComponent->SetupAttachment(RootComponent);
    AshCloudComponent->SetAutoActivate(false);

    VolcanicDustComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("VolcanicDustComponent"));
    VolcanicDustComponent->SetupAttachment(RootComponent);
    VolcanicDustComponent->SetAutoActivate(false);

    GroundParticlesComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("GroundParticlesComponent"));
    GroundParticlesComponent->SetupAttachment(RootComponent);
    GroundParticlesComponent->SetAutoActivate(false);

    // Create audio components
    VolcanicRumbleAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("VolcanicRumbleAudio"));
    VolcanicRumbleAudio->SetupAttachment(RootComponent);
    VolcanicRumbleAudio->SetAutoActivate(false);

    WindAshAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("WindAshAudio"));
    WindAshAudio->SetupAttachment(RootComponent);
    WindAshAudio->SetAutoActivate(false);

    // Initialize default settings
    VolcanicSettings.AshDensity = 1.0f;
    VolcanicSettings.WindStrength = 0.5f;
    VolcanicSettings.WindDirection = FVector(1.0f, 0.0f, 0.0f);
    VolcanicSettings.EffectRadius = 10000.0f;
    VolcanicSettings.bEnableDistanceAttenuation = true;

    CurrentIntensity = EVFX_VolcanicIntensity::LowActivity;
    IntensityChangeRate = 0.1f;
    bAutoIntensityVariation = true;
}

void AVFX_VolcanicEffectManager::BeginPlay()
{
    Super::BeginPlay();

    // Load Niagara systems if not set in Blueprint
    if (!AshCloudSystem)
    {
        AshCloudSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/NS_Volcanic_Ash"));
    }
    if (!VolcanicDustSystem)
    {
        VolcanicDustSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/NS_Dust_Impact"));
    }
    if (!GroundParticleSystem)
    {
        GroundParticleSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/NS_Ground_Particles"));
    }

    // Assign Niagara systems to components
    if (AshCloudSystem && AshCloudComponent)
    {
        AshCloudComponent->SetAsset(AshCloudSystem);
    }
    if (VolcanicDustSystem && VolcanicDustComponent)
    {
        VolcanicDustComponent->SetAsset(VolcanicDustSystem);
    }
    if (GroundParticleSystem && GroundParticlesComponent)
    {
        GroundParticlesComponent->SetAsset(GroundParticleSystem);
    }

    // Start with low activity
    StartVolcanicActivity();
}

void AVFX_VolcanicEffectManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle auto intensity variation
    if (bAutoIntensityVariation)
    {
        HandleAutoIntensityVariation(DeltaTime);
    }

    // Handle volcanic events
    if (bVolcanicEventActive)
    {
        VolcanicEventTimer += DeltaTime;
        if (VolcanicEventTimer >= VolcanicEventDuration)
        {
            bVolcanicEventActive = false;
            SetVolcanicIntensity(EVFX_VolcanicIntensity::LowActivity);
        }
    }

    // Update effect parameters
    UpdateIntensityParameters();
    UpdateNiagaraParameters();
    UpdateAudioParameters();
}

void AVFX_VolcanicEffectManager::SetVolcanicIntensity(EVFX_VolcanicIntensity NewIntensity)
{
    CurrentIntensity = NewIntensity;

    // Set target ash density based on intensity
    switch (CurrentIntensity)
    {
    case EVFX_VolcanicIntensity::Dormant:
        TargetAshDensity = 0.0f;
        break;
    case EVFX_VolcanicIntensity::LowActivity:
        TargetAshDensity = 0.3f;
        break;
    case EVFX_VolcanicIntensity::Moderate:
        TargetAshDensity = 0.6f;
        break;
    case EVFX_VolcanicIntensity::HighActivity:
        TargetAshDensity = 0.9f;
        break;
    case EVFX_VolcanicIntensity::Erupting:
        TargetAshDensity = 1.5f;
        break;
    }
}

void AVFX_VolcanicEffectManager::TriggerVolcanicEvent(float Duration)
{
    bVolcanicEventActive = true;
    VolcanicEventTimer = 0.0f;
    VolcanicEventDuration = Duration;
    
    // Increase intensity during event
    SetVolcanicIntensity(EVFX_VolcanicIntensity::HighActivity);
}

void AVFX_VolcanicEffectManager::UpdateWindParameters(FVector NewWindDirection, float NewWindStrength)
{
    VolcanicSettings.WindDirection = NewWindDirection.GetSafeNormal();
    VolcanicSettings.WindStrength = FMath::Clamp(NewWindStrength, 0.0f, 2.0f);
}

void AVFX_VolcanicEffectManager::SetAshDensity(float NewDensity)
{
    VolcanicSettings.AshDensity = FMath::Clamp(NewDensity, 0.0f, 2.0f);
    TargetAshDensity = VolcanicSettings.AshDensity;
}

float AVFX_VolcanicEffectManager::GetDistanceToPlayer() const
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                return FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
            }
        }
    }
    return 10000.0f; // Default large distance
}

void AVFX_VolcanicEffectManager::StartVolcanicActivity()
{
    if (AshCloudComponent)
    {
        AshCloudComponent->Activate(true);
    }
    if (VolcanicDustComponent)
    {
        VolcanicDustComponent->Activate(true);
    }
    if (GroundParticlesComponent)
    {
        GroundParticlesComponent->Activate(true);
    }
    if (VolcanicRumbleAudio && VolcanicRumbleSound)
    {
        VolcanicRumbleAudio->SetSound(VolcanicRumbleSound);
        VolcanicRumbleAudio->Play();
    }
    if (WindAshAudio && WindAshSound)
    {
        WindAshAudio->SetSound(WindAshSound);
        WindAshAudio->Play();
    }
}

void AVFX_VolcanicEffectManager::StopVolcanicActivity()
{
    if (AshCloudComponent)
    {
        AshCloudComponent->Deactivate();
    }
    if (VolcanicDustComponent)
    {
        VolcanicDustComponent->Deactivate();
    }
    if (GroundParticlesComponent)
    {
        GroundParticlesComponent->Deactivate();
    }
    if (VolcanicRumbleAudio)
    {
        VolcanicRumbleAudio->Stop();
    }
    if (WindAshAudio)
    {
        WindAshAudio->Stop();
    }
}

void AVFX_VolcanicEffectManager::UpdateIntensityParameters()
{
    // Smoothly interpolate ash density
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    CurrentAshDensity = FMath::FInterpTo(CurrentAshDensity, TargetAshDensity, DeltaTime, IntensityChangeRate);
}

void AVFX_VolcanicEffectManager::UpdateNiagaraParameters()
{
    float DistanceAttenuation = CalculateDistanceAttenuation();
    float EffectiveAshDensity = CurrentAshDensity * DistanceAttenuation;

    // Update ash cloud parameters
    if (AshCloudComponent)
    {
        AshCloudComponent->SetFloatParameter(TEXT("AshDensity"), EffectiveAshDensity);
        AshCloudComponent->SetVectorParameter(TEXT("WindDirection"), VolcanicSettings.WindDirection);
        AshCloudComponent->SetFloatParameter(TEXT("WindStrength"), VolcanicSettings.WindStrength);
    }

    // Update volcanic dust parameters
    if (VolcanicDustComponent)
    {
        VolcanicDustComponent->SetFloatParameter(TEXT("DustAmount"), EffectiveAshDensity * 0.8f);
        VolcanicDustComponent->SetVectorParameter(TEXT("WindDirection"), VolcanicSettings.WindDirection);
    }

    // Update ground particles
    if (GroundParticlesComponent)
    {
        GroundParticlesComponent->SetFloatParameter(TEXT("ParticleRate"), EffectiveAshDensity * 100.0f);
        GroundParticlesComponent->SetFloatParameter(TEXT("ParticleSize"), 0.5f + EffectiveAshDensity * 0.5f);
    }
}

void AVFX_VolcanicEffectManager::UpdateAudioParameters()
{
    float DistanceAttenuation = CalculateDistanceAttenuation();
    float EffectiveVolume = CurrentAshDensity * DistanceAttenuation;

    // Update volcanic rumble audio
    if (VolcanicRumbleAudio)
    {
        VolcanicRumbleAudio->SetVolumeMultiplier(EffectiveVolume * 0.7f);
        VolcanicRumbleAudio->SetPitchMultiplier(0.8f + CurrentAshDensity * 0.4f);
    }

    // Update wind ash audio
    if (WindAshAudio)
    {
        WindAshAudio->SetVolumeMultiplier(EffectiveVolume * VolcanicSettings.WindStrength);
        WindAshAudio->SetPitchMultiplier(1.0f + VolcanicSettings.WindStrength * 0.3f);
    }
}

void AVFX_VolcanicEffectManager::HandleAutoIntensityVariation(float DeltaTime)
{
    IntensityTimer += DeltaTime;

    // Change intensity every 60-120 seconds
    float ChangeInterval = FMath::RandRange(60.0f, 120.0f);
    if (IntensityTimer >= ChangeInterval)
    {
        IntensityTimer = 0.0f;

        // Randomly select new intensity (avoid Erupting for auto variation)
        int32 IntensityIndex = FMath::RandRange(0, 3);
        EVFX_VolcanicIntensity NewIntensity = static_cast<EVFX_VolcanicIntensity>(IntensityIndex);
        SetVolcanicIntensity(NewIntensity);
    }
}

float AVFX_VolcanicEffectManager::CalculateDistanceAttenuation() const
{
    if (!VolcanicSettings.bEnableDistanceAttenuation)
    {
        return 1.0f;
    }

    float Distance = GetDistanceToPlayer();
    float AttenuationFactor = FMath::Clamp(1.0f - (Distance / VolcanicSettings.EffectRadius), 0.1f, 1.0f);
    return AttenuationFactor;
}