#include "VFXManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UVFX_Manager::UVFX_Manager()
{
    PrimaryComponentTick.bCanEverTick = true;
    GlobalLODScalar = 1.0f;
}

void UVFX_Manager::BeginPlay()
{
    Super::BeginPlay();

    // Register default prehistoric VFX effects
    FVFX_EffectConfig CampfireConfig;
    CampfireConfig.EffectID = FName("NS_Fire_Campfire");
    CampfireConfig.Category = EVFX_EffectCategory::Environment;
    CampfireConfig.LOD0_Distance = 600.f;
    CampfireConfig.LOD1_Distance = 2000.f;
    CampfireConfig.LOD2_Distance = 5000.f;
    CampfireConfig.bCastShadows = true;
    RegisterEffect(CampfireConfig);

    FVFX_EffectConfig FootstepConfig;
    FootstepConfig.EffectID = FName("NS_Dino_Footstep");
    FootstepConfig.Category = EVFX_EffectCategory::Dinosaur;
    FootstepConfig.LOD0_Distance = 400.f;
    FootstepConfig.LOD1_Distance = 1200.f;
    FootstepConfig.LOD2_Distance = 3000.f;
    FootstepConfig.bCastShadows = false;
    RegisterEffect(FootstepConfig);

    FVFX_EffectConfig RainConfig;
    RainConfig.EffectID = FName("NS_Weather_Rain");
    RainConfig.Category = EVFX_EffectCategory::Weather;
    RainConfig.LOD0_Distance = 1500.f;
    RainConfig.LOD1_Distance = 4000.f;
    RainConfig.LOD2_Distance = 8000.f;
    RainConfig.bCastShadows = false;
    RegisterEffect(RainConfig);

    FVFX_EffectConfig BloodConfig;
    BloodConfig.EffectID = FName("NS_Combat_BloodImpact");
    BloodConfig.Category = EVFX_EffectCategory::Combat;
    BloodConfig.LOD0_Distance = 300.f;
    BloodConfig.LOD1_Distance = 800.f;
    BloodConfig.LOD2_Distance = 1500.f;
    BloodConfig.bCastShadows = false;
    RegisterEffect(BloodConfig);

    FVFX_EffectConfig DustConfig;
    DustConfig.EffectID = FName("NS_Dino_DustCloud");
    DustConfig.Category = EVFX_EffectCategory::Dinosaur;
    DustConfig.LOD0_Distance = 800.f;
    DustConfig.LOD1_Distance = 2500.f;
    DustConfig.LOD2_Distance = 6000.f;
    DustConfig.bCastShadows = false;
    RegisterEffect(DustConfig);
}

void UVFX_Manager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    // LOD updates are driven by player position via UpdateLOD()
}

void UVFX_Manager::RegisterEffect(const FVFX_EffectConfig& Config)
{
    if (Config.EffectID == NAME_None)
    {
        return;
    }
    EffectRegistry.Add(Config.EffectID, Config);
    RegisteredEffects.AddUnique(Config);
}

void UVFX_Manager::SpawnEffect(FName EffectID, FVector Location, FRotator Rotation)
{
    if (!EffectRegistry.Contains(EffectID))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXManager: Effect '%s' not registered."), *EffectID.ToString());
        return;
    }
    // Niagara spawn is handled via Blueprint or NiagaraFunctionLibrary
    // This stub logs the intent for integration with Blueprint layer
    UE_LOG(LogTemp, Log, TEXT("VFXManager: SpawnEffect '%s' at %s"), *EffectID.ToString(), *Location.ToString());
}

void UVFX_Manager::StopEffect(FName EffectID)
{
    if (!EffectRegistry.Contains(EffectID))
    {
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("VFXManager: StopEffect '%s'"), *EffectID.ToString());
}

void UVFX_Manager::UpdateLOD(FVector PlayerLocation)
{
    // Iterate registered effects and adjust LOD based on distance
    // Actual LOD switching is delegated to Niagara scalability settings
    for (const auto& Pair : EffectRegistry)
    {
        const FVFX_EffectConfig& Config = Pair.Value;
        float ScaledLOD0 = Config.LOD0_Distance * GlobalLODScalar;
        float ScaledLOD1 = Config.LOD1_Distance * GlobalLODScalar;
        float ScaledLOD2 = Config.LOD2_Distance * GlobalLODScalar;
        // LOD distances are exposed for Niagara system to consume via component parameters
        (void)ScaledLOD0;
        (void)ScaledLOD1;
        (void)ScaledLOD2;
    }
}
