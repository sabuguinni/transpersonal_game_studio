#include "VFX_CampfireSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UVFX_CampfireSystem::UVFX_CampfireSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance
    
    // Initialize default values
    CurrentFuel = 50.0f;
    CurrentIntensity = EVFX_FireIntensity::Medium;
    bFireActive = false;
    LightFlickerTimer = 0.0f;
    BaseLightIntensity = 2000.0f;
    
    // Create components in constructor
    CreateComponents();
}

void UVFX_CampfireSystem::CreateComponents()
{
    // Create Niagara components
    FlameEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FlameEffect"));
    SmokeEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SmokeEffect"));
    SparkEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SparkEffect"));
    EmberEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EmberEffect"));
    
    // Create light component
    FireLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FireLight"));
    FireLight->SetIntensity(BaseLightIntensity);
    FireLight->SetAttenuationRadius(500.0f);
    FireLight->SetLightColor(FLinearColor(1.0f, 0.6f, 0.2f, 1.0f));
    FireLight->SetCastShadows(true);
    FireLight->SetSourceRadius(25.0f);
    
    // Create audio component
    FireAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("FireAudio"));
    FireAudio->bAutoActivate = false;
    
    // Set initial states
    if (FlameEffect) FlameEffect->SetAutoActivate(false);
    if (SmokeEffect) SmokeEffect->SetAutoActivate(false);
    if (SparkEffect) SparkEffect->SetAutoActivate(false);
    if (EmberEffect) EmberEffect->SetAutoActivate(false);
    if (FireLight) FireLight->SetVisibility(false);
}

void UVFX_CampfireSystem::BeginPlay()
{
    Super::BeginPlay();
    
    LoadAssets();
    
    // Initialize fire parameters
    FireParams = GetParametersForIntensity(CurrentIntensity);
    
    // Start with a medium fire if desired
    if (bFireActive)
    {
        StartFire(CurrentIntensity);
    }
}

void UVFX_CampfireSystem::LoadAssets()
{
    // Load Niagara systems (these would be created in the editor)
    // For now, we'll set them to null and log that they need to be assigned
    if (!NS_CampfireFlames)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_CampfireSystem: NS_CampfireFlames not assigned. Please assign in Blueprint."));
    }
    
    if (!NS_CampfireSmoke)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_CampfireSystem: NS_CampfireSmoke not assigned. Please assign in Blueprint."));
    }
    
    if (!NS_CampfireSparks)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_CampfireSystem: NS_CampfireSparks not assigned. Please assign in Blueprint."));
    }
    
    if (!NS_CampfireEmbers)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_CampfireSystem: NS_CampfireEmbers not assigned. Please assign in Blueprint."));
    }
    
    // Assign systems to components if available
    if (FlameEffect && NS_CampfireFlames)
    {
        FlameEffect->SetAsset(NS_CampfireFlames);
    }
    
    if (SmokeEffect && NS_CampfireSmoke)
    {
        SmokeEffect->SetAsset(NS_CampfireSmoke);
    }
    
    if (SparkEffect && NS_CampfireSparks)
    {
        SparkEffect->SetAsset(NS_CampfireSparks);
    }
    
    if (EmberEffect && NS_CampfireEmbers)
    {
        EmberEffect->SetAsset(NS_CampfireEmbers);
    }
    
    // Load audio assets
    if (!FireCrackleSound)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_CampfireSystem: FireCrackleSound not assigned. Please assign in Blueprint."));
    }
    
    if (FireAudio && FireCrackleSound)
    {
        FireAudio->SetSound(FireCrackleSound);
    }
}

void UVFX_CampfireSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bFireActive)
    {
        // Consume fuel over time
        ConsumeFuel(DeltaTime);
        
        // Update light flickering
        UpdateLightFlicker(DeltaTime);
        
        // Update wind effects
        if (bEnableWindEffect)
        {
            UpdateWindEffects();
        }
        
        // Update fire effects based on current state
        UpdateFireEffects();
        
        // Auto-extinguish if out of fuel
        if (bAutoExtinguishWhenOutOfFuel && CurrentFuel <= 0.0f)
        {
            ExtinguishFire();
        }
    }
}

void UVFX_CampfireSystem::StartFire(EVFX_FireIntensity Intensity)
{
    if (bFireActive)
    {
        SetFireIntensity(Intensity);
        return;
    }
    
    bFireActive = true;
    CurrentIntensity = Intensity;
    
    // Get parameters for this intensity
    FireParams = GetParametersForIntensity(Intensity);
    
    // Activate effects
    if (FlameEffect && NS_CampfireFlames)
    {
        FlameEffect->Activate();
    }
    
    if (SmokeEffect && NS_CampfireSmoke)
    {
        SmokeEffect->Activate();
    }
    
    if (SparkEffect && NS_CampfireSparks)
    {
        SparkEffect->Activate();
    }
    
    if (EmberEffect && NS_CampfireEmbers)
    {
        EmberEffect->Activate();
    }
    
    // Turn on light
    if (FireLight)
    {
        FireLight->SetVisibility(true);
        BaseLightIntensity = FireParams.LightIntensity;
        FireLight->SetIntensity(BaseLightIntensity);
        FireLight->SetAttenuationRadius(FireParams.LightRadius);
        FireLight->SetLightColor(FireParams.FireColor);
    }
    
    // Start audio
    if (FireAudio && FireCrackleSound)
    {
        FireAudio->Play();
    }
    
    // Play ignite sound
    if (FireIgniteSound && GetOwner())
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireIgniteSound, GetOwner()->GetActorLocation());
    }
    
    // Apply fire parameters to effects
    ApplyFireParameters(FireParams);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_CampfireSystem: Fire started with intensity %d"), (int32)Intensity);
}

void UVFX_CampfireSystem::ExtinguishFire()
{
    if (!bFireActive) return;
    
    bFireActive = false;
    
    // Deactivate effects
    if (FlameEffect) FlameEffect->Deactivate();
    if (SmokeEffect) SmokeEffect->Deactivate();
    if (SparkEffect) SparkEffect->Deactivate();
    if (EmberEffect) EmberEffect->Deactivate();
    
    // Turn off light
    if (FireLight)
    {
        FireLight->SetVisibility(false);
    }
    
    // Stop audio
    if (FireAudio)
    {
        FireAudio->Stop();
    }
    
    // Play extinguish sound
    if (FireExtinguishSound && GetOwner())
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireExtinguishSound, GetOwner()->GetActorLocation());
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX_CampfireSystem: Fire extinguished"));
}

void UVFX_CampfireSystem::SetFireIntensity(EVFX_FireIntensity NewIntensity)
{
    if (!bFireActive) return;
    
    CurrentIntensity = NewIntensity;
    FireParams = GetParametersForIntensity(NewIntensity);
    
    // Update light
    if (FireLight)
    {
        BaseLightIntensity = FireParams.LightIntensity;
        FireLight->SetAttenuationRadius(FireParams.LightRadius);
        FireLight->SetLightColor(FireParams.FireColor);
    }
    
    // Apply new parameters
    ApplyFireParameters(FireParams);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_CampfireSystem: Fire intensity changed to %d"), (int32)NewIntensity);
}

void UVFX_CampfireSystem::AddFuel(float FuelAmount)
{
    CurrentFuel = FMath::Clamp(CurrentFuel + FuelAmount, 0.0f, MaxFuel);
    UE_LOG(LogTemp, Log, TEXT("VFX_CampfireSystem: Added %.1f fuel. Current fuel: %.1f"), FuelAmount, CurrentFuel);
}

void UVFX_CampfireSystem::SetWindDirection(FVector WindDir)
{
    WindDirection = WindDir.GetSafeNormal();
}

void UVFX_CampfireSystem::SetWindStrength(float Strength)
{
    WindStrength = FMath::Clamp(Strength, 0.0f, 2.0f);
}

FVFX_FireParameters UVFX_CampfireSystem::GetParametersForIntensity(EVFX_FireIntensity Intensity)
{
    FVFX_FireParameters Params;
    
    switch (Intensity)
    {
        case EVFX_FireIntensity::Embers:
            Params.FlameHeight = 30.0f;
            Params.FlameIntensity = 0.2f;
            Params.SmokeAmount = 0.1f;
            Params.SparkCount = 5.0f;
            Params.LightRadius = 200.0f;
            Params.LightIntensity = 500.0f;
            Params.FireColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);
            break;
            
        case EVFX_FireIntensity::Low:
            Params.FlameHeight = 60.0f;
            Params.FlameIntensity = 0.5f;
            Params.SmokeAmount = 0.3f;
            Params.SparkCount = 15.0f;
            Params.LightRadius = 350.0f;
            Params.LightIntensity = 1200.0f;
            Params.FireColor = FLinearColor(1.0f, 0.5f, 0.15f, 1.0f);
            break;
            
        case EVFX_FireIntensity::Medium:
            Params.FlameHeight = 100.0f;
            Params.FlameIntensity = 1.0f;
            Params.SmokeAmount = 0.5f;
            Params.SparkCount = 30.0f;
            Params.LightRadius = 500.0f;
            Params.LightIntensity = 2000.0f;
            Params.FireColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);
            break;
            
        case EVFX_FireIntensity::High:
            Params.FlameHeight = 150.0f;
            Params.FlameIntensity = 1.5f;
            Params.SmokeAmount = 0.7f;
            Params.SparkCount = 50.0f;
            Params.LightRadius = 700.0f;
            Params.LightIntensity = 3000.0f;
            Params.FireColor = FLinearColor(1.0f, 0.7f, 0.3f, 1.0f);
            break;
            
        case EVFX_FireIntensity::Bonfire:
            Params.FlameHeight = 250.0f;
            Params.FlameIntensity = 2.0f;
            Params.SmokeAmount = 1.0f;
            Params.SparkCount = 80.0f;
            Params.LightRadius = 1000.0f;
            Params.LightIntensity = 4500.0f;
            Params.FireColor = FLinearColor(1.0f, 0.8f, 0.4f, 1.0f);
            break;
    }
    
    return Params;
}

void UVFX_CampfireSystem::ApplyFireParameters(const FVFX_FireParameters& Params)
{
    // Apply parameters to Niagara effects
    if (FlameEffect && FlameEffect->GetAsset())
    {
        FlameEffect->SetFloatParameter(TEXT("FlameHeight"), Params.FlameHeight);
        FlameEffect->SetFloatParameter(TEXT("FlameIntensity"), Params.FlameIntensity);
        FlameEffect->SetColorParameter(TEXT("FireColor"), Params.FireColor);
    }
    
    if (SmokeEffect && SmokeEffect->GetAsset())
    {
        SmokeEffect->SetFloatParameter(TEXT("SmokeAmount"), Params.SmokeAmount);
    }
    
    if (SparkEffect && SparkEffect->GetAsset())
    {
        SparkEffect->SetFloatParameter(TEXT("SparkCount"), Params.SparkCount);
    }
    
    if (EmberEffect && EmberEffect->GetAsset())
    {
        EmberEffect->SetFloatParameter(TEXT("EmberIntensity"), Params.FlameIntensity * 0.5f);
    }
}

void UVFX_CampfireSystem::UpdateFireEffects()
{
    // Adjust effects based on fuel level
    float FuelRatio = CurrentFuel / MaxFuel;
    float IntensityMultiplier = FMath::Clamp(FuelRatio, 0.1f, 1.0f);
    
    // Reduce intensity as fuel runs low
    if (FlameEffect && FlameEffect->GetAsset())
    {
        FlameEffect->SetFloatParameter(TEXT("FuelMultiplier"), IntensityMultiplier);
    }
    
    // Smoke increases as fuel runs low (incomplete combustion)
    if (SmokeEffect && SmokeEffect->GetAsset())
    {
        float SmokeMultiplier = FMath::Lerp(1.5f, 1.0f, FuelRatio);
        SmokeEffect->SetFloatParameter(TEXT("SmokeMultiplier"), SmokeMultiplier);
    }
}

void UVFX_CampfireSystem::UpdateLightFlicker(float DeltaTime)
{
    if (!FireLight) return;
    
    LightFlickerTimer += DeltaTime;
    
    // Create realistic fire flicker using multiple sine waves
    float Flicker1 = FMath::Sin(LightFlickerTimer * 8.0f) * 0.1f;
    float Flicker2 = FMath::Sin(LightFlickerTimer * 12.0f) * 0.05f;
    float Flicker3 = FMath::Sin(LightFlickerTimer * 20.0f) * 0.03f;
    
    float TotalFlicker = Flicker1 + Flicker2 + Flicker3;
    float FlickeredIntensity = BaseLightIntensity * (1.0f + TotalFlicker);
    
    // Reduce flicker based on fuel level
    float FuelRatio = CurrentFuel / MaxFuel;
    FlickeredIntensity *= FMath::Clamp(FuelRatio, 0.1f, 1.0f);
    
    FireLight->SetIntensity(FlickeredIntensity);
}

void UVFX_CampfireSystem::UpdateWindEffects()
{
    // Apply wind to flame direction
    if (FlameEffect && FlameEffect->GetAsset())
    {
        FVector WindForce = WindDirection * WindStrength;
        FlameEffect->SetVectorParameter(TEXT("WindDirection"), WindForce);
    }
    
    // Apply wind to smoke
    if (SmokeEffect && SmokeEffect->GetAsset())
    {
        FVector SmokeWind = WindDirection * WindStrength * 0.7f; // Smoke is less affected by wind
        SmokeEffect->SetVectorParameter(TEXT("WindDirection"), SmokeWind);
    }
    
    // Sparks are more affected by wind
    if (SparkEffect && SparkEffect->GetAsset())
    {
        FVector SparkWind = WindDirection * WindStrength * 1.5f;
        SparkEffect->SetVectorParameter(TEXT("WindDirection"), SparkWind);
    }
}

void UVFX_CampfireSystem::ConsumeFuel(float DeltaTime)
{
    if (CurrentFuel <= 0.0f) return;
    
    // Consumption rate varies by intensity
    float IntensityMultiplier = 1.0f;
    switch (CurrentIntensity)
    {
        case EVFX_FireIntensity::Embers:    IntensityMultiplier = 0.2f; break;
        case EVFX_FireIntensity::Low:       IntensityMultiplier = 0.5f; break;
        case EVFX_FireIntensity::Medium:    IntensityMultiplier = 1.0f; break;
        case EVFX_FireIntensity::High:      IntensityMultiplier = 1.5f; break;
        case EVFX_FireIntensity::Bonfire:   IntensityMultiplier = 2.5f; break;
    }
    
    float Consumption = FuelConsumptionRate * IntensityMultiplier * DeltaTime;
    CurrentFuel = FMath::Max(0.0f, CurrentFuel - Consumption);
}