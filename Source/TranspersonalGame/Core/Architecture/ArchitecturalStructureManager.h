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
    StonePillar     UMETA(DisplayName = "Stone Pillar"),
    RockFormation   UMETA(DisplayName = "Rock Formation"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    AncientRuin     UMETA(DisplayName = "Ancient Ruin"),
    TribalMarker    UMETA(DisplayName = "Tribal Marker")
};

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::StonePillar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator SpawnRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasTribalMarkings = false;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::StonePillar;
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        WeatheringLevel = 0.5f;
        bHasTribalMarkings = false;
    }
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
    UStaticMeshComponent* StructureMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<UStaticMesh*> PillarMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<UStaticMesh*> RockFormationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<UMaterialInterface*> WeatheredMaterials;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeStructure(const FArch_StructureData& InStructureData);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void AddTribalMarkings(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FVector GetStructureLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    EArch_StructureType GetStructureType() const;

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnStructureInitialized();

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnWeatheringApplied(float WeatheringLevel);

private:
    void UpdateStructureMesh();
    void ApplyWeatheringMaterial();
    UStaticMesh* GetMeshForStructureType(EArch_StructureType Type);
    UMaterialInterface* GetWeatheredMaterial(float WeatheringLevel);
};