#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "VFXManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    DinosaurFootstep    UMETA(DisplayName = "Dinosaur Footstep"),
    CampfireFlames      UMETA(DisplayName = "Campfire Flames"),
    WeatherRain         UMETA(DisplayName = "Weather Rain"),
    BloodSplatter       UMETA(DisplayName = "Blood Splatter"),
    DustCloud           UMETA(DisplayName = "Dust Cloud"),
    Sparks              UMETA(DisplayName = "Sparks"),
    Smoke               UMETA(DisplayName = "Smoke"),
    WindParticles       UMETA(DisplayName = "Wind Particles")
};

USTRUCT(BlueprintType)
struct FVFX_EffectConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::DinosaurFootstep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FLinearColor Color = FLinearColor::White;

    FVFX_EffectConfig()
    {
        EffectType = EVFX_EffectType::DinosaurFootstep;
        Duration = 2.0f;
        Intensity = 1.0f;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        Color = FLinearColor::White;
    }
};

USTRUCT(BlueprintType)
struct FVFX_BiomeEffects
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TArray<FVFX_EffectConfig> AmbientEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float WindIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FLinearColor DustColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    FVFX_BiomeEffects()
    {
        BiomeType = EBiomeType::Savana;
        WindIntensity = 0.5f;
        DustColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_Manager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_Manager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TArray<FVFX_BiomeEffects> BiomeEffectsConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TMap<EVFX_EffectType, class UNiagaraSystem*> EffectSystems;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    TArray<class UNiagaraComponent*> ActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float EffectUpdateInterval = 0.1f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    float LastUpdateTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    EBiomeType CurrentBiome = EBiomeType::Savana;

public:
    // VFX Management Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, FVFX_EffectConfig Config = FVFX_EffectConfig());

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDinosaurFootstep(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfire(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodSplatter(FVector Location, FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void UpdateBiomeEffects(EBiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    EBiomeType DetectBiomeFromLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void InitializeEffectSystems();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetGlobalVFXQuality(int32 QualityLevel);

    // Biome-specific VFX
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnSwampMist(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDesertDust(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnSnowfall(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnForestParticles(FVector Location);

private:
    void UpdateActiveEffects(float DeltaTime);
    void LoadEffectSystems();
    FVFX_BiomeEffects* GetBiomeConfig(EBiomeType BiomeType);
};