#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "ArchitecturalManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    None = 0,
    Platform,
    Pillar,
    Archway,
    Ruins,
    CaveDwelling,
    StoneBridge
};

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Dimensions = FVector(400.0f, 400.0f, 100.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasMossGrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasCarvings = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EBiomeType BiomeType = EBiomeType::Savanna;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::Platform;
        Dimensions = FVector(400.0f, 400.0f, 100.0f);
        WeatheringLevel = 0.5f;
        bHasMossGrowth = true;
        bHasCarvings = false;
        BiomeType = EBiomeType::Savanna;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitecturalManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitecturalManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<UStaticMesh*> PlatformMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<UStaticMesh*> PillarMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<UStaticMesh*> ArchwayMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<UMaterialInterface*> StoneMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<UMaterialInterface*> WeatheredMaterials;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetBiomeAdaptation(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateStructure();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void AddMossGrowth(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void AddCarvings(bool bEnable);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void RegenerateStructure();

protected:
    UFUNCTION()
    void UpdateStructureMesh();

    UFUNCTION()
    void ApplyBiomeSpecificMaterials();

    UFUNCTION()
    UStaticMesh* GetMeshForStructureType(EArch_StructureType Type);

    UFUNCTION()
    UMaterialInterface* GetMaterialForBiome(EBiomeType BiomeType, bool bWeathered);
};