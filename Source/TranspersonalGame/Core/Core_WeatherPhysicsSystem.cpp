#include "Core_WeatherPhysicsSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UCore_WeatherPhysicsSystem::UCore_WeatherPhysicsSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize default weather data
    CurrentWeatherData = FCore_WeatherPhysicsData();
    PrecipitationData = FCore_PrecipitationPhysics();

    // Set default physics parameters
    WindForceMultiplier = 1.0f;
    WindTurbulenceScale = 0.1f;
    MaxWindForce = 1000.0f;
    RainDropDensity = 100.0f;
    HailImpactForce = 50.0f;
    WetnessDecayRate = 0.1f;
    LightningStrikeForce = 10000.0f;
    LightningRadius = 500.0f;
    ElectromagneticPulseStrength = 100.0f;
    UpdateFrequency = 0.1f;
    MaxAffectedActors = 100.0f;
    bEnableAdvancedPhysics = true;
}

void UCore_WeatherPhysicsSystem::BeginPlay()
{
    Super::BeginPlay();

    // Cache world reference for performance
    CachedWorld = GetWorld();
    
    if (CachedWorld)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_WeatherPhysicsSystem: Initialized in world %s"), 
               *CachedWorld->GetName());
    }

    // Initialize weather system
    LastUpdateTime = CachedWorld ? CachedWorld->GetTimeSeconds() : 0.0f;
    ProcessedActorsThisFrame = 0;
}

void UCore_WeatherPhysicsSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!CachedWorld || !bEnableAdvancedPhysics)
    {
        return;
    }

    // Update weather transition if active
    if (bIsTransitioning)
    {
        UpdateWeatherTransition(DeltaTime);
    }

    // Process weather effects based on update frequency
    float CurrentTime = CachedWorld->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime >= UpdateFrequency)
    {
        ProcessWeatherEffects(DeltaTime);
        LastUpdateTime = CurrentTime;
        ProcessedActorsThisFrame = 0;
    }
}

void UCore_WeatherPhysicsSystem::SetWeatherType(ECore_WeatherType NewWeatherType, float NewIntensity)
{
    CurrentWeatherData.WeatherType = NewWeatherType;
    CurrentWeatherData.Intensity = FMath::Clamp(NewIntensity, 0.0f, 1.0f);

    // Adjust physics parameters based on weather type
    switch (NewWeatherType)
    {
        case ECore_WeatherType::Clear:
            CurrentWeatherData.WindSpeed = 5.0f;
            PrecipitationData.SurfaceWetness = 0.0f;
            break;
            
        case ECore_WeatherType::LightRain:
            CurrentWeatherData.WindSpeed = 10.0f;
            PrecipitationData.AccumulationRate = 0.01f * NewIntensity;
            break;
            
        case ECore_WeatherType::HeavyRain:
            CurrentWeatherData.WindSpeed = 20.0f;
            PrecipitationData.AccumulationRate = 0.05f * NewIntensity;
            break;
            
        case ECore_WeatherType::Storm:
            CurrentWeatherData.WindSpeed = 40.0f;
            PrecipitationData.AccumulationRate = 0.1f * NewIntensity;
            break;
            
        case ECore_WeatherType::Hail:
            CurrentWeatherData.WindSpeed = 30.0f;
            PrecipitationData.ImpactForce = HailImpactForce * NewIntensity;
            break;
            
        default:
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("Weather changed to %d with intensity %.2f"), 
           (int32)NewWeatherType, NewIntensity);
}

void UCore_WeatherPhysicsSystem::SetWindParameters(FVector Direction, float Speed)
{
    CurrentWeatherData.WindDirection = Direction.GetSafeNormal();
    CurrentWeatherData.WindSpeed = FMath::Max(0.0f, Speed);
}

void UCore_WeatherPhysicsSystem::SetAtmosphericConditions(float Temperature, float Humidity, float Pressure)
{
    CurrentWeatherData.Temperature = FMath::Clamp(Temperature, -50.0f, 50.0f);
    CurrentWeatherData.Humidity = FMath::Clamp(Humidity, 0.0f, 1.0f);
    CurrentWeatherData.AtmosphericPressure = FMath::Clamp(Pressure, 900.0f, 1100.0f);
}

void UCore_WeatherPhysicsSystem::ApplyWindForceToActor(AActor* TargetActor, float ForceMultiplier)
{
    if (!TargetActor || !CachedWorld)
    {
        return;
    }

    UPrimitiveComponent* PrimComp = TargetActor->FindComponentByClass<UPrimitiveComponent>();
    if (!PrimComp || !PrimComp->IsSimulatingPhysics())
    {
        return;
    }

    // Calculate wind force based on actor properties
    float ActorMass = PrimComp->GetMass();
    FVector WindForce = CalculateWindForce(TargetActor->GetActorLocation(), ActorMass);
    WindForce *= ForceMultiplier * CurrentWeatherData.Intensity;

    // Apply turbulence for realistic wind behavior
    FVector Turbulence = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-0.5f, 0.5f)
    ) * WindTurbulenceScale * CurrentWeatherData.WindSpeed;

    FVector TotalForce = WindForce + Turbulence;
    TotalForce = FVector::ClampMaxSize(TotalForce, MaxWindForce);

    PrimComp->AddForce(TotalForce);
}

void UCore_WeatherPhysicsSystem::ApplyPrecipitationEffects(AActor* TargetActor)
{
    if (!TargetActor || CurrentWeatherData.WeatherType == ECore_WeatherType::Clear)
    {
        return;
    }

    UPrimitiveComponent* PrimComp = TargetActor->FindComponentByClass<UPrimitiveComponent>();
    if (!PrimComp)
    {
        return;
    }

    // Calculate precipitation impact based on weather type
    float ImpactForce = PrecipitationData.ImpactForce * CurrentWeatherData.Intensity;
    
    if (CurrentWeatherData.WeatherType == ECore_WeatherType::Hail)
    {
        ImpactForce *= 5.0f; // Hail hits much harder
    }

    // Apply downward force to simulate precipitation impact
    FVector PrecipitationForce = FVector(0.0f, 0.0f, -ImpactForce);
    
    if (PrimComp->IsSimulatingPhysics())
    {
        PrimComp->AddForce(PrecipitationForce);
    }

    // Increase surface wetness for rain effects
    if (CurrentWeatherData.WeatherType == ECore_WeatherType::LightRain ||
        CurrentWeatherData.WeatherType == ECore_WeatherType::HeavyRain ||
        CurrentWeatherData.WeatherType == ECore_WeatherType::Storm)
    {
        PrecipitationData.SurfaceWetness = FMath::Min(1.0f, 
            PrecipitationData.SurfaceWetness + PrecipitationData.AccumulationRate);
    }
}

void UCore_WeatherPhysicsSystem::SimulateStormLightning(FVector StrikeLocation)
{
    if (!CachedWorld || CurrentWeatherData.WeatherType != ECore_WeatherType::Storm)
    {
        return;
    }

    // Find all actors within lightning radius
    TArray<AActor*> OverlappingActors;
    UKismetSystemLibrary::SphereOverlapActors(
        CachedWorld,
        StrikeLocation,
        LightningRadius,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        nullptr,
        TArray<AActor*>(),
        OverlappingActors
    );

    // Apply electromagnetic pulse and force effects
    for (AActor* Actor : OverlappingActors)
    {
        if (!Actor)
        {
            continue;
        }

        UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
        if (!PrimComp || !PrimComp->IsSimulatingPhysics())
        {
            continue;
        }

        float Distance = FVector::Dist(Actor->GetActorLocation(), StrikeLocation);
        float ForceFalloff = 1.0f - (Distance / LightningRadius);
        ForceFalloff = FMath::Max(0.0f, ForceFalloff);

        // Calculate radial force from lightning strike
        FVector ForceDirection = (Actor->GetActorLocation() - StrikeLocation).GetSafeNormal();
        FVector LightningForce = ForceDirection * LightningStrikeForce * ForceFalloff;

        PrimComp->AddImpulse(LightningForce);
    }

    // Spawn visual and audio effects
    SpawnLightningEffects(StrikeLocation);

    UE_LOG(LogTemp, Warning, TEXT("Lightning strike at %s affecting %d actors"), 
           *StrikeLocation.ToString(), OverlappingActors.Num());
}

float UCore_WeatherPhysicsSystem::CalculateMovementModifier(AActor* Actor) const
{
    if (!Actor)
    {
        return 1.0f;
    }

    float Modifier = 1.0f;

    // Wind resistance
    float WindResistance = CurrentWeatherData.WindSpeed / 100.0f;
    Modifier -= WindResistance * 0.1f;

    // Surface wetness affects movement
    float WetnessEffect = PrecipitationData.SurfaceWetness * 0.3f;
    Modifier -= WetnessEffect;

    // Atmospheric pressure effects
    float PressureNormal = (CurrentWeatherData.AtmosphericPressure - 1013.25f) / 100.0f;
    Modifier -= FMath::Abs(PressureNormal) * 0.05f;

    return FMath::Clamp(Modifier, 0.1f, 1.0f);
}

float UCore_WeatherPhysicsSystem::CalculateVisibilityModifier() const
{
    float Visibility = 1.0f;

    switch (CurrentWeatherData.WeatherType)
    {
        case ECore_WeatherType::Fog:
            Visibility = 0.3f - (CurrentWeatherData.Intensity * 0.2f);
            break;
            
        case ECore_WeatherType::HeavyRain:
            Visibility = 0.7f - (CurrentWeatherData.Intensity * 0.3f);
            break;
            
        case ECore_WeatherType::Storm:
            Visibility = 0.5f - (CurrentWeatherData.Intensity * 0.4f);
            break;
            
        default:
            Visibility = 1.0f - (CurrentWeatherData.Intensity * 0.1f);
            break;
    }

    return FMath::Clamp(Visibility, 0.1f, 1.0f);
}

float UCore_WeatherPhysicsSystem::CalculateFrictionModifier(const FVector& SurfaceNormal) const
{
    float BaseFriction = 1.0f;

    // Wetness reduces friction significantly
    float WetnessReduction = PrecipitationData.SurfaceWetness * 0.6f;
    BaseFriction -= WetnessReduction;

    // Temperature effects (ice formation below 0°C)
    if (CurrentWeatherData.Temperature < 0.0f)
    {
        float IceEffect = FMath::Abs(CurrentWeatherData.Temperature) / 20.0f;
        BaseFriction -= IceEffect * 0.8f;
    }

    // Surface angle affects friction in wet conditions
    float SurfaceAngle = FMath::Acos(FVector::DotProduct(SurfaceNormal, FVector::UpVector));
    float AngleEffect = SurfaceAngle / PI * PrecipitationData.SurfaceWetness;
    BaseFriction -= AngleEffect * 0.3f;

    return FMath::Clamp(BaseFriction, 0.05f, 1.5f);
}

void UCore_WeatherPhysicsSystem::TransitionToWeather(ECore_WeatherType TargetWeather, float TransitionDuration)
{
    if (bIsTransitioning)
    {
        return; // Already transitioning
    }

    TargetWeatherData = CurrentWeatherData;
    TargetWeatherData.WeatherType = TargetWeather;
    
    // Set target parameters based on weather type
    switch (TargetWeather)
    {
        case ECore_WeatherType::Clear:
            TargetWeatherData.WindSpeed = 5.0f;
            TargetWeatherData.Intensity = 0.1f;
            break;
        case ECore_WeatherType::Storm:
            TargetWeatherData.WindSpeed = 40.0f;
            TargetWeatherData.Intensity = 0.9f;
            break;
        default:
            TargetWeatherData.Intensity = 0.5f;
            break;
    }

    bIsTransitioning = true;
    TransitionProgress = 0.0f;
    this->TransitionDuration = FMath::Max(0.1f, TransitionDuration);
}

void UCore_WeatherPhysicsSystem::DebugWeatherEffects()
{
    if (!CachedWorld)
    {
        return;
    }

    FString WeatherInfo = FString::Printf(
        TEXT("Weather: %d | Intensity: %.2f | Wind: %.1f m/s | Temp: %.1f°C | Humidity: %.1f%%"),
        (int32)CurrentWeatherData.WeatherType,
        CurrentWeatherData.Intensity,
        CurrentWeatherData.WindSpeed,
        CurrentWeatherData.Temperature,
        CurrentWeatherData.Humidity * 100.0f
    );

    UE_LOG(LogTemp, Warning, TEXT("Weather Debug: %s"), *WeatherInfo);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, WeatherInfo);
    }
}

void UCore_WeatherPhysicsSystem::TestStormSimulation()
{
    if (!CachedWorld)
    {
        return;
    }

    // Set storm weather
    SetWeatherType(ECore_WeatherType::Storm, 0.8f);
    
    // Simulate lightning strike at random location
    FVector StrikeLocation = GetOwner()->GetActorLocation() + 
                           FVector(FMath::RandRange(-1000.0f, 1000.0f),
                                  FMath::RandRange(-1000.0f, 1000.0f),
                                  FMath::RandRange(100.0f, 500.0f));
    
    SimulateStormLightning(StrikeLocation);
    
    UE_LOG(LogTemp, Warning, TEXT("Storm simulation test executed"));
}

void UCore_WeatherPhysicsSystem::UpdateWeatherTransition(float DeltaTime)
{
    TransitionProgress += DeltaTime / TransitionDuration;
    
    if (TransitionProgress >= 1.0f)
    {
        // Transition complete
        CurrentWeatherData = TargetWeatherData;
        bIsTransitioning = false;
        TransitionProgress = 1.0f;
        return;
    }

    // Interpolate weather parameters
    float Alpha = TransitionProgress;
    
    CurrentWeatherData.Intensity = FMath::Lerp(
        CurrentWeatherData.Intensity, 
        TargetWeatherData.Intensity, 
        Alpha
    );
    
    CurrentWeatherData.WindSpeed = FMath::Lerp(
        CurrentWeatherData.WindSpeed,
        TargetWeatherData.WindSpeed,
        Alpha
    );
    
    CurrentWeatherData.Temperature = FMath::Lerp(
        CurrentWeatherData.Temperature,
        TargetWeatherData.Temperature,
        Alpha
    );
}

void UCore_WeatherPhysicsSystem::ProcessWeatherEffects(float DeltaTime)
{
    if (!CachedWorld)
    {
        return;
    }

    // Update surface wetness decay
    UpdateSurfaceWetness(DeltaTime);

    // Get all actors in the world (with performance limit)
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(CachedWorld, AActor::StaticClass(), AllActors);

    int32 ProcessedCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (!Actor || ProcessedCount >= MaxAffectedActors)
        {
            break;
        }

        // Apply weather effects based on current weather
        if (CurrentWeatherData.WindSpeed > 5.0f)
        {
            ApplyWindForceToActor(Actor, 1.0f);
        }

        if (CurrentWeatherData.WeatherType != ECore_WeatherType::Clear)
        {
            ApplyPrecipitationEffects(Actor);
        }

        ProcessedCount++;
    }

    ProcessedActorsThisFrame = ProcessedCount;
}

void UCore_WeatherPhysicsSystem::ApplyWindTurbulence(AActor* Actor, float DeltaTime)
{
    if (!Actor)
    {
        return;
    }

    UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (!PrimComp || !PrimComp->IsSimulatingPhysics())
    {
        return;
    }

    // Generate turbulence based on time and position
    float Time = CachedWorld ? CachedWorld->GetTimeSeconds() : 0.0f;
    FVector ActorPos = Actor->GetActorLocation();
    
    float TurbulenceX = FMath::Sin(Time * 2.0f + ActorPos.X * 0.001f) * WindTurbulenceScale;
    float TurbulenceY = FMath::Cos(Time * 1.5f + ActorPos.Y * 0.001f) * WindTurbulenceScale;
    float TurbulenceZ = FMath::Sin(Time * 3.0f + ActorPos.Z * 0.001f) * WindTurbulenceScale * 0.5f;

    FVector TurbulenceForce = FVector(TurbulenceX, TurbulenceY, TurbulenceZ) * CurrentWeatherData.WindSpeed;
    PrimComp->AddForce(TurbulenceForce);
}

void UCore_WeatherPhysicsSystem::UpdateSurfaceWetness(float DeltaTime)
{
    if (CurrentWeatherData.WeatherType == ECore_WeatherType::Clear ||
        CurrentWeatherData.WeatherType == ECore_WeatherType::Wind)
    {
        // Wetness decays in clear weather
        PrecipitationData.SurfaceWetness = FMath::Max(0.0f, 
            PrecipitationData.SurfaceWetness - WetnessDecayRate * DeltaTime);
    }
    else if (CurrentWeatherData.WeatherType == ECore_WeatherType::LightRain ||
             CurrentWeatherData.WeatherType == ECore_WeatherType::HeavyRain ||
             CurrentWeatherData.WeatherType == ECore_WeatherType::Storm)
    {
        // Wetness accumulates during precipitation
        PrecipitationData.SurfaceWetness = FMath::Min(1.0f,
            PrecipitationData.SurfaceWetness + PrecipitationData.AccumulationRate * DeltaTime);
    }
}

FVector UCore_WeatherPhysicsSystem::CalculateWindForce(const FVector& ActorLocation, float ActorMass) const
{
    // Base wind force
    FVector BaseForce = CurrentWeatherData.WindDirection * CurrentWeatherData.WindSpeed * WindForceMultiplier;
    
    // Scale by atmospheric density
    float AtmosphericDensity = CalculateAtmosphericDensity();
    BaseForce *= AtmosphericDensity;
    
    // Scale by mass (lighter objects affected more)
    float MassEffect = FMath::Clamp(100.0f / FMath::Max(1.0f, ActorMass), 0.1f, 10.0f);
    BaseForce *= MassEffect;
    
    return BaseForce;
}

float UCore_WeatherPhysicsSystem::CalculateAtmosphericDensity() const
{
    // Simplified atmospheric density calculation
    // Based on pressure, temperature, and humidity
    float PressureRatio = CurrentWeatherData.AtmosphericPressure / 1013.25f;
    float TemperatureK = CurrentWeatherData.Temperature + 273.15f;
    float TemperatureRatio = 288.15f / TemperatureK; // Standard temperature ratio
    
    float Density = PressureRatio * TemperatureRatio;
    
    // Humidity slightly reduces air density
    Density *= (1.0f - CurrentWeatherData.Humidity * 0.05f);
    
    return FMath::Clamp(Density, 0.5f, 1.5f);
}

void UCore_WeatherPhysicsSystem::SpawnLightningEffects(const FVector& Location)
{
    if (!CachedWorld)
    {
        return;
    }

    // Draw debug lightning bolt
    DrawDebugLine(
        CachedWorld,
        Location + FVector(0, 0, 2000),
        Location,
        FColor::White,
        false,
        2.0f,
        0,
        10.0f
    );

    // Draw debug sphere for electromagnetic pulse
    DrawDebugSphere(
        CachedWorld,
        Location,
        LightningRadius,
        16,
        FColor::Blue,
        false,
        1.0f,
        0,
        5.0f
    );

    UE_LOG(LogTemp, Log, TEXT("Lightning effects spawned at %s"), *Location.ToString());
}