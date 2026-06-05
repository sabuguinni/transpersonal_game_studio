#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassCommonTypes.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "SharedTypes.h"
#include "Crowd_LODSystem.generated.h"

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    High        UMETA(DisplayName = "High Detail"),
    Medium      UMETA(DisplayName = "Medium Detail"),
    Low         UMETA(DisplayName = "Low Detail"),
    Culled      UMETA(DisplayName = "Culled")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Distances")
    float HighDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Distances")
    float MediumDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Distances")
    float LowDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Distances")
    float CullDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxHighDetailAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxMediumDetailAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODUpdateFrequency;

    FCrowd_LODSettings()
    {
        HighDetailDistance = 500.0f;
        MediumDetailDistance = 1500.0f;
        LowDetailDistance = 3000.0f;
        CullDistance = 5000.0f;
        MaxHighDetailAgents = 100;
        MaxMediumDetailAgents = 500;
        LODUpdateFrequency = 0.1f; // 10 times per second
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityLOD
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity LOD")
    FMassEntityHandle EntityHandle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity LOD")
    ECrowd_LODLevel CurrentLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity LOD")
    float DistanceToPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity LOD")
    float LastUpdateTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity LOD")
    bool bIsVisible;

    FCrowd_EntityLOD()
    {
        CurrentLOD = ECrowd_LODLevel::Culled;
        DistanceToPlayer = 0.0f;
        LastUpdateTime = 0.0f;
        bIsVisible = false;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowd_LODSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_LODSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    FCrowd_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODUpdateTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxEntitiesPerFrameUpdate;

private:
    UPROPERTY()
    TArray<FCrowd_EntityLOD> ManagedEntities;

    UPROPERTY()
    APlayerController* CachedPlayerController;

    UPROPERTY()
    APawn* CachedPlayerPawn;

    int32 CurrentUpdateIndex;
    float LastLODUpdateTime;

public:
    UFUNCTION(BlueprintCallable, Category = "Crowd LOD")
    void RegisterEntity(FMassEntityHandle EntityHandle);

    UFUNCTION(BlueprintCallable, Category = "Crowd LOD")
    void UnregisterEntity(FMassEntityHandle EntityHandle);

    UFUNCTION(BlueprintCallable, Category = "Crowd LOD")
    ECrowd_LODLevel GetEntityLOD(FMassEntityHandle EntityHandle) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd LOD")
    void UpdateEntityLOD(FCrowd_EntityLOD& EntityLOD);

    UFUNCTION(BlueprintCallable, Category = "Crowd LOD")
    void UpdateAllEntityLODs();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeLODDistribution();

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetLODSettings(const FCrowd_LODSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    FCrowd_LODSettings GetLODSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawLODDebugInfo();

    UFUNCTION(BlueprintCallable, Category = "Statistics")
    void GetLODStatistics(int32& HighDetail, int32& MediumDetail, int32& LowDetail, int32& Culled) const;

private:
    void UpdatePlayerReferences();
    FVector GetPlayerLocation() const;
    ECrowd_LODLevel CalculateLODLevel(float Distance) const;
    void EnforceLODLimits();
};