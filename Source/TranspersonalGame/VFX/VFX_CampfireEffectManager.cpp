#include "VFX_CampfireEffectManager.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

UVFX_CampfireEffectManager::UVFX_CampfireEffectManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for performance

    // Initialize default values
    CurrentState = EVFX_CampfireState::Unlit;
    MaxFuel = 100.0f;
    CurrentFuel = 0.0f;
    CurrentIntensity = 0.0f;
    FuelConsumptionRate = 1.0f; // 1 fuel per second when burning
    IgnitionTime = 3.0f;
    ExtinguishTime = 2.0f;
    WindMultiplier = 1.0f;
    RainExtinguishThreshold = 0.5f;
    CurrentWindStrength = 0.0f;
    CurrentRainIntensity = 0.0f;

    // Create VFX components
    FlameComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FlameComponent"));
    SmokeComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SmokeComponent"));
    EmberComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EmberComponent"));
    SparkComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SparkComponent"));

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));

    // Initialize effect data for different states
    StateEffects.Add(EVFX_CampfireState::Unlit, FVFX_CampfireEffectData());
    StateEffects.Add(EVFX_CampfireState::Igniting, FVFX_CampfireEffectData());
    StateEffects.Add(EVFX_CampfireState::Burning, FVFX_CampfireEffectData());
    StateEffects.Add(EVFX_CampfireState::Dying, FVFX_CampfireEffectData());
    StateEffects.Add(EVFX_CampfireState::Extinguished, FVFX_CampfireEffectData());

    // Set default intensities for each state
    StateEffects[EVFX_CampfireState::Igniting].IntensityMultiplier = 0.3f;
    StateEffects[EVFX_CampfireState::Burning].IntensityMultiplier = 1.0f;
    StateEffects[EVFX_CampfireState::Dying].IntensityMultiplier = 0.2f;

    // Set default volume multipliers
    StateEffects[EVFX_CampfireState::Igniting].VolumeMultiplier = 0.4f;
    StateEffects[EVFX_CampfireState::Burning].VolumeMultiplier = 1.0f;
    StateEffects[EVFX_CampfireState::Dying].VolumeMultiplier = 0.3f;
}

void UVFX_CampfireEffectManager::BeginPlay()
{
    Super::BeginPlay();

    // Attach components to owner's root component
    if (AActor* Owner = GetOwner())
    {
        USceneComponent* RootComp = Owner->GetRootComponent();
        if (RootComp)
        {
            FlameComponent->AttachToComponent(RootComp, FAttachmentTransformRules::KeepRelativeTransform);
            SmokeComponent->AttachToComponent(RootComp, FAttachmentTransformRules::KeepRelativeTransform);
            EmberComponent->AttachToComponent(RootComp, FAttachmentTransformRules::KeepRelativeTransform);
            SparkComponent->AttachToComponent(RootComp, FAttachmentTransformRules::KeepRelativeTransform);
            AudioComponent->AttachToComponent(RootComp, FAttachmentTransformRules::KeepRelativeTransform);
        }
    }

    // Initialize VFX for current state
    UpdateVFXForState();
    UpdateAudioForState();

    // Start fuel consumption timer if burning
    if (CurrentState == EVFX_CampfireState::Burning)
    {
        GetWorld()->GetTimerManager().SetTimer(FuelConsumptionTimer, this, &UVFX_CampfireEffectManager::ConsumeFuel, 1.0f, true);
    }
}

void UVFX_CampfireEffectManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update intensity based on current state and environmental factors
    CalculateIntensity();

    // Check environmental effects
    CheckEnvironmentalEffects();

    // Update VFX parameters
    UpdateEffectIntensity(CurrentIntensity);
}

void UVFX_CampfireEffectManager::IgniteCampfire()
{
    if (CurrentState == EVFX_CampfireState::Unlit && CurrentFuel > 0.0f)
    {
        TransitionToState(EVFX_CampfireState::Igniting);
        
        // Start ignition timer
        GetWorld()->GetTimerManager().SetTimer(StateTransitionTimer, this, &UVFX_CampfireEffectManager::HandleIgnitionProcess, IgnitionTime, false);
        
        UE_LOG(LogTemp, Warning, TEXT("Campfire ignition started"));
    }
    else if (CurrentFuel <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot ignite campfire: No fuel"));
    }
}

void UVFX_CampfireEffectManager::ExtinguishCampfire()
{
    if (CurrentState == EVFX_CampfireState::Burning || CurrentState == EVFX_CampfireState::Igniting)
    {
        TransitionToState(EVFX_CampfireState::Dying);
        
        // Start extinguish timer
        GetWorld()->GetTimerManager().SetTimer(StateTransitionTimer, this, &UVFX_CampfireEffectManager::HandleExtinguishProcess, ExtinguishTime, false);
        
        UE_LOG(LogTemp, Warning, TEXT("Campfire extinguishing started"));
    }
}

void UVFX_CampfireEffectManager::AddFuel(float FuelAmount)
{
    float PreviousFuel = CurrentFuel;
    CurrentFuel = FMath::Clamp(CurrentFuel + FuelAmount, 0.0f, MaxFuel);
    
    // Broadcast fuel change event
    OnFuelChanged.Broadcast(CurrentFuel / MaxFuel);
    
    UE_LOG(LogTemp, Log, TEXT("Fuel added: %.1f, Total: %.1f/%.1f"), FuelAmount, CurrentFuel, MaxFuel);
}

void UVFX_CampfireEffectManager::SetWindStrength(float WindStrength)
{
    CurrentWindStrength = FMath::Clamp(WindStrength, 0.0f, 2.0f);
    
    // Update flame and ember effects based on wind
    if (FlameComponent && FlameComponent->GetAsset())
    {
        FlameComponent->SetFloatParameter(TEXT("WindStrength"), CurrentWindStrength);
    }
    
    if (EmberComponent && EmberComponent->GetAsset())
    {
        EmberComponent->SetFloatParameter(TEXT("WindStrength"), CurrentWindStrength);
    }
}

void UVFX_CampfireEffectManager::SetRainIntensity(float RainIntensity)
{
    CurrentRainIntensity = FMath::Clamp(RainIntensity, 0.0f, 1.0f);
    
    // Rain affects smoke and can extinguish fire
    if (SmokeComponent && SmokeComponent->GetAsset())
    {
        SmokeComponent->SetFloatParameter(TEXT("RainSuppression"), CurrentRainIntensity);
    }
}

void UVFX_CampfireEffectManager::UpdateEffectIntensity(float NewIntensity)
{
    CurrentIntensity = FMath::Clamp(NewIntensity, 0.0f, 1.0f);
    
    // Update all VFX components with new intensity
    if (FlameComponent && FlameComponent->GetAsset())
    {
        FlameComponent->SetFloatParameter(TEXT("Intensity"), CurrentIntensity);
    }
    
    if (SmokeComponent && SmokeComponent->GetAsset())
    {
        SmokeComponent->SetFloatParameter(TEXT("Intensity"), CurrentIntensity * 0.8f); // Smoke slightly less intense
    }
    
    if (EmberComponent && EmberComponent->GetAsset())
    {
        EmberComponent->SetFloatParameter(TEXT("Intensity"), CurrentIntensity);
    }
    
    // Update audio volume
    if (AudioComponent && AudioComponent->GetSound())
    {
        float VolumeMultiplier = StateEffects.Contains(CurrentState) ? StateEffects[CurrentState].VolumeMultiplier : 1.0f;
        AudioComponent->SetVolumeMultiplier(CurrentIntensity * VolumeMultiplier);
    }
}

void UVFX_CampfireEffectManager::SpawnSparkBurst()
{
    if (SparkComponent && SparkComponent->GetAsset())
    {
        SparkComponent->SetFloatParameter(TEXT("BurstIntensity"), 2.0f);
        SparkComponent->Activate(true);
        
        // Reset burst intensity after a short time
        GetWorld()->GetTimerManager().SetTimer(SparkTimer, [this]()
        {
            if (SparkComponent && SparkComponent->GetAsset())
            {
                SparkComponent->SetFloatParameter(TEXT("BurstIntensity"), 1.0f);
            }
        }, 0.5f, false);
    }
}

void UVFX_CampfireEffectManager::TransitionToState(EVFX_CampfireState NewState)
{
    if (CurrentState != NewState)
    {
        EVFX_CampfireState PreviousState = CurrentState;
        CurrentState = NewState;
        
        // Update VFX and audio for new state
        UpdateVFXForState();
        UpdateAudioForState();
        
        // Broadcast state change event
        OnStateChanged.Broadcast(CurrentState);
        
        UE_LOG(LogTemp, Log, TEXT("Campfire state changed from %d to %d"), (int32)PreviousState, (int32)CurrentState);
    }
}

void UVFX_CampfireEffectManager::UpdateVFXForState()
{
    // Deactivate all effects first
    if (FlameComponent) FlameComponent->Deactivate();
    if (SmokeComponent) SmokeComponent->Deactivate();
    if (EmberComponent) EmberComponent->Deactivate();
    if (SparkComponent) SparkComponent->Deactivate();
    
    // Activate effects based on current state
    switch (CurrentState)
    {
        case EVFX_CampfireState::Unlit:
            // No effects active
            break;
            
        case EVFX_CampfireState::Igniting:
            if (SmokeComponent) SmokeComponent->Activate();
            if (SparkComponent) SparkComponent->Activate();
            break;
            
        case EVFX_CampfireState::Burning:
            if (FlameComponent) FlameComponent->Activate();
            if (SmokeComponent) SmokeComponent->Activate();
            if (EmberComponent) EmberComponent->Activate();
            if (SparkComponent) SparkComponent->Activate();
            break;
            
        case EVFX_CampfireState::Dying:
            if (SmokeComponent) SmokeComponent->Activate();
            if (EmberComponent) EmberComponent->Activate();
            break;
            
        case EVFX_CampfireState::Extinguished:
            if (SmokeComponent) SmokeComponent->Activate(); // Residual smoke
            break;
    }
}

void UVFX_CampfireEffectManager::UpdateAudioForState()
{
    if (!AudioComponent) return;
    
    if (StateEffects.Contains(CurrentState) && StateEffects[CurrentState].CracklingSound)
    {
        AudioComponent->SetSound(StateEffects[CurrentState].CracklingSound);
        AudioComponent->Play();
    }
    else if (CurrentState == EVFX_CampfireState::Unlit || CurrentState == EVFX_CampfireState::Extinguished)
    {
        AudioComponent->Stop();
    }
}

void UVFX_CampfireEffectManager::ConsumeFuel()
{
    if (CurrentState == EVFX_CampfireState::Burning && CurrentFuel > 0.0f)
    {
        float ConsumptionAmount = FuelConsumptionRate * WindMultiplier;
        CurrentFuel = FMath::Max(0.0f, CurrentFuel - ConsumptionAmount);
        
        // Broadcast fuel change
        OnFuelChanged.Broadcast(CurrentFuel / MaxFuel);
        
        // Check if fuel is depleted
        if (CurrentFuel <= 0.0f)
        {
            TransitionToState(EVFX_CampfireState::Dying);
            GetWorld()->GetTimerManager().SetTimer(StateTransitionTimer, this, &UVFX_CampfireEffectManager::HandleExtinguishProcess, ExtinguishTime, false);
        }
        // Check if fuel is low (start dying process)
        else if (CurrentFuel <= MaxFuel * 0.1f && CurrentState == EVFX_CampfireState::Burning)
        {
            TransitionToState(EVFX_CampfireState::Dying);
        }
    }
}

void UVFX_CampfireEffectManager::HandleIgnitionProcess()
{
    if (CurrentState == EVFX_CampfireState::Igniting)
    {
        TransitionToState(EVFX_CampfireState::Burning);
        
        // Start fuel consumption timer
        GetWorld()->GetTimerManager().SetTimer(FuelConsumptionTimer, this, &UVFX_CampfireEffectManager::ConsumeFuel, 1.0f, true);
        
        // Start random spark timer
        GetWorld()->GetTimerManager().SetTimer(SparkTimer, this, &UVFX_CampfireEffectManager::SpawnRandomSparks, FMath::RandRange(2.0f, 5.0f), true);
    }
}

void UVFX_CampfireEffectManager::HandleExtinguishProcess()
{
    if (CurrentState == EVFX_CampfireState::Dying)
    {
        TransitionToState(EVFX_CampfireState::Extinguished);
        
        // Clear fuel consumption timer
        GetWorld()->GetTimerManager().ClearTimer(FuelConsumptionTimer);
        GetWorld()->GetTimerManager().ClearTimer(SparkTimer);
        
        // Transition to unlit after smoke dissipates
        GetWorld()->GetTimerManager().SetTimer(StateTransitionTimer, [this]()
        {
            TransitionToState(EVFX_CampfireState::Unlit);
        }, 5.0f, false);
    }
}

void UVFX_CampfireEffectManager::CalculateIntensity()
{
    float BaseIntensity = 0.0f;
    
    if (StateEffects.Contains(CurrentState))
    {
        BaseIntensity = StateEffects[CurrentState].IntensityMultiplier;
    }
    
    // Modify intensity based on fuel level
    if (CurrentState == EVFX_CampfireState::Burning || CurrentState == EVFX_CampfireState::Dying)
    {
        float FuelRatio = CurrentFuel / MaxFuel;
        BaseIntensity *= FMath::Lerp(0.2f, 1.0f, FuelRatio);
    }
    
    // Apply wind effects (increases intensity slightly)
    BaseIntensity *= (1.0f + CurrentWindStrength * 0.2f);
    
    // Apply rain effects (decreases intensity)
    BaseIntensity *= (1.0f - CurrentRainIntensity * 0.5f);
    
    CurrentIntensity = FMath::Clamp(BaseIntensity, 0.0f, 1.0f);
}

void UVFX_CampfireEffectManager::SpawnRandomSparks()
{
    if (CurrentState == EVFX_CampfireState::Burning && FMath::RandRange(0.0f, 1.0f) < 0.3f)
    {
        SpawnSparkBurst();
    }
}

void UVFX_CampfireEffectManager::CheckEnvironmentalEffects()
{
    // Rain can extinguish fire
    if (CurrentRainIntensity >= RainExtinguishThreshold)
    {
        if (CurrentState == EVFX_CampfireState::Burning || CurrentState == EVFX_CampfireState::Igniting)
        {
            ExtinguishCampfire();
            UE_LOG(LogTemp, Warning, TEXT("Campfire extinguished by rain"));
        }
    }
    
    // Strong wind increases fuel consumption
    if (CurrentWindStrength > 1.5f && CurrentState == EVFX_CampfireState::Burning)
    {
        WindMultiplier = 1.0f + (CurrentWindStrength - 1.0f) * 0.5f;
    }
    else
    {
        WindMultiplier = 1.0f;
    }
}