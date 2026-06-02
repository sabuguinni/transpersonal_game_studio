#include "Light_TorchLightingSystem.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

ALight_TorchLightingSystem::ALight_TorchLightingSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create torch mesh component
    TorchMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TorchMesh"));
    TorchMesh->SetupAttachment(RootComponent);

    // Create point light component
    TorchLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("TorchLight"));
    TorchLight->SetupAttachment(TorchMesh);
    TorchLight->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));

    // Create flame particle system
    FlameParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FlameParticles"));
    FlameParticles->SetupAttachment(TorchMesh);
    FlameParticles->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));

    // Initialize default values
    CurrentState = ELight_TorchState::Unlit;
    FuelRemaining = MaxFuel;
    FlickerTime = 0.0f;
    CurrentIntensityMultiplier = 1.0f;

    // Set default flicker data
    FlickerData.BaseIntensity = 2000.0f;
    FlickerData.FlickerAmount = 0.3f;
    FlickerData.FlickerSpeed = 2.0f;
    FlickerData.BaseColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    FlickerData.AttenuationRadius = 800.0f;

    // Configure initial light settings
    TorchLight->SetIntensity(0.0f);
    TorchLight->SetLightColor(FlickerData.BaseColor);
    TorchLight->SetAttenuationRadius(FlickerData.AttenuationRadius);
    TorchLight->SetCastShadows(true);
    TorchLight->SetVisibility(false);

    // Configure flame particles
    FlameParticles->SetVisibility(false);
    FlameParticles->SetAutoActivate(false);
}

void ALight_TorchLightingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UpdateTorchState();
}

void ALight_TorchLightingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentState == ELight_TorchState::Burning)
    {
        UpdateFlicker(DeltaTime);
        UpdateFuelConsumption(DeltaTime);
    }
}

void ALight_TorchLightingSystem::LightTorch()
{
    if (CanBeLit())
    {
        SetTorchState(ELight_TorchState::Lighting);
        
        // Start lighting sequence
        GetWorld()->GetTimerManager().SetTimer(FlickerTimerHandle, [this]()
        {
            SetTorchState(ELight_TorchState::Burning);
        }, 1.0f, false);
    }
}

void ALight_TorchLightingSystem::ExtinguishTorch()
{
    SetTorchState(ELight_TorchState::Extinguished);
}

void ALight_TorchLightingSystem::AddFuel(float Amount)
{
    FuelRemaining = FMath::Clamp(FuelRemaining + Amount, 0.0f, MaxFuel);
    
    if (CurrentState == ELight_TorchState::Extinguished && FuelRemaining > 0.0f)
    {
        SetTorchState(ELight_TorchState::Unlit);
    }
}

bool ALight_TorchLightingSystem::CanBeLit() const
{
    return (CurrentState == ELight_TorchState::Unlit || CurrentState == ELight_TorchState::Extinguished) 
           && FuelRemaining > 0.0f;
}

float ALight_TorchLightingSystem::GetFuelPercentage() const
{
    return MaxFuel > 0.0f ? (FuelRemaining / MaxFuel) * 100.0f : 0.0f;
}

void ALight_TorchLightingSystem::UpdateFlicker(float DeltaTime)
{
    FlickerTime += DeltaTime * FlickerData.FlickerSpeed;
    
    // Create realistic flame flicker using sine waves with noise
    float FlickerNoise = FMath::Sin(FlickerTime) * 0.5f + 
                        FMath::Sin(FlickerTime * 2.3f) * 0.3f + 
                        FMath::Sin(FlickerTime * 4.7f) * 0.2f;
    
    CurrentIntensityMultiplier = 1.0f + (FlickerNoise * FlickerData.FlickerAmount);
    CurrentIntensityMultiplier = FMath::Clamp(CurrentIntensityMultiplier, 0.3f, 1.5f);
    
    // Apply flicker to light intensity
    float NewIntensity = FlickerData.BaseIntensity * CurrentIntensityMultiplier;
    TorchLight->SetIntensity(NewIntensity);
    
    // Slightly vary the color temperature for realism
    float ColorVariation = FlickerNoise * 0.1f;
    FLinearColor FlickerColor = FlickerData.BaseColor;
    FlickerColor.R += ColorVariation;
    FlickerColor.G += ColorVariation * 0.5f;
    TorchLight->SetLightColor(FlickerColor);
}

void ALight_TorchLightingSystem::UpdateFuelConsumption(float DeltaTime)
{
    FuelRemaining -= FuelConsumptionRate * DeltaTime;
    
    if (FuelRemaining <= 0.0f)
    {
        FuelRemaining = 0.0f;
        SetTorchState(ELight_TorchState::Dying);
    }
    else if (FuelRemaining < 20.0f && CurrentState == ELight_TorchState::Burning)
    {
        // Torch starts dying when fuel is low
        SetTorchState(ELight_TorchState::Dying);
    }
}

void ALight_TorchLightingSystem::UpdateTorchState()
{
    switch (CurrentState)
    {
        case ELight_TorchState::Unlit:
            TorchLight->SetVisibility(false);
            TorchLight->SetIntensity(0.0f);
            FlameParticles->SetVisibility(false);
            FlameParticles->Deactivate();
            break;
            
        case ELight_TorchState::Lighting:
            TorchLight->SetVisibility(true);
            TorchLight->SetIntensity(FlickerData.BaseIntensity * 0.3f);
            FlameParticles->SetVisibility(true);
            FlameParticles->Activate();
            break;
            
        case ELight_TorchState::Burning:
            TorchLight->SetVisibility(true);
            FlameParticles->SetVisibility(true);
            FlameParticles->Activate();
            break;
            
        case ELight_TorchState::Dying:
            // Reduce intensity as fuel runs low
            float DyingIntensity = FlickerData.BaseIntensity * (FuelRemaining / 20.0f) * 0.5f;
            TorchLight->SetIntensity(DyingIntensity);
            
            if (FuelRemaining <= 0.0f)
            {
                SetTorchState(ELight_TorchState::Extinguished);
            }
            break;
            
        case ELight_TorchState::Extinguished:
            TorchLight->SetVisibility(false);
            TorchLight->SetIntensity(0.0f);
            FlameParticles->SetVisibility(false);
            FlameParticles->Deactivate();
            break;
    }
}

void ALight_TorchLightingSystem::SetTorchState(ELight_TorchState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        UpdateTorchState();
    }
}

void ALight_TorchLightingSystem::OnRainStart()
{
    if (bAffectedByRain && (CurrentState == ELight_TorchState::Burning || CurrentState == ELight_TorchState::Dying))
    {
        // Rain extinguishes torch
        ExtinguishTorch();
    }
}

void ALight_TorchLightingSystem::OnRainEnd()
{
    // Rain has stopped, torch can be relit if it has fuel
    if (CurrentState == ELight_TorchState::Extinguished && FuelRemaining > 0.0f)
    {
        SetTorchState(ELight_TorchState::Unlit);
    }
}

void ALight_TorchLightingSystem::OnWindChange(float WindStrength)
{
    if (bAffectedByWind && CurrentState == ELight_TorchState::Burning)
    {
        // Strong wind affects flicker intensity
        if (WindStrength > 0.7f)
        {
            FlickerData.FlickerAmount = FMath::Clamp(0.3f + (WindStrength * 0.4f), 0.3f, 0.8f);
            FlickerData.FlickerSpeed = FMath::Clamp(2.0f + (WindStrength * 3.0f), 2.0f, 6.0f);
        }
        else
        {
            // Reset to normal flicker
            FlickerData.FlickerAmount = 0.3f;
            FlickerData.FlickerSpeed = 2.0f;
        }
    }
}