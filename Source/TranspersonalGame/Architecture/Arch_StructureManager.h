#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../Core/SharedTypes.h"
#include "Arch_StructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    StoneCircle     UMETA(DisplayName = "Stone Circle"),
    CaveDwelling    UMETA(DisplayName = "Cave Dwelling"),
    Platform        UMETA(DisplayName = "Platform"),
    Archway         UMETA(DisplayName = "Archway"),
    Pillar          UMETA(DisplayName = "Pillar")
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
    EBiomeType BiomeType;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::Platform;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        BiomeType = EBiomeType::Savanna;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_StructureManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_StructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_StructureData> StructureDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    class UStaticMesh* DefaultStoneMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    class UMaterialInterface* StoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    class UMaterialInterface* WeatheredStoneMaterial;

public:
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void CreateStructureAtBiome(EBiomeType BiomeType, EArch_StructureType StructureType, FVector Offset = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void CreateStoneCircle(FVector CenterLocation, float Radius = 1500.0f, int32 NumStones = 8);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void CreatePlatform(FVector Location, FVector PlatformScale = FVector(8.0f, 8.0f, 0.5f));

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void CreateArchway(FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void PopulateAllBiomesWithStructures();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FVector GetBiomeCoordinates(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void GenerateStructuresInEditor();

private:
    class UStaticMeshComponent* CreateStructureComponent(FVector Location, FRotator Rotation, FVector Scale);
    void ApplyWeatheredMaterial(UStaticMeshComponent* MeshComponent, EBiomeType BiomeType);
};