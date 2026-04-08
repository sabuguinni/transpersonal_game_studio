#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/SceneComponent.h"
#include "VFXSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Survival        UMETA(DisplayName = "Survival"),
    Wildlife        UMETA(DisplayName = "Wildlife"), 
    Environmental   UMETA(DisplayName = "Environmental"),
    Combat          UMETA(DisplayName = "Combat"),
    Atmospheric     UMETA(DisplayName = "Atmospheric")
};

UENUM(BlueprintType)
enum class EVFXPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low")
};

USTRUCT(BlueprintType)
struct FVFXPoolEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxInstances = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CullDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoPool = true;

    FVFXPoolEntry()
    {
        Category = EVFXCategory::Environmental;
        Priority = EVFXPriority::Medium;
    }
};

/**
 * Central manager for all VFX systems in the Jurassic survival game
 * Handles pooling, LOD management, and performance optimization
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFXSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFXSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // VFX Pool Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    TMap<FString, FVFXPoolEntry> VFXPool;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveVFX = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceUpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODSystem = true;

    // Active VFX tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveVFXComponents;

    UPROPERTY()
    TArray<UNiagaraComponent*> PooledVFXComponents;

public:
    // Core VFX Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFX(const FString& VFXName, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, AActor* AttachToActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void ReturnVFXToPool(UNiagaraComponent* VFXComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void RegisterVFXSystem(const FString& Name, const FVFXPoolEntry& PoolEntry);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateVFXPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetVFXQuality(int32 QualityLevel); // 0=Low, 1=Medium, 2=High, 3=Ultra

    // Survival-specific VFX
    UFUNCTION(BlueprintCallable, Category = "Survival VFX")
    UNiagaraComponent* SpawnFireEffect(const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Survival VFX")
    UNiagaraComponent* SpawnSmokeEffect(const FVector& Location, float Density = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Survival VFX")
    UNiagaraComponent* SpawnSparkEffect(const FVector& Location, const FVector& Direction);

    // Wildlife VFX
    UFUNCTION(BlueprintCallable, Category = "Wildlife VFX")
    UNiagaraComponent* SpawnFootstepEffect(const FVector& Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Wildlife VFX")
    UNiagaraComponent* SpawnBreathEffect(const FVector& Location, float Temperature = 20.0f);

    // Environmental VFX
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    UNiagaraComponent* SpawnRainEffect(const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    UNiagaraComponent* SpawnWindEffect(const FVector& Location, const FVector& WindDirection);

private:
    float LastPerformanceUpdate;
    int32 CurrentQualityLevel;

    // Internal helper functions
    UNiagaraComponent* GetPooledVFX(const FString& VFXName);
    void InitializeVFXPool();
    void CullDistantVFX();
    void UpdateVFXLOD();
};