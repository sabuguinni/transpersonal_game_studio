#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "VFXArchitecture.generated.h"

/**
 * VFX Architecture for Transpersonal Game Studio
 * 3-Layer LOD system for optimal performance
 * 
 * LAYER 1 (0-50m): Full detail Niagara systems
 * LAYER 2 (50-200m): Medium detail with reduced particles
 * LAYER 3 (200m+): Simple billboard/mesh replacements
 */

UENUM(BlueprintType)
enum class EVFXLODLevel : uint8
{
    High        UMETA(DisplayName = "High Detail (0-50m)"),
    Medium      UMETA(DisplayName = "Medium Detail (50-200m)"),
    Low         UMETA(DisplayName = "Low Detail (200m+)"),
    Disabled    UMETA(DisplayName = "Disabled")
};

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    // Core gameplay VFX
    DinosaurBreathing       UMETA(DisplayName = "Dinosaur Breathing"),
    DinosaurFootsteps       UMETA(DisplayName = "Dinosaur Footsteps"),
    DinosaurEyes            UMETA(DisplayName = "Dinosaur Eye Glow"),
    
    // Environmental VFX
    AmbientDust             UMETA(DisplayName = "Ambient Dust"),
    LeafFalling             UMETA(DisplayName = "Falling Leaves"),
    WaterDroplets           UMETA(DisplayName = "Water Droplets"),
    LightShafts             UMETA(DisplayName = "God Rays"),
    
    // Interaction VFX
    PlayerFootsteps         UMETA(DisplayName = "Player Footsteps"),
    ToolImpact              UMETA(DisplayName = "Tool Impact"),
    FireSparks              UMETA(DisplayName = "Fire Sparks"),
    
    // Emotional VFX
    TensionParticles        UMETA(DisplayName = "Tension Atmosphere"),
    SafetyAura              UMETA(DisplayName = "Safety Indicators"),
    DomesticationProgress   UMETA(DisplayName = "Domestication Visual")
};

USTRUCT(BlueprintType)
struct FVFXLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxParticles = 1000;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float UpdateFrequency = 60.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableCollision = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCastShadows = true;
    
    FVFXLODSettings()
    {
        MaxParticles = 1000;
        UpdateFrequency = 60.0f;
        bEnableCollision = true;
        bCastShadows = true;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFXManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFXManagerComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnVFX(EVFXCategory Category, FVector Location, FRotator Rotation = FRotator::ZeroRotator);
    
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXLOD(EVFXLODLevel NewLODLevel);
    
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void UpdateVFXBasedOnDistance(float DistanceToPlayer);
    
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveParticleCount() const;
    
    UFUNCTION(BlueprintCallable, Category = "VFX")
    float GetVFXPerformanceMetric() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TMap<EVFXCategory, TSoftObjectPtr<UNiagaraSystem>> VFXSystems;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    TMap<EVFXLODLevel, FVFXLODSettings> LODSettings;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    EVFXLODLevel CurrentLODLevel;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<UNiagaraComponent*> ActiveVFXComponents;
    
private:
    void InitializeLODSettings();
    void CleanupInactiveVFX();
    UNiagaraComponent* CreateVFXComponent(UNiagaraSystem* System, FVector Location, FRotator Rotation);
};