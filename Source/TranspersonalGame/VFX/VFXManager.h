#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/SceneComponent.h"
#include "VFXManager.generated.h"

UENUM(BlueprintType)
enum class EVFXType : uint8
{
    // Atmospheric Effects
    DustMotes,
    Fog,
    LightRays,
    
    // Danger Indicators
    DinosaurFootprint,
    BrokenVegetation,
    BloodSplatter,
    
    // Environmental
    WaterSplash,
    RockDebris,
    LeafFall,
    
    // Player Interaction
    CraftingSparkles,
    ToolImpact,
    FireEmbers,
    
    // Weather
    RainDrops,
    WindGusts,
    
    MAX UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FVFXDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXType Type;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DefaultLifetime = 5.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoDestroy = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxSimultaneousInstances = 10;
    
    // LOD Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LODDistance1 = 1000.0f; // High quality
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LODDistance2 = 2500.0f; // Medium quality
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LODDistance3 = 5000.0f; // Low quality (beyond this = culled)
};

UCLASS()
class TRANSPERSONALGAME_API AVFXManager : public AActor
{
    GENERATED_BODY()
    
public:    
    AVFXManager();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void Tick(float DeltaTime) override;
    
    // Core VFX Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFX(EVFXType Type, FVector Location, FRotator Rotation = FRotator::ZeroRotator, FVector Scale = FVector::OneVector);
    
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnVFXAtLocation(EVFXType Type, FVector Location);
    
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnVFXAttached(EVFXType Type, USceneComponent* AttachToComponent, FName SocketName = NAME_None);
    
    // Dinosaur Interaction VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void CreateDinosaurFootprint(FVector Location, float DinosaurSize = 1.0f, bool bIsRunning = false);
    
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void CreateVegetationDisturbance(FVector Location, FVector Direction, float Intensity = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void CreateBloodEffect(FVector Location, FVector Direction, bool bIsLarge = false);
    
    // Environmental VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void CreateWaterSplash(FVector Location, float Intensity = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void CreateRockImpact(FVector Location, FVector ImpactDirection);
    
    // Player Interaction VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Player")
    void CreateCraftingEffect(FVector Location);
    
    UFUNCTION(BlueprintCallable, Category = "VFX|Player")
    void CreateToolImpact(FVector Location, FVector Direction);
    
    UFUNCTION(BlueprintCallable, Category = "VFX|Player")
    void CreateFireEffect(FVector Location, float Intensity = 1.0f);
    
    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void SetVFXQualityLevel(int32 QualityLevel); // 0=Low, 1=Medium, 2=High, 3=Ultra
    
    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void CullDistantVFX(FVector ViewerLocation);
    
protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TArray<FVFXDefinition> VFXDefinitions;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;
    
    // Active VFX tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveVFXComponents;
    
    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 CurrentQualityLevel = 2; // Default to High
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxVFXDistance = 5000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveVFX = 50;
    
    // Helper functions
    FVFXDefinition* GetVFXDefinition(EVFXType Type);
    void CleanupFinishedVFX();
    float CalculateLODLevel(FVector VFXLocation, FVector ViewerLocation);
    void ApplyLODSettings(UNiagaraComponent* VFXComponent, float LODLevel);
};