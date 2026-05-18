#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Arch_StructureManager.generated.h"

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
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator SpawnRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasMossGrowth;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::StonePillar;
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        WeatheringLevel = 0.5f;
        bHasMossGrowth = true;
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
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<UStaticMesh*> PillarMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<UStaticMesh*> RockMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<UMaterialInterface*> WeatheredMaterials;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeStructure(const FArch_StructureData& InStructureData);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetMossGrowth(bool bEnableMoss);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnStructureAtLocation(EArch_StructureType Type, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void GenerateRandomStructures();

private:
    void UpdateStructureMesh();
    void ApplyMaterialEffects();
    UStaticMesh* GetMeshForStructureType(EArch_StructureType Type);
};