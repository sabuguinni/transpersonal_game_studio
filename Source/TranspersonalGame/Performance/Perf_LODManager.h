#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perf_LODManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_LODLevel : uint8
{
    LOD0_Ultra = 0,
    LOD1_High = 1,
    LOD2_Medium = 2,
    LOD3_Low = 3,
    LOD4_Culled = 4
};

USTRUCT(BlueprintType)
struct FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD0_Distance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD1_Distance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD2_Distance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD3_Distance = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnablePhysicsLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableAnimationLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableParticleLOD = true;

    FPerf_LODSettings()
    {
        LOD0_Distance = 500.0f;
        LOD1_Distance = 1500.0f;
        LOD2_Distance = 3000.0f;
        LOD3_Distance = 6000.0f;
        CullDistance = 10000.0f;
        bEnablePhysicsLOD = true;
        bEnableAnimationLOD = true;
        bEnableParticleLOD = true;
    }
};

USTRUCT(BlueprintType)
struct FPerf_ActorLODData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    TWeakObjectPtr<AActor> Actor;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    EPerf_LODLevel CurrentLOD;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    float LastDistance;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    float LastUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    bool bIsVisible;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    bool bPhysicsEnabled;

    FPerf_ActorLODData()
    {
        Actor = nullptr;
        CurrentLOD = EPerf_LODLevel::LOD0_Ultra;
        LastDistance = 0.0f;
        LastUpdateTime = 0.0f;
        bIsVisible = true;
        bPhysicsEnabled = true;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_LODManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_LODManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // LOD Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    FPerf_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    int32 MaxActorsPerFrame = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    bool bEnableLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    bool bDebugLOD = false;

    // Runtime Data
    UPROPERTY(BlueprintReadOnly, Category = "LOD Runtime")
    TArray<FPerf_ActorLODData> ManagedActors;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Runtime")
    int32 CurrentUpdateIndex;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Runtime")
    float LastUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Runtime")
    int32 TotalManagedActors;

    // LOD Management Functions
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void RegisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UnregisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UpdateActorLOD(FPerf_ActorLODData& ActorData, float Distance);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    EPerf_LODLevel CalculateLODLevel(float Distance) const;

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void ApplyLODToActor(AActor* Actor, EPerf_LODLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetPhysicsLOD(AActor* Actor, bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetAnimationLOD(USkeletalMeshComponent* SkelMesh, EPerf_LODLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetMeshLOD(UStaticMeshComponent* StaticMesh, EPerf_LODLevel LODLevel);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetPlayerDistance(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsActorInFrustum(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceUpdateAllActors();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODSettings(const FPerf_LODSettings& NewSettings);

    // Debug Functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugPrintLODStatus();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawLODDebugInfo();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    int32 GetActorsAtLODLevel(EPerf_LODLevel LODLevel) const;

private:
    // Internal Functions
    void UpdateLODSystem(float DeltaTime);
    void ProcessActorBatch();
    void CleanupInvalidActors();
    AActor* GetPlayerActor() const;
    FVector GetPlayerLocation() const;
};