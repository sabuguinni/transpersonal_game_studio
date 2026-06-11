#include "VFX_NiagaraLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UVFX_NiagaraLibrary::UVFX_NiagaraLibrary()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick every 100ms for performance
    
    MaxActiveEffects = 50;
    EffectCullDistance = 5000.0f;
    bEnableLODSystem = true;
}

void UVFX_NiagaraLibrary::BeginPlay()
{
    Super::BeginPlay();
    InitializeEffectLibrary();
}

void UVFX_NiagaraLibrary::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    CleanupExpiredEffects();
}

UNiagaraComponent* UVFX_NiagaraLibrary::SpawnVFXEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: No valid world for spawning effect"));
        return nullptr;
    }

    // Check active effect limit
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: Maximum active effects reached (%d)"), MaxActiveEffects);
        return nullptr;
    }

    UNiagaraSystem* EffectAsset = GetEffectAsset(EffectType);
    if (!EffectAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: No asset found for effect type %d"), (int32)EffectType);
        return nullptr;
    }

    // Spawn Niagara effect
    UNiagaraComponent* EffectComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        EffectAsset,
        Location,
        Rotation,
        FVector::OneVector,
        true,
        true,
        ENCPoolMethod::None,
        true
    );

    if (EffectComponent)
    {
        ActiveEffects.Add(EffectComponent);
        
        // Apply effect-specific settings
        if (EffectLibrary.Contains(EffectType))
        {
            const FVFX_EffectData& EffectData = EffectLibrary[EffectType];
            SetEffectIntensity(EffectComponent, EffectData.Intensity);
            SetEffectScale(EffectComponent, EffectData.Scale);
        }

        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Spawned effect type %d at location %s"), 
               (int32)EffectType, *Location.ToString());
    }

    return EffectComponent;
}

void UVFX_NiagaraLibrary::StopVFXEffect(UNiagaraComponent* EffectComponent)
{
    if (EffectComponent && IsValid(EffectComponent))
    {
        EffectComponent->Deactivate();
        ActiveEffects.Remove(EffectComponent);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Stopped VFX effect"));
    }
}

void UVFX_NiagaraLibrary::SetEffectIntensity(UNiagaraComponent* EffectComponent, float Intensity)
{
    if (EffectComponent && IsValid(EffectComponent))
    {
        EffectComponent->SetFloatParameter(FName("Intensity"), FMath::Clamp(Intensity, 0.0f, 2.0f));
    }
}

void UVFX_NiagaraLibrary::SetEffectScale(UNiagaraComponent* EffectComponent, FVector Scale)
{
    if (EffectComponent && IsValid(EffectComponent))
    {
        EffectComponent->SetVectorParameter(FName("Scale"), Scale);
    }
}

UNiagaraComponent* UVFX_NiagaraLibrary::CreateCampfireEffect(FVector Location)
{
    UNiagaraComponent* FireEffect = SpawnVFXEffect(EVFX_EffectType::Fire_Campfire, Location);
    if (FireEffect)
    {
        // Set campfire-specific parameters
        FireEffect->SetFloatParameter(FName("FireSize"), 1.5f);
        FireEffect->SetFloatParameter(FName("SmokeAmount"), 0.8f);
        FireEffect->SetFloatParameter(FName("SparkCount"), 20.0f);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Created campfire effect at %s"), *Location.ToString());
    }
    return FireEffect;
}

UNiagaraComponent* UVFX_NiagaraLibrary::CreateWeatherEffect(EVFX_EffectType WeatherType, FVector Location, float Coverage)
{
    UNiagaraComponent* WeatherEffect = SpawnVFXEffect(WeatherType, Location);
    if (WeatherEffect)
    {
        WeatherEffect->SetFloatParameter(FName("Coverage"), FMath::Clamp(Coverage, 0.0f, 1.0f));
        
        switch (WeatherType)
        {
        case EVFX_EffectType::Weather_Rain:
            WeatherEffect->SetFloatParameter(FName("RainIntensity"), Coverage);
            break;
        case EVFX_EffectType::Weather_Snow:
            WeatherEffect->SetFloatParameter(FName("SnowDensity"), Coverage);
            break;
        case EVFX_EffectType::Weather_Fog:
            WeatherEffect->SetFloatParameter(FName("FogThickness"), Coverage);
            break;
        default:
            break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Created weather effect type %d"), (int32)WeatherType);
    }
    return WeatherEffect;
}

UNiagaraComponent* UVFX_NiagaraLibrary::CreateWaterEffect(FVector Location, float FlowRate)
{
    UNiagaraComponent* WaterEffect = SpawnVFXEffect(EVFX_EffectType::Water_Splash, Location);
    if (WaterEffect)
    {
        WaterEffect->SetFloatParameter(FName("FlowRate"), FMath::Clamp(FlowRate, 0.1f, 5.0f));
        WaterEffect->SetFloatParameter(FName("SplashSize"), FlowRate * 0.5f);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Created water effect with flow rate %f"), FlowRate);
    }
    return WaterEffect;
}

UNiagaraComponent* UVFX_NiagaraLibrary::CreateFootstepDust(FVector Location, float DinosaurSize)
{
    UNiagaraComponent* DustEffect = SpawnVFXEffect(EVFX_EffectType::Dust_Footstep, Location);
    if (DustEffect)
    {
        float DustScale = FMath::Clamp(DinosaurSize, 0.5f, 3.0f);
        DustEffect->SetFloatParameter(FName("DustAmount"), DustScale * 100.0f);
        DustEffect->SetFloatParameter(FName("ParticleSize"), DustScale);
        DustEffect->SetVectorParameter(FName("Scale"), FVector(DustScale));
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Created footstep dust for size %f dinosaur"), DinosaurSize);
    }
    return DustEffect;
}

UNiagaraComponent* UVFX_NiagaraLibrary::CreateBreathVapor(FVector Location, float Temperature)
{
    UNiagaraComponent* VaporEffect = SpawnVFXEffect(EVFX_EffectType::Breath_Vapor, Location);
    if (VaporEffect)
    {
        float VaporIntensity = 1.0f - FMath::Clamp(Temperature, 0.0f, 1.0f);
        VaporEffect->SetFloatParameter(FName("VaporDensity"), VaporIntensity);
        VaporEffect->SetFloatParameter(FName("LifeSpan"), 2.0f + VaporIntensity * 3.0f);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Created breath vapor with intensity %f"), VaporIntensity);
    }
    return VaporEffect;
}

UNiagaraComponent* UVFX_NiagaraLibrary::CreateBloodEffect(FVector Location, EVFX_EffectType BloodType)
{
    UNiagaraComponent* BloodEffect = SpawnVFXEffect(BloodType, Location);
    if (BloodEffect)
    {
        switch (BloodType)
        {
        case EVFX_EffectType::Blood_Splatter:
            BloodEffect->SetFloatParameter(FName("SplatterRadius"), 150.0f);
            BloodEffect->SetFloatParameter(FName("BloodAmount"), 1.0f);
            break;
        case EVFX_EffectType::Blood_Drip:
            BloodEffect->SetFloatParameter(FName("DripRate"), 0.5f);
            BloodEffect->SetFloatParameter(FName("DripSize"), 1.0f);
            break;
        default:
            break;
        }
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Created blood effect type %d"), (int32)BloodType);
    }
    return BloodEffect;
}

UNiagaraComponent* UVFX_NiagaraLibrary::CreateCraftingSparks(FVector Location)
{
    UNiagaraComponent* SparkEffect = SpawnVFXEffect(EVFX_EffectType::Sparks_Tool, Location);
    if (SparkEffect)
    {
        SparkEffect->SetFloatParameter(FName("SparkCount"), 50.0f);
        SparkEffect->SetFloatParameter(FName("SparkLifetime"), 1.5f);
        SparkEffect->SetFloatParameter(FName("SparkSpeed"), 300.0f);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Created crafting sparks effect"));
    }
    return SparkEffect;
}

UNiagaraComponent* UVFX_NiagaraLibrary::CreateImpactEffect(FVector Location, EVFX_EffectType ImpactType)
{
    UNiagaraComponent* ImpactEffect = SpawnVFXEffect(ImpactType, Location);
    if (ImpactEffect)
    {
        ImpactEffect->SetFloatParameter(FName("ImpactForce"), 1.0f);
        ImpactEffect->SetFloatParameter(FName("DebrisCount"), 25.0f);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Created impact effect type %d"), (int32)ImpactType);
    }
    return ImpactEffect;
}

void UVFX_NiagaraLibrary::InitializeEffectLibrary()
{
    // Initialize Fire Effects
    FVFX_EffectData CampfireData;
    CampfireData.EffectType = EVFX_EffectType::Fire_Campfire;
    CampfireData.Duration = -1.0f; // Infinite
    CampfireData.Intensity = 1.0f;
    CampfireData.Scale = FVector(1.5f, 1.5f, 2.0f);
    EffectLibrary.Add(EVFX_EffectType::Fire_Campfire, CampfireData);

    // Initialize Dust Effects
    FVFX_EffectData FootstepData;
    FootstepData.EffectType = EVFX_EffectType::Dust_Footstep;
    FootstepData.Duration = 2.0f;
    FootstepData.Intensity = 0.8f;
    FootstepData.Scale = FVector(1.0f, 1.0f, 0.5f);
    EffectLibrary.Add(EVFX_EffectType::Dust_Footstep, FootstepData);

    // Initialize Weather Effects
    FVFX_EffectData RainData;
    RainData.EffectType = EVFX_EffectType::Weather_Rain;
    RainData.Duration = -1.0f; // Infinite
    RainData.Intensity = 1.0f;
    RainData.Scale = FVector(10.0f, 10.0f, 5.0f);
    EffectLibrary.Add(EVFX_EffectType::Weather_Rain, RainData);

    // Initialize Blood Effects
    FVFX_EffectData BloodData;
    BloodData.EffectType = EVFX_EffectType::Blood_Splatter;
    BloodData.Duration = 3.0f;
    BloodData.Intensity = 1.0f;
    BloodData.Scale = FVector(1.0f, 1.0f, 1.0f);
    EffectLibrary.Add(EVFX_EffectType::Blood_Splatter, BloodData);

    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Initialized effect library with %d effects"), EffectLibrary.Num());
}

void UVFX_NiagaraLibrary::CleanupExpiredEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (!IsValid(Effect) || !Effect->IsActive())
        {
            ActiveEffects.RemoveAt(i);
        }
        else if (bEnableLODSystem)
        {
            // Apply LOD based on distance to player
            APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
            if (Player)
            {
                float Distance = FVector::Dist(Effect->GetComponentLocation(), Player->GetActorLocation());
                ApplyLODSettings(Effect, Distance);
            }
        }
    }
}

UNiagaraSystem* UVFX_NiagaraLibrary::GetEffectAsset(EVFX_EffectType EffectType)
{
    // In a real implementation, this would load actual Niagara assets
    // For now, return nullptr and log the request
    UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: Asset loading not implemented for effect type %d"), (int32)EffectType);
    return nullptr;
}

void UVFX_NiagaraLibrary::ApplyLODSettings(UNiagaraComponent* EffectComponent, float Distance)
{
    if (!EffectComponent || !IsValid(EffectComponent))
    {
        return;
    }

    // Cull effects beyond maximum distance
    if (Distance > EffectCullDistance)
    {
        EffectComponent->SetVisibility(false);
        return;
    }

    EffectComponent->SetVisibility(true);

    // Apply LOD scaling based on distance
    float LODScale = 1.0f;
    if (Distance > 1000.0f)
    {
        LODScale = FMath::Clamp(1.0f - (Distance - 1000.0f) / 4000.0f, 0.3f, 1.0f);
    }

    EffectComponent->SetFloatParameter(FName("LODScale"), LODScale);
}