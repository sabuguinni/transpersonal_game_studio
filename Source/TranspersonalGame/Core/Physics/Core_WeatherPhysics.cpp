#include "Core_WeatherPhysics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Components/LightComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Components/PostProcessComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogWeatherPhysics, Log, All);

UCore_WeatherPhysics::UCore_WeatherPhysics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    // Initialize weather parameters
    WindStrength = 0.5f;
    WindDirection = FVector(1.0f, 0.0f, 0.0f);
    RainIntensity = 0.0f;
    Temperature = 25.0f;
    Humidity = 50.0f;
    AtmosphericPressure = 1013.25f;
    
    // Initialize physics parameters
    AirDensity = 1.225f;
    GravityModifier = 1.0f;
    WindDragCoefficient = 0.47f;
    
    // Initialize weather state
    CurrentWeatherState = ECore_WeatherState::Clear;
    WeatherTransitionSpeed = 1.0f;
    
    // Initialize arrays
    AffectedActors.Empty();
    WindZones.Empty();
    WeatherEffects.Empty();
    
    UE_LOG(LogWeatherPhysics, Log, TEXT("Core_WeatherPhysics component initialized"));
}

void UCore_WeatherPhysics::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize weather system
    InitializeWeatherSystem();
    
    // Start weather simulation
    GetWorld()->GetTimerManager().SetTimer(WeatherUpdateTimer, this, 
        &UCore_WeatherPhysics::UpdateWeatherSimulation, 1.0f, true);
    
    UE_LOG(LogWeatherPhysics, Log, TEXT("Weather physics system started"));
}

void UCore_WeatherPhysics::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up weather system
    CleanupWeatherSystem();
    
    // Clear timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(WeatherUpdateTimer);
    }
    
    Super::EndPlay(EndPlayReason);
}

void UCore_WeatherPhysics::TickComponent(float DeltaTime, ELevelTick TickType, 
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update wind effects on actors
    UpdateWindEffects(DeltaTime);
    
    // Update weather transitions
    UpdateWeatherTransitions(DeltaTime);
    
    // Update atmospheric effects
    UpdateAtmosphericEffects(DeltaTime);
}

void UCore_WeatherPhysics::InitializeWeatherSystem()
{
    if (!GetWorld())
    {
        UE_LOG(LogWeatherPhysics, Error, TEXT("No valid world for weather system"));
        return;
    }
    
    // Find all actors that can be affected by weather
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (CanActorBeAffectedByWeather(Actor))
        {
            AffectedActors.Add(Actor);
        }
    }
    
    // Initialize default wind zones
    CreateDefaultWindZones();
    
    UE_LOG(LogWeatherPhysics, Log, TEXT("Weather system initialized with %d affected actors"), 
        AffectedActors.Num());
}

void UCore_WeatherPhysics::CleanupWeatherSystem()
{
    // Clear all arrays
    AffectedActors.Empty();
    WindZones.Empty();
    WeatherEffects.Empty();
    
    UE_LOG(LogWeatherPhysics, Log, TEXT("Weather system cleaned up"));
}

bool UCore_WeatherPhysics::CanActorBeAffectedByWeather(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }
    
    // Check if actor has physics components
    UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (PrimComp && PrimComp->IsSimulatingPhysics())
    {
        return true;
    }
    
    // Check if actor is a static mesh with specific tags
    if (Actor->ActorHasTag(FName("WeatherAffected")))
    {
        return true;
    }
    
    return false;
}

void UCore_WeatherPhysics::UpdateWindEffects(float DeltaTime)
{
    if (WindStrength <= 0.0f)
    {
        return;
    }
    
    for (AActor* Actor : AffectedActors)
    {
        if (!IsValid(Actor))
        {
            continue;
        }
        
        ApplyWindForceToActor(Actor, DeltaTime);
    }
}

void UCore_WeatherPhysics::ApplyWindForceToActor(AActor* Actor, float DeltaTime)
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
    
    // Calculate wind force based on actor properties
    FVector WindForce = CalculateWindForce(Actor);
    
    // Apply force to the actor
    PrimComp->AddForce(WindForce * DeltaTime, NAME_None, true);
}

FVector UCore_WeatherPhysics::CalculateWindForce(AActor* Actor) const
{
    if (!Actor)
    {
        return FVector::ZeroVector;
    }
    
    // Get actor bounds for surface area calculation
    FVector Origin, BoxExtent;
    Actor->GetActorBounds(false, Origin, BoxExtent);
    
    // Calculate approximate surface area facing wind
    float SurfaceArea = FMath::Abs(FVector::DotProduct(BoxExtent, WindDirection.GetSafeNormal())) * 2.0f;
    
    // Calculate drag force: F = 0.5 * ρ * v² * Cd * A
    float WindSpeed = WindStrength * 10.0f; // Convert to m/s
    float DragForce = 0.5f * AirDensity * FMath::Square(WindSpeed) * WindDragCoefficient * SurfaceArea;
    
    return WindDirection.GetSafeNormal() * DragForce;
}

void UCore_WeatherPhysics::UpdateWeatherTransitions(float DeltaTime)
{
    // Smooth weather parameter transitions
    float TransitionRate = WeatherTransitionSpeed * DeltaTime;
    
    // Update wind based on weather state
    float TargetWindStrength = GetTargetWindStrength();
    WindStrength = FMath::FInterpTo(WindStrength, TargetWindStrength, DeltaTime, TransitionRate);
    
    // Update rain intensity
    float TargetRainIntensity = GetTargetRainIntensity();
    RainIntensity = FMath::FInterpTo(RainIntensity, TargetRainIntensity, DeltaTime, TransitionRate);
    
    // Update temperature
    float TargetTemperature = GetTargetTemperature();
    Temperature = FMath::FInterpTo(Temperature, TargetTemperature, DeltaTime, TransitionRate * 0.1f);
}

float UCore_WeatherPhysics::GetTargetWindStrength() const
{
    switch (CurrentWeatherState)
    {
        case ECore_WeatherState::Clear:
            return 0.2f;
        case ECore_WeatherState::Cloudy:
            return 0.4f;
        case ECore_WeatherState::Rainy:
            return 0.7f;
        case ECore_WeatherState::Stormy:
            return 1.5f;
        case ECore_WeatherState::Foggy:
            return 0.1f;
        default:
            return 0.5f;
    }
}

float UCore_WeatherPhysics::GetTargetRainIntensity() const
{
    switch (CurrentWeatherState)
    {
        case ECore_WeatherState::Rainy:
            return 0.6f;
        case ECore_WeatherState::Stormy:
            return 1.0f;
        default:
            return 0.0f;
    }
}

float UCore_WeatherPhysics::GetTargetTemperature() const
{
    switch (CurrentWeatherState)
    {
        case ECore_WeatherState::Clear:
            return 28.0f;
        case ECore_WeatherState::Cloudy:
            return 22.0f;
        case ECore_WeatherState::Rainy:
            return 18.0f;
        case ECore_WeatherState::Stormy:
            return 15.0f;
        case ECore_WeatherState::Foggy:
            return 12.0f;
        default:
            return 20.0f;
    }
}

void UCore_WeatherPhysics::UpdateAtmosphericEffects(float DeltaTime)
{
    // Update atmospheric pressure based on weather
    float TargetPressure = GetTargetAtmosphericPressure();
    AtmosphericPressure = FMath::FInterpTo(AtmosphericPressure, TargetPressure, DeltaTime, 0.1f);
    
    // Update humidity
    float TargetHumidity = GetTargetHumidity();
    Humidity = FMath::FInterpTo(Humidity, TargetHumidity, DeltaTime, 0.2f);
    
    // Update air density based on temperature and pressure
    UpdateAirDensity();
}

float UCore_WeatherPhysics::GetTargetAtmosphericPressure() const
{
    switch (CurrentWeatherState)
    {
        case ECore_WeatherState::Clear:
            return 1020.0f;
        case ECore_WeatherState::Cloudy:
            return 1010.0f;
        case ECore_WeatherState::Rainy:
            return 1000.0f;
        case ECore_WeatherState::Stormy:
            return 990.0f;
        case ECore_WeatherState::Foggy:
            return 1005.0f;
        default:
            return 1013.25f;
    }
}

float UCore_WeatherPhysics::GetTargetHumidity() const
{
    switch (CurrentWeatherState)
    {
        case ECore_WeatherState::Clear:
            return 30.0f;
        case ECore_WeatherState::Cloudy:
            return 60.0f;
        case ECore_WeatherState::Rainy:
            return 90.0f;
        case ECore_WeatherState::Stormy:
            return 95.0f;
        case ECore_WeatherState::Foggy:
            return 98.0f;
        default:
            return 50.0f;
    }
}

void UCore_WeatherPhysics::UpdateAirDensity()
{
    // Calculate air density using ideal gas law approximation
    // ρ = (P * M) / (R * T)
    // Where P = pressure, M = molar mass of air, R = gas constant, T = temperature
    
    float TemperatureKelvin = Temperature + 273.15f;
    float PressurePascals = AtmosphericPressure * 100.0f; // Convert hPa to Pa
    
    // Simplified calculation for air density
    AirDensity = (PressurePascals * 0.0289644f) / (8.314f * TemperatureKelvin);
    
    // Clamp to reasonable values
    AirDensity = FMath::Clamp(AirDensity, 0.5f, 2.0f);
}

void UCore_WeatherPhysics::CreateDefaultWindZones()
{
    // Create a global wind zone covering the entire map
    FCore_WindZone GlobalWindZone;
    GlobalWindZone.Location = FVector::ZeroVector;
    GlobalWindZone.Radius = 50000.0f; // 500m radius
    GlobalWindZone.WindDirection = WindDirection;
    GlobalWindZone.WindStrength = WindStrength;
    GlobalWindZone.bIsActive = true;
    
    WindZones.Add(GlobalWindZone);
    
    UE_LOG(LogWeatherPhysics, Log, TEXT("Created default global wind zone"));
}

void UCore_WeatherPhysics::UpdateWeatherSimulation()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Simulate weather changes over time
    static float WeatherTimer = 0.0f;
    WeatherTimer += 1.0f;
    
    // Change weather every 5 minutes (300 seconds)
    if (WeatherTimer >= 300.0f)
    {
        ChangeWeatherState();
        WeatherTimer = 0.0f;
    }
    
    // Update wind direction gradually
    UpdateWindDirection();
}

void UCore_WeatherPhysics::ChangeWeatherState()
{
    // Random weather state change
    int32 NewStateIndex = FMath::RandRange(0, 4);
    ECore_WeatherState NewState = static_cast<ECore_WeatherState>(NewStateIndex);
    
    if (NewState != CurrentWeatherState)
    {
        CurrentWeatherState = NewState;
        UE_LOG(LogWeatherPhysics, Log, TEXT("Weather state changed to: %d"), (int32)NewState);
        
        // Broadcast weather change event
        OnWeatherStateChanged.Broadcast(CurrentWeatherState);
    }
}

void UCore_WeatherPhysics::UpdateWindDirection()
{
    // Gradually rotate wind direction
    static float WindRotationTimer = 0.0f;
    WindRotationTimer += GetWorld()->GetDeltaSeconds();
    
    if (WindRotationTimer >= 60.0f) // Change direction every minute
    {
        float RandomAngle = FMath::RandRange(-30.0f, 30.0f);
        FRotator WindRotation = WindDirection.Rotation();
        WindRotation.Yaw += RandomAngle;
        WindDirection = WindRotation.Vector();
        
        WindRotationTimer = 0.0f;
    }
}

void UCore_WeatherPhysics::SetWeatherState(ECore_WeatherState NewState)
{
    if (NewState != CurrentWeatherState)
    {
        CurrentWeatherState = NewState;
        OnWeatherStateChanged.Broadcast(CurrentWeatherState);
        
        UE_LOG(LogWeatherPhysics, Log, TEXT("Weather state manually set to: %d"), (int32)NewState);
    }
}

void UCore_WeatherPhysics::SetWindParameters(float NewStrength, FVector NewDirection)
{
    WindStrength = FMath::Clamp(NewStrength, 0.0f, 2.0f);
    WindDirection = NewDirection.GetSafeNormal();
    
    UE_LOG(LogWeatherPhysics, Log, TEXT("Wind parameters updated: Strength=%.2f, Direction=%s"), 
        WindStrength, *WindDirection.ToString());
}

FCore_WeatherData UCore_WeatherPhysics::GetCurrentWeatherData() const
{
    FCore_WeatherData WeatherData;
    WeatherData.WeatherState = CurrentWeatherState;
    WeatherData.WindStrength = WindStrength;
    WeatherData.WindDirection = WindDirection;
    WeatherData.RainIntensity = RainIntensity;
    WeatherData.Temperature = Temperature;
    WeatherData.Humidity = Humidity;
    WeatherData.AtmosphericPressure = AtmosphericPressure;
    WeatherData.AirDensity = AirDensity;
    
    return WeatherData;
}

bool UCore_WeatherPhysics::IsActorInWindZone(AActor* Actor, const FCore_WindZone& WindZone) const
{
    if (!Actor)
    {
        return false;
    }
    
    float Distance = FVector::Dist(Actor->GetActorLocation(), WindZone.Location);
    return Distance <= WindZone.Radius && WindZone.bIsActive;
}