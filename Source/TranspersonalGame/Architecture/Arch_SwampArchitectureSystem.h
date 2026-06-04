#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "Arch_SwampArchitectureSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_SwampPlatformData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Platform")
    FVector PlatformLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Platform")
    float PlatformHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Platform")
    float WaterLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Platform")
    int32 SupportPillarCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Platform")
    bool bHasMossGrowth;

    FArch_SwampPlatformData()
    {
        PlatformLocation = FVector::ZeroVector;
        PlatformHeight = 200.0f;
        WaterLevel = 0.0f;
        SupportPillarCount = 4;
        bHasMossGrowth = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_SwampDwellingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Dwelling")
    FVector DwellingLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Dwelling")
    float FloorArea;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Dwelling")
    bool bHasFirePit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Dwelling")
    bool bHasStorageBaskets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Dwelling")
    int32 SleepingAreaCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Dwelling")
    float StructuralIntegrity;

    FArch_SwampDwellingData()
    {
        DwellingLocation = FVector::ZeroVector;
        FloorArea = 16.0f;
        bHasFirePit = true;
        bHasStorageBaskets = true;
        SleepingAreaCount = 2;
        StructuralIntegrity = 0.8f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_SwampPlatformActor : public AActor
{
    GENERATED_BODY()

public:
    AArch_SwampPlatformActor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* PlatformMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* SupportPillars;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Platform")
    FArch_SwampPlatformData PlatformData;

public:
    UFUNCTION(BlueprintCallable, Category = "Swamp Platform")
    void InitializePlatform(const FArch_SwampPlatformData& InPlatformData);

    UFUNCTION(BlueprintCallable, Category = "Swamp Platform")
    void UpdateWaterLevel(float NewWaterLevel);

    UFUNCTION(BlueprintCallable, Category = "Swamp Platform")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintPure, Category = "Swamp Platform")
    bool IsAboveWaterLevel() const;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_SwampDwellingActor : public AActor
{
    GENERATED_BODY()

public:
    AArch_SwampDwellingActor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DwellingStructure;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* InteriorFurnishing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Dwelling")
    FArch_SwampDwellingData DwellingData;

public:
    UFUNCTION(BlueprintCallable, Category = "Swamp Dwelling")
    void InitializeDwelling(const FArch_SwampDwellingData& InDwellingData);

    UFUNCTION(BlueprintCallable, Category = "Swamp Dwelling")
    void AddInteriorElement(const FString& ElementType);

    UFUNCTION(BlueprintCallable, Category = "Swamp Dwelling")
    void UpdateStructuralIntegrity(float IntegrityChange);

    UFUNCTION(BlueprintPure, Category = "Swamp Dwelling")
    bool CanSupportOccupants(int32 OccupantCount) const;
};

UCLASS()
class TRANSPERSONALGAME_API UArch_SwampArchitectureSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

protected:
    UPROPERTY()
    TArray<AArch_SwampPlatformActor*> ActivePlatforms;

    UPROPERTY()
    TArray<AArch_SwampDwellingActor*> ActiveDwellings;

public:
    UFUNCTION(BlueprintCallable, Category = "Swamp Architecture")
    AArch_SwampPlatformActor* CreateSwampPlatform(const FVector& Location, const FArch_SwampPlatformData& PlatformData);

    UFUNCTION(BlueprintCallable, Category = "Swamp Architecture")
    AArch_SwampDwellingActor* CreateSwampDwelling(const FVector& Location, const FArch_SwampDwellingData& DwellingData);

    UFUNCTION(BlueprintCallable, Category = "Swamp Architecture")
    void UpdateAllStructuresForWaterLevel(float NewWaterLevel);

    UFUNCTION(BlueprintCallable, Category = "Swamp Architecture")
    void ApplyEnvironmentalWeathering(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Swamp Architecture")
    int32 GetActiveStructureCount() const;

    UFUNCTION(BlueprintCallable, Category = "Swamp Architecture")
    void CleanupDestroyedStructures();
};