#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "Arch_StructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    CaveDwelling,
    ElevatedShelter,
    GroundShelter,
    StorageHut,
    CraftingArea,
    DefensiveWall
};

USTRUCT(BlueprintType)
struct FArch_StructureConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::CaveDwelling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Dimensions = FVector(400, 300, 200);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 MaxOccupants = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasStorage = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 100.0f;
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
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MainStructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* InteriorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Config")
    FArch_StructureConfig StructureConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Config")
    TArray<AActor*> InteriorProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Config")
    FVector SpawnLocation = FVector::ZeroVector;

public:
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void CreateStructure(EArch_StructureType Type, FVector Location, FVector Scale = FVector::OneVector);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void AddInteriorProp(AActor* PropActor, FVector RelativeLocation);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetStructuralIntegrity(float NewIntegrity);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float GetStructuralIntegrity() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CanAccommodateOccupants(int32 NumOccupants) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RepairStructure(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FVector> GetInteriorSpawnPoints() const;

private:
    void SetupCaveDwelling();
    void SetupElevatedShelter();
    void SetupGroundShelter();
    void CreateFirePit();
    void CreateStorageAreas();
    void UpdateStructuralMesh();
};