#include "VFX_FireEffectManager.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

UVFX_FireEffectManager::UVFX_FireEffectManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize fire state
    bIsFireActive = false;
    FuelRemaining = 100.0f;
    BurnRate = 1.0f;
    FireFlickerTime = 0.0f;
    LightFlickerIntensity = 1.0f;
    CurrentFireType = EVFX_FireType::Campfire;

    // Create VFX components
    CreateVFXComponents();
}

void UVFX_FireEffectManager::BeginPlay()
{
    Super::BeginPlay();
    
    LoadFireAssets();
    
    // Initialize components if owner is valid
    if (GetOwner())
    {
        if (FlameEffect)
        {
            FlameEffect->SetAutoActivate(false);
        }
        if (SmokeEffect)
        {
            SmokeEffect->SetAutoActivate(false);
        }
        if (EmberEffect)
        {
            EmberEffect->SetAutoActivate(false);
        }
        if (FireLight)
        {
            FireLight->SetVisibility(false);
        }
    }
}

void UVFX_FireEffectManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsFireActive)
    {
        // Update fire flicker
        FireFlickerTime += DeltaTime * 3.0f;
        LightFlickerIntensity = 0.8f + 0.4f * FMath::Sin(FireFlickerTime) * FMath::Sin(FireFlickerTime * 1.7f);

        // Consume fuel
        FuelRemaining -= BurnRate * DeltaTime;
        if (FuelRemaining <= 0.0f)
        {
            FuelRemaining = 0.0f;
            CurrentSettings.FireIntensity *= 0.95f; // Gradually reduce intensity
            
            if (CurrentSettings.FireIntensity < 0.1f)
            {
                ExtinguishFire();
            }
        }

        UpdateFireEffects();
        UpdateLighting();
        UpdateAudio();
    }
}

void UVFX_FireEffectManager::StartFire(EVFX_FireType FireType, const FVFX_FireSettings& Settings)
{
    CurrentFireType = FireType;
    CurrentSettings = Settings;
    bIsFireActive = true;
    
    // Set burn rate based on fire type
    switch (FireType)
    {
        case EVFX_FireType::Campfire:
            BurnRate = 0.5f;
            break;
        case EVFX_FireType::Torch:
            BurnRate = 1.0f;
            break;
        case EVFX_FireType::Wildfire:
            BurnRate = 2.0f;
            break;
        case EVFX_FireType::Cooking:
            BurnRate = 0.3f;
            break;
    }

    // Activate VFX components
    if (FlameEffect && FlameNiagaraSystem)
    {
        FlameEffect->SetAsset(FlameNiagaraSystem);
        FlameEffect->Activate();
    }
    
    if (SmokeEffect && SmokeNiagaraSystem)
    {
        SmokeEffect->SetAsset(SmokeNiagaraSystem);
        SmokeEffect->Activate();
    }
    
    if (EmberEffect && EmberNiagaraSystem)
    {
        EmberEffect->SetAsset(EmberNiagaraSystem);
        EmberEffect->Activate();
    }

    // Activate lighting
    if (FireLight && Settings.bCastLight)
    {
        FireLight->SetVisibility(true);
        FireLight->SetLightColor(Settings.FlameColor);
        FireLight->SetAttenuationRadius(Settings.LightRadius);
    }

    // Start audio
    if (FireAudio && FireCrackleSound)
    {
        FireAudio->SetSound(FireCrackleSound);
        FireAudio->Play();
    }

    UpdateFireEffects();
}

void UVFX_FireEffectManager::StopFire()
{
    ExtinguishFire();
}

void UVFX_FireEffectManager::SetFireIntensity(float NewIntensity)
{
    CurrentSettings.FireIntensity = FMath::Clamp(NewIntensity, 0.0f, 2.0f);
    UpdateFireEffects();
}

bool UVFX_FireEffectManager::IsFireActive() const
{
    return bIsFireActive;
}

void UVFX_FireEffectManager::AddFuel(float FuelAmount)
{
    if (bIsFireActive)
    {
        FuelRemaining = FMath::Clamp(FuelRemaining + FuelAmount, 0.0f, 200.0f);
        
        // Boost intensity temporarily
        CurrentSettings.FireIntensity = FMath::Min(CurrentSettings.FireIntensity * 1.2f, 2.0f);
    }
}

void UVFX_FireEffectManager::ExtinguishFire()
{
    bIsFireActive = false;
    FuelRemaining = 0.0f;

    // Deactivate all effects
    if (FlameEffect)
    {
        FlameEffect->Deactivate();
    }
    if (SmokeEffect)
    {
        SmokeEffect->Deactivate();
    }
    if (EmberEffect)
    {
        EmberEffect->Deactivate();
    }
    if (FireLight)
    {
        FireLight->SetVisibility(false);
    }
    if (FireAudio)
    {
        FireAudio->Stop();
    }
}

void UVFX_FireEffectManager::UpdateFireEffects()
{
    if (!bIsFireActive) return;

    float EffectiveIntensity = CurrentSettings.FireIntensity * (FuelRemaining / 100.0f);
    
    // Update flame effect
    if (FlameEffect)
    {
        FlameEffect->SetFloatParameter(FName("Intensity"), EffectiveIntensity);
        FlameEffect->SetVectorParameter(FName("FlameColor"), FVector(CurrentSettings.FlameColor.R, CurrentSettings.FlameColor.G, CurrentSettings.FlameColor.B));
    }

    // Update smoke effect
    if (SmokeEffect)
    {
        float SmokeIntensity = CurrentSettings.SmokeAmount * EffectiveIntensity;
        SmokeEffect->SetFloatParameter(FName("SmokeAmount"), SmokeIntensity);
    }

    // Update ember effect
    if (EmberEffect)
    {
        float EmberIntensity = CurrentSettings.EmberCount * EffectiveIntensity;
        EmberEffect->SetFloatParameter(FName("EmberCount"), EmberIntensity);
    }
}

void UVFX_FireEffectManager::UpdateLighting()
{
    if (!bIsFireActive || !FireLight || !CurrentSettings.bCastLight) return;

    float EffectiveIntensity = CurrentSettings.FireIntensity * LightFlickerIntensity * (FuelRemaining / 100.0f);
    
    FireLight->SetIntensity(EffectiveIntensity * 2000.0f); // Base intensity
    
    // Subtle color variation for realism
    FLinearColor FlickerColor = CurrentSettings.FlameColor;
    FlickerColor.R += FMath::RandRange(-0.1f, 0.1f);
    FlickerColor.G += FMath::RandRange(-0.05f, 0.05f);
    FireLight->SetLightColor(FlickerColor);
}

void UVFX_FireEffectManager::UpdateAudio()
{
    if (!bIsFireActive || !FireAudio) return;

    float EffectiveIntensity = CurrentSettings.FireIntensity * (FuelRemaining / 100.0f);
    FireAudio->SetVolumeMultiplier(EffectiveIntensity);
    
    // Vary pitch slightly for realism
    float PitchVariation = 1.0f + FMath::RandRange(-0.1f, 0.1f);
    FireAudio->SetPitchMultiplier(PitchVariation);
}

void UVFX_FireEffectManager::LoadFireAssets()
{
    // Load Niagara systems (these would be created in the editor)
    // For now, we'll set them to null and they can be assigned in Blueprint or editor
    FlameNiagaraSystem = nullptr;
    SmokeNiagaraSystem = nullptr;
    EmberNiagaraSystem = nullptr;
    FireCrackleSound = nullptr;
    
    // In a real implementation, you would load these from content:
    // FlameNiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Fire/NS_Fire_Flame"));
    // SmokeNiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Fire/NS_Fire_Smoke"));
    // EmberNiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Fire/NS_Fire_Embers"));
    // FireCrackleSound = LoadObject<USoundCue>(nullptr, TEXT("/Game/Audio/Fire/SC_Fire_Crackle"));
}

void UVFX_FireEffectManager::CreateVFXComponents()
{
    // Create Niagara components
    FlameEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FlameEffect"));
    SmokeEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SmokeEffect"));
    EmberEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EmberEffect"));
    
    // Create light component
    FireLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FireLight"));
    if (FireLight)
    {
        FireLight->SetLightColor(FLinearColor(1.0f, 0.4f, 0.1f));
        FireLight->SetIntensity(2000.0f);
        FireLight->SetAttenuationRadius(500.0f);
        FireLight->SetCastShadows(true);
    }
    
    // Create audio component
    FireAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("FireAudio"));
    if (FireAudio)
    {
        FireAudio->SetAutoActivate(false);
        FireAudio->bOverrideAttenuation = true;
        FireAudio->OverrideAttenuationSettings.FalloffDistance = 1000.0f;
    }
}