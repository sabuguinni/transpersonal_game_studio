#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "VFXManager.generated.h"

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Environment     UMETA(DisplayName = "Environment"),
    Combat          UMETA(DisplayName = "Combat"),
    Atmosphere      UMETA(DisplayName = "Atmosphere"),
    Interaction     UMETA(DisplayName = "Interaction"),
    Weather         UMETA(DisplayName = "Weather"),
    Destruction     UMETA(DisplayName = "Destruction"),
    Magic           UMETA(DisplayName = "Magic")
};

UENUM(BlueprintType)
enum class EVFXPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Background  UMETA(DisplayName = "Background")
};

USTRUCT(BlueprintType)
struct FVFXDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxInstances = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAttachToActor = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName AttachSocket = NAME_None;

    FVFXDefinition()
    {
        EffectName = NAME_None;
        Category = EVFXCategory::Environment;
        Priority = EVFXPriority::Medium;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFXManager : public AActor
{
    GENERATED_BODY()

public:
    AVFXManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // VFX Registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Registry")
    TMap<FName, FVFXDefinition> VFXRegistry;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEffects = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance1 = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance2 = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance3 = 5000.0f;

    // Active Effects Tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    UPROPERTY()
    TMap<FName, int32> EffectInstanceCounts;

public:
    // Core VFX Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFX(FName EffectName, FVector Location, FRotator Rotation = FRotator::ZeroRotator, AActor* AttachActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFX(UNiagaraComponent* Effect);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXByCategory(EVFXCategory Category);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void RegisterVFX(FName EffectName, FVFXDefinition Definition);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool IsVFXRegistered(FName EffectName);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateVFXLOD();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CleanupFinishedEffects();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveEffectCount() const { return ActiveEffects.Num(); }

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    float GetDistanceToPlayer(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetLODLevel(float Distance);

private:
    void InitializeVFXRegistry();
    bool CanSpawnEffect(FName EffectName, FVector Location);
    void UpdateEffectLOD(UNiagaraComponent* Effect, int32 LODLevel);
};