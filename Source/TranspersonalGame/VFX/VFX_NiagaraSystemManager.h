#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFX_NiagaraSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire UMETA(DisplayName = "Campfire"),
    Dust_Footstep UMETA(DisplayName = "Dinosaur Footstep"),
    Weather_Rain UMETA(DisplayName = "Rain"),
    Combat_Blood UMETA(DisplayName = "Blood Impact"),
    Water_Splash UMETA(DisplayName = "Water Splash"),
    Environment_Dust UMETA(DisplayName = "Environmental Dust")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVector Scale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bAutoDestroy = true;

    FVFX_EffectSettings()
    {
        Duration = 5.0f;
        Intensity = 1.0f;
        Scale = FVector(1.0f);
        bAutoDestroy = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_NiagaraSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_NiagaraSystemManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    TMap<EVFX_EffectType, TSoftObjectPtr<UNiagaraSystem>> VFXSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVFX_EffectSettings DefaultSettings;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Runtime")
    TArray<class UNiagaraComponent*> ActiveVFXComponents;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    class UNiagaraComponent* SpawnVFXEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, const FVFX_EffectSettings& Settings = FVFX_EffectSettings());

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFXEffect(class UNiagaraComponent* VFXComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXIntensity(class UNiagaraComponent* VFXComponent, float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    TArray<class UNiagaraComponent*> GetActiveVFXComponents() const { return ActiveVFXComponents; }

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveVFXCount() const { return ActiveVFXComponents.Num(); }

protected:
    UFUNCTION()
    void OnVFXFinished(class UNiagaraComponent* FinishedComponent);

    void CleanupFinishedVFX();

private:
    float CleanupTimer = 0.0f;
    static constexpr float CleanupInterval = 1.0f;
};