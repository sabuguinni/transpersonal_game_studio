#include "VFX_CampfireSystem.h"
#include "NiagaraComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UVFX_CampfireSystem::UVFX_CampfireSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for VFX updates

    // Initialize state
    CurrentState = EVFX_CampfireState::Unlit;
    BurnDuration = 300.0f; // 5 minutes default burn time
    CurrentBurnTime = 0.0f;
    bIsLit = false;

    // Initialize effect parameters
    FlameIntensity = 1.0f;
    SmokeIntensity = 1.0f;
    EmberSpawnRate = 0.5f;
    SparkFrequency = 0.3f;
    WindStrength = 0.2f;
    WindDirection = FVector(1.0f, 0.0f, 0.0f);

    // Initialize timers
    SparkTimer = 0.0f;
    StateTransitionTimer = 0.0f;
    LightFlickerTimer = 0.0f;

    // Initialize components to null
    FlameComponent = nullptr;
    SmokeComponent = nullptr;
    EmberComponent = nullptr;
    SparkComponent = nullptr;
    FireLight = nullptr;
}

void UVFX_CampfireSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeVFXComponents();
    
    // Set initial state
    TransitionToState(EVFX_CampfireState::Unlit);
}

void UVFX_CampfireSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetOwner())
    {
        return;
    }

    UpdateCampfireState(DeltaTime);
    UpdateEffectIntensities();
    UpdateLighting();
    
    // Update timers
    SparkTimer += DeltaTime;
    LightFlickerTimer += DeltaTime;

    // Spawn random sparks
    if (bIsLit && SparkTimer >= FMath::RandRange(SPARK_INTERVAL_MIN, SPARK_INTERVAL_MAX))
    {
        SpawnRandomSparks();
        SparkTimer = 0.0f;
    }
}

void UVFX_CampfireSystem::InitializeVFXComponents()
{
    if (!GetOwner())
    {
        return;
    }

    // Create Niagara components
    FlameComponent = NewObject<UNiagaraComponent>(GetOwner());
    if (FlameComponent)
    {
        FlameComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        FlameComponent->RegisterComponent();
        FlameComponent->SetAutoActivate(false);
    }

    SmokeComponent = NewObject<UNiagaraComponent>(GetOwner());
    if (SmokeComponent)
    {
        SmokeComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        SmokeComponent->RegisterComponent();
        SmokeComponent->SetAutoActivate(false);
    }

    EmberComponent = NewObject<UNiagaraComponent>(GetOwner());
    if (EmberComponent)
    {
        EmberComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        EmberComponent->RegisterComponent();
        EmberComponent->SetAutoActivate(false);
    }

    SparkComponent = NewObject<UNiagaraComponent>(GetOwner());
    if (SparkComponent)
    {
        SparkComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        SparkComponent->RegisterComponent();
        SparkComponent->SetAutoActivate(false);
    }

    // Create point light
    FireLight = NewObject<UPointLightComponent>(GetOwner());
    if (FireLight)
    {
        FireLight->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        FireLight->RegisterComponent();
        FireLight->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
        FireLight->SetIntensity(0.0f);
        FireLight->SetLightColor(EffectAssets.FireLightColor);
        FireLight->SetAttenuationRadius(800.0f);
        FireLight->SetSourceRadius(20.0f);
        FireLight->SetCastShadows(true);
    }

    // Set Niagara systems if available
    if (EffectAssets.FlameSystem && FlameComponent)
    {
        FlameComponent->SetAsset(EffectAssets.FlameSystem);
    }
    
    if (EffectAssets.SmokeSystem && SmokeComponent)
    {
        SmokeComponent->SetAsset(EffectAssets.SmokeSystem);
    }
    
    if (EffectAssets.EmberSystem && EmberComponent)
    {
        EmberComponent->SetAsset(EffectAssets.EmberSystem);
    }
    
    if (EffectAssets.SparkSystem && SparkComponent)
    {
        SparkComponent->SetAsset(EffectAssets.SparkSystem);
    }
}

void UVFX_CampfireSystem::UpdateCampfireState(float DeltaTime)
{
    switch (CurrentState)
    {
        case EVFX_CampfireState::Unlit:
            // No updates needed when unlit
            break;

        case EVFX_CampfireState::Igniting:
            StateTransitionTimer += DeltaTime;
            if (StateTransitionTimer >= IGNITION_TIME)
            {
                TransitionToState(EVFX_CampfireState::Burning);
            }
            break;

        case EVFX_CampfireState::Burning:
            CurrentBurnTime += DeltaTime;
            if (CurrentBurnTime >= BurnDuration)
            {
                TransitionToState(EVFX_CampfireState::Dying);
            }
            break;

        case EVFX_CampfireState::Dying:
            StateTransitionTimer += DeltaTime;
            if (StateTransitionTimer >= DYING_TIME)
            {
                TransitionToState(EVFX_CampfireState::Extinguished);
            }
            break;

        case EVFX_CampfireState::Extinguished:
            // Fire is out, no updates needed
            break;
    }
}

void UVFX_CampfireSystem::UpdateEffectIntensities()
{
    float IntensityMultiplier = 1.0f;
    
    switch (CurrentState)
    {
        case EVFX_CampfireState::Unlit:
        case EVFX_CampfireState::Extinguished:
            IntensityMultiplier = 0.0f;
            break;

        case EVFX_CampfireState::Igniting:
            IntensityMultiplier = StateTransitionTimer / IGNITION_TIME;
            break;

        case EVFX_CampfireState::Burning:
            {
                float BurnPercentage = GetBurnPercentage();
                IntensityMultiplier = FMath::Lerp(1.0f, 0.7f, BurnPercentage);
            }
            break;

        case EVFX_CampfireState::Dying:
            IntensityMultiplier = 1.0f - (StateTransitionTimer / DYING_TIME);
            break;
    }

    // Update Niagara component parameters
    if (FlameComponent)
    {
        FlameComponent->SetFloatParameter(TEXT("Intensity"), FlameIntensity * IntensityMultiplier);
        FlameComponent->SetVectorParameter(TEXT("WindDirection"), WindDirection);
        FlameComponent->SetFloatParameter(TEXT("WindStrength"), WindStrength);
    }

    if (SmokeComponent)
    {
        SmokeComponent->SetFloatParameter(TEXT("Intensity"), SmokeIntensity * IntensityMultiplier);
        SmokeComponent->SetVectorParameter(TEXT("WindDirection"), WindDirection);
        SmokeComponent->SetFloatParameter(TEXT("WindStrength"), WindStrength * 2.0f); // Smoke more affected by wind
    }

    if (EmberComponent)
    {
        EmberComponent->SetFloatParameter(TEXT("SpawnRate"), EmberSpawnRate * IntensityMultiplier);
        EmberComponent->SetVectorParameter(TEXT("WindDirection"), WindDirection);
    }
}

void UVFX_CampfireSystem::UpdateLighting()
{
    if (!FireLight)
    {
        return;
    }

    float LightIntensity = 0.0f;
    
    switch (CurrentState)
    {
        case EVFX_CampfireState::Unlit:
        case EVFX_CampfireState::Extinguished:
            LightIntensity = 0.0f;
            break;

        case EVFX_CampfireState::Igniting:
            LightIntensity = EffectAssets.FireLightIntensity * (StateTransitionTimer / IGNITION_TIME);
            break;

        case EVFX_CampfireState::Burning:
            {
                float BurnPercentage = GetBurnPercentage();
                float BaseIntensity = FMath::Lerp(EffectAssets.FireLightIntensity, EffectAssets.FireLightIntensity * 0.6f, BurnPercentage);
                
                // Add flickering effect
                float FlickerAmount = FMath::Sin(LightFlickerTimer * LIGHT_FLICKER_SPEED * 2.0f * PI) * 0.1f + 
                                    FMath::Sin(LightFlickerTimer * LIGHT_FLICKER_SPEED * 3.0f * PI) * 0.05f;
                LightIntensity = BaseIntensity * (1.0f + FlickerAmount);
            }
            break;

        case EVFX_CampfireState::Dying:
            {
                float DyingIntensity = EffectAssets.FireLightIntensity * 0.3f * (1.0f - StateTransitionTimer / DYING_TIME);
                float FlickerAmount = FMath::Sin(LightFlickerTimer * LIGHT_FLICKER_SPEED * 4.0f * PI) * 0.3f;
                LightIntensity = DyingIntensity * (1.0f + FlickerAmount);
            }
            break;
    }

    FireLight->SetIntensity(FMath::Max(0.0f, LightIntensity));
}

void UVFX_CampfireSystem::TransitionToState(EVFX_CampfireState NewState)
{
    if (CurrentState == NewState)
    {
        return;
    }

    EVFX_CampfireState PreviousState = CurrentState;
    CurrentState = NewState;
    StateTransitionTimer = 0.0f;

    // Handle state-specific logic
    switch (NewState)
    {
        case EVFX_CampfireState::Unlit:
            bIsLit = false;
            if (FlameComponent) FlameComponent->Deactivate();
            if (SmokeComponent) SmokeComponent->Deactivate();
            if (EmberComponent) EmberComponent->Deactivate();
            if (SparkComponent) SparkComponent->Deactivate();
            break;

        case EVFX_CampfireState::Igniting:
            bIsLit = true;
            if (FlameComponent) FlameComponent->Activate();
            if (SmokeComponent) SmokeComponent->Activate();
            break;

        case EVFX_CampfireState::Burning:
            if (EmberComponent) EmberComponent->Activate();
            if (SparkComponent) SparkComponent->Activate();
            break;

        case EVFX_CampfireState::Dying:
            if (SparkComponent) SparkComponent->Deactivate();
            break;

        case EVFX_CampfireState::Extinguished:
            bIsLit = false;
            if (FlameComponent) FlameComponent->Deactivate();
            if (EmberComponent) EmberComponent->Deactivate();
            // Keep smoke active for a bit longer
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("Campfire state changed from %d to %d"), (int32)PreviousState, (int32)NewState);
}

void UVFX_CampfireSystem::SpawnRandomSparks()
{
    if (SparkComponent && CurrentState == EVFX_CampfireState::Burning)
    {
        int32 SparkCount = FMath::RandRange(3, 8);
        SparkComponent->SetIntParameter(TEXT("BurstCount"), SparkCount);
        SparkComponent->SetFloatParameter(TEXT("BurstIntensity"), SparkFrequency);
    }
}

// === PUBLIC METHODS ===

void UVFX_CampfireSystem::LightCampfire()
{
    if (CurrentState == EVFX_CampfireState::Unlit || CurrentState == EVFX_CampfireState::Extinguished)
    {
        CurrentBurnTime = 0.0f;
        TransitionToState(EVFX_CampfireState::Igniting);
        
        UE_LOG(LogTemp, Log, TEXT("Campfire lit"));
    }
}

void UVFX_CampfireSystem::ExtinguishCampfire()
{
    if (bIsLit)
    {
        TransitionToState(EVFX_CampfireState::Extinguished);
        
        UE_LOG(LogTemp, Log, TEXT("Campfire extinguished"));
    }
}

void UVFX_CampfireSystem::AddFuel(float FuelAmount)
{
    if (bIsLit)
    {
        BurnDuration += FuelAmount;
        
        // If fire was dying, bring it back to burning
        if (CurrentState == EVFX_CampfireState::Dying)
        {
            TransitionToState(EVFX_CampfireState::Burning);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Added %.1f seconds of fuel to campfire"), FuelAmount);
    }
}

void UVFX_CampfireSystem::SetWindEffect(FVector NewWindDirection, float NewWindStrength)
{
    WindDirection = NewWindDirection.GetSafeNormal();
    WindStrength = FMath::Clamp(NewWindStrength, 0.0f, 2.0f);
}

void UVFX_CampfireSystem::SetFlameIntensity(float NewIntensity)
{
    FlameIntensity = FMath::Clamp(NewIntensity, 0.1f, 5.0f);
}

void UVFX_CampfireSystem::SetSmokeIntensity(float NewIntensity)
{
    SmokeIntensity = FMath::Clamp(NewIntensity, 0.1f, 3.0f);
}

void UVFX_CampfireSystem::SetEmberSpawnRate(float NewRate)
{
    EmberSpawnRate = FMath::Clamp(NewRate, 0.0f, 2.0f);
}

void UVFX_CampfireSystem::TriggerSparkBurst(int32 SparkCount)
{
    if (SparkComponent && bIsLit)
    {
        SparkComponent->SetIntParameter(TEXT("BurstCount"), SparkCount);
        SparkComponent->SetFloatParameter(TEXT("BurstIntensity"), 1.0f);
        
        UE_LOG(LogTemp, Log, TEXT("Triggered spark burst with %d sparks"), SparkCount);
    }
}