// VFXNiagaraManager.h — Agent #17 VFX Agent
// Manages Niagara particle systems for prehistoric survival game VFX
// Systems: NS_Fire_Campfire, NS_Dino_FootstepDust, NS_Weather_Rain, NS_Volcano_AshDrift
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "VFXNiagaraManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectCategory : uint8
{
    Environment     UMETA(DisplayName = "Environment"),
    Dinosaur        UMETA(DisplayName = "Dinosaur"),
    Combat          UMETA(DisplayName = "Combat"),
    Weather         UMETA(DisplayName = "Weather"),
    Volcanic        UMETA(DisplayName = "Volcanic")
};

USTRUCT(BlueprintType)
struct FVFX_NiagaraEffectConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FName EffectID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LOD0_Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LOD1_Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LOD2_Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoActivate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bLooping;

    FVFX_NiagaraEffectConfig()
        : EffectID(NAME_None)
        , Category(EVFX_EffectCategory::Environment)
        , LOD0_Distance(1500.0f)
        , LOD1_Distance(4000.0f)
        , LOD2_Distance(8000.0f)
        , bAutoActivate(true)
        , bLooping(true)
    {}
};

UCLASS(ClassGroup = (VFX), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_NiagaraManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_NiagaraManager();

    // --- Effect Configs ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    TArray<FVFX_NiagaraEffectConfig> RegisteredEffects;

    // --- Active Components ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|State")
    TMap<FName, UNiagaraComponent*> ActiveEffects;

    // --- Spawn Effect at World Location ---
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffectAtLocation(FName EffectID, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    // --- Attach Effect to Actor ---
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* AttachEffectToActor(FName EffectID, AActor* TargetActor, FName SocketName = NAME_None);

    // --- Stop Effect ---
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(FName EffectID, bool bImmediate = false);

    // --- Weather VFX ---
    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void SetRainIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void SetFogDensity(float Density);

    // --- Campfire VFX ---
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    UNiagaraComponent* SpawnCampfire(FVector Location);

    // --- Dinosaur Impact VFX ---
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void SpawnFootstepDust(FVector ImpactLocation, float DinoMassKg);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void SpawnBreathVapor(AActor* DinoActor, FName NostrilSocket);

    // --- Combat VFX ---
    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnBloodImpact(FVector HitLocation, FVector HitNormal, float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnWeaponImpact(FVector HitLocation, FVector HitNormal, FName SurfaceType);

    // --- Volcanic VFX ---
    UFUNCTION(BlueprintCallable, Category = "VFX|Volcanic")
    void SetAshDriftIntensity(float Intensity);

    // --- LOD Management ---
    UFUNCTION(BlueprintCallable, Category = "VFX|LOD")
    void UpdateLODForPlayerDistance(float DistanceToPlayer);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    float CurrentRainIntensity;

    UPROPERTY()
    float CurrentAshIntensity;

    UPROPERTY()
    float LastLODUpdateDistance;

    void RegisterDefaultEffects();
    FVFX_NiagaraEffectConfig* FindEffectConfig(FName EffectID);
};
