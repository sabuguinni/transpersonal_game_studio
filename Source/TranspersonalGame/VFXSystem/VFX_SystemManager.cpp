#include "VFX_SystemManager.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"

AVFX_SystemManager::AVFX_SystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update every 100ms for performance

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize VFX components
    FootstepDustComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FootstepDustComponent"));
    FootstepDustComponent->SetupAttachment(RootComponent);
    FootstepDustComponent->bAutoActivate = false;

    CampfireComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("CampfireComponent"));
    CampfireComponent->SetupAttachment(RootComponent);
    CampfireComponent->bAutoActivate = false;

    BloodImpactComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("BloodImpactComponent"));
    BloodImpactComponent->SetupAttachment(RootComponent);
    BloodImpactComponent->bAutoActivate = false;

    WeatherRainComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("WeatherRainComponent"));
    WeatherRainComponent->SetupAttachment(RootComponent);
    WeatherRainComponent->bAutoActivate = false;

    DinoBreathComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("DinoBreathComponent"));
    DinoBreathComponent->SetupAttachment(RootComponent);
    DinoBreathComponent->bAutoActivate = false;

    WeaponImpactComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("WeaponImpactComponent"));
    WeaponImpactComponent->SetupAttachment(RootComponent);
    WeaponImpactComponent->bAutoActivate = false;

    // Set default values
    GlobalVFXIntensity = 1.0f;
    bVFXEnabled = true;
    MaxVFXDistance = 5000.0f;
    MaxActiveEffects = 50;
}

void AVFX_SystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeVFXComponents();
    
    // Start cleanup timer
    GetWorldTimerManager().SetTimer(CleanupTimerHandle, this, &AVFX_SystemManager::CleanupFinishedEffects, 2.0f, true);
    
    UE_LOG(LogTemp, Warning, TEXT("VFX System Manager initialized with %d max effects"), MaxActiveEffects);
}

void AVFX_SystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update active effects count for performance monitoring
    if (ActiveEffects.Num() > MaxActiveEffects)
    {
        CleanupFinishedEffects();
    }
}

void AVFX_SystemManager::InitializeVFXComponents()
{
    // Configure footstep dust component
    if (FootstepDustComponent)
    {
        FootstepDustComponent->SetRelativeScale3D(FVector(1.5f, 1.5f, 1.0f));
    }

    // Configure campfire component
    if (CampfireComponent)
    {
        CampfireComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.2f));
    }

    // Configure blood impact component
    if (BloodImpactComponent)
    {
        BloodImpactComponent->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.8f));
    }

    // Configure weather rain component
    if (WeatherRainComponent)
    {
        WeatherRainComponent->SetRelativeScale3D(FVector(10.0f, 10.0f, 5.0f));
    }

    // Configure dino breath component
    if (DinoBreathComponent)
    {
        DinoBreathComponent->SetRelativeScale3D(FVector(2.0f, 2.0f, 1.5f));
    }

    // Configure weapon impact component
    if (WeaponImpactComponent)
    {
        WeaponImpactComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
    }
}

void AVFX_SystemManager::TriggerFootstepDust(FVector Location, float Intensity)
{
    if (!bVFXEnabled || !ShouldPlayEffect(Location))
    {
        return;
    }

    FVFX_EffectSettings Settings;
    Settings.EffectType = EVFX_EffectType::FootstepDust;
    Settings.Duration = 1.5f;
    Settings.Intensity = Intensity * GlobalVFXIntensity;
    Settings.Scale = FVector(Intensity * 1.5f, Intensity * 1.5f, 1.0f);
    Settings.bAutoDestroy = true;
    Settings.FadeOutTime = 0.8f;

    PlayEffect(EVFX_EffectType::FootstepDust, Location, Settings);
    
    UE_LOG(LogTemp, Log, TEXT("Footstep dust triggered at location: %s with intensity: %f"), 
           *Location.ToString(), Intensity);
}

void AVFX_SystemManager::StartCampfire(FVector Location)
{
    if (!bVFXEnabled || !ShouldPlayEffect(Location))
    {
        return;
    }

    if (CampfireComponent)
    {
        CampfireComponent->SetWorldLocation(Location);
        CampfireComponent->Activate(true);
        
        UE_LOG(LogTemp, Log, TEXT("Campfire started at location: %s"), *Location.ToString());
    }
}

void AVFX_SystemManager::StopCampfire()
{
    if (CampfireComponent && CampfireComponent->IsActive())
    {
        CampfireComponent->Deactivate();
        UE_LOG(LogTemp, Log, TEXT("Campfire stopped"));
    }
}

void AVFX_SystemManager::TriggerBloodImpact(FVector Location, FVector Direction, float Intensity)
{
    if (!bVFXEnabled || !ShouldPlayEffect(Location))
    {
        return;
    }

    FVFX_EffectSettings Settings;
    Settings.EffectType = EVFX_EffectType::BloodImpact;
    Settings.Duration = 0.8f;
    Settings.Intensity = Intensity * GlobalVFXIntensity;
    Settings.Scale = FVector(Intensity, Intensity, Intensity);
    Settings.bAutoDestroy = true;
    Settings.FadeOutTime = 0.5f;

    PlayEffect(EVFX_EffectType::BloodImpact, Location, Settings);
    
    UE_LOG(LogTemp, Log, TEXT("Blood impact triggered at location: %s"), *Location.ToString());
}

void AVFX_SystemManager::StartWeatherRain(float Intensity)
{
    if (!bVFXEnabled)
    {
        return;
    }

    if (WeatherRainComponent)
    {
        WeatherRainComponent->SetFloatParameter(FName("Intensity"), Intensity * GlobalVFXIntensity);
        WeatherRainComponent->Activate(true);
        
        UE_LOG(LogTemp, Log, TEXT("Weather rain started with intensity: %f"), Intensity);
    }
}

void AVFX_SystemManager::StopWeatherRain()
{
    if (WeatherRainComponent && WeatherRainComponent->IsActive())
    {
        WeatherRainComponent->Deactivate();
        UE_LOG(LogTemp, Log, TEXT("Weather rain stopped"));
    }
}

void AVFX_SystemManager::TriggerDinoBreath(FVector Location, FVector Direction)
{
    if (!bVFXEnabled || !ShouldPlayEffect(Location))
    {
        return;
    }

    FVFX_EffectSettings Settings;
    Settings.EffectType = EVFX_EffectType::DinoBreath;
    Settings.Duration = 2.0f;
    Settings.Intensity = GlobalVFXIntensity;
    Settings.Scale = FVector(1.5f, 1.5f, 1.0f);
    Settings.bAutoDestroy = true;
    Settings.FadeOutTime = 1.0f;

    PlayEffect(EVFX_EffectType::DinoBreath, Location, Settings);
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur breath triggered at location: %s"), *Location.ToString());
}

void AVFX_SystemManager::TriggerWeaponImpact(FVector Location, FVector Direction, float Damage)
{
    if (!bVFXEnabled || !ShouldPlayEffect(Location))
    {
        return;
    }

    FVFX_EffectSettings Settings;
    Settings.EffectType = EVFX_EffectType::WeaponImpact;
    Settings.Duration = 1.0f;
    Settings.Intensity = FMath::Clamp(Damage / 100.0f, 0.1f, 2.0f) * GlobalVFXIntensity;
    Settings.Scale = FVector(Settings.Intensity, Settings.Intensity, Settings.Intensity);
    Settings.bAutoDestroy = true;
    Settings.FadeOutTime = 0.6f;

    PlayEffect(EVFX_EffectType::WeaponImpact, Location, Settings);
    
    UE_LOG(LogTemp, Log, TEXT("Weapon impact triggered at location: %s with damage: %f"), 
           *Location.ToString(), Damage);
}

void AVFX_SystemManager::PlayEffect(EVFX_EffectType EffectType, FVector Location, const FVFX_EffectSettings& Settings)
{
    if (!bVFXEnabled || ActiveEffects.Num() >= MaxActiveEffects)
    {
        return;
    }

    UParticleSystemComponent* EffectComponent = CreateEffectComponent(EffectType);
    if (EffectComponent)
    {
        EffectComponent->SetWorldLocation(Location);
        ConfigureEffectComponent(EffectComponent, Settings);
        EffectComponent->Activate(true);
        
        ActiveEffects.Add(EffectComponent);
        
        // Auto-destroy after duration
        if (Settings.bAutoDestroy)
        {
            FTimerHandle DestroyTimer;
            GetWorldTimerManager().SetTimer(DestroyTimer, [this, EffectComponent]()
            {
                if (IsValid(EffectComponent))
                {
                    EffectComponent->DestroyComponent();
                    ActiveEffects.Remove(EffectComponent);
                }
            }, Settings.Duration + Settings.FadeOutTime, false);
        }
    }
}

void AVFX_SystemManager::StopAllEffects()
{
    for (UParticleSystemComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            Effect->Deactivate();
        }
    }
    
    // Stop persistent effects
    if (CampfireComponent && CampfireComponent->IsActive())
    {
        CampfireComponent->Deactivate();
    }
    
    if (WeatherRainComponent && WeatherRainComponent->IsActive())
    {
        WeatherRainComponent->Deactivate();
    }
    
    UE_LOG(LogTemp, Log, TEXT("All VFX effects stopped"));
}

void AVFX_SystemManager::SetGlobalVFXIntensity(float NewIntensity)
{
    GlobalVFXIntensity = FMath::Clamp(NewIntensity, 0.0f, 3.0f);
    UE_LOG(LogTemp, Log, TEXT("Global VFX intensity set to: %f"), GlobalVFXIntensity);
}

void AVFX_SystemManager::CleanupFinishedEffects()
{
    ActiveEffects.RemoveAll([](UParticleSystemComponent* Effect)
    {
        return !IsValid(Effect) || !Effect->IsActive();
    });
}

bool AVFX_SystemManager::ShouldPlayEffect(FVector Location) const
{
    if (!bVFXEnabled)
    {
        return false;
    }

    // Check distance from player
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && PC->GetPawn())
    {
        float Distance = FVector::Dist(Location, PC->GetPawn()->GetActorLocation());
        return Distance <= MaxVFXDistance;
    }

    return true;
}

UParticleSystemComponent* AVFX_SystemManager::CreateEffectComponent(EVFX_EffectType EffectType)
{
    UParticleSystemComponent* NewComponent = NewObject<UParticleSystemComponent>(this);
    if (NewComponent)
    {
        NewComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
        NewComponent->RegisterComponent();
    }
    
    return NewComponent;
}

void AVFX_SystemManager::ConfigureEffectComponent(UParticleSystemComponent* Component, const FVFX_EffectSettings& Settings)
{
    if (!Component)
    {
        return;
    }

    Component->SetRelativeScale3D(Settings.Scale);
    Component->SetFloatParameter(FName("Intensity"), Settings.Intensity);
    Component->SetFloatParameter(FName("Duration"), Settings.Duration);
    
    // Configure fade out
    if (Settings.FadeOutTime > 0.0f)
    {
        FTimerHandle FadeTimer;
        GetWorldTimerManager().SetTimer(FadeTimer, [Component, Settings]()
        {
            if (IsValid(Component))
            {
                Component->SetFloatParameter(FName("FadeOut"), 1.0f);
            }
        }, Settings.Duration - Settings.FadeOutTime, false);
    }
}