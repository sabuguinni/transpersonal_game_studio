#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "VFXSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Environmental,      // Atmospheric, weather, ambient
    Creature,          // Dinosaur-specific effects
    Combat,            // Impact, damage, destruction
    Interaction,       // Domestication, feeding, bonding
    Temporal,          // Time gem, portal effects
    Survival,          // Crafting, building, tools
    Atmospheric        // Tension, fear, mood
};

UENUM(BlueprintType)
enum class EVFXPriority : uint8
{
    Critical = 0,      // Always render (temporal gem, death)
    High = 1,          // Important gameplay (combat, predator alerts)
    Medium = 2,        // Standard effects (ambient, interactions)
    Low = 3            // Cosmetic only (distant particles, decorative)
};

USTRUCT(BlueprintType)
struct FVFXEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxActiveInstances = 10;

    FVFXEffectData()
    {
        Category = EVFXCategory::Environmental;
        Priority = EVFXPriority::Medium;
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
    UNiagaraComponent* SpawnVFXAtLocation(const FString& EffectName, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* AttachVFXToActor(const FString& EffectName, AActor* TargetActor, const FName& SocketName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFXByName(const FString& EffectName);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXByCategory(EVFXCategory Category);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void SetVFXQualityLevel(int32 QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void UpdateLODDistances();

    // Creature-Specific VFX
    UFUNCTION(BlueprintCallable, Category = "VFX Creatures")
    void PlayCreatureBreathingEffect(AActor* Creature, float IntensityMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Creatures")
    void PlayCreatureFootstepEffect(AActor* Creature, const FVector& ImpactLocation, float CreatureWeight);

    UFUNCTION(BlueprintCallable, Category = "VFX Creatures")
    void PlayDomesticationProgressEffect(AActor* Creature, float TrustLevel);

    // Temporal VFX
    UFUNCTION(BlueprintCallable, Category = "VFX Temporal")
    void PlayTemporalGemEffect(const FVector& GemLocation, bool bIsActivating);

    UFUNCTION(BlueprintCallable, Category = "VFX Temporal")
    void PlayTimeDistortionEffect(const FVector& Location, float Radius, float Duration);

    // Environmental VFX
    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SetAtmosphericTension(float TensionLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void PlayPredatorPresenceEffect(const FVector& PredatorLocation, float ThreatRadius);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Database")
    TMap<FString, FVFXEffectData> VFXDatabase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Runtime")
    TArray<UNiagaraComponent*> ActiveVFXComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    int32 CurrentQualityLevel = 2; // 0=Low, 1=Medium, 2=High, 3=Ultra

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float MaxVFXDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    int32 MaxTotalVFXInstances = 50;

private:
    void InitializeVFXDatabase();
    void CleanupInactiveVFX();
    bool ShouldCullVFXByDistance(const FVector& VFXLocation) const;
    int32 GetLODLevel(float Distance) const;
};