#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "VFXSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFXPriority : uint8
{
    Critical = 0,    // Combat impacts, predator attacks
    High = 1,        // Environmental dangers, large creature movements  
    Medium = 2,      // Ambient effects, small creature interactions
    Low = 3,         // Decorative effects, distant ambience
    Background = 4   // Subtle atmospheric effects
};

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Combat,          // Impacts, blood, damage effects
    Creature,        // Dinosaur-specific effects (breathing, movement)
    Environment,     // Weather, vegetation, geological
    Interaction,     // Domestication, feeding, tool use
    Atmospheric,     // Fog, particles, ambient lighting
    Survival,        // Crafting, fire, shelter effects
    Discovery        // Gem effects, exploration rewards
};

USTRUCT(BlueprintType)
struct FVFXEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UNiagaraSystem* NiagaraSystem;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXPriority Priority;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXCategory Category;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDistance = 5000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxInstances = 10;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUseDistanceLOD = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPooled = true;

    FVFXEffectData()
    {
        NiagaraSystem = nullptr;
        Priority = EVFXPriority::Medium;
        Category = EVFXCategory::Environment;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFXSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFXSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFX(const FString& EffectName, FVector Location, FRotator Rotation = FRotator::ZeroRotator, AActor* AttachToActor = nullptr);
    
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFX(UNiagaraComponent* VFXComponent);
    
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXByCategory(EVFXCategory Category);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void SetGlobalVFXQuality(float QualityLevel); // 0.0 = lowest, 1.0 = highest
    
    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void UpdateLODSystem(FVector ViewerLocation);
    
    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    int32 GetActiveVFXCount() const;

    // Creature-Specific VFX
    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    void PlayCreatureBreathingEffect(AActor* Creature, float IntensityMultiplier = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    void PlayCreatureFootstepEffect(FVector Location, float CreatureSize);
    
    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    void PlayDomesticationProgressEffect(AActor* Creature, float TrustLevel);

    // Environmental VFX
    UFUNCTION(BlueprintCallable, Category = "Environment VFX")
    void PlayWeatherEffect(const FString& WeatherType, float Intensity);
    
    UFUNCTION(BlueprintCallable, Category = "Environment VFX")
    void PlayVegetationDisturbance(FVector Location, float Radius);

    // Combat VFX
    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void PlayImpactEffect(FVector Location, FVector Normal, const FString& SurfaceType);
    
    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void PlayBloodEffect(FVector Location, FVector Direction, float Intensity);

protected:
    // VFX Library
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Library")
    TMap<FString, FVFXEffectData> VFXLibrary;
    
    // Active VFX Tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveVFXComponents;
    
    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float GlobalQualityMultiplier = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxActiveVFX = 50;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float LODUpdateInterval = 0.5f;
    
    // LOD Distances
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD")
    float HighQualityDistance = 1000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD")
    float MediumQualityDistance = 2500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD")
    float LowQualityDistance = 5000.0f;

private:
    void InitializeVFXLibrary();
    void CleanupInactiveVFX();
    void ApplyLODToVFX(UNiagaraComponent* VFXComponent, float Distance);
    float CalculateQualityMultiplier(float Distance, EVFXPriority Priority);
    
    float LastLODUpdateTime = 0.0f;
};