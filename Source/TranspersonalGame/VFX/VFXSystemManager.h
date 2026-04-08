#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/DataTable.h"
#include "VFXSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Environmental,      // Atmospheric, weather, ambient
    Creature,          // Dinosaur-specific effects
    Combat,            // Impact, damage, destruction
    Interaction,       // Domestication, feeding, bonding
    Survival,          // Crafting, building, resource gathering
    Narrative          // Story moments, discovery, tension
};

UENUM(BlueprintType)
enum class EVFXIntensity : uint8
{
    Subtle,            // Background ambience
    Moderate,          // Noticeable but not distracting
    Dramatic,          // Key moments, high impact
    Critical           // Life/death, major story beats
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFXDefinition : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXIntensity Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DefaultScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;
};

UCLASS()
class TRANSPERSONALGAME_API AVFXSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFXSystemManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Configuration")
    UDataTable* VFXDefinitionsTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimultaneousEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceNear = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceMedium = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceFar = 5000.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAtLocation(const FString& EffectName, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* AttachVFXToComponent(const FString& EffectName, USceneComponent* AttachComponent, const FName& SocketName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFXByName(const FString& EffectName);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXQualityLevel(int32 QualityLevel);

private:
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    void CleanupFinishedEffects();
    int32 GetLODLevel(const FVector& EffectLocation) const;
    
    UFUNCTION()
    void OnEffectFinished(UNiagaraComponent* FinishedComponent);
};