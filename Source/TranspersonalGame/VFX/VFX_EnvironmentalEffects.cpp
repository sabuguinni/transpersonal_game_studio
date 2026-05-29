#include "VFX_EnvironmentalEffects.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

UVFX_EnvironmentalEffects::UVFX_EnvironmentalEffects()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize Niagara components
    RainEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RainEffect"));
    DustEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DustEffect"));
    FogEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FogEffect"));
    AmbientEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AmbientEffect"));

    // Initialize default weather settings
    WeatherSettings.Intensity = 1.0f;
    WeatherSettings.ParticleCount = 100.0f;
    WeatherSettings.WindDirection = FVector(1.0f, 0.0f, 0.0f);
    WeatherSettings.WindStrength = 50.0f;
    WeatherSettings.ParticleColor = FLinearColor::White;

    CurrentWeatherType = EVFX_WeatherType::Clear;
    CurrentBiome = EVFX_BiomeType::Savana;
    EffectUpdateInterval = 0.1f;
    MaxEffectDistance = 5000.0f;
    MaxActiveEffects = 50;
}

void UVFX_EnvironmentalEffects::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeData();
    
    // Start with clear weather
    SetWeatherType(EVFX_WeatherType::Clear);
    SetCurrentBiome(EVFX_BiomeType::Savana);
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_EnvironmentalEffects: System initialized with %d biome configurations"), BiomeEffectData.Num());
}

void UVFX_EnvironmentalEffects::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastEffectUpdate += DeltaTime;
    
    if (LastEffectUpdate >= EffectUpdateInterval)
    {
        UpdateWeatherEffects();
        UpdateBiomeEffects();
        CleanupExpiredEffects();
        LastEffectUpdate = 0.0f;
    }
}

void UVFX_EnvironmentalEffects::SetWeatherType(EVFX_WeatherType NewWeatherType)
{
    CurrentWeatherType = NewWeatherType;
    
    // Deactivate all weather effects first
    if (RainEffect) RainEffect->SetVisibility(false);
    if (DustEffect) DustEffect->SetVisibility(false);
    if (FogEffect) FogEffect->SetVisibility(false);
    
    // Activate appropriate effect
    switch (CurrentWeatherType)
    {
        case EVFX_WeatherType::Rain:
        case EVFX_WeatherType::Storm:
            if (RainEffect)
            {
                RainEffect->SetVisibility(true);
                RainEffect->SetFloatParameter(TEXT("Intensity"), WeatherSettings.Intensity);
                RainEffect->SetFloatParameter(TEXT("ParticleCount"), WeatherSettings.ParticleCount);
                RainEffect->SetVectorParameter(TEXT("WindDirection"), WeatherSettings.WindDirection);
            }
            break;
            
        case EVFX_WeatherType::Dust:
            if (DustEffect)
            {
                DustEffect->SetVisibility(true);
                DustEffect->SetFloatParameter(TEXT("Intensity"), WeatherSettings.Intensity);
                DustEffect->SetVectorParameter(TEXT("WindDirection"), WeatherSettings.WindDirection);
            }
            break;
            
        case EVFX_WeatherType::Fog:
            if (FogEffect)
            {
                FogEffect->SetVisibility(true);
                FogEffect->SetFloatParameter(TEXT("Density"), WeatherSettings.Intensity * 0.5f);
                FogEffect->SetColorParameter(TEXT("FogColor"), WeatherSettings.ParticleColor);
            }
            break;
            
        case EVFX_WeatherType::Clear:
        default:
            // All effects already disabled
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_EnvironmentalEffects: Weather changed to %d"), (int32)CurrentWeatherType);
}

void UVFX_EnvironmentalEffects::UpdateWeatherIntensity(float NewIntensity)
{
    WeatherSettings.Intensity = FMath::Clamp(NewIntensity, 0.0f, 2.0f);
    
    // Update active weather effects
    SetWeatherType(CurrentWeatherType);
}

void UVFX_EnvironmentalEffects::SetWindDirection(FVector NewWindDirection)
{
    WeatherSettings.WindDirection = NewWindDirection.GetSafeNormal();
    
    // Update wind parameters on active effects
    if (RainEffect && RainEffect->IsVisible())
    {
        RainEffect->SetVectorParameter(TEXT("WindDirection"), WeatherSettings.WindDirection);
    }
    if (DustEffect && DustEffect->IsVisible())
    {
        DustEffect->SetVectorParameter(TEXT("WindDirection"), WeatherSettings.WindDirection);
    }
}

void UVFX_EnvironmentalEffects::SetCurrentBiome(EVFX_BiomeType NewBiome)
{
    CurrentBiome = NewBiome;
    
    // Find biome data
    FVFX_BiomeEffectData* BiomeData = nullptr;
    for (FVFX_BiomeEffectData& Data : BiomeEffectData)
    {
        if (Data.BiomeType == CurrentBiome)
        {
            BiomeData = &Data;
            break;
        }
    }
    
    if (BiomeData)
    {
        // Update fog settings for biome
        UpdateFogSettings(BiomeData->FogColor, BiomeData->FogDensity);
        
        // Update ambient effects
        if (AmbientEffect)
        {
            AmbientEffect->SetFloatParameter(TEXT("SpawnRate"), BiomeData->AmbientParticleChance * 10.0f);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("VFX_EnvironmentalEffects: Biome changed to %d"), (int32)CurrentBiome);
    }
}

void UVFX_EnvironmentalEffects::SpawnBiomeAmbientEffect(FVector Location)
{
    if (ActiveEffects.Num() >= MaxActiveEffects) return;
    
    // Find current biome data
    FVFX_BiomeEffectData* BiomeData = nullptr;
    for (FVFX_BiomeEffectData& Data : BiomeEffectData)
    {
        if (Data.BiomeType == CurrentBiome)
        {
            BiomeData = &Data;
            break;
        }
    }
    
    if (BiomeData && BiomeData->ParticleEffectNames.Num() > 0)
    {
        // Random chance to spawn ambient effect
        if (FMath::RandRange(0.0f, 1.0f) <= BiomeData->AmbientParticleChance)
        {
            UNiagaraComponent* NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(), 
                nullptr, // Will use default system for now
                Location,
                FRotator::ZeroRotator
            );
            
            if (NewEffect)
            {
                ActiveEffects.Add(NewEffect);
                UE_LOG(LogTemp, Log, TEXT("VFX_EnvironmentalEffects: Spawned ambient effect at %s"), *Location.ToString());
            }
        }
    }
}

void UVFX_EnvironmentalEffects::SpawnDustCloud(FVector Location, float Size)
{
    if (ActiveEffects.Num() >= MaxActiveEffects) return;
    
    UNiagaraComponent* DustCloud = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        nullptr, // Will use default dust system
        Location,
        FRotator::ZeroRotator
    );
    
    if (DustCloud)
    {
        DustCloud->SetFloatParameter(TEXT("Size"), Size);
        DustCloud->SetFloatParameter(TEXT("Lifetime"), 3.0f);
        DustCloud->SetColorParameter(TEXT("DustColor"), FLinearColor(0.8f, 0.7f, 0.5f, 1.0f));
        
        ActiveEffects.Add(DustCloud);
        UE_LOG(LogTemp, Log, TEXT("VFX_EnvironmentalEffects: Spawned dust cloud at %s"), *Location.ToString());
    }
}

void UVFX_EnvironmentalEffects::SpawnWaterSplash(FVector Location, float Intensity)
{
    if (ActiveEffects.Num() >= MaxActiveEffects) return;
    
    UNiagaraComponent* WaterSplash = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        nullptr, // Will use default water system
        Location,
        FRotator::ZeroRotator
    );
    
    if (WaterSplash)
    {
        WaterSplash->SetFloatParameter(TEXT("Intensity"), Intensity);
        WaterSplash->SetFloatParameter(TEXT("Lifetime"), 2.0f);
        WaterSplash->SetColorParameter(TEXT("WaterColor"), FLinearColor(0.2f, 0.5f, 0.8f, 0.8f));
        
        ActiveEffects.Add(WaterSplash);
        UE_LOG(LogTemp, Log, TEXT("VFX_EnvironmentalEffects: Spawned water splash at %s"), *Location.ToString());
    }
}

void UVFX_EnvironmentalEffects::SpawnFireEffect(FVector Location, float Duration)
{
    if (ActiveEffects.Num() >= MaxActiveEffects) return;
    
    UNiagaraComponent* Fire = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        nullptr, // Will use default fire system
        Location,
        FRotator::ZeroRotator
    );
    
    if (Fire)
    {
        Fire->SetFloatParameter(TEXT("Lifetime"), Duration);
        Fire->SetFloatParameter(TEXT("Intensity"), 1.0f);
        Fire->SetColorParameter(TEXT("FireColor"), FLinearColor(1.0f, 0.5f, 0.1f, 1.0f));
        
        ActiveEffects.Add(Fire);
        UE_LOG(LogTemp, Log, TEXT("VFX_EnvironmentalEffects: Spawned fire effect at %s"), *Location.ToString());
    }
}

void UVFX_EnvironmentalEffects::UpdateFogSettings(FLinearColor Color, float Density)
{
    if (FogEffect)
    {
        FogEffect->SetColorParameter(TEXT("FogColor"), Color);
        FogEffect->SetFloatParameter(TEXT("Density"), Density);
    }
}

void UVFX_EnvironmentalEffects::CreateVolumetricLightRays(FVector SunDirection, float Intensity)
{
    // Spawn volumetric light rays effect
    UNiagaraComponent* LightRays = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        nullptr, // Will use default light rays system
        GetOwner()->GetActorLocation() + SunDirection * 1000.0f,
        SunDirection.Rotation()
    );
    
    if (LightRays)
    {
        LightRays->SetFloatParameter(TEXT("Intensity"), Intensity);
        LightRays->SetVectorParameter(TEXT("Direction"), SunDirection);
        ActiveEffects.Add(LightRays);
        UE_LOG(LogTemp, Log, TEXT("VFX_EnvironmentalEffects: Created volumetric light rays"));
    }
}

void UVFX_EnvironmentalEffects::UpdateWeatherEffects()
{
    // Update weather-based effects based on current conditions
    switch (CurrentWeatherType)
    {
        case EVFX_WeatherType::Storm:
            // Increase intensity during storms
            if (RainEffect && RainEffect->IsVisible())
            {
                float StormIntensity = WeatherSettings.Intensity * 1.5f;
                RainEffect->SetFloatParameter(TEXT("Intensity"), StormIntensity);
            }
            break;
            
        case EVFX_WeatherType::Dust:
            // Vary dust intensity based on wind
            if (DustEffect && DustEffect->IsVisible())
            {
                float DustIntensity = WeatherSettings.Intensity * WeatherSettings.WindStrength / 50.0f;
                DustEffect->SetFloatParameter(TEXT("Intensity"), DustIntensity);
            }
            break;
            
        default:
            break;
    }
}

void UVFX_EnvironmentalEffects::UpdateBiomeEffects()
{
    // Randomly spawn biome-specific ambient effects
    if (GetOwner() && FMath::RandRange(0.0f, 1.0f) < 0.1f)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-MaxEffectDistance, MaxEffectDistance),
            FMath::RandRange(-MaxEffectDistance, MaxEffectDistance),
            FMath::RandRange(0.0f, 500.0f)
        );
        
        SpawnBiomeAmbientEffect(GetOwner()->GetActorLocation() + RandomOffset);
    }
}

void UVFX_EnvironmentalEffects::CleanupExpiredEffects()
{
    // Remove null or completed effects
    ActiveEffects.RemoveAll([](UNiagaraComponent* Effect) {
        return !Effect || !IsValid(Effect) || !Effect->IsActive();
    });
}

void UVFX_EnvironmentalEffects::InitializeBiomeData()
{
    BiomeEffectData.Empty();
    
    // Savana biome
    FVFX_BiomeEffectData SavanaData;
    SavanaData.BiomeType = EVFX_BiomeType::Savana;
    SavanaData.AmbientParticleChance = 0.05f;
    SavanaData.FogColor = FLinearColor(0.9f, 0.8f, 0.6f, 0.2f);
    SavanaData.FogDensity = 0.01f;
    SavanaData.ParticleEffectNames.Add(TEXT("Dust"));
    SavanaData.ParticleEffectNames.Add(TEXT("Heat_Shimmer"));
    BiomeEffectData.Add(SavanaData);
    
    // Forest biome
    FVFX_BiomeEffectData ForestData;
    ForestData.BiomeType = EVFX_BiomeType::Forest;
    ForestData.AmbientParticleChance = 0.08f;
    ForestData.FogColor = FLinearColor(0.6f, 0.8f, 0.7f, 0.3f);
    ForestData.FogDensity = 0.03f;
    ForestData.ParticleEffectNames.Add(TEXT("Pollen"));
    ForestData.ParticleEffectNames.Add(TEXT("Falling_Leaves"));
    BiomeEffectData.Add(ForestData);
    
    // Swamp biome
    FVFX_BiomeEffectData SwampData;
    SwampData.BiomeType = EVFX_BiomeType::Swamp;
    SwampData.AmbientParticleChance = 0.12f;
    SwampData.FogColor = FLinearColor(0.5f, 0.7f, 0.5f, 0.4f);
    SwampData.FogDensity = 0.05f;
    SwampData.ParticleEffectNames.Add(TEXT("Mist"));
    SwampData.ParticleEffectNames.Add(TEXT("Fireflies"));
    BiomeEffectData.Add(SwampData);
    
    // Desert biome
    FVFX_BiomeEffectData DesertData;
    DesertData.BiomeType = EVFX_BiomeType::Desert;
    DesertData.AmbientParticleChance = 0.03f;
    DesertData.FogColor = FLinearColor(1.0f, 0.9f, 0.7f, 0.15f);
    DesertData.FogDensity = 0.005f;
    DesertData.ParticleEffectNames.Add(TEXT("Sand"));
    DesertData.ParticleEffectNames.Add(TEXT("Heat_Waves"));
    BiomeEffectData.Add(DesertData);
    
    // Mountain biome
    FVFX_BiomeEffectData MountainData;
    MountainData.BiomeType = EVFX_BiomeType::Mountain;
    MountainData.AmbientParticleChance = 0.06f;
    MountainData.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 0.25f);
    MountainData.FogDensity = 0.02f;
    MountainData.ParticleEffectNames.Add(TEXT("Snow"));
    MountainData.ParticleEffectNames.Add(TEXT("Rock_Dust"));
    BiomeEffectData.Add(MountainData);
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_EnvironmentalEffects: Initialized %d biome effect configurations"), BiomeEffectData.Num());
}