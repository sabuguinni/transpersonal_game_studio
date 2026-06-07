#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "VFX_NiagaraLibrary.generated.h"

/**
 * VFX Niagara Library for prehistoric particle effects
 * Manages campfire, footsteps, impacts, weather, and atmospheric effects
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_NiagaraLibrary : public UObject
{
    GENERATED_BODY()

public:
    UVFX_NiagaraLibrary();

    // === CAMPFIRE EFFECTS ===
    UFUNCTION(BlueprintCallable, Category = "VFX|Fire")
    static class UNiagaraComponent* SpawnCampfireEffect(UWorld* World, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX|Fire")
    static void StopCampfireEffect(class UNiagaraComponent* FireComponent);

    // === FOOTSTEP EFFECTS ===
    UFUNCTION(BlueprintCallable, Category = "VFX|Footsteps")
    static void SpawnFootstepDust(UWorld* World, FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Footsteps")
    static void SpawnHeavyFootstep(UWorld* World, FVector Location, FVector Velocity);

    // === IMPACT EFFECTS ===
    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    static void SpawnBloodImpact(UWorld* World, FVector Location, FVector Normal);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    static void SpawnRockImpact(UWorld* World, FVector Location, FVector Normal);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    static void SpawnWoodImpact(UWorld* World, FVector Location, FVector Normal);

    // === WEATHER EFFECTS ===
    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    static class UNiagaraComponent* SpawnRainEffect(UWorld* World, FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    static class UNiagaraComponent* SpawnFogEffect(UWorld* World, FVector Location, float Density = 1.0f);

    // === DINOSAUR EFFECTS ===
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaurs")
    static void SpawnBreathVapor(UWorld* World, FVector Location, FRotator Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaurs")
    static void SpawnRoarDistortion(UWorld* World, FVector Location, float Intensity);

    // === ENVIRONMENTAL EFFECTS ===
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    static void SpawnDustCloud(UWorld* World, FVector Location, FVector Size);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    static void SpawnInsectSwarm(UWorld* World, FVector Location, float Radius);

protected:
    // === NIAGARA SYSTEM REFERENCES ===
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX Assets")
    static class UNiagaraSystem* CampfireSystem;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX Assets")
    static class UNiagaraSystem* FootstepDustSystem;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX Assets")
    static class UNiagaraSystem* BloodImpactSystem;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX Assets")
    static class UNiagaraSystem* RainSystem;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX Assets")
    static class UNiagaraSystem* FogSystem;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX Assets")
    static class UNiagaraSystem* BreathVaporSystem;

private:
    // === HELPER FUNCTIONS ===
    static FVector CalculateParticleVelocity(FVector ImpactNormal, float Speed);
    static float GetEnvironmentalWindStrength(UWorld* World, FVector Location);
    static bool IsLocationUnderwater(UWorld* World, FVector Location);
};