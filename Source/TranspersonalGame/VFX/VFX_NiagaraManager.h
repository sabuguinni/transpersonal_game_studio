#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "SharedTypes.h"
#include "VFX_NiagaraManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector DefaultScale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float DefaultLifetime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    FVFX_EffectData()
    {
        NiagaraSystem = nullptr;
        DefaultScale = FVector(1.0f, 1.0f, 1.0f);
        DefaultLifetime = 5.0f;
        bAutoDestroy = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_BiomeEffects
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVFX_EffectData AmbientParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVFX_EffectData WeatherEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVFX_EffectData GroundImpact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float EffectIntensity = 1.0f;

    FVFX_BiomeEffects()
    {
        EffectIntensity = 1.0f;
    }
};

/**
 * VFX Niagara Manager - Sistema central de gestão de efeitos visuais
 * Gere todos os efeitos de partículas Niagara do jogo pré-histórico
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_NiagaraManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UVFX_NiagaraManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Spawn de efeitos básicos
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffectAtLocation(const FString& EffectName, FVector Location, FRotator Rotation = FRotator::ZeroRotator, FVector Scale = FVector(1.0f));

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffectAttached(const FString& EffectName, USceneComponent* AttachComponent, FName AttachPointName = NAME_None);

    // Efeitos específicos de dinossauros
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayFootstepEffect(FVector Location, EBiomeType BiomeType, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayBloodEffect(FVector Location, FVector ImpactDirection, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayBreathEffect(FVector Location, FRotator Direction, float Temperature = 20.0f);

    // Efeitos ambientais
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StartBiomeAmbientEffects(EBiomeType BiomeType, FVector CenterLocation, float Radius = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopBiomeAmbientEffects(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void UpdateWeatherEffects(EWeatherType WeatherType, float Intensity = 1.0f);

    // Efeitos de combate
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayWeaponImpactEffect(FVector Location, FVector ImpactDirection, EWeaponType WeaponType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayExplosionEffect(FVector Location, float Radius = 500.0f, float Intensity = 1.0f);

    // Gestão de performance
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetEffectQuality(int32 QualityLevel); // 0=Low, 1=Medium, 2=High, 3=Ultra

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetMaxActiveEffects(int32 MaxEffects);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveEffectCount() const;

protected:
    // Configuração de efeitos por bioma
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TMap<EBiomeType, FVFX_BiomeEffects> BiomeEffectConfigs;

    // Efeitos base registados
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TMap<FString, FVFX_EffectData> RegisteredEffects;

    // Componentes activos
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    // Configurações de performance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 CurrentQualityLevel = 2; // Medium by default

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float EffectCullingDistance = 10000.0f;

private:
    // Métodos internos
    void InitializeEffectDatabase();
    void RegisterDefaultEffects();
    void CleanupExpiredEffects();
    UNiagaraSystem* LoadNiagaraAsset(const FString& AssetPath);
    FString GetEffectPathForBiome(EBiomeType BiomeType, const FString& EffectType);

    // Timer para limpeza
    FTimerHandle CleanupTimerHandle;
};