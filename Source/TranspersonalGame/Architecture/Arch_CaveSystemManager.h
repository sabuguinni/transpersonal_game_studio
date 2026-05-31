#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "Arch_CaveSystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_CaveConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Config")
    float CaveDepth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Config")
    float CaveWidth = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Config")
    float CaveHeight = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Config")
    bool bHasShelterArea = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Config")
    bool bHasFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Config")
    EBiomeType PreferredBiome = EBiomeType::Mountain;

    FArch_CaveConfiguration()
    {
        CaveDepth = 1000.0f;
        CaveWidth = 800.0f;
        CaveHeight = 400.0f;
        bHasShelterArea = true;
        bHasFirePit = true;
        PreferredBiome = EBiomeType::Mountain;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_CaveSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_CaveSystemManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CaveEntranceMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CaveInteriorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FirePitMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* CaveInteriorVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    FArch_CaveConfiguration CaveConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    TArray<UStaticMesh*> CaveEntranceMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    TArray<UStaticMesh*> CaveInteriorMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    UMaterialInterface* CaveWallMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    UMaterialInterface* CaveFloorMaterial;

public:
    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void GenerateCaveSystem();

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void SetupCaveInterior();

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void PlaceFirePit();

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void AddShelterElements();

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    bool IsLocationSuitableForCave(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void SpawnCaveAtLocation(const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Cave System")
    void GenerateTestCave();

private:
    void InitializeCaveComponents();
    void SetupCaveMaterials();
    FVector CalculateOptimalCaveLocation() const;
};