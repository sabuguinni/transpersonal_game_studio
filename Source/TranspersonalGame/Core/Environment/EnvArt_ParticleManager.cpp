#include "EnvArt_ParticleManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Math/UnrealMathUtility.h"

AEnvArt_ParticleManager::AEnvArt_ParticleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    WindDirection = FVector(1.0f, 0.0f, 0.0f);
    WindStrength = 1.0f;
    UpdateFrequency = 1.0f;
    MaxViewDistance = 5000.0f;
    LastUpdateTime = 0.0f;
    
    // Create audio components
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    DinosaurAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DinosaurAudio"));
    
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->bAutoActivate = false;
        AmbientAudioComponent->SetVolumeMultiplier(0.3f);
    }
    
    if (DinosaurAudioComponent)
    {
        DinosaurAudioComponent->bAutoActivate = false;
        DinosaurAudioComponent->SetVolumeMultiplier(0.5f);
    }
}

void AEnvArt_ParticleManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Create default particle zones
    CreateForestDustZone(FVector(0.0f, 0.0f, 200.0f), FVector(2000.0f, 2000.0f, 400.0f));
    CreatePollenZone(FVector(1000.0f, 1000.0f, 300.0f), FVector(1500.0f, 1500.0f, 300.0f));
    CreateFireflyZone(FVector(-1000.0f, -1000.0f, 100.0f), FVector(800.0f, 800.0f, 200.0f));
    
    PlayAmbientForestSounds();
}

void AEnvArt_ParticleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= UpdateFrequency)
    {
        UpdateParticleZones();
        LastUpdateTime = 0.0f;
    }
}

void AEnvArt_ParticleManager::CreateForestDustZone(FVector Location, FVector Extent)
{
    FEnvArt_ParticleZone DustZone;
    DustZone.ZoneCenter = Location;
    DustZone.ZoneExtent = Extent;
    DustZone.ParticleType = EEnvArt_ParticleType::Dust;
    DustZone.ParticleDensity = 0.8f;
    DustZone.bActiveInDaylight = true;
    DustZone.bActiveAtNight = false;
    
    ParticleZones.Add(DustZone);
    SpawnParticleSystem(DustZone);
}

void AEnvArt_ParticleManager::CreatePollenZone(FVector Location, FVector Extent)
{
    FEnvArt_ParticleZone PollenZone;
    PollenZone.ZoneCenter = Location;
    PollenZone.ZoneExtent = Extent;
    PollenZone.ParticleType = EEnvArt_ParticleType::Pollen;
    PollenZone.ParticleDensity = 1.2f;
    PollenZone.bActiveInDaylight = true;
    PollenZone.bActiveAtNight = false;
    
    ParticleZones.Add(PollenZone);
    SpawnParticleSystem(PollenZone);
}

void AEnvArt_ParticleManager::CreateFireflyZone(FVector Location, FVector Extent)
{
    FEnvArt_ParticleZone FireflyZone;
    FireflyZone.ZoneCenter = Location;
    FireflyZone.ZoneExtent = Extent;
    FireflyZone.ParticleType = EEnvArt_ParticleType::Fireflies;
    FireflyZone.ParticleDensity = 0.5f;
    FireflyZone.bActiveInDaylight = false;
    FireflyZone.bActiveAtNight = true;
    
    ParticleZones.Add(FireflyZone);
    SpawnParticleSystem(FireflyZone);
}

void AEnvArt_ParticleManager::UpdateParticleZones()
{
    for (int32 i = 0; i < ActiveParticleSystems.Num(); i++)
    {
        if (ActiveParticleSystems[i] && i < ParticleZones.Num())
        {
            const FEnvArt_ParticleZone& Zone = ParticleZones[i];
            
            // Check if zone should be active based on time of day
            bool bShouldBeActive = IsZoneActiveForTimeOfDay(Zone);
            
            if (bShouldBeActive)
            {
                ActiveParticleSystems[i]->Activate();
                UpdateParticleSystemParameters(ActiveParticleSystems[i], Zone);
            }
            else
            {
                ActiveParticleSystems[i]->Deactivate();
            }
        }
    }
}

void AEnvArt_ParticleManager::SetWindParameters(FVector NewDirection, float NewStrength)
{
    WindDirection = NewDirection.GetSafeNormal();
    WindStrength = FMath::Clamp(NewStrength, 0.0f, 10.0f);
    
    // Update all active particle systems with new wind parameters
    for (UNiagaraComponent* ParticleSystem : ActiveParticleSystems)
    {
        if (ParticleSystem)
        {
            ParticleSystem->SetVectorParameter(FName("WindDirection"), WindDirection);
            ParticleSystem->SetFloatParameter(FName("WindStrength"), WindStrength);
        }
    }
}

void AEnvArt_ParticleManager::PlayAmbientForestSounds()
{
    if (AmbientAudioComponent)
    {
        // This would load and play forest ambient sounds
        AmbientAudioComponent->Play();
        UE_LOG(LogTemp, Warning, TEXT("ParticleManager: Playing ambient forest sounds"));
    }
}

void AEnvArt_ParticleManager::PlayDistantDinosaurRoars()
{
    if (DinosaurAudioComponent)
    {
        // This would play distant dinosaur roars at random intervals
        DinosaurAudioComponent->Play();
        UE_LOG(LogTemp, Warning, TEXT("ParticleManager: Playing distant dinosaur roars"));
    }
}

void AEnvArt_ParticleManager::SpawnParticleSystem(const FEnvArt_ParticleZone& Zone)
{
    if (UWorld* World = GetWorld())
    {
        // Create a Niagara component for this zone
        UNiagaraComponent* ParticleComponent = NewObject<UNiagaraComponent>(this);
        if (ParticleComponent)
        {
            ParticleComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
            ParticleComponent->SetWorldLocation(Zone.ZoneCenter);
            
            // Set initial parameters
            ParticleComponent->SetVectorParameter(FName("ZoneExtent"), Zone.ZoneExtent);
            ParticleComponent->SetFloatParameter(FName("ParticleDensity"), Zone.ParticleDensity);
            ParticleComponent->SetVectorParameter(FName("WindDirection"), WindDirection);
            ParticleComponent->SetFloatParameter(FName("WindStrength"), WindStrength);
            
            // Set particle type specific parameters
            switch (Zone.ParticleType)
            {
                case EEnvArt_ParticleType::Dust:
                    ParticleComponent->SetFloatParameter(FName("ParticleSize"), 0.5f);
                    ParticleComponent->SetVectorParameter(FName("ParticleColor"), FVector(0.8f, 0.7f, 0.6f));
                    break;
                case EEnvArt_ParticleType::Pollen:
                    ParticleComponent->SetFloatParameter(FName("ParticleSize"), 0.3f);
                    ParticleComponent->SetVectorParameter(FName("ParticleColor"), FVector(1.0f, 0.9f, 0.2f));
                    break;
                case EEnvArt_ParticleType::Fireflies:
                    ParticleComponent->SetFloatParameter(FName("ParticleSize"), 1.0f);
                    ParticleComponent->SetVectorParameter(FName("ParticleColor"), FVector(0.8f, 1.0f, 0.3f));
                    ParticleComponent->SetFloatParameter(FName("EmissiveStrength"), 5.0f);
                    break;
                default:
                    break;
            }
            
            ActiveParticleSystems.Add(ParticleComponent);
            
            UE_LOG(LogTemp, Warning, TEXT("ParticleManager: Spawned particle system at %s"), *Zone.ZoneCenter.ToString());
        }
    }
}

void AEnvArt_ParticleManager::UpdateParticleSystemParameters(UNiagaraComponent* ParticleSystem, const FEnvArt_ParticleZone& Zone)
{
    if (ParticleSystem)
    {
        // Update wind parameters
        ParticleSystem->SetVectorParameter(FName("WindDirection"), WindDirection);
        ParticleSystem->SetFloatParameter(FName("WindStrength"), WindStrength);
        
        // Update density based on current conditions
        float CurrentDensity = Zone.ParticleDensity;
        
        // Reduce density in strong wind
        if (WindStrength > 3.0f)
        {
            CurrentDensity *= 0.5f;
        }
        
        ParticleSystem->SetFloatParameter(FName("ParticleDensity"), CurrentDensity);
    }
}

bool AEnvArt_ParticleManager::IsZoneActiveForTimeOfDay(const FEnvArt_ParticleZone& Zone) const
{
    float TimeOfDay = GetCurrentTimeOfDay();
    
    // Day time is 6-18, night time is 18-6
    bool bIsDayTime = (TimeOfDay >= 6.0f && TimeOfDay < 18.0f);
    
    if (bIsDayTime)
    {
        return Zone.bActiveInDaylight;
    }
    else
    {
        return Zone.bActiveAtNight;
    }
}

float AEnvArt_ParticleManager::GetCurrentTimeOfDay() const
{
    // This would get the actual time of day from the atmosphere manager
    // For now, return a default value
    return 12.0f; // Noon
}