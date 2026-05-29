#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "VFX_NiagaraSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire       UMETA(DisplayName = "Campfire"),
    Impact_Dust         UMETA(DisplayName = "Dust Impact"),
    Impact_Blood        UMETA(DisplayName = "Blood Impact"),
    Weather_Rain        UMETA(DisplayName = "Rain"),
    Weather_Fog         UMETA(DisplayName = "Fog"),
    Dino_Footstep       UMETA(DisplayName = "Dinosaur Footstep"),
    Dino_Breath         UMETA(DisplayName = "Dinosaur Breath"),
    Water_Splash        UMETA(DisplayName = "Water Splash"),
    Crafting_Sparks     UMETA(DisplayName = "Crafting Sparks")
};

USTRUCT(BlueprintType)
struct FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bLooping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FLinearColor TintColor = FLinearColor::White;

    FVFX_EffectData()
    {
        Duration = 5.0f;
        Scale = 1.0f;
        bLooping = false;
        TintColor = FLinearColor::White;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AVFX_NiagaraSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_NiagaraSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core VFX Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
    UNiagaraComponent* PrimaryEffectComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
    UNiagaraComponent* SecondaryEffectComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
    UNiagaraComponent* AmbientEffectComponent;

    // VFX Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Configuration")
    TMap<EVFX_EffectType, FVFX_EffectData> EffectDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Configuration")
    EVFX_EffectType CurrentEffectType = EVFX_EffectType::Fire_Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Configuration")
    float GlobalVFXIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Configuration")
    bool bAutoActivateOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Configuration")
    float EffectCullDistance = 5000.0f;

public:
    // VFX Control Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(EVFX_EffectType EffectType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetEffectIntensity(EVFX_EffectType EffectType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetGlobalVFXIntensity(float Intensity);

    // Dinosaur-specific VFX
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void PlayDinosaurFootstepEffect(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void PlayDinosaurBreathEffect(FVector Location, FRotator Direction);

    // Environmental VFX
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void PlayCampfireEffect(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void PlayWeatherEffect(EVFX_EffectType WeatherType, float Intensity);

    // Combat VFX
    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void PlayImpactEffect(FVector Location, EVFX_EffectType ImpactType, float ImpactForce = 1.0f);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "VFX Utility")
    bool IsEffectActive(EVFX_EffectType EffectType) const;

    UFUNCTION(BlueprintCallable, Category = "VFX Utility")
    float GetDistanceToPlayer() const;

private:
    void InitializeEffectDatabase();
    void UpdateEffectCulling();
    UNiagaraComponent* GetAvailableComponent();
    void ConfigureNiagaraComponent(UNiagaraComponent* Component, const FVFX_EffectData& EffectData);

    // Active effect tracking
    TArray<UNiagaraComponent*> ActiveComponents;
    TMap<EVFX_EffectType, UNiagaraComponent*> ActiveEffects;
    
    // Performance optimization
    float LastCullCheckTime = 0.0f;
    float CullCheckInterval = 1.0f;
    bool bIsWithinCullDistance = true;
};