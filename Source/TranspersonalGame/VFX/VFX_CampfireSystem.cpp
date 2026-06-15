#include "VFX_CampfireSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UVFX_CampfireSystem::UVFX_CampfireSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize default values
    CurrentState = EVFX_CampfireState::Unlit;
    FuelLevel = 100.0f;
    FuelConsumptionRate = 1.0f;
    WindInfluence = 0.0f;
    bAffectedByWeather = true;
    StateTransitionTimer = 0.0f;
    LastSparkTime = 0.0f;
    SparkInterval = 2.0f;

    // Create components
    FlameComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FlameComponent"));
    SmokeComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SmokeComponent"));
    EmberComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EmberComponent"));
    SparkComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SparkComponent"));
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));

    // Initialize effects data
    CampfireEffects.FlameIntensity = 1.0f;
    CampfireEffects.SmokeIntensity = 0.8f;
}

void UVFX_CampfireSystem::BeginPlay()
{
    Super::BeginPlay();
    
    CreateNiagaraComponents();
    
    // Set initial state
    if (CurrentState == EVFX_CampfireState::Unlit)
    {
        if (FlameComponent) FlameComponent->Deactivate();
        if (SmokeComponent) SmokeComponent->Deactivate();
        if (EmberComponent) EmberComponent->Deactivate();
        if (SparkComponent) SparkComponent->Deactivate();
        if (AudioComponent) AudioComponent->Stop();
    }
}

void UVFX_CampfireSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTransitionTimer += DeltaTime;

    switch (CurrentState)
    {
        case EVFX_CampfireState::Burning:
        {
            // Consume fuel
            FuelLevel = FMath::Max(0.0f, FuelLevel - (FuelConsumptionRate * DeltaTime));
            
            // Check if fuel is running low
            if (FuelLevel <= 10.0f)
            {
                TransitionToState(EVFX_CampfireState::Dying);
            }
            
            // Generate sparks periodically
            if (StateTransitionTimer - LastSparkTime >= SparkInterval)
            {
                if (SparkComponent && SparkComponent->GetAsset())
                {
                    SparkComponent->Activate(true);
                    LastSparkTime = StateTransitionTimer;
                }
            }
            
            UpdateEffectIntensities();
            break;
        }
        
        case EVFX_CampfireState::Dying:
        {
            FuelLevel = FMath::Max(0.0f, FuelLevel - (FuelConsumptionRate * 0.5f * DeltaTime));
            
            if (FuelLevel <= 0.0f)
            {
                TransitionToState(EVFX_CampfireState::Extinguished);
            }
            
            // Reduce effect intensities
            float DyingIntensity = FuelLevel / 10.0f;
            CampfireEffects.FlameIntensity = DyingIntensity;
            CampfireEffects.SmokeIntensity = 0.3f + (DyingIntensity * 0.5f);
            UpdateEffectIntensities();
            break;
        }
        
        case EVFX_CampfireState::Igniting:
        {
            if (StateTransitionTimer >= 3.0f)
            {
                TransitionToState(EVFX_CampfireState::Burning);
            }
            
            // Gradually increase flame intensity during ignition
            float IgnitionProgress = StateTransitionTimer / 3.0f;
            CampfireEffects.FlameIntensity = IgnitionProgress;
            UpdateEffectIntensities();
            break;
        }
        
        case EVFX_CampfireState::Extinguished:
        {
            // Only smoke remains for a while
            if (StateTransitionTimer >= 10.0f)
            {
                if (SmokeComponent) SmokeComponent->Deactivate();
            }
            break;
        }
    }

    UpdateAudioVolume();
}

void UVFX_CampfireSystem::IgniteCampfire()
{
    if (CurrentState == EVFX_CampfireState::Unlit && FuelLevel > 0.0f)
    {
        TransitionToState(EVFX_CampfireState::Igniting);
        
        // Start effects
        if (FlameComponent && CampfireEffects.FlameEffect)
        {
            FlameComponent->SetAsset(CampfireEffects.FlameEffect);
            FlameComponent->Activate(true);
        }
        
        if (SmokeComponent && CampfireEffects.SmokeEffect)
        {
            SmokeComponent->SetAsset(CampfireEffects.SmokeEffect);
            SmokeComponent->Activate(true);
        }
        
        if (AudioComponent && CampfireEffects.CracklingSound)
        {
            AudioComponent->SetSound(CampfireEffects.CracklingSound);
            AudioComponent->Play();
        }
        
        UE_LOG(LogTemp, Log, TEXT("Campfire ignited"));
    }
}

void UVFX_CampfireSystem::ExtinguishCampfire()
{
    if (CurrentState == EVFX_CampfireState::Burning || CurrentState == EVFX_CampfireState::Dying)
    {
        TransitionToState(EVFX_CampfireState::Extinguished);
        
        // Stop flame and ember effects
        if (FlameComponent) FlameComponent->Deactivate();
        if (EmberComponent) EmberComponent->Deactivate();
        if (SparkComponent) SparkComponent->Deactivate();
        
        // Keep smoke for a while
        CampfireEffects.SmokeIntensity = 0.6f;
        UpdateEffectIntensities();
        
        // Stop audio
        if (AudioComponent) AudioComponent->Stop();
        
        UE_LOG(LogTemp, Log, TEXT("Campfire extinguished"));
    }
}

void UVFX_CampfireSystem::AddFuel(float FuelAmount)
{
    FuelLevel = FMath::Min(100.0f, FuelLevel + FuelAmount);
    
    if (CurrentState == EVFX_CampfireState::Dying && FuelLevel > 10.0f)
    {
        TransitionToState(EVFX_CampfireState::Burning);
        CampfireEffects.FlameIntensity = 1.0f;
        CampfireEffects.SmokeIntensity = 0.8f;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Fuel added. Current level: %f"), FuelLevel);
}

void UVFX_CampfireSystem::SetWindStrength(float WindStrength)
{
    WindInfluence = FMath::Clamp(WindStrength, 0.0f, 2.0f);
    
    // Wind affects flame and smoke direction/intensity
    if (FlameComponent)
    {
        FlameComponent->SetFloatParameter(TEXT("WindStrength"), WindInfluence);
    }
    
    if (SmokeComponent)
    {
        SmokeComponent->SetFloatParameter(TEXT("WindStrength"), WindInfluence);
        SmokeComponent->SetVectorParameter(TEXT("WindDirection"), FVector(WindInfluence, 0.0f, 0.0f));
    }
}

void UVFX_CampfireSystem::SetWeatherInfluence(bool bRaining, float RainIntensity)
{
    if (!bAffectedByWeather) return;
    
    if (bRaining && RainIntensity > 0.3f)
    {
        // Rain reduces flame intensity and may extinguish the fire
        float RainReduction = RainIntensity * 0.5f;
        CampfireEffects.FlameIntensity = FMath::Max(0.1f, CampfireEffects.FlameIntensity - RainReduction);
        
        if (RainIntensity > 0.8f && CurrentState == EVFX_CampfireState::Burning)
        {
            // Heavy rain extinguishes the fire
            ExtinguishCampfire();
        }
        
        // Increase smoke when rain hits fire
        CampfireEffects.SmokeIntensity = FMath::Min(2.0f, CampfireEffects.SmokeIntensity + (RainIntensity * 0.3f));
    }
    
    UpdateEffectIntensities();
}

void UVFX_CampfireSystem::UpdateEffectIntensities()
{
    if (FlameComponent)
    {
        FlameComponent->SetFloatParameter(TEXT("FlameIntensity"), CampfireEffects.FlameIntensity);
        FlameComponent->SetFloatParameter(TEXT("FuelLevel"), FuelLevel / 100.0f);
    }
    
    if (SmokeComponent)
    {
        SmokeComponent->SetFloatParameter(TEXT("SmokeIntensity"), CampfireEffects.SmokeIntensity);
    }
    
    if (EmberComponent && CurrentState == EVFX_CampfireState::Burning)
    {
        EmberComponent->SetFloatParameter(TEXT("EmberIntensity"), CampfireEffects.FlameIntensity * 0.8f);
        if (!EmberComponent->IsActive() && CampfireEffects.EmberEffect)
        {
            EmberComponent->SetAsset(CampfireEffects.EmberEffect);
            EmberComponent->Activate(true);
        }
    }
}

void UVFX_CampfireSystem::UpdateAudioVolume()
{
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        float VolumeMultiplier = 1.0f;
        
        switch (CurrentState)
        {
            case EVFX_CampfireState::Igniting:
                VolumeMultiplier = StateTransitionTimer / 3.0f;
                break;
            case EVFX_CampfireState::Burning:
                VolumeMultiplier = CampfireEffects.FlameIntensity;
                break;
            case EVFX_CampfireState::Dying:
                VolumeMultiplier = FuelLevel / 10.0f;
                break;
            default:
                VolumeMultiplier = 0.0f;
                break;
        }
        
        AudioComponent->SetVolumeMultiplier(FMath::Clamp(VolumeMultiplier, 0.0f, 1.0f));
    }
}

void UVFX_CampfireSystem::TransitionToState(EVFX_CampfireState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTransitionTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Campfire state changed to: %d"), (int32)NewState);
    }
}

void UVFX_CampfireSystem::CreateNiagaraComponents()
{
    // Ensure all Niagara components are properly initialized
    if (FlameComponent)
    {
        FlameComponent->SetAutoActivate(false);
        FlameComponent->bAutoManageAttachment = true;
    }
    
    if (SmokeComponent)
    {
        SmokeComponent->SetAutoActivate(false);
        SmokeComponent->bAutoManageAttachment = true;
    }
    
    if (EmberComponent)
    {
        EmberComponent->SetAutoActivate(false);
        EmberComponent->bAutoManageAttachment = true;
    }
    
    if (SparkComponent)
    {
        SparkComponent->SetAutoActivate(false);
        SparkComponent->bAutoManageAttachment = true;
    }
    
    if (AudioComponent)
    {
        AudioComponent->SetAutoActivate(false);
        AudioComponent->bAutoManageAttachment = true;
    }
}