#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "VFXSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFXType : uint8
{
    None = 0,
    Consciousness,      // Mystical energy effects
    Environmental,      // Weather, atmosphere
    Combat,            // Impacts, explosions
    Interaction,       // Object highlights, UI feedback
    Destruction,       // Breaking, crumbling
    Magical           // Ancient powers, rituals
};

UENUM(BlueprintType)
enum class EVFXLODLevel : uint8
{
    High = 0,         // Full quality - close range
    Medium = 1,       // Reduced particles - medium range
    Low = 2          // Minimal particles - far range
};

USTRUCT(BlueprintType)
struct FVFXDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXType VFXType = EVFXType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDrawDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxInstances = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 5.0f;

    FVFXDefinition()
    {
        VFXType = EVFXType::None;
        EffectName = TEXT("DefaultVFX");
        MaxDrawDistance = 5000.0f;
        MaxInstances = 10;
        bAutoDestroy = true;
        Duration = 5.0f;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFXSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFXSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core VFX spawning functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAtLocation(EVFXType VFXType, const FString& EffectName, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, const FVector& Scale = FVector::OneVector);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAttached(EVFXType VFXType, const FString& EffectName, USceneComponent* AttachComponent, const FName& AttachPointName = NAME_None, const FVector& Location = FVector::ZeroVector, const FRotator& Rotation = FRotator::ZeroRotator, const FVector& Scale = FVector::OneVector);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFX(UNiagaraComponent* VFXComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXOfType(EVFXType VFXType);

    // LOD and performance management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXLODLevel(EVFXLODLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void UpdateVFXLODBasedOnDistance(const FVector& ViewerLocation);

    // VFX registry management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void RegisterVFXDefinition(const FVFXDefinition& VFXDef);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    FVFXDefinition GetVFXDefinition(EVFXType VFXType, const FString& EffectName) const;

    // Consciousness-specific VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Consciousness")
    UNiagaraComponent* SpawnConsciousnessAura(const FVector& Location, float Intensity = 1.0f, const FLinearColor& Color = FLinearColor::Blue);

    UFUNCTION(BlueprintCallable, Category = "VFX|Consciousness")
    UNiagaraComponent* SpawnEnergySwirl(const FVector& Location, float Radius = 200.0f, float Speed = 1.0f);

    // Environmental VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    UNiagaraComponent* SpawnWeatherEffect(const FString& WeatherType, const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    UNiagaraComponent* SpawnAtmosphericFog(const FVector& Location, float Density = 0.5f, const FLinearColor& Color = FLinearColor::White);

    // Combat VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    UNiagaraComponent* SpawnImpactEffect(const FVector& Location, const FVector& ImpactNormal, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    UNiagaraComponent* SpawnExplosion(const FVector& Location, float Radius = 300.0f, float Intensity = 1.0f);

protected:
    // VFX definitions registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TMap<FString, FVFXDefinition> VFXRegistry;

    // Active VFX tracking
    UPROPERTY()
    TArray<TWeakObjectPtr<UNiagaraComponent>> ActiveVFXComponents;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    EVFXLODLevel CurrentLODLevel = EVFXLODLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    int32 MaxActiveVFX = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    float VFXCullDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    float TickInterval = 0.1f;

private:
    // Internal helper functions
    void CleanupInvalidVFX();
    void ApplyLODToVFX(UNiagaraComponent* VFXComponent, EVFXLODLevel LODLevel);
    FString GenerateVFXKey(EVFXType VFXType, const FString& EffectName) const;
    void InitializeDefaultVFXDefinitions();
    
    float LastCleanupTime = 0.0f;
    float CleanupInterval = 5.0f;
};