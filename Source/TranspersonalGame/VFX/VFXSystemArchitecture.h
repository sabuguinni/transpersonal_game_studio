#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "VFXSystemArchitecture.generated.h"

/**
 * VFX System Architecture for Transpersonal Game Studio
 * Jurassic Survival Game - VFX Agent #17
 * 
 * Core philosophy: VFX serves narrative and mechanics, never itself
 * Performance target: 3-level LOD chain for all effects
 * 
 * EFFECT CATEGORIES:
 * - Environmental: Weather, atmosphere, ambient life
 * - Creature: Breathing, movement, interaction effects  
 * - Survival: Crafting, fire, tool usage
 * - Emotional: Trust building, fear responses, discovery
 * - Combat: Impact, damage, defensive behaviors
 */

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Environmental   UMETA(DisplayName = "Environmental"),
    Creature        UMETA(DisplayName = "Creature"),
    Survival        UMETA(DisplayName = "Survival"), 
    Emotional       UMETA(DisplayName = "Emotional"),
    Combat          UMETA(DisplayName = "Combat"),
    Discovery       UMETA(DisplayName = "Discovery")
};

UENUM(BlueprintType)
enum class EVFXIntensity : uint8
{
    Subtle      UMETA(DisplayName = "Subtle"),      // Background atmosphere
    Noticeable  UMETA(DisplayName = "Noticeable"), // Clear but not distracting
    Prominent   UMETA(DisplayName = "Prominent"),  // Key gameplay moments
    Dramatic    UMETA(DisplayName = "Dramatic")    // Critical story beats
};

UENUM(BlueprintType)
enum class EVFXLODLevel : uint8
{
    High        UMETA(DisplayName = "High Quality"),    // Close range, full detail
    Medium      UMETA(DisplayName = "Medium Quality"),  // Mid range, optimized
    Low         UMETA(DisplayName = "Low Quality")      // Far range, minimal
};

USTRUCT(BlueprintType)
struct FVFXEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXIntensity Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem_High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem_Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem_Low;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresLineOfSight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAffectsGameplay = false;

    FVFXEffectData()
    {
        EffectName = TEXT("DefaultEffect");
        Category = EVFXCategory::Environmental;
        Intensity = EVFXIntensity::Subtle;
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

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayEffect(const FString& EffectName, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(const FString& EffectName);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXQuality(EVFXLODLevel NewLODLevel);

    // Creature-specific VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Creature")
    void PlayCreatureBreathing(AActor* Creature, float IntensityMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Creature")
    void PlayFootstepEffect(const FVector& Location, float CreatureSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Creature")
    void PlayTrustBuildingEffect(AActor* Player, AActor* Creature, float TrustLevel);

    // Environmental VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SetAtmosphericTension(float TensionLevel); // 0.0 = calm, 1.0 = maximum danger

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void PlayAmbientLifeEffect(const FVector& Location, const FString& BiomeType);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    int32 GetActiveEffectCount() const { return ActiveEffects.Num(); }

    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void CullDistantEffects(const FVector& ViewerLocation, float MaxDistance = 10000.0f);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
    TMap<FString, FVFXEffectData> EffectDatabase;

    UPROPERTY()
    TMap<FString, UNiagaraComponent*> ActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
    EVFXLODLevel CurrentLODLevel = EVFXLODLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Performance")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Performance")
    float EffectCullDistance = 15000.0f;

private:
    void InitializeEffectDatabase();
    UNiagaraSystem* GetEffectForLOD(const FVFXEffectData& EffectData) const;
    void CleanupFinishedEffects();
};