#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "VFX_NiagaraLibrary.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    CampfireFire        UMETA(DisplayName = "Campfire Fire"),
    CombatBlood         UMETA(DisplayName = "Combat Blood"),
    CombatDust          UMETA(DisplayName = "Combat Dust"),
    DinosaurFootstep    UMETA(DisplayName = "Dinosaur Footstep"),
    WeatherRain         UMETA(DisplayName = "Weather Rain"),
    WeatherFog          UMETA(DisplayName = "Weather Fog"),
    ImpactSparks        UMETA(DisplayName = "Impact Sparks"),
    BreathingVapor      UMETA(DisplayName = "Breathing Vapor")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoActivate;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::CampfireFire;
        Duration = 5.0f;
        Intensity = 1.0f;
        bAutoActivate = false;
    }
};

/**
 * VFX Niagara Library - Central management for all prehistoric survival VFX
 * Handles campfire effects, combat impacts, dinosaur interactions, and weather
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_NiagaraLibrary : public UObject
{
    GENERATED_BODY()

public:
    UVFX_NiagaraLibrary();

    // Core VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    static UNiagaraComponent* SpawnVFXAtLocation(UWorld* World, EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    static UNiagaraComponent* AttachVFXToActor(AActor* Actor, EVFX_EffectType EffectType, FName SocketName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    static void StopVFXEffect(UNiagaraComponent* NiagaraComponent);

    // Campfire VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    static UNiagaraComponent* CreateCampfireVFX(UWorld* World, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    static void UpdateCampfireIntensity(UNiagaraComponent* CampfireVFX, float Intensity);

    // Combat VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    static UNiagaraComponent* SpawnBloodImpact(UWorld* World, FVector Location, FVector ImpactNormal);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    static UNiagaraComponent* SpawnDustCloud(UWorld* World, FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    static UNiagaraComponent* SpawnImpactSparks(UWorld* World, FVector Location, FVector Direction);

    // Dinosaur VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    static UNiagaraComponent* SpawnFootstepDust(UWorld* World, FVector Location, float DinosaurSize);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    static UNiagaraComponent* SpawnBreathingVapor(AActor* DinosaurActor, FName MouthSocket);

    // Weather VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    static UNiagaraComponent* CreateRainSystem(UWorld* World, FVector Location, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    static UNiagaraComponent* CreateFogSystem(UWorld* World, FVector Location, float Density);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    static void UpdateWeatherIntensity(UNiagaraComponent* WeatherVFX, float NewIntensity);

    // VFX Library Management
    UFUNCTION(BlueprintCallable, Category = "VFX|Library")
    static FVFX_EffectData GetEffectData(EVFX_EffectType EffectType);

    UFUNCTION(BlueprintCallable, Category = "VFX|Library")
    static TArray<FVFX_EffectData> GetAllEffectData();

protected:
    // Static effect library
    static TMap<EVFX_EffectType, FVFX_EffectData> EffectLibrary;

    // Initialize effect library
    static void InitializeEffectLibrary();

    // Helper functions
    static UNiagaraSystem* LoadNiagaraSystem(const FString& AssetPath);
    static void SetNiagaraParameter(UNiagaraComponent* Component, const FString& ParameterName, float Value);
    static void SetNiagaraVectorParameter(UNiagaraComponent* Component, const FString& ParameterName, FVector Value);
};