#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "ArchitectureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_BuildingType : uint8
{
    None = 0,
    RoundHut = 1,
    CaveDwelling = 2,
    LeanToShelter = 3,
    StoragePlatform = 4,
    DefensiveWall = 5,
    Watchtower = 6,
    CommunalFirePit = 7,
    WorkshopArea = 8
};

UENUM(BlueprintType)
enum class EArch_ConstructionMaterial : uint8
{
    Stone = 0,
    Wood = 1,
    AnimalHide = 2,
    ThatchedGrass = 3,
    Bone = 4,
    Clay = 5,
    Vine = 6
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_BuildingComponent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Component")
    EArch_ConstructionMaterial Material = EArch_ConstructionMaterial::Wood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Component")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Component")
    float DurabilityPercent = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Component")
    bool bIsStructural = true;

    FArch_BuildingComponent()
    {
        Material = EArch_ConstructionMaterial::Wood;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        DurabilityPercent = 100.0f;
        bIsStructural = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_SettlementLayout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    float DefensiveRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    int32 MaxBuildings = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    bool bHasDefensiveWall = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    bool bHasCentralFirePit = true;

    FArch_SettlementLayout()
    {
        CenterLocation = FVector::ZeroVector;
        DefensiveRadius = 500.0f;
        MaxBuildings = 12;
        bHasDefensiveWall = false;
        bHasCentralFirePit = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitectureManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitectureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Settings")
    FArch_SettlementLayout SettlementConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Settings")
    TArray<EArch_BuildingType> AllowedBuildingTypes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture State")
    TArray<AActor*> PlacedBuildings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Settings")
    float MinBuildingSpacing = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Settings")
    bool bAutoGenerateSettlement = true;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool PlaceBuilding(EArch_BuildingType BuildingType, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateSettlement();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ClearSettlement();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FVector> GetValidBuildingLocations(int32 NumLocations);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsLocationValidForBuilding(FVector Location, EArch_BuildingType BuildingType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    AActor* CreateBuildingActor(EArch_BuildingType BuildingType, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateBuildingDurability(AActor* Building, float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FArch_BuildingComponent GetBuildingComponent(EArch_BuildingType BuildingType);

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnSettlementGenerated();

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnBuildingPlaced(AActor* NewBuilding, EArch_BuildingType BuildingType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnBuildingDestroyed(AActor* DestroyedBuilding);

private:
    void InitializeDefaultBuildingTypes();
    FVector FindNearestValidLocation(FVector DesiredLocation, EArch_BuildingType BuildingType);
    bool CheckTerrainSuitability(FVector Location);
};