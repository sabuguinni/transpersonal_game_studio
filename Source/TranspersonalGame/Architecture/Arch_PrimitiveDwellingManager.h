#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "Arch_PrimitiveDwellingManager.generated.h"

USTRUCT(BlueprintType)
struct FArch_DwellingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    FVector BaseSize = FVector(400.0f, 600.0f, 300.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    float WallThickness = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    int32 NumRooms = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    bool bHasFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    EBiomeType BiomeType = EBiomeType::Savanna;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_PrimitiveDwellingManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_PrimitiveDwellingManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling Generation")
    TArray<FArch_DwellingConfig> DwellingConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* StoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* WoodMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* ThatchMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    class UStaticMesh* WallMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    class UStaticMesh* RoofMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    class UStaticMesh* DoorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Props")
    TArray<class UStaticMesh*> InteriorPropMeshes;

public:
    UFUNCTION(BlueprintCallable, Category = "Dwelling Generation")
    void GenerateDwellingsInBiomes();

    UFUNCTION(BlueprintCallable, Category = "Dwelling Generation")
    AActor* CreateDwelling(const FVector& Location, const FArch_DwellingConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Dwelling Generation")
    void PopulateInterior(AActor* DwellingActor, const FArch_DwellingConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Dwelling Generation")
    void AddFirePit(const FVector& Location, AActor* ParentActor);

    UFUNCTION(BlueprintCallable, Category = "Dwelling Generation")
    void AddInteriorProps(const FVector& RoomCenter, const FVector& RoomSize, AActor* ParentActor);

protected:
    FVector GetBiomeCenter(EBiomeType BiomeType) const;
    class UMaterialInterface* GetBiomeMaterial(EBiomeType BiomeType) const;
    void CreateWalls(AActor* DwellingActor, const FVector& Location, const FArch_DwellingConfig& Config);
    void CreateRoof(AActor* DwellingActor, const FVector& Location, const FArch_DwellingConfig& Config);
    void CreateDoor(AActor* DwellingActor, const FVector& Location, const FArch_DwellingConfig& Config);
};