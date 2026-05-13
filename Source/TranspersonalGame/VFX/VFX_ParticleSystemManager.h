#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "SharedTypes.h"
#include "VFX_ParticleSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ParticleType : uint8
{
    FootstepDust        UMETA(DisplayName = "Footstep Dust"),
    BloodImpact         UMETA(DisplayName = "Blood Impact"),
    FireSmoke           UMETA(DisplayName = "Fire Smoke"),
    EnvironmentalDust   UMETA(DisplayName = "Environmental Dust"),
    WaterSplash         UMETA(DisplayName = "Water Splash"),
    RockDebris          UMETA(DisplayName = "Rock Debris")
};

USTRUCT(BlueprintType)
struct FVFX_ParticleSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float ParticleScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 ParticleCount = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVector VelocityRange = FVector(100, 100, 200);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FLinearColor ParticleColor = FLinearColor::White;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_ParticleSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ParticleSystemManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    UNiagaraComponent* MainParticleComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TMap<EVFX_ParticleType, FVFX_ParticleSettings> ParticleTypeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> FootstepDustSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> BloodImpactSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> FireSmokeSystem;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SpawnParticleEffect(EVFX_ParticleType ParticleType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SpawnFootstepEffect(FVector Location, float DinosaurMass = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SpawnBloodImpact(FVector Location, FVector ImpactNormal);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SpawnEnvironmentalDust(FVector Location, float WindStrength = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SetParticleSettings(EVFX_ParticleType ParticleType, FVFX_ParticleSettings NewSettings);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    FVFX_ParticleSettings GetParticleSettings(EVFX_ParticleType ParticleType) const;

    UFUNCTION(BlueprintCallable, Category = "VFX Debug")
    void TestAllParticleTypes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "VFX Debug")
    void DebugSpawnTestEffects();

protected:
    UFUNCTION()
    void InitializeParticleSettings();

    UFUNCTION()
    UNiagaraComponent* CreateParticleComponent(EVFX_ParticleType ParticleType, FVector Location);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    int32 MaxActiveParticles = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float ParticleLifetime = 5.0f;

    TArray<UNiagaraComponent*> ActiveParticleComponents;
};