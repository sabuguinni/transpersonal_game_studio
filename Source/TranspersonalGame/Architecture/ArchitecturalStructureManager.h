#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "ArchitecturalStructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    CaveDwelling        UMETA(DisplayName = "Cave Dwelling"),
    WoodenShelter       UMETA(DisplayName = "Wooden Shelter"),
    StoneCircle         UMETA(DisplayName = "Stone Circle"),
    Village             UMETA(DisplayName = "Village"),
    Watchtower          UMETA(DisplayName = "Watchtower"),
    StoragePit          UMETA(DisplayName = "Storage Pit"),
    CraftingArea        UMETA(DisplayName = "Crafting Area"),
    DefensiveWall       UMETA(DisplayName = "Defensive Wall")
};

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::CaveDwelling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Dimensions = FVector(500.0f, 500.0f, 300.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 Capacity = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsWeatherproof = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bProvidesWarmth = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    TArray<FString> RequiredMaterials;

    FArch_StructureData()
    {
        RequiredMaterials.Add(TEXT("Stone"));
        RequiredMaterials.Add(TEXT("Wood"));
    }
};

USTRUCT(BlueprintType)
struct FArch_InteriorLayout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> SleepingAreas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector FirePitLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> StorageLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> ToolRackLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasVentilation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float LightLevel = 0.3f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitecturalStructureManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitecturalStructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MainStructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FArch_InteriorLayout InteriorLayout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* StoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WoodMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* ThatchMaterial;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void InitializeStructure(EArch_StructureType Type, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void SetupInteriorLayout();

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void AddInteriorProp(FVector Location, FRotator Rotation, const FString& PropType);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    bool CanAccommodateOccupants(int32 OccupantCount) const;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    float GetStructuralIntegrity() const { return StructureData.StructuralIntegrity; }

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void DamageStructure(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void RepairStructure(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    TArray<FVector> GetSleepingPositions() const;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    FVector GetFirePitLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    bool IsWeatherproof() const { return StructureData.bIsWeatherproof; }

    UFUNCTION(BlueprintCallable, Category = "Structure")
    bool ProvidesWarmth() const { return StructureData.bProvidesWarmth; }

private:
    void SetupCaveDwelling();
    void SetupWoodenShelter();
    void SetupStoneCircle();
    void SetupVillageLayout();
    void ApplyMaterialToStructure();
    void CreateInteriorProps();
    
    UPROPERTY()
    TArray<UStaticMeshComponent*> InteriorProps;
};