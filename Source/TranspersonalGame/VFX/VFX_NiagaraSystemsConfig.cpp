#include "VFX_NiagaraSystemsConfig.h"

UVFX_NiagaraSystemsConfig::UVFX_NiagaraSystemsConfig()
{
    InitDefaultEffects();
}

void UVFX_NiagaraSystemsConfig::InitDefaultEffects()
{
    RegisteredEffects.Empty();

    // --- CATEGORY 1: ENVIRONMENT ---

    // Campfire fire + embers
    {
        FVFX_NiagaraEntry E;
        E.SystemName = FName("NS_Fire_Campfire");
        E.AssetPath = FSoftObjectPath("/Game/VFX/Niagara/NS_Fire_Campfire.NS_Fire_Campfire");
        E.Category = EVFX_EffectCategory::Environment;
        E.MaxInstances = 4;
        E.CullDistanceMeters = 80.0f;
        E.bLooping = true;
        RegisteredEffects.Add(E);
    }

    // Campfire smoke column
    {
        FVFX_NiagaraEntry E;
        E.SystemName = FName("NS_Smoke_Campfire");
        E.AssetPath = FSoftObjectPath("/Game/VFX/Niagara/NS_Smoke_Campfire.NS_Smoke_Campfire");
        E.Category = EVFX_EffectCategory::Environment;
        E.MaxInstances = 4;
        E.CullDistanceMeters = 200.0f;
        E.bLooping = true;
        RegisteredEffects.Add(E);
    }

    // Waterfall spray mist
    {
        FVFX_NiagaraEntry E;
        E.SystemName = FName("NS_Water_WaterfallSpray");
        E.AssetPath = FSoftObjectPath("/Game/VFX/Niagara/NS_Water_WaterfallSpray.NS_Water_WaterfallSpray");
        E.Category = EVFX_EffectCategory::Environment;
        E.MaxInstances = 3;
        E.CullDistanceMeters = 120.0f;
        E.bLooping = true;
        RegisteredEffects.Add(E);
    }

    // Ambient insects/pollen particles
    {
        FVFX_NiagaraEntry E;
        E.SystemName = FName("NS_Ambient_InsectsPollen");
        E.AssetPath = FSoftObjectPath("/Game/VFX/Niagara/NS_Ambient_InsectsPollen.NS_Ambient_InsectsPollen");
        E.Category = EVFX_EffectCategory::WorldAmbient;
        E.MaxInstances = 8;
        E.CullDistanceMeters = 30.0f;
        E.bLooping = true;
        RegisteredEffects.Add(E);
    }

    // Volcanic ash fall (distant)
    {
        FVFX_NiagaraEntry E;
        E.SystemName = FName("NS_Volcanic_AshFall");
        E.AssetPath = FSoftObjectPath("/Game/VFX/Niagara/NS_Volcanic_AshFall.NS_Volcanic_AshFall");
        E.Category = EVFX_EffectCategory::WorldAmbient;
        E.MaxInstances = 2;
        E.CullDistanceMeters = 500.0f;
        E.bLooping = true;
        RegisteredEffects.Add(E);
    }

    // --- CATEGORY 2: DINOSAUR IMPACTS ---

    // Footstep dust cloud (large dino)
    {
        FVFX_NiagaraEntry E;
        E.SystemName = FName("NS_Dino_FootstepDust");
        E.AssetPath = FSoftObjectPath("/Game/VFX/Niagara/NS_Dino_FootstepDust.NS_Dino_FootstepDust");
        E.Category = EVFX_EffectCategory::DinosaurImpact;
        E.MaxInstances = 16;
        E.CullDistanceMeters = 60.0f;
        E.bLooping = false;
        RegisteredEffects.Add(E);
    }

    // Breath vapor (cold environment)
    {
        FVFX_NiagaraEntry E;
        E.SystemName = FName("NS_Dino_BreathVapor");
        E.AssetPath = FSoftObjectPath("/Game/VFX/Niagara/NS_Dino_BreathVapor.NS_Dino_BreathVapor");
        E.Category = EVFX_EffectCategory::DinosaurImpact;
        E.MaxInstances = 8;
        E.CullDistanceMeters = 40.0f;
        E.bLooping = true;
        RegisteredEffects.Add(E);
    }

    // Roar air distortion (heat shimmer)
    {
        FVFX_NiagaraEntry E;
        E.SystemName = FName("NS_Dino_RoarDistortion");
        E.AssetPath = FSoftObjectPath("/Game/VFX/Niagara/NS_Dino_RoarDistortion.NS_Dino_RoarDistortion");
        E.Category = EVFX_EffectCategory::DinosaurImpact;
        E.MaxInstances = 4;
        E.CullDistanceMeters = 50.0f;
        E.bLooping = false;
        RegisteredEffects.Add(E);
    }

    // Blood splatter on hit
    {
        FVFX_NiagaraEntry E;
        E.SystemName = FName("NS_Dino_BloodImpact");
        E.AssetPath = FSoftObjectPath("/Game/VFX/Niagara/NS_Dino_BloodImpact.NS_Dino_BloodImpact");
        E.Category = EVFX_EffectCategory::DinosaurImpact;
        E.MaxInstances = 12;
        E.CullDistanceMeters = 30.0f;
        E.bLooping = false;
        RegisteredEffects.Add(E);
    }

    // --- CATEGORY 3: WEATHER ---

    // Rain particles
    {
        FVFX_NiagaraEntry E;
        E.SystemName = FName("NS_Weather_Rain");
        E.AssetPath = FSoftObjectPath("/Game/VFX/Niagara/NS_Weather_Rain.NS_Weather_Rain");
        E.Category = EVFX_EffectCategory::Weather;
        E.MaxInstances = 2;
        E.CullDistanceMeters = 1000.0f;
        E.bLooping = true;
        RegisteredEffects.Add(E);
    }

    // Ground splash (rain hitting mud)
    {
        FVFX_NiagaraEntry E;
        E.SystemName = FName("NS_Weather_RainSplash");
        E.AssetPath = FSoftObjectPath("/Game/VFX/Niagara/NS_Weather_RainSplash.NS_Weather_RainSplash");
        E.Category = EVFX_EffectCategory::Weather;
        E.MaxInstances = 4;
        E.CullDistanceMeters = 20.0f;
        E.bLooping = true;
        RegisteredEffects.Add(E);
    }

    // --- CATEGORY 4: COMBAT (PLAYER) ---

    // Spear impact on ground
    {
        FVFX_NiagaraEntry E;
        E.SystemName = FName("NS_Combat_SpearImpact");
        E.AssetPath = FSoftObjectPath("/Game/VFX/Niagara/NS_Combat_SpearImpact.NS_Combat_SpearImpact");
        E.Category = EVFX_EffectCategory::CombatPlayer;
        E.MaxInstances = 8;
        E.CullDistanceMeters = 40.0f;
        E.bLooping = false;
        RegisteredEffects.Add(E);
    }

    // Stone knapping sparks (crafting)
    {
        FVFX_NiagaraEntry E;
        E.SystemName = FName("NS_Crafting_StoneSparks");
        E.AssetPath = FSoftObjectPath("/Game/VFX/Niagara/NS_Crafting_StoneSparks.NS_Crafting_StoneSparks");
        E.Category = EVFX_EffectCategory::CombatPlayer;
        E.MaxInstances = 6;
        E.CullDistanceMeters = 15.0f;
        E.bLooping = false;
        RegisteredEffects.Add(E);
    }
}

bool UVFX_NiagaraSystemsConfig::GetEffectEntry(FName SystemName, FVFX_NiagaraEntry& OutEntry) const
{
    for (const FVFX_NiagaraEntry& Entry : RegisteredEffects)
    {
        if (Entry.SystemName == SystemName)
        {
            OutEntry = Entry;
            return true;
        }
    }
    return false;
}

void UVFX_NiagaraSystemsConfig::RegisterEffect(const FVFX_NiagaraEntry& Entry)
{
    // Remove existing entry with same name if present
    RegisteredEffects.RemoveAll([&Entry](const FVFX_NiagaraEntry& E)
    {
        return E.SystemName == Entry.SystemName;
    });
    RegisteredEffects.Add(Entry);
}

TArray<FVFX_NiagaraEntry> UVFX_NiagaraSystemsConfig::GetEffectsByCategory(EVFX_EffectCategory Category) const
{
    TArray<FVFX_NiagaraEntry> Result;
    for (const FVFX_NiagaraEntry& Entry : RegisteredEffects)
    {
        if (Entry.Category == Category)
        {
            Result.Add(Entry);
        }
    }
    return Result;
}

void UVFX_NiagaraSystemsConfig::SetWeatherState(const FVFX_WeatherState& NewWeather)
{
    CurrentWeather = NewWeather;
}
