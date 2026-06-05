#include "EnvArt_AtmosphericParticleSystem.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AEnvArt_AtmosphericParticleSystem::AEnvArt_AtmosphericParticleSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create particle system component
    ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
    ParticleSystemComponent->SetupAttachment(RootComponent);

    // Create activation trigger
    ActivationTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("ActivationTrigger"));
    ActivationTrigger->SetupAttachment(RootComponent);
    ActivationTrigger->SetSphereRadius(2000.0f);
    ActivationTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ActivationTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    ActivationTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize default values
    ParticleType = EEnvArt_AtmosphericParticleType::Dust;
    ParticleIntensity = 0.5f;
    WindDirection = FVector(1.0f, 0.0f, 0.0f);
    WindStrength = 2.0f;
    bWeatherResponsive = true;
    CurrentWeather = EEnvArt_WeatherType::Clear;
    BiomeType = EEnvArt_BiomeType::Desert;
    BiomeSpawnRateMultiplier = 1.0f;
    bIsActive = false;
    bPlayerInRange = false;

    // Bind trigger events
    ActivationTrigger->OnComponentBeginOverlap.AddDynamic(this, &AEnvArt_AtmosphericParticleSystem::OnTriggerBeginOverlap);
    ActivationTrigger->OnComponentEndOverlap.AddDynamic(this, &AEnvArt_AtmosphericParticleSystem::OnTriggerEndOverlap);
}

void AEnvArt_AtmosphericParticleSystem::BeginPlay()
{
    Super::BeginPlay();

    // Configure particle system for current biome
    ConfigureForBiome(BiomeType);

    // Start weather update timer
    if (bWeatherResponsive)
    {
        GetWorldTimerManager().SetTimer(WeatherUpdateTimer, [this]()
        {
            // Check for weather changes (placeholder - would integrate with weather system)
            UpdateParticleParameters();
        }, 5.0f, true);
    }

    // Initially deactivate particles until player is in range
    DeactivateParticleSystem();
}

void AEnvArt_AtmosphericParticleSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsActive && ParticleSystemComponent)
    {
        // Update particle parameters based on wind and weather
        UpdateParticleParameters();
    }
}

void AEnvArt_AtmosphericParticleSystem::UpdateWeatherEffects(EEnvArt_WeatherType NewWeather)
{
    CurrentWeather = NewWeather;

    if (!bWeatherResponsive)
        return;

    // Adjust particle intensity based on weather
    float WeatherIntensityMultiplier = 1.0f;
    
    switch (CurrentWeather)
    {
        case EEnvArt_WeatherType::Clear:
            WeatherIntensityMultiplier = 0.7f;
            break;
        case EEnvArt_WeatherType::Windy:
            WeatherIntensityMultiplier = 1.5f;
            WindStrength = FMath::Clamp(WindStrength * 1.3f, 0.0f, 10.0f);
            break;
        case EEnvArt_WeatherType::Stormy:
            WeatherIntensityMultiplier = 2.0f;
            WindStrength = FMath::Clamp(WindStrength * 1.8f, 0.0f, 10.0f);
            break;
        case EEnvArt_WeatherType::Rain:
            WeatherIntensityMultiplier = 0.3f; // Dust settles in rain
            break;
        case EEnvArt_WeatherType::Fog:
            WeatherIntensityMultiplier = 0.8f;
            break;
        default:
            WeatherIntensityMultiplier = 1.0f;
            break;
    }

    // Apply weather multiplier to base intensity
    float AdjustedIntensity = ParticleIntensity * WeatherIntensityMultiplier;
    AdjustedIntensity = FMath::Clamp(AdjustedIntensity, 0.0f, 1.0f);

    if (ParticleSystemComponent)
    {
        // Update particle system parameters
        ParticleSystemComponent->SetFloatParameter(FName("Intensity"), AdjustedIntensity);
        ParticleSystemComponent->SetVectorParameter(FName("WindDirection"), WindDirection);
        ParticleSystemComponent->SetFloatParameter(FName("WindStrength"), WindStrength);
    }
}

void AEnvArt_AtmosphericParticleSystem::SetParticleIntensity(float NewIntensity)
{
    ParticleIntensity = FMath::Clamp(NewIntensity, 0.0f, 1.0f);
    UpdateParticleParameters();
}

void AEnvArt_AtmosphericParticleSystem::SetWindParameters(FVector NewDirection, float NewStrength)
{
    WindDirection = NewDirection.GetSafeNormal();
    WindStrength = FMath::Clamp(NewStrength, 0.0f, 10.0f);
    UpdateParticleParameters();
}

void AEnvArt_AtmosphericParticleSystem::ActivateParticleSystem()
{
    if (ParticleSystemComponent && !bIsActive)
    {
        ParticleSystemComponent->Activate();
        bIsActive = true;
        
        UE_LOG(LogTemp, Log, TEXT("Atmospheric particle system activated: %s"), *GetName());
    }
}

void AEnvArt_AtmosphericParticleSystem::DeactivateParticleSystem()
{
    if (ParticleSystemComponent && bIsActive)
    {
        ParticleSystemComponent->Deactivate();
        bIsActive = false;
        
        UE_LOG(LogTemp, Log, TEXT("Atmospheric particle system deactivated: %s"), *GetName());
    }
}

bool AEnvArt_AtmosphericParticleSystem::IsParticleSystemActive() const
{
    return bIsActive && ParticleSystemComponent && ParticleSystemComponent->IsActive();
}

void AEnvArt_AtmosphericParticleSystem::ConfigureForBiome(EEnvArt_BiomeType NewBiomeType)
{
    BiomeType = NewBiomeType;

    // Configure particle type and settings based on biome
    switch (BiomeType)
    {
        case EEnvArt_BiomeType::Desert:
            ParticleType = EEnvArt_AtmosphericParticleType::Dust;
            BiomeSpawnRateMultiplier = 1.5f;
            WindStrength = 3.0f;
            break;
            
        case EEnvArt_BiomeType::Forest:
            ParticleType = EEnvArt_AtmosphericParticleType::Pollen;
            BiomeSpawnRateMultiplier = 1.2f;
            WindStrength = 1.5f;
            break;
            
        case EEnvArt_BiomeType::Swamp:
            ParticleType = EEnvArt_AtmosphericParticleType::Mist;
            BiomeSpawnRateMultiplier = 0.8f;
            WindStrength = 0.8f;
            break;
            
        case EEnvArt_BiomeType::Volcanic:
            ParticleType = EEnvArt_AtmosphericParticleType::VolcanicAsh;
            BiomeSpawnRateMultiplier = 2.0f;
            WindStrength = 4.0f;
            break;
            
        case EEnvArt_BiomeType::Plains:
            ParticleType = EEnvArt_AtmosphericParticleType::Dust;
            BiomeSpawnRateMultiplier = 1.0f;
            WindStrength = 2.5f;
            break;
            
        default:
            ParticleType = EEnvArt_AtmosphericParticleType::Dust;
            BiomeSpawnRateMultiplier = 1.0f;
            WindStrength = 2.0f;
            break;
    }

    UpdateParticleParameters();
}

void AEnvArt_AtmosphericParticleSystem::UpdateParticleParameters()
{
    if (!ParticleSystemComponent)
        return;

    // Calculate final intensity with biome and weather multipliers
    float FinalIntensity = ParticleIntensity * BiomeSpawnRateMultiplier;
    
    // Apply weather effects if responsive
    if (bWeatherResponsive)
    {
        UpdateWeatherEffects(CurrentWeather);
    }
    else
    {
        // Set basic parameters
        ParticleSystemComponent->SetFloatParameter(FName("Intensity"), FinalIntensity);
        ParticleSystemComponent->SetVectorParameter(FName("WindDirection"), WindDirection);
        ParticleSystemComponent->SetFloatParameter(FName("WindStrength"), WindStrength);
    }

    // Set biome-specific parameters
    ParticleSystemComponent->SetFloatParameter(FName("BiomeMultiplier"), BiomeSpawnRateMultiplier);
}

void AEnvArt_AtmosphericParticleSystem::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Check if the overlapping actor is the player
    if (OtherActor && OtherActor->IsA<APawn>())
    {
        APawn* PlayerPawn = Cast<APawn>(OtherActor);
        if (PlayerPawn && PlayerPawn->IsPlayerControlled())
        {
            bPlayerInRange = true;
            ActivateParticleSystem();
            
            UE_LOG(LogTemp, Log, TEXT("Player entered atmospheric particle trigger: %s"), *GetName());
        }
    }
}

void AEnvArt_AtmosphericParticleSystem::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    // Check if the overlapping actor is the player
    if (OtherActor && OtherActor->IsA<APawn>())
    {
        APawn* PlayerPawn = Cast<APawn>(OtherActor);
        if (PlayerPawn && PlayerPawn->IsPlayerControlled())
        {
            bPlayerInRange = false;
            DeactivateParticleSystem();
            
            UE_LOG(LogTemp, Log, TEXT("Player left atmospheric particle trigger: %s"), *GetName());
        }
    }
}