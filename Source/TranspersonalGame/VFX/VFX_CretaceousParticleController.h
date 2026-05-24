#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Materials/MaterialInterface.h"
#include "VFX_CretaceousParticleController.generated.h"

/**
 * Controls realistic Cretaceous period particle effects
 * Manages atmospheric dust, pollen, volcanic ash, and insect swarms
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_CretaceousParticleController : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_CretaceousParticleController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Particle System References
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cretaceous VFX")
    class UNiagaraSystem* AtmosphericDustSystem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cretaceous VFX")
    class UNiagaraSystem* VolcanicAshSystem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cretaceous VFX")
    class UNiagaraSystem* PollenDriftSystem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cretaceous VFX")
    class UNiagaraSystem* InsectSwarmSystem;

    // Active Niagara Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* DustComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* AshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* PollenComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* InsectComponent;

    // Environmental Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WindStrength = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Humidity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "15.0", ClampMax = "45.0"))
    float Temperature = 28.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VolcanicActivity = 0.2f;

    // Control Functions
    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StartAtmosphericDust();

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StopAtmosphericDust();

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StartVolcanicAsh();

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StopVolcanicAsh();

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StartPollenDrift();

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StopPollenDrift();

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StartInsectSwarm();

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StopInsectSwarm();

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void UpdateEnvironmentalParameters(float NewWindStrength, float NewHumidity, float NewTemperature);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SetVolcanicActivity(float ActivityLevel);

private:
    void InitializeParticleSystems();
    void UpdateParticleParameters();
    void CreateNiagaraComponent(class UNiagaraSystem* System, class UNiagaraComponent*& Component, const FString& ComponentName);
};