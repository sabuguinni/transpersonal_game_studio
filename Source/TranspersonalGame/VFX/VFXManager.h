#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "VFXManager.generated.h"

UENUM(BlueprintType)
enum class EVFXType : uint8
{
    Combat,
    Environment,
    Magic,
    Weather,
    UI,
    Cinematic
};

UENUM(BlueprintType)
enum class EVFXQuality : uint8
{
    Low,
    Medium,
    High,
    Ultra
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFXConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXType VFXType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxInstances = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LifeTime = 5.0f;

    FVFXConfig()
    {
        VFXType = EVFXType::Environment;
        MaxDistance = 5000.0f;
        MaxInstances = 10;
        bAutoDestroy = true;
        LifeTime = 5.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FActiveVFX
{
    GENERATED_BODY()

    UPROPERTY()
    UNiagaraComponent* Component;

    UPROPERTY()
    FVector Location;

    UPROPERTY()
    EVFXType Type;

    UPROPERTY()
    float SpawnTime;

    UPROPERTY()
    float LifeTime;

    FActiveVFX()
    {
        Component = nullptr;
        Location = FVector::ZeroVector;
        Type = EVFXType::Environment;
        SpawnTime = 0.0f;
        LifeTime = 5.0f;
    }
};

/**
 * VFX Manager - Centralized system for managing all visual effects
 * Handles Niagara systems, performance optimization, and effect pooling
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFXManager : public AActor
{
    GENERATED_BODY()

public:
    AVFXManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // VFX Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TMap<FString, FVFXConfig> VFXConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFXQuality CurrentQuality = EVFXQuality::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    int32 MaxActiveVFX = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float CullingDistance = 10000.0f;

    // Active VFX tracking
    UPROPERTY()
    TArray<FActiveVFX> ActiveVFXList;

    UPROPERTY()
    TMap<EVFXType, int32> VFXCounts;

    // VFX Spawning Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFX(const FString& VFXName, const FVector& Location, 
                               const FRotator& Rotation = FRotator::ZeroRotator, 
                               AActor* AttachActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAtLocation(UNiagaraSystem* NiagaraSystem, 
                                         const FVector& Location, 
                                         const FRotator& Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAttached(UNiagaraSystem* NiagaraSystem, 
                                       USceneComponent* AttachComponent,
                                       const FVector& LocationOffset = FVector::ZeroVector);

    // VFX Management Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFX(UNiagaraComponent* VFXComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXOfType(EVFXType VFXType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFX();

    // Performance Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXQuality(EVFXQuality NewQuality);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void UpdateVFXLOD();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CullDistantVFX();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveVFXCount(EVFXType VFXType = EVFXType::Environment);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool CanSpawnVFX(EVFXType VFXType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void RegisterVFXConfig(const FString& Name, const FVFXConfig& Config);

private:
    // Internal management
    void CleanupFinishedVFX();
    void ApplyQualitySettings();
    float GetDistanceToPlayer(const FVector& Location);
    bool ShouldCullVFX(const FActiveVFX& VFX);

    // Timer handles
    FTimerHandle CleanupTimerHandle;
    FTimerHandle CullingTimerHandle;

    // Performance tracking
    float LastFrameTime;
    int32 VFXSpawnedThisFrame;
    
    static AVFXManager* Instance;

public:
    // Singleton access
    UFUNCTION(BlueprintCallable, Category = "VFX")
    static AVFXManager* GetInstance();
};