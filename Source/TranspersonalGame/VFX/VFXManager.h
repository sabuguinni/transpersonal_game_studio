#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SceneComponent.h"
#include "VFXManager.generated.h"

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Environmental   UMETA(DisplayName = "Environmental"),
    Combat         UMETA(DisplayName = "Combat"),
    Mystical       UMETA(DisplayName = "Mystical"),
    Natural        UMETA(DisplayName = "Natural"),
    UI             UMETA(DisplayName = "UI")
};

UENUM(BlueprintType)
enum class EVFXQuality : uint8
{
    Low     UMETA(DisplayName = "Low"),
    Medium  UMETA(DisplayName = "Medium"),
    High    UMETA(DisplayName = "High"),
    Ultra   UMETA(DisplayName = "Ultra")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFXSystemData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FString EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFXCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float MaxDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    int32 MaxParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LifeTime;

    FVFXSystemData()
    {
        EffectName = TEXT("");
        Category = EVFXCategory::Environmental;
        MaxDistance = 2000.0f;
        MaxParticles = 1000;
        bAutoDestroy = true;
        LifeTime = 5.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFXPerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EVFXQuality QualityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODNearDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODFarDistance;

    FVFXPerformanceSettings()
    {
        MaxActiveEffects = 50;
        CullingDistance = 3000.0f;
        QualityLevel = EVFXQuality::High;
        bEnableLODSystem = true;
        LODNearDistance = 500.0f;
        LODFarDistance = 2000.0f;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    TMap<FString, FVFXSystemData> RegisteredVFXSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FVFXPerformanceSettings PerformanceSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<UNiagaraComponent*> ActiveVFXComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    int32 CurrentActiveEffects;

public:
    // Core VFX Management Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXEffect(const FString& EffectName, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, AActor* AttachToActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFXEffect(UNiagaraComponent* VFXComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void RegisterVFXSystem(const FString& EffectName, UNiagaraSystem* NiagaraSystem, EVFXCategory Category);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void UnregisterVFXSystem(const FString& EffectName);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetVFXQuality(EVFXQuality NewQuality);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceSettings(const FVFXPerformanceSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantEffects(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveEffectCount() const { return CurrentActiveEffects; }

    // Preset VFX Effects
    UFUNCTION(BlueprintCallable, Category = "Preset Effects")
    UNiagaraComponent* SpawnFireEffect(const FVector& Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Preset Effects")
    UNiagaraComponent* SpawnSmokeEffect(const FVector& Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Preset Effects")
    UNiagaraComponent* SpawnExplosionEffect(const FVector& Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Preset Effects")
    UNiagaraComponent* SpawnConsciousnessEffect(const FVector& Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Preset Effects")
    UNiagaraComponent* SpawnHealingEffect(AActor* TargetActor, float Scale = 1.0f);

private:
    void InitializeDefaultVFXSystems();
    void CleanupFinishedEffects();
    void ApplyLODSettings(UNiagaraComponent* VFXComponent, float Distance);
    FVFXSystemData* GetVFXSystemData(const FString& EffectName);
};