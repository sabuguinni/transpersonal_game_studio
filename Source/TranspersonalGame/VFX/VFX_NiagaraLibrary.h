#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VFX_NiagaraLibrary.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire       UMETA(DisplayName = "Campfire"),
    Fire_Torch          UMETA(DisplayName = "Torch"),
    Impact_Dust         UMETA(DisplayName = "Dust Impact"),
    Impact_Blood        UMETA(DisplayName = "Blood Impact"),
    Impact_Rock         UMETA(DisplayName = "Rock Impact"),
    Weather_Rain        UMETA(DisplayName = "Rain"),
    Weather_Snow        UMETA(DisplayName = "Snow"),
    Weather_Fog         UMETA(DisplayName = "Fog"),
    Dino_Footstep       UMETA(DisplayName = "Dinosaur Footstep"),
    Dino_Breath         UMETA(DisplayName = "Dinosaur Breath"),
    Water_Splash        UMETA(DisplayName = "Water Splash"),
    Water_Ripple        UMETA(DisplayName = "Water Ripple")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::Fire_Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::Fire_Campfire;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        Duration = 5.0f;
        bAutoDestroy = true;
    }
};

/**
 * VFX Niagara Library - Blueprint function library for spawning and managing Niagara particle effects
 * Provides easy access to prehistoric-themed VFX systems for the survival game
 */
UCLASS()
class TRANSPERSONALGAME_API UVFX_NiagaraLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // Spawn a Niagara effect at a specific location
    UFUNCTION(BlueprintCallable, Category = "VFX|Niagara", CallInEditor = true)
    static class UNiagaraComponent* SpawnNiagaraEffectAtLocation(
        const UObject* WorldContext,
        EVFX_EffectType EffectType,
        FVector Location,
        FRotator Rotation = FRotator::ZeroRotator,
        FVector Scale = FVector(1.0f, 1.0f, 1.0f),
        bool bAutoDestroy = true
    );

    // Spawn a Niagara effect attached to a component
    UFUNCTION(BlueprintCallable, Category = "VFX|Niagara", CallInEditor = true)
    static class UNiagaraComponent* SpawnNiagaraEffectAttached(
        EVFX_EffectType EffectType,
        class USceneComponent* AttachToComponent,
        FName AttachPointName = NAME_None,
        FVector Location = FVector::ZeroVector,
        FRotator Rotation = FRotator::ZeroRotator,
        FVector Scale = FVector(1.0f, 1.0f, 1.0f),
        bool bAutoDestroy = true
    );

    // Get Niagara system asset for a specific effect type
    UFUNCTION(BlueprintCallable, Category = "VFX|Niagara", CallInEditor = true)
    static class UNiagaraSystem* GetNiagaraSystemForEffect(EVFX_EffectType EffectType);

    // Create campfire effect with customizable parameters
    UFUNCTION(BlueprintCallable, Category = "VFX|Fire", CallInEditor = true)
    static class UNiagaraComponent* CreateCampfireEffect(
        const UObject* WorldContext,
        FVector Location,
        float FireIntensity = 1.0f,
        float SmokeAmount = 1.0f
    );

    // Create dinosaur footstep dust effect
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur", CallInEditor = true)
    static class UNiagaraComponent* CreateFootstepDustEffect(
        const UObject* WorldContext,
        FVector Location,
        float DinosaurSize = 1.0f,
        FVector GroundNormal = FVector(0.0f, 0.0f, 1.0f)
    );

    // Create weather rain effect
    UFUNCTION(BlueprintCallable, Category = "VFX|Weather", CallInEditor = true)
    static class UNiagaraComponent* CreateRainEffect(
        const UObject* WorldContext,
        FVector Location,
        float RainIntensity = 1.0f,
        FVector RainDirection = FVector(0.0f, 0.0f, -1.0f)
    );

    // Create blood impact effect for combat
    UFUNCTION(BlueprintCallable, Category = "VFX|Combat", CallInEditor = true)
    static class UNiagaraComponent* CreateBloodImpactEffect(
        const UObject* WorldContext,
        FVector Location,
        FVector ImpactNormal = FVector(0.0f, 0.0f, 1.0f),
        float BloodAmount = 1.0f
    );

    // Validate and initialize VFX library
    UFUNCTION(BlueprintCallable, Category = "VFX|System", CallInEditor = true)
    static bool InitializeVFXLibrary();

private:
    // Static map of effect types to Niagara system paths
    static TMap<EVFX_EffectType, FString> EffectSystemPaths;

    // Initialize the effect system paths
    static void InitializeEffectPaths();

    // Helper function to load Niagara system from path
    static class UNiagaraSystem* LoadNiagaraSystemFromPath(const FString& SystemPath);
};