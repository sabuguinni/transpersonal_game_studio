#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Arch_PrehistoricStructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    StoneCircle     UMETA(DisplayName = "Stone Circle"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    RockFormation   UMETA(DisplayName = "Rock Formation"),
    AncientRuin     UMETA(DisplayName = "Ancient Ruin"),
    NaturalArch     UMETA(DisplayName = "Natural Arch")
};

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasMossGrowth;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::StoneCircle;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        WeatheringLevel = 0.5f;
        bHasMossGrowth = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_PrehistoricStructureManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_PrehistoricStructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Management")
    TArray<FArch_StructureData> StructureDataArray;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Management")
    int32 MaxStructuresPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Management")
    float MinDistanceBetweenStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WeatheredStoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* MossyRockMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    UStaticMesh* StoneCircleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    UStaticMesh* CaveEntranceMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    UStaticMesh* RockFormationMesh;

public:
    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void SpawnStructureAtLocation(EArch_StructureType StructureType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void GenerateRandomStructures(int32 Count, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void ApplyWeathering(float WeatheringIntensity);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void ClearAllStructures();

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    TArray<FArch_StructureData> GetNearbyStructures(FVector CenterLocation, float SearchRadius);

protected:
    UFUNCTION()
    UStaticMeshComponent* CreateStructureComponent(EArch_StructureType StructureType);

    UFUNCTION()
    void ApplyMaterialToStructure(UStaticMeshComponent* MeshComponent, float WeatheringLevel, bool bHasMoss);

    UFUNCTION()
    bool IsValidStructureLocation(FVector Location);

private:
    TArray<UStaticMeshComponent*> SpawnedStructures;
    
    void InitializeDefaultMeshes();
    void InitializeDefaultMaterials();
};