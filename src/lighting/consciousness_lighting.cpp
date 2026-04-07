#include "consciousness_lighting.h"
#include "Engine/Engine.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Particles/ParticleSystemComponent.h"

AConsciousnessLighting::AConsciousnessLighting()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Core lighting components
    AmbientLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("AmbientLight"));
    RootComponent = AmbientLight;
    
    PrimaryLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("PrimaryLight"));
    PrimaryLight->SetupAttachment(RootComponent);
    
    SecondaryLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("SecondaryLight"));
    SecondaryLight->SetupAttachment(RootComponent);
    
    // Particle systems for energy effects
    EnergyParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("EnergyParticles"));
    EnergyParticles->SetupAttachment(RootComponent);
    
    // Initialize consciousness state
    ConsciousnessLevel = 0.0f;
    TransitionSpeed = 1.0f;
    PulseIntensity = 0.5f;
    ColorShiftSpeed = 0.3f;
    
    // Set default values
    BaseAmbientIntensity = 0.3f;
    MaxAmbientIntensity = 2.0f;
    BaseLightTemperature = 6500.0f;
    MaxLightTemperature = 3000.0f;
}

void AConsciousnessLighting::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize material parameter collection
    if (MaterialParameters)
    {
        UKismetMaterialLibrary::SetScalarParameterValue(
            GetWorld(), MaterialParameters, 
            TEXT("ConsciousnessLevel"), ConsciousnessLevel
        );
    }
    
    // Set initial lighting state
    UpdateLightingForConsciousnessLevel(ConsciousnessLevel);
}

void AConsciousnessLighting::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update consciousness-based lighting
    UpdateConsciousnessEffects(DeltaTime);
    UpdateAtmosphericParticles(DeltaTime);
    UpdateColorTemperature(DeltaTime);
    
    // Pulse effect for meditation states
    if (bEnablePulseEffect)
    {
        UpdatePulseEffect(DeltaTime);
    }
}

void AConsciousnessLighting::UpdateConsciousnessEffects(float DeltaTime)
{
    // Smooth transition to target consciousness level
    if (FMath::Abs(ConsciousnessLevel - TargetConsciousnessLevel) > 0.01f)
    {
        ConsciousnessLevel = FMath::FInterpTo(
            ConsciousnessLevel, 
            TargetConsciousnessLevel, 
            DeltaTime, 
            TransitionSpeed
        );
        
        UpdateLightingForConsciousnessLevel(ConsciousnessLevel);
    }
}

void AConsciousnessLighting::UpdateLightingForConsciousnessLevel(float Level)
{
    // Ambient light intensity based on consciousness
    float AmbientIntensity = FMath::Lerp(BaseAmbientIntensity, MaxAmbientIntensity, Level);
    AmbientLight->SetIntensity(AmbientIntensity);
    
    // Primary light color and intensity
    FLinearColor LightColor = GetConsciousnessColor(Level);
    PrimaryLight->SetLightColor(LightColor);
    
    float PrimaryIntensity = FMath::Lerp(1.0f, 3.0f, Level);
    PrimaryLight->SetIntensity(PrimaryIntensity);
    
    // Secondary light for depth
    SecondaryLight->SetLightColor(LightColor * 0.7f);
    SecondaryLight->SetIntensity(FMath::Lerp(0.5f, 2.0f, Level));
    
    // Update material parameters
    if (MaterialParameters)
    {
        UKismetMaterialLibrary::SetScalarParameterValue(
            GetWorld(), MaterialParameters, 
            TEXT("ConsciousnessLevel"), Level
        );
        
        UKismetMaterialLibrary::SetVectorParameterValue(
            GetWorld(), MaterialParameters, 
            TEXT("ConsciousnessColor"), LightColor
        );
    }
}

FLinearColor AConsciousnessLighting::GetConsciousnessColor(float Level)
{
    // Color progression through consciousness states
    TArray<FLinearColor> ConsciousnessColors = {
        FLinearColor(0.8f, 0.9f, 1.0f, 1.0f),    // Deep blue - grounded
        FLinearColor(0.6f, 0.8f, 1.0f, 1.0f),    // Light blue - calm
        FLinearColor(0.4f, 0.9f, 0.8f, 1.0f),    // Turquoise - balanced
        FLinearColor(0.9f, 0.8f, 0.4f, 1.0f),    // Golden - awakened
        FLinearColor(1.0f, 0.6f, 0.8f, 1.0f),    // Pink - transcendent
        FLinearColor(0.9f, 0.4f, 1.0f, 1.0f)     // Purple - unity
    };
    
    int32 ColorIndex = FMath::FloorToInt(Level * (ConsciousnessColors.Num() - 1));
    ColorIndex = FMath::Clamp(ColorIndex, 0, ConsciousnessColors.Num() - 2);
    
    float Alpha = (Level * (ConsciousnessColors.Num() - 1)) - ColorIndex;
    
    return FMath::Lerp(ConsciousnessColors[ColorIndex], ConsciousnessColors[ColorIndex + 1], Alpha);
}

void AConsciousnessLighting::UpdateAtmosphericParticles(float DeltaTime)
{
    if (EnergyParticles && EnergyParticles->Template)
    {
        // Adjust particle density based on consciousness level
        float ParticleDensity = FMath::Lerp(10.0f, 100.0f, ConsciousnessLevel);
        
        // Update particle parameters
        UParticleSystemComponent* PSC = EnergyParticles;
        PSC->SetFloatParameter(TEXT("SpawnRate"), ParticleDensity);
        PSC->SetColorParameter(TEXT("ParticleColor"), GetConsciousnessColor(ConsciousnessLevel));
        
        // Energy flow direction based on meditation state
        FVector FlowDirection = bMeditationMode ? FVector::UpVector : FVector::ForwardVector;
        PSC->SetVectorParameter(TEXT("FlowDirection"), FlowDirection);
    }
}

void AConsciousnessLighting::UpdateColorTemperature(float DeltaTime)
{
    // Warmer colors for higher consciousness states
    float TargetTemperature = FMath::Lerp(BaseLightTemperature, MaxLightTemperature, ConsciousnessLevel);
    CurrentLightTemperature = FMath::FInterpTo(CurrentLightTemperature, TargetTemperature, DeltaTime, ColorShiftSpeed);
    
    PrimaryLight->SetTemperature(CurrentLightTemperature);
}

void AConsciousnessLighting::UpdatePulseEffect(float DeltaTime)
{
    PulseTime += DeltaTime;
    
    // Breathing-like pulse for meditation
    float PulseFrequency = FMath::Lerp(0.5f, 2.0f, ConsciousnessLevel);
    float PulseValue = FMath::Sin(PulseTime * PulseFrequency) * PulseIntensity;
    
    // Apply pulse to ambient light
    float PulsedIntensity = BaseAmbientIntensity + PulseValue;
    AmbientLight->SetIntensity(FMath::Max(0.1f, PulsedIntensity));
}

void AConsciousnessLighting::SetConsciousnessLevel(float NewLevel)
{
    TargetConsciousnessLevel = FMath::Clamp(NewLevel, 0.0f, 1.0f);
}

void AConsciousnessLighting::SetMeditationMode(bool bEnabled)
{
    bMeditationMode = bEnabled;
    bEnablePulseEffect = bEnabled;
    
    if (bEnabled)
    {
        // Softer, more rhythmic lighting for meditation
        TransitionSpeed = 0.5f;
        PulseIntensity = 0.3f;
    }
    else
    {
        // Normal exploration lighting
        TransitionSpeed = 1.0f;
        PulseIntensity = 0.5f;
    }
}

void AConsciousnessLighting::TriggerEnergyBurst(float Intensity, FLinearColor Color)
{
    // Temporary intense lighting effect
    if (SecondaryLight)
    {
        SecondaryLight->SetIntensity(Intensity);
        SecondaryLight->SetLightColor(Color);
        
        // Fade back to normal over time
        GetWorld()->GetTimerManager().SetTimer(
            EnergyBurstTimer, 
            [this]() { 
                SecondaryLight->SetIntensity(FMath::Lerp(1.0f, 2.0f, ConsciousnessLevel)); 
                SecondaryLight->SetLightColor(GetConsciousnessColor(ConsciousnessLevel) * 0.7f);
            }, 
            2.0f, 
            false
        );
    }
}