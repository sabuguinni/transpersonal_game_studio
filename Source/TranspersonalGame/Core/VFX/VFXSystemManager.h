#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "Subsystems/WorldSubsystem.h"
#include "VFXSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFXType : uint8
{
    None = 0,
    // Environmental Effects
    DinosaurFootsteps,
    DinosaurBreathing,
    DinosaurRoar,
    TreeSway,
    LeafFall,
    WaterRipples,
    RainDrops,
    Mist,
    Dust,
    
    // Combat Effects
    ClawSlash,
    BiteImpact,
    BloodSpray,
    TeethGlint,
    TailWhip,
    ChargeTrail,
    
    // Survival Effects
    FireSpark,
    SmokeTrail,
    ToolCrafting,
    FoodCooking,
    ShelterBuilding,
    
    // Mystical Effects
    GemGlow,
    TimeRift,
    TransportationEffect,
    AncientMagic,
    ConsciousnessShift,
    
    // Destruction Effects
    RockCrumble,
    TreeFall,
    GroundCrack,
    ImpactCrater,
    
    // Weather Effects
    Lightning,
    Thunder,
    WindGust,
    FogRoll,
    SunRays
};

UENUM(BlueprintType)
enum class EVFXQuality : uint8
{
    Low = 0,     // Mobile/Low-end PC
    Medium = 1,  // Console/Mid-range PC
    High = 2,    // High-end PC
    Cinematic = 3 // Cinematics only
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFXDefinition : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFXType VFXType = EVFXType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector DefaultScale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float DefaultLifetime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAttachToActor = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FName AttachSocketName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFXQuality MinimumQuality = EVFXQuality::Low;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TMap<FString, float> DefaultParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TMap<FString, FLinearColor> DefaultColors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TMap<FString, FVector> DefaultVectors;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFXInstance
{
    GENERATED_BODY()

    UPROPERTY()
    TWeakObjectPtr<UNiagaraComponent> Component;

    UPROPERTY()
    EVFXType VFXType = EVFXType::None;

    UPROPERTY()
    float SpawnTime = 0.0f;

    UPROPERTY()
    float Lifetime = 0.0f;

    UPROPERTY()
    bool bAutoDestroy = true;

    FVFXInstance()
    {
        Component = nullptr;
        VFXType = EVFXType::None;
        SpawnTime = 0.0f;
        Lifetime = 0.0f;
        bAutoDestroy = true;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVFXSpawned, EVFXType, VFXType, UNiagaraComponent*, Component, FVector, Location);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVFXDestroyed, EVFXType, VFXType, UNiagaraComponent*, Component);

/**
 * VFX System Manager - Centralized management of all visual effects in the game
 * Handles Niagara system spawning, pooling, LOD management, and performance optimization
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFXSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UVFXSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFX(EVFXType VFXType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, AActor* AttachActor = nullptr, FName AttachSocket = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAtLocation(EVFXType VFXType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAttached(EVFXType VFXType, AActor* AttachActor, FName AttachSocket = NAME_None, FVector LocationOffset = FVector::ZeroVector, FRotator RotationOffset = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void DestroyVFX(UNiagaraComponent* VFXComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void DestroyAllVFXOfType(EVFXType VFXType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void DestroyAllVFX();

    // VFX Control
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXParameter(UNiagaraComponent* VFXComponent, const FString& ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXColorParameter(UNiagaraComponent* VFXComponent, const FString& ParameterName, FLinearColor Color);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXVectorParameter(UNiagaraComponent* VFXComponent, const FString& ParameterName, FVector Vector);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXBoolParameter(UNiagaraComponent* VFXComponent, const FString& ParameterName, bool bValue);

    // Quality and Performance
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXQuality(EVFXQuality NewQuality);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    EVFXQuality GetVFXQuality() const { return CurrentQuality; }

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetMaxActiveVFX(int32 MaxCount) { MaxActiveVFXCount = MaxCount; }

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveVFXCount() const { return ActiveVFXInstances.Num(); }

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void UpdateVFXLOD(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXLODDistance(float NearDistance, float FarDistance);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "VFX")
    FOnVFXSpawned OnVFXSpawned;

    UPROPERTY(BlueprintAssignable, Category = "VFX")
    FOnVFXDestroyed OnVFXDestroyed;

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool IsVFXTypeAvailable(EVFXType VFXType) const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    TArray<UNiagaraComponent*> GetActiveVFXOfType(EVFXType VFXType) const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PauseAllVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void ResumeAllVFX();

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Config")
    UDataTable* VFXDefinitionTable;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Config")
    int32 MaxActiveVFXCount = 100;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Config")
    float VFXLODNearDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Config")
    float VFXLODFarDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Config")
    bool bEnableVFXPooling = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Config")
    int32 VFXPoolSize = 50;

    // Runtime State
    UPROPERTY()
    EVFXQuality CurrentQuality = EVFXQuality::Medium;

    UPROPERTY()
    TArray<FVFXInstance> ActiveVFXInstances;

    UPROPERTY()
    TMap<EVFXType, TArray<UNiagaraComponent*>> VFXPool;

    UPROPERTY()
    TMap<EVFXType, FVFXDefinition> VFXDefinitions;

    // Internal Methods
    void LoadVFXDefinitions();
    UNiagaraComponent* GetPooledVFX(EVFXType VFXType);
    void ReturnVFXToPool(UNiagaraComponent* VFXComponent, EVFXType VFXType);
    void CleanupExpiredVFX();
    void ApplyVFXDefinition(UNiagaraComponent* VFXComponent, const FVFXDefinition& Definition);
    float GetDistanceToPlayer(const FVector& Location) const;
    void UpdateVFXQualitySettings();

    // Tick
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return true; }
    virtual TStatId GetStatId() const override { return TStatId(); }
};

/**
 * VFX Component - Attach to actors that need to spawn VFX
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFXComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFXComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // VFX Spawning
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFX(EVFXType VFXType, FVector LocationOffset = FVector::ZeroVector, FRotator RotationOffset = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAtSocket(EVFXType VFXType, FName SocketName, FVector LocationOffset = FVector::ZeroVector, FRotator RotationOffset = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void DestroyAllVFX();

    // Auto VFX
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void EnableAutoVFX(EVFXType VFXType, float Interval = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void DisableAutoVFX();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Config")
    TArray<EVFXType> AutoSpawnVFXTypes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Config")
    float AutoSpawnInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Config")
    bool bAutoSpawnOnBeginPlay = false;

    UPROPERTY()
    TArray<UNiagaraComponent*> SpawnedVFX;

    UPROPERTY()
    FTimerHandle AutoVFXTimer;

    UPROPERTY()
    EVFXType CurrentAutoVFXType = EVFXType::None;

    void AutoSpawnVFX();
    UVFXSystemManager* GetVFXManager() const;
};